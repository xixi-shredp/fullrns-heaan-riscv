#!/bin/env python

import glob
import os

from pyxixi.tableGen import MarkdownTableGen, TypstTableGen

nrCore = 4
FuBusyList = [{} for _ in range(nrCore)]


def fuBusy_analysis_line(line: str):
    items = line.strip().split()
    if not items:
        return
    name = items[0]
    value = items[1]
    if "statFuBusy" not in name:
        return
    if int(value) == 0:
        return
    fu = name.strip().split("::")[-1]
    for i in range(nrCore):
        if f"cpu{i}" in name:
            FuBusyList[i][fu] = value
            return


def fuBusy_analysis(stat_file: str):
    with open(stat_file, "r") as f:
        lines = f.readlines()
    for line in lines:
        fuBusy_analysis_line(line)


def gem5_fuBusy_stat(logN: int):
    fuBusyPTable = []
    global FuBusyList
    path_list = glob.glob(
        f"{root}/workload/FullRNS-HEAAN/result/gem5-riscv64/{logN}/*/stats.txt.out_1"
    )
    for path in path_list:
        case = path.split("/")[-2]
        if "step4" in case:
            continue
        if "rvv" in case:
            continue
        if "fhe" not in case:
            continue
        fuBusy_analysis(path)
        fr = [case]
        sr = [""]
        tr = [""]
        for fu, num in FuBusyList[0].items():
            fr.append(fu)
            sr.append(num)
        total_num = sum(map(int, sr[1:]))
        tr += [f"{(int(i)/total_num):.2%}" for i in sr[1:]]
        fuBusyPTable.append(fr)
        fuBusyPTable.append(sr)
        fuBusyPTable.append(tr)
        # print(case)
        # print(FuBusyList[0])
        # print("")
        FuBusyList = [{} for _ in range(nrCore)]
    row_len = 1
    for row in fuBusyPTable:
        if len(row) > row_len:
            row_len = len(row)
    for row in fuBusyPTable:
        row += ["" for _ in range(row_len - len(row))]
    headline = ["case_name"] + ["" for _ in range(row_len - 1)]

    TypstTableGen(headline, fuBusyPTable).generate()


def stat_result(stdout: str):
    with open(stdout, "r") as f:
        lines = f.readlines()
    condPredicted = 0
    condIncorrect = 0
    for line in lines:
        if "condPredicted" in line:
            tmp = int(line.strip().split()[1])
            if tmp == 0:
                continue
            condPredicted = tmp
        if "condIncorrect" in line:
            tmp = int(line.strip().split()[1])
            if tmp == 0:
                continue
            condIncorrect = tmp
    condCorrect = condPredicted - condIncorrect
    accuracy = "{:.2%}".format(condCorrect / condPredicted)
    return [condCorrect, condPredicted, accuracy]


def case_sort(items: list[list]):
    items.sort(key=lambda x: len(x[0]))


def process_gem5_out(logNs: list[int]):
    first = True
    items = []
    head = ["accuracy \\\\ logN "]
    rows = 0
    for logN in logNs:
        path_list = glob.glob(
            f"{root}/workload/FullRNS-HEAAN/result/gem5-riscv64/{logN}/*/stats.txt.out_1"
        )
        if first:
            rows = len(path_list)
            for i in range(rows):
                items.append([path_list[i].split("/")[-2]])
            first = False
        else:
            assert rows == len(path_list)
        for i in range(rows):
            res = stat_result(path_list[i])
            items[i].append(res[2])
        # print(items)
        head.append(f"{logN}")
    case_sort(items)
    MarkdownTableGen(head, items).generate()
    TypstTableGen(head, items).generate()


if __name__ == "__main__":
    root = os.getenv("FHE_ROOT")
    logNs = [11, 12, 13, 14, 15, 16]
    # process_mcpat(logNs)
    # process_gem5_out(logNs)
    gem5_fuBusy_stat(16)
