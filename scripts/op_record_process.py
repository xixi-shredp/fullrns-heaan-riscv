#!/bin/env python

import os

work_dir = f"{os.getenv("FHE_ROOT")}/workload/FullRNS-HEAAN/result"


class OpClass:
    cnt = 0
    result = -1

    def __init__(self, name: str, output_dir: str) -> None:
        self.fp = open(f"{output_dir}/{name}.out", "w")
        self.name = name

    def close(self):
        self.fp.close()

    def inc(self):
        self.cnt += 1

    def dump(self, ops: list[str]):
        wb = " ".join(ops)
        match self.name:
            case "addmod":
                wb += " 0"
            case "submod":
                wb += " 1"

        if self.result == -1:
            self.fp.write(wb + "\n")
        else:
            self.fp.write(wb + f" {self.result}\n")


class OPParser:
    def __init__(self, op_class: dict[str, OpClass]) -> None:
        self.op_class = op_class

    def parse_line(self, line: str) -> None:
        items = line.strip(" ").split(" ")
        op_name = items[0][len("rd_"):]
        op = self.op_class[op_name]
        op.inc()
        if items[-1][0] == "=":
            op.result = int(items[-1][1:])
            op.dump(items[1:-1])
        else:
            op.dump(items[1:])

    def read_in(self, path: str):
        with open(path, "r") as f:
            for line in f.readlines():
                self.parse_line(line)
        for i in self.op_class.keys():
            self.op_class[i].close()

    def stat(self):
        for name, op in self.op_class.items():
            print(f"{name} : {op.cnt}")
        print("")


class MontParser(OPParser):
    def __init__(self, output_dir: str) -> None:
        super().__init__(
            {
                "addmod": OpClass("addmod", output_dir),
                "submod": OpClass("submod", output_dir),
                "montredc": OpClass("montredc", output_dir),
            },
        )


class BarParser(OPParser):
    def __init__(self, output_dir: str) -> None:
        super().__init__(
            {
                "addmod": OpClass("addmod", output_dir),
                "submod": OpClass("submod", output_dir),
                "barmulmod": OpClass("barmulmod", output_dir),
            },
        )


if __name__ == "__main__":

    dir = f"{work_dir}/op_record/16"
    bar = BarParser(dir)
    bar.read_in(f"{dir}/ori_bar.out")
    bar.stat()
    # for i in range(11, 17):
    #     dir = f"{work_dir}/op_record/{i}"
    #     mont = MontParser(dir)
    #     mont.read_in(f"{dir}/step4_mont.out")
    #     mont.stat()
    #     bar = BarParser(dir)
    #     bar.read_in(f"{dir}/step4_bar.out")
    #     bar.stat()
