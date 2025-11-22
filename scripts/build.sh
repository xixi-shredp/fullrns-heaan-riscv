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
build() {
  mul="$1"
  opt="$2"
  def_path="$base_dir/configs/"
  if [ "$opt" != "" ]; then
    def_path="$def_path""$opt"_
  fi
  def_path="$def_path""$mul""_defconfig"
  defconfig "$def_path"
  genconfig
  make -C "$base_dir" bin
  # echo "$bin_path" "$def_path"
}

# should be built in sequence
for cfg in "${config_list[@]}"; do
  build $cfg
done

