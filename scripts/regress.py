#!/bin/env python3

import os
from ansi.color import fg
from pyxixi.spaceExplore import TaskDispatcher, TaskThread, TaskStatus

root = os.getenv("FHE_ROOT")
work_dir = f"{root}/workload/FullRNS-HEAAN"
if root:
    gem5_run_dir = f"{work_dir}/run/gem5"
    gem5_target = f"{work_dir}/run/FRNSHEAAN-gem5-riscv64"
else:
    print("need env: FHE_ROOT")
    exit(1)


def check_dir(path: str):
    if not os.path.isdir(path):
        print(fg.red("[ERROR] dir not found") + f": {path}")
        return False
    return True


def check_file(path: str):
    if not os.path.isfile(path):
        print(fg.red("[ERROR] file not found") + f": {path}")
        return False
    return True


if not check_file(gem5_target):
    exit(1)


class MyTask(TaskThread):
    arch: str
    logN: int
    bar: bool
    rvv: bool
    mt: bool
    ext: bool
    step: bool
    args = {
        "arch": ["gem5-riscv64"],
        "logN": [11, 12, 13, 14, 15, 16],
        "bar": [True],
        "rvv": [True],
        "mt": [False],
        "step": [False],
        "ext": [True],
    }
    parallel = True
    slient = False

    def arg_init(self) -> None:
        self.cfg_prompt = f"logN={self.logN}, bar={self.bar}, mt={self.mt}, rvv={self.rvv}, ext={self.ext}, step={self.step}"
        self.error_prompt = fg.red("stop task: " + self.cfg_prompt)
        self.start_prompt = fg.yellow("start task: " + self.cfg_prompt)
        self.finish_prompt = fg.green("finish task: " + self.cfg_prompt)
        case_name = ""
        if self.mt:
            case_name += "mt_"
        if self.rvv:
            case_name += "rvv_"
        if self.ext:
            case_name += "ext_"
        if self.step:
            case_name += "step4_"
        else:
            case_name += "ori_"
        if self.bar:
            case_name += "bar"
        else:
            case_name += "mont"
        self.case_name = case_name

    def hello(self):
        self.run_process(cmd=["echo", "hello"])

    def root_run(self):
        test_option = f"--check --logN {self.logN} --case {self.case_name}"
        target_dir = f"{work_dir}/result/{self.arch}/{self.logN}"
        if not os.path.exists(target_dir):
            os.makedirs(target_dir)
        cmd = [
            "make",
            f"--directory={work_dir}",
            "run",
            f"ARCH={self.arch}",
            f"RESULT_FILE={target_dir}/{self.case_name}.out",
            f"TEST_OPTION={test_option}",
        ]
        self.run_process(cmd, f"{self.arch} Running")

    def gem5_run(self):
        def check_pass(path: str) -> bool:
            with open(path, "r") as f:
                stdout = f.readlines()[-2]
                if "pass" in stdout:
                    return True
                else:
                    return False

        build_dir = f"{work_dir}/result/gem5-riscv64/{self.logN}/{self.case_name}"
        target_option = f"--check --logN {self.logN} --case {self.case_name}"
        cmd = [
            "make",
            f"--directory={gem5_run_dir}",
            "se",
            f"TARGET={gem5_target}",
            f"TARGET_OPTION={target_option}",
            f"BUILD_DIR={build_dir}",
        ]
        self.run_process(cmd, "Gem5 Running")

        cfg_file = f"{build_dir}/config.json"
        stat_file = f"{build_dir}/stats.txt.out_1"
        check = check_dir(build_dir) and check_file(cfg_file) and check_file(stat_file)
        if not check:
            self.status = TaskStatus.FILE_ERROR
        if not check_pass(f"{build_dir}/stdout.txt"):
            self.status = TaskStatus.VALUE_ERROR


