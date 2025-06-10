# FullRNS-HEAAN-RVMod

This code is implementation of the paper "A RISC-V Extended Infrastructure for Edge FHE Through Software and Hardware Co-Design"

```bash
.
├── gem5-modify        # patched srcs in gem5
├── toolchain-modify   # patched srcs in riscv-gnu-toolchain
├── Kconfig            # for menuconfig
├── lib
├── LICENSE
├── Makefile
├── README.md
├── run            # running src and gem5 runing configuration
├── scripts        # running scripts and statistic scripts
└── src            # FullRNS-HEAAN srcs
```

## Preparation

### Gem5 Configuration

1. clone gem5

the version we used is `24.1.0.0`.

2. patch gem5 (replace `$GEM5` with you gem5 root directory)

```bash
GEM5=/opt/gem5
cp gem5-modify/fhe.isa $GEM5/src/arch/riscv/isa/format/
mv gem5-modify/isa_parser.py $GEM5/src/arch/isa_parser/isa_parser.py
mv gem5-modify/decoder.isa $GEM5/src/arch/riscv/decoder.isa
mv gem5-modify/includes.isa $GEM5/src/arch/riscv/isa/includes.isa
mv gem5-modify/operands.isa $GEM5/src/arch/riscv/isa/operands.isa
cp gem5-modify/fhe.cc $GEM5/src/arch/riscv/insts/
cp gem5-modify/fhe.cc $GEM5/src/arch/riscv/insts/
echo "Source('fhe.cc', tags='riscv isa')" >> $GEM5/src/arch/riscv/insts/SConscript
mv gem5-modify/pcstate.hh $GEM5/src/arch/riscv/pcstate.hh
mv gem5-modify/op_class.hh $GEM5/src/cpu/op_class.hh
mv gem5-modify/FuncUnit.py $GEM5/src/cpu/FuncUnit.py
cp gem5-modify/c910_bi_mode.hh $GEM5/src/cpu/pred/
cp gem5-modify/c910_bi_mode.cc $GEM5/src/cpu/pred/
mv gem5-modify/pred-SConscript.hh $GEM5/src/cpu/pred/SConscript
```

3. build gem5

### Compiler Requirement

1. clone riscv-gnu-toolchain

2. patch riscv-gnu-toolchain

```bash
RISCV=/opt/riscv-gnu-toolchain
mv toolchain-modify/riscv-opc.h $RISCV/binutils/include/opcode/riscv-opc.h
mv toolchain-modify/riscv-opc.c $RISCV/binutils/opcodes/riscv-opc.c
```

The tag of binutils we used is `binutils_2_41`.

3. build gcc

## Run

```bash
make menuconfig # need kconfiglib for python
make run
```

The `ARCH` we defined in menuconfig:

| ARCH         | ISA            | usage                      |
| ------------ | -------------- | -------------------------- |
| native       | x86-64         | test lib                   |
| XuanTie-Riscv| rv64gcv0p7     | diff-test with gem5        |
| Gem5-Riscv   | rv64gcv1p0_fhe | mainly used in our project |
| Qemu-Riscv   | rv64gcv1p0     | diff-test with gem5        |

## License

???
