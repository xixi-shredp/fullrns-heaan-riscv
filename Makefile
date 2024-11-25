CONFIG_H   = ./config.h

include ./scripts/config.mk
include ./scripts/filelist.mk

NTT_ALGO = barrett
ifeq ($(CONFIG_NTT_BARRETT),)
NTT_ALGO = montgomeny
endif

EXT = _fhe
ifeq ($(CONFIG_FHE_EXT),)
EXT =
endif

RVV = _rvv
ifeq ($(CONFIG_RVV),)
RVV =
endif

# ALL Test Cases
TEST_LOGNS  = 14 15 16
RESULT_DIR  = $(patsubst %,./result/$(ARCH)/%,$(TEST_LOGNS))

# Default Test Case
TEST_LOGN   ?= 11
RESULT_FILE ?= ./result/$(ARCH)/$(TEST_LOGN)/$(NTT_ALGO)$(RVV)$(EXT).out
$(shell mkdir -p ./result/$(ARCH)/$(TEST_LOGN))

all: $(RESULT_DIR)


$(RESULT_DIR):
	$(shell mkdir -p $@)
	make run RESULT_FILE="$@/$(NTT_ALGO)$(RVV)$(EXT).out" TEST_LOGN=$(notdir $@)



include ./scripts/build.mk

menuconfig:
	@menuconfig Kconfig
	@genconfig

clean:
	rm -rf $(OBJS) $(TARGET_LIB)
	rm -rf $(BIN)

.PHONY: menuconfig clean $(RESULT_DIR)
