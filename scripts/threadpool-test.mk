CSRCS = ../src/ThreadPool.cpp \
			 ../run/threadpool-test.cpp

CXXFLAGS += -march=rv64gcv
CXXFLAGS += -static -I$(GEM5_ROOT)/include/
LDFLAGS  += -L$(GEM5_ROOT)/util/m5/build/riscv/out -lm5

BIN = ../run/threadpool-test
CXX = /opt/riscv/fhe-toolchain/bin/riscv64-unknown-linux-gnu-g++

$(BIN): $(CSRCS)
	$(CXX) $(CXXFLAGS) $(CSRCS) -o $(BIN) $(LDFLAGS) 

compile: $(BIN)

run: $(BIN)
	make -C ../run/gem5/ se \
		TARGET=$(abspath $(BIN))
