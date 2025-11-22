#!/bin/bash

export base_dir="$HOME/xx/ict-lab/fhe/workload/FullRNS-HEAAN"
export result_path="$base_dir/result/gem5-riscv64-logp50"

mkdir -p $result_path

logN_list=(10 12 14)


  # "ori_bar"
  # "ori_mont"
case_list=(
  "rvv_ori_bar"
  "rvv_ori_mont"
)
arg_list=()
for logN in "${logN_list[@]}"; do
  for case in "${case_list[@]}"; do
    arg_list+=("$logN $case")
  done
done

run_cfg() {
  read logN case <<<"$1"
  echo -e "run for\nlogN: $logN, case: $case"

  bin_path="$base_dir/run/FRNSHEAAN-gem5-riscv64-CO-S4Mont"

  gem5_run_dir="$base_dir/run/gem5"

  make -C "$gem5_run_dir" se \
    TARGET="$bin_path" \
    TARGET_OPTION="--logN $logN --logQ 50 -e --case $case" \
    BUILD_DIR="$result_path/$logN/$case"
}

export -f run_cfg

parallel -j3 run_cfg ::: "${arg_list[@]}"
