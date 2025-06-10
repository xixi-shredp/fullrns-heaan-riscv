CONFIG_H   = ./config.h

include ./scripts/config.mk
include ./scripts/filelist.mk

RESULT_DIR = ./result

TEST_LOGN ?= 12
TEST_CASE ?= ext_ori_bar

# Test Case
TEST_OPTION ?= --check --logN $(TEST_LOGN) --case $(TEST_CASE)
RESULT_FILE ?= $(RESULT_DIR)/$(ARCH)/$(TEST_LOGN)/$(TEST_CASE).out
$(shell mkdir -p ./result/$(ARCH)/$(TEST_LOGN))

include ./scripts/build.mk

regress:
	./scripts/regress.py && ./scripts/generate_md_table.py

menuconfig:
	@menuconfig Kconfig
	@genconfig

clean:
	rm -rf $(OBJS) $(TARGET_LIB)
	rm -rf $(BIN)

mod_test:
	make -C scripts -f mod_test.mk run

threadpool_test:
	make -C scripts -f threadpool-test.mk run

.PHONY: menuconfig clean $(RESULT_DIR)
