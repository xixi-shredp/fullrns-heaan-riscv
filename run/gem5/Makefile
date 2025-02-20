TARGET        ?= ../FRNSHEAAN-gem5-riscv64
# TARGET        ?= ~/ysyx/ysyx-workbench/am-kernels/tests/riscv-vector-tests-am/repo/out/v128x64user/bin/stage2/vse64_v-0
TARGET_OPTION ?= --check --logN 11 --case rvv_ext_ori_bar
BUILD_DIR     ?= ./result/m5out

GEM5 = gem5-riscv

include ./config/c910.mk
include ../../scripts/util.mk


SE_FLAGS += -c $(TARGET)

ifneq ($(TARGET_OPTION),)
SE_FLAGS += -o "$(TARGET_OPTION)"
endif

GEM5_ARGS += -d $(BUILD_DIR)
GEM5_CONFIG_PY = config/se.py

# GEM5_ARGS += --debug-flag=ExecEnable,ExecUser,ExecKernel,ExecSymbol --debug-file=trace.out.gz 

se: $(TARGET)
	@mkdir -p $(BUILD_DIR)
	@echo "=================================="
	@$(GEM5) $(GEM5_ARGS) $(GEM5_CONFIG_PY) $(SE_FLAGS) | tee $(BUILD_DIR)/stdout.txt
	@echo "$(ANSI_FG_YELLOW)[$(GEM5)] run: $(notdir $(TARGET))$(ANSI_NONE)"
	@echo "=================================="
	@./split_stat.py $(BUILD_DIR)/stats.txt

# ============================ O3CPU PipeLine View =========================
PIPEVIEW_START =  1000000
PIPEVIEW_STOP  = 23000000
PIPEVIEW_OUT   = $(BUILD_DIR)/pipeview.out

PIPEVIEW_FLAG = --debug-flags=O3PipeView \
								--debug-start=$(PIPEVIEW_START) \
								--debug-file=trace.out 

pipe-view:
	@$(GEM5) $(GEM5_ARGS) $(PIPEVIEW_FLAG) $(GEM5_CONFIG_PY) $(SE_FLAGS) -m $(PIPEVIEW_STOP)
	@$(GEM5_ROOT)/util/o3-pipeview.py -c 500 -o $(PIPEVIEW_OUT) --color $(BUILD_DIR)/trace.out
	@less -r $(PIPEVIEW_OUT)
