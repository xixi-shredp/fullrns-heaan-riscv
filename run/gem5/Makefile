TARGET ?= ../FRNSHEAAN-riscv64
TARGET_OPTION ?=

GEM5 = gem5-riscv

include ./c910.mk
include ../../scripts/util.mk

BUILD_DIR = ./result/m5out

simple:
	$(GEM5) simple.py
	
SE_FLAGS += -c $(TARGET)

ifneq ($(TARGET_OPTION),)
SE_FLAGS += -o "$(TARGET_OPTION)"
endif

GEM5_ARGS += -q -d $(BUILD_DIR)
SE_FLAGS += --output stdout.txt \
						--errout stderr.txt

# GEM5_ARGS += --debug-flag=ExecEnable,ExecUser,ExecKernel,ExecSymbol --debug-file=trace.out.gz 

se: $(TARGET)
	@$(GEM5) $(GEM5_ARGS) config/se.py $(SE_FLAGS)
	@echo "=================================="
	@echo "$(ANSI_FG_YELLOW)[$(GEM5)] run: $(notdir $(TARGET))$(ANSI_NONE)"
	@echo "$(ANSI_FG_GREEN)Stdout:$(ANSI_NONE)"
	@cat $(BUILD_DIR)/stdout.txt
	@echo "$(ANSI_FG_RED)Stderr:$(ANSI_NONE)"
	@cat $(BUILD_DIR)/stderr.txt
	@echo "=================================="

# Gem5 Mcpat v0.8
# MCPAT = $(GEM5_ROOT)/build/mcpat/mcpat
# Mcpat-7nm
# MCPAT = ../../../../mcpat-calib/mcpat/mcpat
# Mcpat v1.3
MCPAT  = ../../../../power_sim/mcpat/mcpat
PMXML ?= c910.xml

pm: $(MCPAT)
	$(MCPAT) -infile $(PMXML) | tee mcpat.out
