ifeq ($(ARCH),native)
run: $(BIN)
	$(BIN) $(TEST_OPTION) | tee $(RESULT_FILE)
endif

ifeq ($(ARCH),xuantie-riscv64)
run: $(BIN)
	# scp $^ $(XUANTIE_HOST):~
	ssh $(XUANTIE_HOST) \
	"./$(notdir $(BIN)) $(TEST_OPTION)" \
	2>&1 | tee $(RESULT_FILE)
endif

ifeq ($(ARCH),gem5-riscv64)
BUILD_DIR = $(RESULT_DIR)/gem5-riscv64/$(TEST_LOGN)/$(TEST_CASE)/
run: $(BIN)
	make -C run/gem5/ se TARGET=$(abspath $(BIN)) \
	TARGET_OPTION="$(TEST_OPTION)" BUILD_DIR=$(BUILD_DIR)
endif

ifeq ($(ARCH),qemu-riscv64)
run: $(BIN)
	qemu-riscv64 $(BIN) $(TEST_OPTION) | tee $(RESULT_FILE)
gdb: $(BIN)
	qemu-riscv64 -g "127.0.0.1:1234" $(BIN) $(TEST_OPTION) &
	gdb-multiarch \
	-ex "set architecture riscv:rv64" \
	-ex "target remote 127.0.0.1:1234"
endif

asm: $(ASMS)

.PHONY: lib bin run

