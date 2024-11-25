#!/usr/bin/python3

import re
import os
import argparse
import glob

parser = argparse.ArgumentParser(description="out file analysis")
parser.add_argument("path", help="path to out file", type=str)
args = parser.parse_args()

result_file = args.path + ".res"
res_f = open(result_file, "w")


def match_item(name: str, line: str):
    pat = name + r":\s*([0-9\.\%]+)"
    match = re.search(pat, line)
    if match:
        res_f.write(f"{name:30}{match.group(1)}\n")
def match_time(name: str, line: str):
    pat = name + r"\s*=\s*([0-9\.]+)\s*ms"
    match = re.search(pat, line)
    if match:
        res_f.write(f"{name:30}{match.group(1)}\n")


def parse_file(file: str):
    with open(file, "r") as f:
        lines = f.readlines()
        for line in lines:
            match_time("org ntt time", line)
            match_time("new ntt time", line)
            match_time("row ntt 1 time", line)
            match_time("row ntt 2 time", line)
            match_time("matrix transpose 1 time", line)
            match_time("matrix transpose 2 time", line)
            match_item("l1 icache miss", line)
            match_item("l1 dcache read miss", line)
            match_item("l1 dcache write miss", line)
            match_item("l2 cache read miss", line)
            match_item("l2 cache write miss", line)
            match_item("i-utlb miss", line)
            match_item("d-utlb miss", line)
            match_item("jtlb miss", line)


if __name__ == "__main__":
    parse_file(args.path)
    res_f.close()
