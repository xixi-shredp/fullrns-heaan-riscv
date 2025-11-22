#!/bin/bash

export base_dir="$HOME/xx/ict-lab/fhe/workload/FullRNS-HEAAN"

config_list=(
  "Bar"
  "Bar SO"
  "Bar HO"
  "Bar CO"
  "Mont"
  "Mont SO"
  "Mont HO"
  "Mont CO"
  "S4Bar"
  "S4Bar SO"
  "S4Bar HO"
  "S4Bar CO"
  "S4Mont"
  "S4Mont SO"
  "S4Mont HO"
  "S4Mont CO"
)

export result_path="$base_dir/result/gem5-riscv64-e2e-L44"

run_cfg() {
  read mul opt <<<"$1"
  echo "run_cfg $opt $mul start "
  arg_list=(14)
  bin_path="$base_dir/run/FRNSHEAAN-gem5-riscv64"

  if [ "$opt" != "" ]; then
    bin_path="$bin_path"-"$opt"
  fi
  bin_path="$bin_path"-"$mul"

  for arg in "${arg_list[@]}"; do
    logN=$arg
    gem5_run_dir="$base_dir/run/gem5"
    make -C "$gem5_run_dir" se \
      TARGET="$bin_path" \
      TARGET_OPTION="--logN $logN --logQ 55" \
      BUILD_DIR="$result_path/$logN/$opt\_$mul"
    echo "run_cfg $opt $mul finished : logN = $logN"
  done
  echo "run_cfg $opt $mul finished "
}

export -f run_cfg

extract_data() {
  local normalized=0
  data_dir="$1"
  mkdir -p "$data_dir"
  cd "$data_dir" || exit
  logN_list=()
  for logN_dir in "$result_path"/*; do
    logN=$(basename $logN_dir)
    logN_list+=("$logN")
    # echo "====== logN: $logN ======"
    echo "" >"logN$logN".dat
    echo "" >"logN$logN+SO".dat
    echo "" >"logN$logN+HO".dat
    echo "" >"logN$logN+SO+HO".dat
    for case_dir in "$logN_dir"/*; do
      case=$(basename "$case_dir")
      result=$(cat "$case_dir/stdout.txt" | grep 'Encrypt single time' | sed -r 's/^.* ([.0-9]+) ms$/\1/')
      opt=$(echo $case | tr '_' ' ')
      read opt _ <<<"$opt"
      mul=${case#"CO_"}
      mul=${mul#"HO_"}
      mul=${mul#"SO_"}
      mul=${mul#"_"}

      if [ $mul == "Bar" ]; then
        mul="barrett"
      elif [ $mul == "Mont" ]; then
        mul="montgomery"
      elif [ $mul == "S4Bar" ]; then
        mul="4-step-barrett"
      else
        mul="4-step-montgomery"
      fi

      if [ $opt == "CO" ]; then
        echo "$mul $result" >>"logN$logN+SO+HO".dat
      elif [ $opt == "HO" ]; then
        echo "$mul $result" >>"logN$logN+HO".dat
      elif [ $opt == "SO" ]; then
        echo "$mul $result" >>"logN$logN+SO".dat
      else
        echo "$mul $result" >>"logN$logN".dat
      fi
      # echo "$case: $result"
    done
    # echo "======================"
  done

  if [ -z $normalized ]; then
    cd - || exit
    exit
  fi

  # Normalized
  mkdir -p speedup
  for logN in "${logN_list[@]}"; do
    local base_Bar
    local base_Mont
    local base_S4Bar
    local base_S4Mont

    read _ base_Bar <<<$(cat "logN$logN".dat | grep "barrett")
    read _ base_Mont <<<$(cat "logN$logN".dat | grep "montgomery")
    read _ base_S4Bar <<<$(cat "logN$logN".dat | grep "4-step-barrett")
    read _ base_S4Mont <<<$(cat "logN$logN".dat | grep "4-step-montgomery")

    echo "====== logN: $logN ======"
    for ms_dat in logN"$logN"*.dat; do
      echo "" >"speedup/$ms_dat"

      local res_Bar
      local res_Mont
      local res_S4Bar
      local res_S4Mont

      read _ res_Bar <<<$(cat $ms_dat | grep "barrett")
      read _ res_Mont <<<$(cat $ms_dat | grep "montgomery")
      read _ res_S4Bar <<<$(cat $ms_dat | grep "4-step-barrett")
      read _ res_S4Mont <<<$(cat $ms_dat | grep "4-step-montgomery")

      res_Bar=$(echo "scale=2; $base_Bar / $res_Bar" | bc)
      res_Mont=$(echo "scale=2; $base_Mont / $res_Mont" | bc)
      res_S4Bar=$(echo "scale=2; $base_S4Bar / $res_S4Bar" | bc)
      res_S4Mont=$(echo "scale=2; $base_S4Mont / $res_S4Mont" | bc)

      echo "barrett $res_Bar" >>"speedup/$ms_dat"
      echo "montgomery $res_Mont" >>"speedup/$ms_dat"
      echo "4-step-barrett $res_S4Bar" >>"speedup/$ms_dat"
      echo "4-step-montgomery $res_S4Mont" >>"speedup/$ms_dat"

      echo -n "speedup/$ms_dat: "
      cat "speedup/$ms_dat"
    done
    echo -e "=======================\n"
  done

  cd - || exit
}

# parallel -j8 run_cfg ::: "${config_list[@]}"
extract_data $base_dir/data/e2e
