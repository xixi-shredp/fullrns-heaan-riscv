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


BIN = ./run/FRNSHEAAN-$(ARCH)
RUN_CPP = ./run/main-opt.cpp ./run/test_case.cpp

$(BIN): $(RUN_CPP) $(TARGET_LIB)
	@echo '  [CXX] $< -> $@'
	@$(CXX) -std=c++11 $(CXXFLAGS) -pthread -o $@ $^ -I./src/ $(LDFLAGS)
	@echo ' '

bin: $(BIN)
