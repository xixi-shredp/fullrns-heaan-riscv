CONFIG_H   = ./config.h

include ./scripts/config.mk
include ./scripts/filelist.mk

RESULT_DIR = ./result

TEST_LOGN ?= 11
TEST_CASE ?= rvv_ori_mont

# Test Case
TEST_OPTION ?= --check --logN $(TEST_LOGN) --case $(TEST_CASE)
RESULT_FILE ?= $(RESULT_DIR)/$(ARCH)/$(TEST_LOGN)/$(TEST_CASE).out
$(shell mkdir -p ./result/$(ARCH)-noxt/$(TEST_LOGN))

include ./scripts/build.mk

menuconfig:
	@menuconfig Kconfig
	@genconfig

clean:
	rm -rf $(OBJS) $(TARGET_LIB)
	rm -rf $(BIN)

.PHONY: menuconfig clean $(RESULT_DIR)
