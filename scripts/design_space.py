#!/bin/env python

import os

root_dir = (
    os.getenv("FHE_ROOT") + "workload/FullRNS-HEAAN/result/vector_assess/fu/2-1/"
)
# dst_dir = (
#     os.getenv("FHE_ROOT")
#     + "workload/FullRNS-HEAAN/result/scalar_assess/static_schedule/"
# )
# items = [
#     "1",
#     "2-1",
#     "4-1",
#     "4-2-1",
#     "8-1",
#     "8-4-2-1",
#     "16-1",
#     "16-8-4-2-1",
#     "32-1",
#     "32-16-8-4-2-1",
#     "all",
# ]

dst_dir = (
    os.getenv("FHE_ROOT") + "workload/FullRNS-HEAAN/result/vector_assess/fu/2-1/"
)
items = ["a1m1", "a2m1", "a2m2"]

data_dirs = [root_dir + i for i in items]

logN = [11, 12, 13, 14, 15, 16]


def get_time(file: str):
    with open(file, "r") as f:
        lines = f.readlines()
    for line in lines:
        if "test time =" in line:
            value = line.split("=")[1].strip().split()[0]
            return float(value)
    return 0.0


def get_one(dir: str):
    speedups = []
    for n in logN:
        dat_file = f"{dir}/{n}/rvv_ext_ori_bar/stdout.txt"
        base_file = f"{dir}/{n}/ori_bar/stdout.txt"
        dat = get_time(dat_file)
        base = get_time(base_file)
        speedup = base / dat
        speedups.append(speedup)
    return speedups


def generate_dat(speedups: list, dir: str):
    assert len(speedups) == len(logN)
    with open(dir, "w") as f:
        for i in range(len(speedups)):
            f.write(f"{logN[i]} {speedups[i]}\n")


print("logN:\n", logN)
for dir in data_dirs:
    speedups = get_one(dir)
    item = dir.split("/")[-1]
    print(f"{item}\n", speedups)
    # dst = f"{dst_dir}/speedup/{item}.dat"
    dst = f"{dst_dir}/{item}.dat"
    generate_dat(speedups, dst)


##################################################
#     to get memory instrcution occupied
##################################################
def do_get_memops(file: str):
    memInsts = 0
    fheInsts = 0
    with open(file, "r") as f:
        lines = f.readlines()
    for line in lines:
        if "system.cpu0.statIssuedInstType_0::FHEAddMod" in line:
            value = line.split()[1]
            fheInsts += int(value)
        if "system.cpu0.statIssuedInstType_0::MemRead" in line:
            value = line.split()[1]
            memInsts += int(value)
        if "system.cpu0.statIssuedInstType_0::MemWrite" in line:
            value = line.split()[1]
            memInsts += int(value)
    return memInsts / fheInsts


def get_one_memops(dir: str):
    memops = []
    for n in logN:
        dat_file = f"{dir}/{n}/ext_ori_bar/stats.txt.out_1"
        dat = do_get_memops(dat_file)
        memops.append(dat)
    return memops


# print("logN:\n", logN)
# for dir in data_dirs:
#     memops = get_one_memops(dir)
#     item = dir.split("/")[-1]
#     print(f"{item}\n", memops)
#     dst = f"{dst_dir}/memops/{item}.dat"
#     generate_dat(memops, dst)

####################################################
#     vector assess
####################################################

vroot_dir = (
    os.getenv("FHE_ROOT") + "workload/FullRNS-HEAAN/result/vector_assess/"
)
vdst_dir = (
    os.getenv("FHE_ROOT")
    + "workload/FullRNS-HEAAN/result/vector_assess/static_schedule/"
)
vitems = ["1", "2-1"]

vdata_dirs = [vroot_dir + i for i in vitems]


def get_vone(dir: str):
    speedups = []
    for n in logN:
        dat_file = f"{dir}/{n}/rvv_ext_ori_bar/stdout.txt"
        base_file = f"{dir}/{n}/ori_bar/stdout.txt"
        dat = get_time(dat_file)
        base = get_time(base_file)
        speedup = base / dat
        speedups.append(speedup)
    return speedups


# print("logN:\n", logN)
# for dir in vdata_dirs:
#     speedups = get_vone(dir)
#     item = dir.split("/")[-1]
#     print(f"{item}\n", speedups)
#     dst = f"{vdst_dir}/{item}.dat"
#     generate_dat(speedups, dst)
