ifeq ($(ARCH),native)
run: $(BIN)
	$(BIN) 0 $(TEST_LOGN) | tee $(RESULT_FILE).ori
	$(BIN) 1 $(TEST_LOGN) | tee $(RESULT_FILE).step4
endif

ifeq ($(ARCH),xuantie-riscv64)
run: $(BIN)
	scp $^ $(XUANTIE_HOST):~
	ssh $(XUANTIE_HOST) \
	"./$(notdir $(BIN)) 0 $(TEST_LOGN) && ./$(notdir $(BIN)) 1 $(TEST_LOGN)" \
	2>&1 | tee $(RESULT_FILE)
endif

ifeq ($(ARCH),gem5-riscv64)
run: $(BIN)
	make -C run/gem5/ se TARGET=$(abspath $(BIN)) TARGET_OPTION="1 $(TEST_LOGN)"
	rm -rf $(RESULT_FILE).step4-ntt/
	mv ./run/gem5/result/m5out/ $(RESULT_FILE).step4-ntt/
	# make -C run/gem5/ se TARGET=$(abspath $(BIN)) TARGET_OPTION="0 $(TEST_LOGN)"
	# rm -rf $(RESULT_FILE).ori-ntt/
	# mv ./run/gem5/result/m5out/ $(RESULT_FILE).ori-ntt/
endif

ifeq ($(ARCH),qemu-riscv64)
run: $(BIN)
	qemu-riscv64 $(BIN) 0 $(TEST_LOGN) | tee $(RESULT_FILE).ori
	qemu-riscv64 $(BIN) 1 $(TEST_LOGN) | tee $(RESULT_FILE).step4
gdb: $(BIN)
	qemu-riscv64 -g "127.0.0.1:1234" $(BIN) 1 $(TEST_LOGN) &
	gdb-multiarch \
	-ex "set architecture riscv:rv64" \
	-ex "target remote 127.0.0.1:1234"
endif

asm: $(ASMS)

.PHONY: lib bin run

