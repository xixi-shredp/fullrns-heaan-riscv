include ./.config

ARCH           ?= $(shell echo $(CONFIG_ARCH))
CROSS_COMPILER ?= $(shell echo $(CONFIG_CROSS_COMPILER))

CXX = $(CROSS_COMPILER)g++
AR  = $(CROSS_COMPILER)ar

ifneq ($(CONFIG_DEBUG_MSG),)
CXXFLAGS += -g
endif

ifneq ($(CONFIG_XUANTIE_RISCV),)
ifneq ($(CONFIG_XTHEADC),)
CXXFLAGS += -march=rv64gcv0p7_xtheadcb
else
CXXFLAGS += -march=rv64gcv0p7
endif
CXXFLAGS += -I$(XUANTIE_TOOLS)/lib/gcc/riscv64-unknown-linux-gnu/10.4.0/include
endif

ifneq ($(CONFIG_QEMU_RISCV),)
CXXFLAGS += -march=rv64gcv
endif

ifneq ($(CONFIG_GEM5_RISCV),)
CXXFLAGS += -march=rv64gcv
CXXFLAGS += -static -I$(GEM5_ROOT)/include/
LDFLAGS  += -L$(GEM5_ROOT)/util/m5/build/riscv/out -lm5
endif


ifneq ($(CONFIG_AUTO_VECTORIZE),)
CXX = /opt/riscv/toolchain/bin/riscv64-unknown-linux-gnu-g++
CXXFLAGS += -march=rv64gcv -O3 -ftree-vectorize
endif

