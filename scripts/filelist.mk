CPP_SRCS += $(wildcard ./src/*.cpp)
OBJS       += $(patsubst ./src/%.cpp,./lib/src/$(ARCH)/%.o,$(CPP_SRCS))
CPP_DEPS   += $(patsubst ./src/%.cpp,./lib/src/$(ARCH)/%.d,$(CPP_SRCS))
# Each subdirectory must supply rules for building sources it contributes
#
./lib/src/$(ARCH)/%.o: ./src/%.cpp
	@mkdir -p ./lib/src/$(ARCH)/
	@echo '  [CXX] $< -> $@'
	@$(CXX) -I/usr/local/include -O3 -c $(CXXFLAGS) -std=c++11 -o "$@" "$<"

$(OBJS): $(CPP_SRCS) $(CONFIG_H)

TARGET_LIB = ./lib/libFRNSHEAAN-$(ARCH).a

$(TARGET_LIB): $(OBJS)
	@echo '  [AR] $^ -> $@'
	@$(AR) -r $@ $(OBJS) $(LIBS)
	@echo ' '

lib: $(TARGET_LIB)


ifeq ($(CONFIG_TEST_LIB),)
RUN_CPP = ./run/main-opt.cpp ./run/test_case.cpp
else
RUN_CPP = ./run/main.cpp
endif

BIN := ./run/FRNSHEAAN-$(ARCH)
ifneq ($(CONFIG_NTT_OP_SO),)
	BIN := $(BIN)-SO
else
ifneq ($(CONFIG_NTT_OP_HO),)
	BIN := $(BIN)-HO
else
ifneq ($(CONFIG_NTT_OP_CO),)
	BIN := $(BIN)-CO
endif
endif
endif

ifneq ($(CONFIG_NTT_BAR),)
ifneq ($(CONFIG_EN_STEP4_NTT),)
	BIN := $(BIN)-S4Bar
else
	BIN := $(BIN)-Bar
endif
else
ifneq ($(CONFIG_NTT_MONT),)
ifneq ($(CONFIG_EN_STEP4_NTT),)
	BIN := $(BIN)-S4Mont
else
	BIN := $(BIN)-Mont
endif
endif
endif


$(BIN): $(RUN_CPP) $(TARGET_LIB)
	@echo '  [CXX] $< -> $@'
	@$(CXX) -std=c++11 $(CXXFLAGS) -pthread -o $@ $^ -I./src/ $(LDFLAGS)
	@echo ' '

bin: $(BIN)
