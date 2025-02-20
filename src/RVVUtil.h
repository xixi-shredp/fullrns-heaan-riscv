#ifndef __RVVUTIL_HH__
#define __RVVUTIL_HH__

#include "Common.h"
#include "utils.h"

#ifdef CONFIG_RVV
    #ifdef CONFIG_XUANTIE_RISCV
        #define CONFIG_RVV0P7
    #else
        #define CONFIG_RVV1P0
    #endif

    #define vsetvli(vl, sew, mul)                            \
        ({                                                   \
            uint64_t res;                                    \
            asm volatile("vsetvli %0, %1, e" #sew ", m" #mul \
                         : "=r"(res)                         \
                         : "r"(vl));                         \
            res;                                             \
        })

    #define rvv_vv_insn(name, vd, vs2, vs1, vm) \
        asm volatile(#name ".vv " #vd ", " #vs2 ", " #vs1 vm)
    #define rvv_vx_insn(name, vd, vs2, rs1, vm) \
        asm volatile(#name ".vx " #vd ", " #vs2 ", %0" vm : : "r"(rs1))

    #ifdef CONFIG_RVV0P7
        #define vle_v(vd, rs1) \
            asm volatile("vle.v " #vd ", (%0)" : : "r"(rs1));
        #define vse_v(vs3, rs1) \
            asm volatile("vse.v " #vs3 ", (%0)" : : "r"(rs1));
        #define vlse_v(vd, rs1, rs2) \
            asm volatile("vlse.v " #vd ", (%0), %1" : : "r"(rs1), "r"(rs2));
        #define vsse_v(vs3, rs1, rs2) \
            asm volatile("vsse.v " #vs3 ", (%0), %1" : : "r"(rs1), "r"(rs2));
    #endif

    #ifdef CONFIG_RVV1P0
        #define vle_v(vd, rs1) \
            asm volatile("vle64.v " #vd ", (%0)" : : "r"(rs1));
        #define vse_v(vs3, rs1) \
            asm volatile("vse64.v " #vs3 ", (%0)" : : "r"(rs1));
        #define vlse_v(vd, rs1, rs2)                  \
            asm volatile("vlse64.v " #vd ", (%0), %1" \
                         :                            \
                         : "r"(rs1), "r"(rs2)         \
                         : "memory");
        #define vsse_v(vs3, rs1, rs2)                  \
            asm volatile("vsse64.v " #vs3 ", (%0), %1" \
                         :                             \
                         : "r"(rs1), "r"(rs2)          \
                         : "memory");
    #endif

    #define vmul_vv(vd, vs2, vs1)   rvv_vv_insn(vmul, vd, vs2, vs1, "")
    #define vmul_vx(vd, vs2, rs1)   rvv_vx_insn(vmul, vd, vs2, rs1, "")
    #define vmulhu_vv(vd, vs2, vs1) rvv_vv_insn(vmulhu, vd, vs2, vs1, "")
    #define vmulhu_vx(vd, vs2, rs1) rvv_vx_insn(vmulhu, vd, vs2, rs1, "")

    #define vsub_vv(vd, vs2, vs1)    rvv_vv_insn(vsub, vd, vs2, vs1, "")
    #define vsub_vx(vd, vs2, rs1)    rvv_vx_insn(vsub, vd, vs2, rs1, "")
    #define vsub_vx_vm(vd, vs2, rs1) rvv_vx_insn(vsub, vd, vs2, rs1, ", v0.t")
    #define vadd_vv(vd, vs2, vs1)    rvv_vv_insn(vadd, vd, vs2, vs1, "")
    #define vadd_vx(vd, vs2, rs1)    rvv_vx_insn(vadd, vd, vs2, rs1, "")
    #define vadd_vx_vm(vd, vs2, rs1) rvv_vx_insn(vadd, vd, vs2, rs1, ", v0.t")

    #define vmsgtu_vx(vd, vs2, rs1) rvv_vx_insn(vmsgtu, vd, vs2, rs1, "")
    #define vmsltu_vv(vd, vs2, vs1) rvv_vv_insn(vmsltu, vd, vs2, vs1, "")
    #define vminu_vv(vd, vs2, vs1)  rvv_vv_insn(vminu, vd, vs2, vs1, "")
    #define vmaxu_vv(vd, vs2, vs1)  rvv_vv_insn(vmaxu, vd, vs2, vs1, "")

    #define vmv_v_v(vd, vs1) asm volatile("vmv.v.v " #vd ", " #vs1)
    #define vmv_v_x(vd, rs1) asm volatile("vmv.v.x " #vd ", %0" : : "r"(rs1))

    #define vsll_vx(vd, vs2, rs1) rvv_vx_insn(vsll, vd, vs2, rs1, "")

    #define rvv_diff_init()             \
        uint64_t *bx = new uint64_t[N]; \
        for (int i = 0; i < N; ++i) {   \
            bx[i] = a[i];               \
        }

    #define rvv_diff(ref_code)                                          \
        ref_code;                                                       \
        for (int i = 0; i < N; ++i) {                                   \
            if (a[i] != bx[i]) {                                        \
                printf("error in diff: m=%ld idx=%d t=%ld\n", m, i, t); \
                printf("ref:0x%lx, dut:0x%lx\n", bx[i], a[i]);          \
                exit(1);                                                \
            }                                                           \
        }

    #define vlen_check()                                    \
        {                                                   \
            uint64_t vlen_b;                                \
            asm volatile("csrr %0,vlenb" : "=r"(vlen_b) :); \
            printf("vlenb : %ld.\n", vlen_b);               \
        }

class RVVUtil
{
  public:
    int vl;
    int lmul;
    RVVUtil(int lmul) : lmul(lmul)
    {
        switch (lmul) {
            case 1: vl = vsetvli(-1, 64, 1); break;
            case 2: vl = vsetvli(-1, 64, 2); break;
            case 4: vl = vsetvli(-1, 64, 4); break;
            case 8: vl = vsetvli(-1, 64, 8); break;
            default:
                fprintf(stderr, "[error] invalid lmul %d.\n", lmul);
                exit(1);
        }
        if (vl != 2 * lmul) {
            printf("[error]: vl != 2*lmul \n");
            exit(1);
        }
    }
};

#endif // CONFIG_RVV

#endif //__RVVUTIL_HH__
