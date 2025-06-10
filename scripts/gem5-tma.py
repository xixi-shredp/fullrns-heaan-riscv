#!/bin/env python3

import json


# Level 3
### 3.1.1
class FrontendLatency:
    value: float
    ICacheMisses: float
    ITLBMisses: float
    Branch_Resteers: float
    DSB_Switches: float
    LCP: float
    MS_Switches: float


### 3.1.2
class FrontendBandwidth:
    value: float
    MITE: float
    DSB: float
    LSD: float


# Level 2
## 2.1
class FrontendBound:
    value: float
    frontend_latency: FrontendLatency
    frontend_bandwidth: FrontendBandwidth


## 2.2
class BackendBound:
    pass


## 2.3
class Retiring:
    pass


## 2.4
class BadSpeculation:
    pass


# Level 1
class Root:
    frontend_bound: FrontendBound
    backend_bound: BackendBound
    retiring: Retiring
    bad_speculation: BadSpeculation


class TMA:
    # stat values
    issue_width: int
    num_cycles: int
    total_slots: int
    retired_slots: int
    issued_slots: int
    fetch_bubbles: int
    recovery_bubbles: int
    no_uop_issued_cycles: int
    icache_stall_cycles:int
    icache_tag_miss_stall_cycles:int

    root: Root

    def __init__(self) -> None:
        root = Root()

        root.frontend_bound = FrontendBound()
        root.backend_bound = BackendBound()
        root.retiring = Retiring()
        root.bad_speculation = BadSpeculation()

        root.frontend_bound.frontend_latency = FrontendLatency()
        root.frontend_bound.frontend_bandwidth = FrontendBandwidth()

        self.root = root

        self.total_slots = self.num_cycles * self.issue_width

    def cal_l1(self):
        l1 = self.root
        l1.frontend_bound = self.fetch_bubbles / self.total_slots
        l1.bad_speculation = (
            self.issued_slots - self.retired_slots + self.recovery_bubbles
        ) / self.total_slots
        l1.retiring = self.retired_slots / self.total_slots
        l1.backend_bound = 1 - (
            l1.frontend_bound + l1.bad_speculation + l1.retiring
        )
        # assert l1.backend_bound > 0

    def cal_l2(self):
        root = self.root

        def cal_l2_frontend_bound():
            root.frontend_bound.frontend_latency = (
                self.no_uop_issued_cycles * self.num_cycles
            )/self.total_slots
        cal_l2_frontend_bound()

    def cal_l3(self):
        root = self.root
        def cal_l3_frontend_latency():
            frontend_latency = root.frontend_bound.frontend_latency
            frontend_latency.ICacheMisses = self.icache_stall_cycles / self.num_cycles
            frontend_latency.ITLBMisses = self.icache_tag_miss_stall_cycles / self.num_cycles

        cal_l3_frontend_latency()

    def print_l1(self):
        print("Level 1")
        print("=" * 20)
        print(f"Frontend Bound...{self.root.frontend_bound:.2%}")
        print(f"Backend Bound....{self.root.backend_bound:.2%}")
        print(f"Retiring.........{self.root.retiring:.2%}")
        print(f"Bad Speculation..{self.root.bad_speculation:.2%}")


class Gem5TMA(TMA):
    target_core = 0
    cfg_json: dict

    # additional stats gem5 needs for TMA
    FuBusy: int

    def __init__(self, config: str, stat: str) -> None:
        with open(config, "r") as f:
            self.cfg_json = json.load(f)
        self.parse_config()
        with open(stat, "r") as f:
            lines = f.readlines()
        for line in lines:
            self.parse_stat_line(line)
        super().__init__()
        self.fetch_bubbles = self.total_slots - self.issued_slots - self.FuBusy

    def parse_config(self):
        cpu = self.cfg_json["system"]["cpu"][self.target_core]
        self.issue_width = cpu["issueWidth"]
        self.FetchWidth = cpu["fetchWidth"]

    def parse_stat_line(self, line: str):
        tem = line.split()
        if len(tem) < 3:
            return
        full_name = tem[0]
        prefix = f"system.cpu{self.target_core}."
        if prefix not in full_name:
            return
        name = ".".join(full_name.split(".")[2:])
        val_str = tem[1]
        if val_str == "nan":
            return

        def val_fp():
            return float(val_str)

        def val_int():
            return int(val_str)

        match name:
            case "numCycles":
                self.num_cycles = val_int()
            case "instsIssued":
                self.issued_slots = val_int()
            case "commit.committedInstType_0::total":
                self.retired_slots = val_int()
            case "iew.squashCycles":
                self.recovery_bubbles = val_int() * self.issue_width
            case "fuBusy":
                self.FuBusy = val_int()
            case "numIssuedDist::0":
                self.no_uop_issued_cycles = val_int()
            case "fetchStats0.icacheStallCycles":
                self.icache_stall_cycles = val_int()


if __name__ == "__main__":
    m5out_dir = "/home/xixi/xx/fhe/workload/FullRNS-HEAAN/result/gem5-riscv64/12/ori_bar"
    test = Gem5TMA(
        config=f"{m5out_dir}/config.json", stat=f"{m5out_dir}/stats.txt.out_1"
    )
    test.cal_l1()
    test.print_l1()
