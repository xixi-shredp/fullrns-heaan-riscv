#!/bin/env python

import glob
import os
from os.path import isdir

from pyxixi.tableGen import MarkdownTableGen, TypstTableGen


def fhe_result(stdout: str):
    with open(stdout, "r") as f:
        lines = f.readlines()
    t_line = None
    for line in lines:
        if "test time" in line:
            t_line = line
            break
    if not t_line:
        return "nan"
    time = float(t_line.split("=")[1].strip().split(" ")[0].strip())
    return time


def case_sort(items: list[list]):
    items.sort(key=lambda x: len(x[0]))


def process_gem5_out(logNs: list[int]):
    first = True
    items = []
    head = ["test time (ms)"]
    rows = 0
    for logN in logNs:
        path_list = glob.glob(
            f"{root}/workload/FullRNS-HEAAN/result/gem5-riscv64/{logN}/*/stdout.txt"
        )
        if first:
            rows = len(path_list)
            for i in range(rows):
                items.append([path_list[i].split("/")[-2]])
            first = False
        else:
            assert rows == len(path_list)
        for i in range(rows):
            items[i].append(fhe_result(path_list[i]))
        # print(items)
        head.append(f"logN{logN}")
    case_sort(items)
    speedups = cal_sppedup(items)
    # ave_speedup = average(speedups)
    # MarkdownTableGen(["case", "ave_speedup"], ave_speedup).generate()
    # generate_dat_file(speedups, logNs, "./data/")
    MarkdownTableGen(head, speedups).generate()
    MarkdownTableGen(head, items).generate()
    # TypstTableGen(head, items).generate()


def process_fhe_out(arch: str, logNs: list[int]):
    first = True
    items = []
    head = ["test time (ms)"]
    rows = 0
    for logN in logNs:
        path_list = glob.glob(
            f"{root}/workload/FullRNS-HEAAN/result/{arch}/{logN}/*.out"
        )
        if first:
            rows = len(path_list)
            for i in range(rows):
                items.append([path_list[i].split("/")[-1].split(".")[0]])
            first = False
        else:
            assert rows == len(path_list)
        for i in range(rows):
            items[i].append(fhe_result(path_list[i]))
        # print(items)
        head.append(f"logN{logN}")
    case_sort(items)
    MarkdownTableGen(head, items).generate()
    # TypstTableGen(head, items).generate()


def cal_sppedup(data: list[list]):
    new_data = [[], [], [], []]
    maps = {
        "ori_bar": new_data[0],
        "ori_mont": new_data[1],
        "step4_bar": new_data[2],
        "step4_mont": new_data[3],
    }
    nr_log = len(data[0]) - 1
    for row in data:
        for name in maps.keys():
            if row[0] == name:
                maps[name] += row.copy()
    for row in data:
        for name in maps.keys():
            if name in row[0] and name != row[0]:
                tem = [row[0]]
                for i in range(1, nr_log + 1):
                    tem.append(round(maps[name][i] / float(row[i]), 3))
                new_data.append(tem)
    for i in range(4):
        for j in range(1, nr_log + 1):
            new_data[i][j] = 1

    return new_data


def average(data: list[list]):
    aves = []
    for row in data:
        d = row[1:]
        aves.append([row[0], round(sum(d) / len(d), 3)])
    return aves


def generate_dat_file(data: list[list], logNs: list[int], dir: str):
    base_maps = {
        "ori_bar": "barrett",
        "ori_mont": "montgomeny",
        "step4_bar": "4-step-barrett",
        "step4_mont": "4-step-montgomeny",
    }
    opt_maps = {"": "", "rvv_": "+SO", "ext_": "+HO", "rvv_ext_": "+SO+HO"}
    nr_log = len(logNs)
    assert nr_log == len(data[0]) - 1
    idx = 1
    if not os.path.isdir(dir):
        os.mkdir(dir)
    for i in glob.glob(f"{dir}/*.dat"):
        os.remove(i)

    for logN in logNs:
        for row in data:
            for prefix in opt_maps.keys():
                for base in base_maps.keys():
                    if row[0] == prefix + base:
                        file_path = f"{dir}/logN{logN}{opt_maps[prefix]}.dat"
                        with open(file_path, "a") as f:
                            f.write(f"{base_maps[base]} {row[idx]}\n")

        idx += 1


def generate_sota_dat(data: list[list], dir: str):
    base = []
    design = []
    for row in data:
        name = row[0].split("-")
        logN, logQ = name[0], name[1]
        is_base = len(name) == 3
        if is_base:
            base.append(f"({logN}-{logQ}) {row[1]}\n")
        else:
            design.append(f"({logN}-{logQ}) {row[1]}\n")
    if not isdir(dir):
        os.mkdir(dir)
    with open(f"{dir}/XT910_4-Step-Montgomeny.dat", "w") as f:
        f.writelines(base)
    with open(f"{dir}/XT910+SO+XT910_4-Step-Montgomeny.dat", "w") as f:
        f.writelines(design)


def process_encode(bar: bool):
    if bar:
        area = 811357
        power = 0.2338
    else:
        area = 814557
        power = 0.2433
    path_list = glob.glob(
        f"{root}/workload/FullRNS-HEAAN/result/gem5-riscv64-sota/*/stdout.txt"
    )
    adps = []
    edps = []
    for path in path_list:
        name = path.split("/")[-2]
        with open(path, "r") as f:
            lines = f.readlines()
        value = 0
        for line in lines:
            if "Encrypt single time" in line:
                value = float(line.split("=")[1].strip().split(" ")[0])
                break
        adp = [name, value * area]
        edp = [name, value * value * power]
        adps.append(adp)
        edps.append(edp)
    adps.sort(key=lambda x: x[0])
    edps.sort(key=lambda x: x[0])
    print("======= ADP ========")
    MarkdownTableGen(["", "ADP"], adps).generate()
    print("\n======= EDP ========")
    MarkdownTableGen(["", "EDP"], edps).generate()
    data_dir = "./data"
    generate_sota_dat(adps, f"{data_dir}/ADP")
    generate_sota_dat(edps, f"{data_dir}/EDP")


if __name__ == "__main__":
    root = os.getenv("FHE_ROOT")
    logNs = [11, 12, 13, 14, 15, 16]
    # process_mcpat(logNs)
    # process_fhe_out("xuantie-riscv64", logNs)
    process_gem5_out(logNs)
    # process_encode(bar=False)
