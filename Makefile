CONFIG_H   = ./config.h

include ./scripts/config.mk
include ./scripts/filelist.mk

RESULT_DIR = ./result

TEST_LOGN ?= 16
TEST_CASE ?= ori_bar

# Test Case
TEST_OPTION ?= --check --logN $(TEST_LOGN) --case $(TEST_CASE)
RESULT_FILE ?= $(RESULT_DIR)/$(ARCH)-noxt/$(TEST_LOGN)/$(TEST_CASE).out
$(shell mkdir -p ./result/$(ARCH)-noxt/$(TEST_LOGN))

include ./scripts/build.mk

menuconfig:
	@menuconfig Kconfig
	@genconfig

clean:
	rm -rf $(OBJS) $(TARGET_LIB)
	rm -rf $(BIN)

.PHONY: menuconfig clean $(RESULT_DIR)
