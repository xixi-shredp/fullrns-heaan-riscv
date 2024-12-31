#ifndef __FHE_UTIL_HH__
#define __FHE_UTIL_HH__

#define MajorCode "0x0b"

#define RTypeInst(FUNCT3, FUNCT7, rs1, rs2)                                    \
  ({                                                                           \
    uint64_t rd;                                                               \
    __asm__ __volatile__(".insn r " MajorCode ", " #FUNCT3 ", " #FUNCT7        \
                         ", %0, %1, %2"                                        \
                         : "=r"(rd)                                            \
                         : "r"(rs1), "r"(rs2));                                \
    rd;                                                                        \
  })

#define R4TypeInst(FUNCT3, FUNCT2, rs1, rs2, rs3)                              \
  ({                                                                           \
    uint64_t rd;                                                               \
    __asm__ __volatile__(".insn r4 " MajorCode ", " #FUNCT3 ", " #FUNCT2       \
                         ", %0, %1, %2, %3"                                    \
                         : "=r"(rd)                                            \
                         : "r"(rs1), "r"(rs2), "r"(rs3));                      \
    rd;                                                                        \
  })

#define set_mod(src1, src2) RTypeInst(0x0, 0x0, src1, src2)
#define mod(src1) RTypeInst(0x1, 0x0, src1, 0)
#define addmod(src1, src2) RTypeInst(0x2, 0x0, src1, src2)
#define submod(src1, src2) RTypeInst(0x2, 0x1, src1, src2)
#define mont_redc(src1, src2) RTypeInst(0x3, 0x0, src1, src2)

#define bar_mulmod_s(src1, src2, pre) R4TypeInst(0x3, 0x1, src1, src2, pre)

#define rvv_vv_insn(name, vd, vs2, vs1, vm)                                    \
  asm volatile(#name ".vv " #vd ", " #vs2 ", " #vs1 vm)
#define rvv_vx_insn(name, vd, vs2, rs1, vm)                                    \
  asm volatile(#name ".vx " #vd ", " #vs2 ", %0" vm : : "r"(rs1))
#define rvv_vxx_insn(name, vd, rs1, rs2, vm)                                   \
  asm volatile(#name ".vxx " #vd ", %0, %1" vm : : "r"(rs1),"r"(rs2))

// need to modify the riscv-opc of gcc.
#define vmod_vx(vd, vs2, rs1) rvv_vx_insn(vmod, vd, vs2, rs1, "")
#define vaddmod_vv(vd, vs2, vs1) rvv_vv_insn(vaddmod, vd, vs2, vs1, "")
#define vsubmod_vv(vd, vs2, vs1) rvv_vv_insn(vsubmod, vd, vs2, vs1, "")
#define vmontredc_vx(vd, vs2, rs1) rvv_vx_insn(vmontredc, vd, vs2, rs1, "")
#define vmontredc_vv(vd, vs2, vs1) rvv_vv_insn(vmontredc, vd, vs2, vs1, "")
#define vbarmodmuls_vv(vd, vs2, vs1) rvv_vv_insn(vbarmodmuls, vd, vs2, vs1, "")
#define vbarmodmuls_vxx(vd, rs1, rs2) rvv_vxx_insn(vbarmodmuls, vd, rs1, rs2, "")

#endif
