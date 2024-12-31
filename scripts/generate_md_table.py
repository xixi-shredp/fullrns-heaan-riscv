#!/bin/env python

import glob
import os

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


def mcpat_result(mcpat_out: str):
    with open(mcpat_out, "r") as f:
        lines = f.readlines()
    core_line = 0
    for line in lines:
        if "Core:" in line:
            break
        core_line += 1
    data_line = lines[core_line + 5]
    assert "Runtime Dynamic" in data_line
    power = float(data_line.split("=")[1].strip().split(" ")[0].strip())
    return power


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
    MarkdownTableGen(head, items).generate()
    TypstTableGen(head, items).generate()

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
    TypstTableGen(head, items).generate()


def process_mcpat(logNs: list[int]):
    first = True
    items = []
    head = ["Runtime Dynamic(W)"]
    rows = 0
    for logN in logNs:
        path_list = glob.glob(
            f"{root}/workload/FullRNS-HEAAN/result/gem5-riscv64/{logN}/*/mcpat.out"
        )
        if first:
            rows = len(path_list)
            for i in range(rows):
                items.append([path_list[i].split("/")[-2]])
            first = False
        else:
            assert rows == len(path_list)
        for i in range(rows):
            items[i].append(mcpat_result(path_list[i]))
        # print(items)
        head.append(f"logN{logN}")
    case_sort(items)
    MarkdownTableGen(head, items).generate()


if __name__ == "__main__":
    root = os.getenv("FHE_ROOT")
    logNs = [11, 12, 13, 14, 15, 16]
    # process_mcpat(logNs)
    process_fhe_out("xuantie-riscv64-noxt", logNs)
    # process_gem5_out(logNs)
