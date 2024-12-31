#!/bin/python3

import argparse

parser = argparse.ArgumentParser(description="Split stats.txt for Gem5")
parser.add_argument("input", help="path to input file")
parser.add_argument("-o", "--output", help="path to output file")
args = parser.parse_args()


output = args.output

if not output:
    output = args.input + ".out"

start_str = "Begin Simulation Statistics"
end_str = "End Simulation Statistics"

line_num = []
with open(args.input, "r") as f:
    global lines
    lines = f.readlines()
    lid = 0
    tem = [0, 0]
    for line in lines:
        if start_str in line:
            tem[0] = lid
        elif end_str in line:
            tem[1] = lid
            line_num.append(tem[0:])
        lid += 1

fid = 0
for i in line_num:
    with open(f"{output}_{fid}", "w") as f:
        f.writelines(lines[i[0] : i[1] + 1])
    fid += 1