class ModTest(TaskThread):
    logN: int
    case: str
    args = {
        "logN": [11, 12, 13, 14, 15, 16],
        "case": ["addmod", "barmulmod", "montredc"],
    }
    parallel = True
    slient = False

    def arg_init(self) -> None:
        self.cfg_prompt = f"logN={self.logN} case={self.case}"
        self.error_prompt = fg.red("stop task: " + self.cfg_prompt)
        self.start_prompt = fg.yellow("start task: " + self.cfg_prompt)
        self.finish_prompt = fg.green("finish task: " + self.cfg_prompt)

    def run(self):
        build_dir = f"{work_dir}/result/mod-test/{self.logN}/{self.case}"
        target_option = f"--logN {self.logN} --case {self.case}"
        cmd = [
            "make",
            f"--directory={gem5_run_dir}",
            "se",
            f"TARGET={work_dir}/run/mod_test",
            f"TARGET_OPTION={target_option}",
            f"BUILD_DIR={build_dir}",
        ]
        self.run_process(cmd, "Gem5 Running")

        stat_file = f"{build_dir}/stdout.txt"
        check = check_dir(build_dir) and check_file(stat_file)
        if not check:
            self.status = TaskStatus.FILE_ERROR


class RiseTest(TaskThread):
    logN: int
    logQ: int
    args = {"logN": [10, 11, 12, 13, 14]}
    parallel = True
    slient = False
    qn_maps = {10: 27, 11: 30, 12: 90, 13: 216, 14: 432}

    def arg_init(self) -> None:
        self.logQ = self.qn_maps[self.logN]
        self.cfg_prompt = f"logN={self.logN} logQ={self.logQ}"
        self.error_prompt = fg.red("stop task: " + self.cfg_prompt)
        self.start_prompt = fg.yellow("start task: " + self.cfg_prompt)
        self.finish_prompt = fg.green("finish task: " + self.cfg_prompt)

    def run(self):
        def check_pass(path: str):
            with open(path, "r") as f:
                lines = f.readlines()
            eval = lines[-4].lstrip("eval:(").split(")")[0].split(",")
            eval = map(float, eval)
            for i in eval:
                if abs(i) > 1e-3:
                    return False
            return True

        build_dir = f"{work_dir}/result/gem5-riscv64-sota/{self.logN}-{self.logQ}-base"
        target_option = f"--sota --logN {self.logN} --logQ {self.logQ}"
        cmd = [
            "make",
            f"--directory={gem5_run_dir}",
            "se",
            f"TARGET={work_dir}/run/FRNSHEAAN-gem5-riscv64",
            f"TARGET_OPTION={target_option}",
            f"BUILD_DIR={build_dir}",
        ]
        self.run_process(cmd, "Gem5 Running")

        stat_file = f"{build_dir}/stdout.txt"
        check = check_dir(build_dir) and check_file(stat_file)
        if not check:
            self.status = TaskStatus.FILE_ERROR
        check = check_pass(stat_file)
        if not check:
            self.status = TaskStatus.VALUE_ERROR


class OPRecoder(MyTask):
    def __init__(self) -> None:
        self.args = {
            "arch": ["native"],
            "logN": [16],
            "bar": [True],
            "rvv": [False],
            "step": [False],
            "ext": [False],
        }

    def op_record_redir(self):
        target_dir = f"{work_dir}/result/op_record/{self.logN}"
        if not os.path.exists(target_dir):
            os.makedirs(target_dir)
        os.rename(
            src=f"{work_dir}/result/op_record/op_recorder.out",
            dst=f"{target_dir}/{self.case_name}.out",
        )

    def echo(self):
        print("ll")


if __name__ == "__main__":

    def scalar_golden_test(t: MyTask):
        t.args["logN"] = [11]
        t.args["rvv"] = [False]

    def scalar_regress(t: MyTask):
        t.args["rvv"] = [False]

    def vector_golden_test(t: MyTask):
        t.args["logN"] = [11]
        t.args["rvv"] = [True]

    def vector_regress(t: MyTask):
        t.args["rvv"] = [True]

    task = MyTask()
    # vector_regress(task)

    task.set_task([MyTask.gem5_run])

    # task = ModTest()
    # task.set_task([ModTest.run])

    # task = RiseTest()
    # task.set_task([RiseTest.run])

    my = TaskDispatcher(task)
    my.start()
    my.report_status()
