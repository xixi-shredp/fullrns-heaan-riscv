#!/bin/bash
export base_dir="$HOME/xx/ict-lab/fhe/workload/FullRNS-HEAAN"
export result_path="$base_dir/result/gem5-riscv64-modtest"

data_dir="$base_dir/data/modtest"
mkdir -p "$data_dir"
cd "$data_dir" || exit
mkdir -p speedup

get_ms() {
  local stdout="$1"
  cat "$stdout" | grep 'ModOP time' | sed -r 's/^.* ([.0-9]+) ms$/\1/'
}

echo "" >"speedup/addmod".dat
echo "" >"speedup/montredc".dat
echo "" >"speedup/barmulmod".dat

for logN_dir in "$result_path"/*; do
  logN=$(basename $logN_dir)
  for case_dir in "$logN_dir"/*; do
    case=$(basename "$case_dir")
    result=$(get_ms "$case_dir/stdout.txt")
    read opt operation <<<$(echo $case | tr '_' ' ')
    if [ $opt == "ext" ]; then
      soft_case="soft_$operation"
      soft_ms=$(get_ms "$case_dir/../$soft_case/stdout.txt")
      speed=$(echo "scale=2; $soft_ms / $result" | bc)
      echo "$logN $speed" >>"speedup/$operation".dat
    fi
  done
done

for op_dat in "$data_dir"/speedup/*; do
  op=$(basename $op_dat .dat)
  echo "====== $op ======"
  cat $op_dat
  echo "==================="
done

cd - || exit
