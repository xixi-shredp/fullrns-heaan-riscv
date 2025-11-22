#!/bin/bash

export base_dir="$HOME/xx/ict-lab/fhe/workload/FullRNS-HEAAN"
export result_path="$base_dir/result/gem5-riscv64-pqc"
export logN="8"

mkdir -p $result_path

algo_list=(
  "kyber768 3329"
  "saber 25570817"
  "dilithium3 8380417"
  )


  # "mt_rvv_step4_bar"
  # "mt_rvv_step4_mont"
  # "ori_bar"
  # "ori_mont"
  # "rvv_ori_mont"
case_list=(
  "rvv_ori_bar"
)
arg_list=()
for algo in "${algo_list[@]}"; do
  for case in "${case_list[@]}"; do
    arg_list+=("$algo $case")
  done
done

run_cfg() {
  read algo q case <<<"$1"
  echo -e "run for\nalgo: $algo, q: $q, case: $case"

  bin_path="$base_dir/run/FRNSHEAAN-gem5-riscv64-CO-S4Mont"

  gem5_run_dir="$base_dir/run/gem5"

  make -C "$gem5_run_dir" se \
    TARGET="$bin_path" \
    TARGET_OPTION="--logN $logN -q $q -e --case $case" \
    BUILD_DIR="$result_path/$algo/$case"
}

export -f run_cfg

parallel -j3 run_cfg ::: "${arg_list[@]}"
