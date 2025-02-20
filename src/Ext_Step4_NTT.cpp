#include "Context.h"

#ifdef CONFIG_FHE_EXT
    #include "Step4NTTUtil.h"

static inline void
ext_rowNTTWithBar(uint64_t *a, long N, long logN, uint64_t q,
                  uint64_t *qRootPows, uint64_t *barPres)
{
    long t     = N;
    long logt1 = logN + 1;

    for (long m = 1; m < N; m <<= 1) {
        t >>= 1;
        logt1 -= 1;
        for (long i = 0; i < m; i++) {
            long j1    = i << logt1;
            long j2    = j1 + t - 1;
            uint64_t W = qRootPows[m + i];
            uint64_t R = barPres[m + i];
    #ifdef CONFIG_FHE_EXT_SCALAR_SCHEDULE
            /// Way 1: 2-butterfly pal
            // if (t % 2 == 0)
            //     for (long j = j1; j <= j2; j += 2) {
            //         uint64_t V1  = bar_mulmod_s(a[j + t], W, R);
            //         uint64_t V2  = bar_mulmod_s(a[j + t + 1], W, R);
            //         a[j + t]     = submod(a[j], V1);
            //         a[j]         = addmod(a[j], V1);
            //         a[j + t + 1] = submod(a[j + 1], V2);
            //         a[j + 1]     = addmod(a[j + 1], V2);
            //     }
            // else
            //     for (long j = j1; j <= j2; j++) {
            //         uint64_t T = a[j + t];
            //         uint64_t V = bar_mulmod_s(T, W, R);
            //         a[j + t]   = submod(a[j], V);
            //         a[j]       = addmod(a[j], V);
            //     }

            /// Way 2: fixed-butterfly pal
            ///        The Barrett modular multiplier is a 6-stage pipeline. To
            ///        fully utilize its pipeline, the maximum parallel
            ///        granularity should be greater than 6.
            ///        Since the 4-step NTT involves a large number of row
            ///        ntts, and the size of each row ntt is small, introducing
            ///        more branches may affect the final performance.
            ///        Therefore, fewer branches may be needed. The best
            ///        performance tested in the end is 8-4-1
            if (t % 8 == 0) { /// 8-butterfly pal
                for (long j = j1; j <= j2; j += 8) {
                    uint64_t V0  = bar_mulmod_s(a[j + t + 0], W, R);
                    uint64_t V1  = bar_mulmod_s(a[j + t + 1], W, R);
                    uint64_t V2  = bar_mulmod_s(a[j + t + 2], W, R);
                    uint64_t V3  = bar_mulmod_s(a[j + t + 3], W, R);
                    uint64_t V4  = bar_mulmod_s(a[j + t + 4], W, R);
                    uint64_t V5  = bar_mulmod_s(a[j + t + 5], W, R);
                    uint64_t V6  = bar_mulmod_s(a[j + t + 6], W, R);
                    uint64_t V7  = bar_mulmod_s(a[j + t + 7], W, R);
                    a[j + t + 0] = submod(a[j + 0], V0);
                    a[j + t + 1] = submod(a[j + 1], V1);
                    a[j + t + 2] = submod(a[j + 2], V2);
                    a[j + t + 3] = submod(a[j + 3], V3);
                    a[j + t + 4] = submod(a[j + 4], V4);
                    a[j + t + 5] = submod(a[j + 5], V5);
                    a[j + t + 6] = submod(a[j + 6], V6);
                    a[j + t + 7] = submod(a[j + 7], V7);
                    a[j + 0]     = addmod(a[j + 0], V0);
                    a[j + 1]     = addmod(a[j + 1], V1);
                    a[j + 2]     = addmod(a[j + 2], V2);
                    a[j + 3]     = addmod(a[j + 3], V3);
                    a[j + 4]     = addmod(a[j + 4], V4);
                    a[j + 5]     = addmod(a[j + 5], V5);
                    a[j + 6]     = addmod(a[j + 6], V6);
                    a[j + 7]     = addmod(a[j + 7], V7);
                }
            } else if (t % 4 == 0) { /// 4-butterfly pal
                for (long j = j1; j <= j2; j += 4) {
                    uint64_t V0  = bar_mulmod_s(a[j + t + 0], W, R);
                    uint64_t V1  = bar_mulmod_s(a[j + t + 1], W, R);
                    uint64_t V2  = bar_mulmod_s(a[j + t + 2], W, R);
                    uint64_t V3  = bar_mulmod_s(a[j + t + 3], W, R);
                    a[j + t + 0] = submod(a[j + 0], V0);
                    a[j + t + 1] = submod(a[j + 1], V1);
                    a[j + t + 2] = submod(a[j + 2], V2);
                    a[j + t + 3] = submod(a[j + 3], V3);
                    a[j + 0]     = addmod(a[j + 0], V0);
                    a[j + 1]     = addmod(a[j + 1], V1);
                    a[j + 2]     = addmod(a[j + 2], V2);
                    a[j + 3]     = addmod(a[j + 3], V3);
                }
            }
            // else if (t % 2 == 0)
            // { /// 2-butterfly pal
            //     for (long j = j1; j <= j2; j += 2) {
            //         uint64_t V0  = bar_mulmod_s(a[j + t + 0], W, R);
            //         uint64_t V1  = bar_mulmod_s(a[j + t + 1], W, R);
            //         a[j + t + 0] = submod(a[j + 0], V0);
            //         a[j + t + 1] = submod(a[j + 1], V1);
            //         a[j + 0]     = addmod(a[j + 0], V0);
            //         a[j + 1]     = addmod(a[j + 1], V1);
            //     }
            // }
            else
            { /// single-butterfly
                for (long j = j1; j <= j2; j++) {
                    uint64_t T = a[j + t];
                    uint64_t V = bar_mulmod_s(a[j + t], W, R);
                    a[j + t]   = submod(a[j], V);
                    a[j]       = addmod(a[j], V);
                }
            }

                /// Way 3: all-pipelined, but without great performance,
                ///        because of more load/store inst.
                // for (long j = j1; j <= j2; j++) {
                //     uint64_t T = a[j + t];
                //     uint64_t V = bar_mulmod_s(T, W, R);
                //     a[j + t]   = V;
                // }
                // for (long j = j1; j <= j2; j++) {
                //     uint64_t V = a[j + t];
                //     a[j + t]   = submod(a[j], V);
                //     a[j]       = addmod(a[j], V);
                // }
    #else
            for (long j = j1; j <= j2; j++) {
                uint64_t T = a[j + t];
                uint64_t V = bar_mulmod_s(T, W, R);
                a[j + t]   = submod(a[j], V);
                a[j]       = addmod(a[j], V);
            }
    #endif
        }
    }
}

static inline void
ext_rowNTTWithMont(uint64_t *a, long N, long logN, uint64_t q, uint64_t qInv,
                   uint64_t *qRootScalePows)
{
    long t     = N;
    long logt1 = logN + 1;

    for (long m = 1; m < N; m <<= 1) {
        t >>= 1;
        logt1 -= 1;
        for (long i = 0; i < m; i++) {
            long j1    = i << logt1;
            long j2    = j1 + t - 1;
            uint64_t W = qRootScalePows[m + i];
    #ifdef CONFIG_FHE_EXT_SCALAR_SCHEDULE
            /// Way 1: 2-butterfly pal
            if (t % 2 == 0)
                for (long j = j1; j <= j2; j += 2) {
                    uint64_t V1  = mont_redc(a[j + t], W);
                    uint64_t V2  = mont_redc(a[j + t + 1], W);
                    a[j + t]     = submod(a[j], V1);
                    a[j + t + 1] = submod(a[j + 1], V2);
                    a[j]         = addmod(a[j], V1);
                    a[j + 1]     = addmod(a[j + 1], V2);
                }
            else
                for (long j = j1; j <= j2; j++) {
                    uint64_t T = a[j + t];
                    uint64_t V = mont_redc(T, W);
                    a[j + t]   = submod(a[j], V);
                    a[j]       = addmod(a[j], V);
                }

            /// Way 2: fixed-butterfly pal
            ///        The Montgomeny modular multiplier is a 8-stage pipeline. To
            ///        fully utilize its pipeline, the maximum parallel
            ///        granularity should be greater than 8.
            ///        Since the 4-step NTT involves a large number of row
            ///        ntts, and the size of each row ntt is small, introducing
            ///        more branches may affect the final performance.
            ///        Therefore, fewer branches may be needed. The best
            ///        performance tested in the end is 2-1
            // if (t % 8 == 0) { /// 8-butterfly pal
            //     for (long j = j1; j <= j2; j += 8) {
            //         uint64_t V0  = mont_redc(a[j + t + 0], W);
            //         uint64_t V1  = mont_redc(a[j + t + 1], W);
            //         uint64_t V2  = mont_redc(a[j + t + 2], W);
            //         uint64_t V3  = mont_redc(a[j + t + 3], W);
            //         uint64_t V4  = mont_redc(a[j + t + 4], W);
            //         uint64_t V5  = mont_redc(a[j + t + 5], W);
            //         uint64_t V6  = mont_redc(a[j + t + 6], W);
            //         uint64_t V7  = mont_redc(a[j + t + 7], W);
            //         a[j + t + 0] = submod(a[j + 0], V0);
            //         a[j + t + 1] = submod(a[j + 1], V1);
            //         a[j + t + 2] = submod(a[j + 2], V2);
            //         a[j + t + 3] = submod(a[j + 3], V3);
            //         a[j + t + 4] = submod(a[j + 4], V4);
            //         a[j + t + 5] = submod(a[j + 5], V5);
            //         a[j + t + 6] = submod(a[j + 6], V6);
            //         a[j + t + 7] = submod(a[j + 7], V7);
            //         a[j + 0]     = addmod(a[j + 0], V0);
            //         a[j + 1]     = addmod(a[j + 1], V1);
            //         a[j + 2]     = addmod(a[j + 2], V2);
            //         a[j + 3]     = addmod(a[j + 3], V3);
            //         a[j + 4]     = addmod(a[j + 4], V4);
            //         a[j + 5]     = addmod(a[j + 5], V5);
            //         a[j + 6]     = addmod(a[j + 6], V6);
            //         a[j + 7]     = addmod(a[j + 7], V7);
            //     }
            // }
            // else if (t % 4 == 0) { /// 4-butterfly pal
            //           for (long j = j1; j <= j2; j += 4) {
            //               uint64_t V0  = mont_redc(a[j + t + 0], W);
            //               uint64_t V1  = mont_redc(a[j + t + 1], W);
            //               uint64_t V2  = mont_redc(a[j + t + 2], W);
            //               uint64_t V3  = mont_redc(a[j + t + 3], W);
            //               a[j + t + 0] = submod(a[j + 0], V0);
            //               a[j + t + 1] = submod(a[j + 1], V1);
            //               a[j + t + 2] = submod(a[j + 2], V2);
            //               a[j + t + 3] = submod(a[j + 3], V3);
            //               a[j + 0]     = addmod(a[j + 0], V0);
            //               a[j + 1]     = addmod(a[j + 1], V1);
            //               a[j + 2]     = addmod(a[j + 2], V2);
            //               a[j + 3]     = addmod(a[j + 3], V3);
            //           }
            //       }
            // else if (t % 2 == 0)
            // { /// 2-butterfly pal
            //     for (long j = j1; j <= j2; j += 2) {
            //         uint64_t V0  = mont_redc(a[j + t + 0], W);
            //         uint64_t V1  = mont_redc(a[j + t + 1], W);
            //         a[j + t + 0] = submod(a[j + 0], V0);
            //         a[j + t + 1] = submod(a[j + 1], V1);
            //         a[j + 0]     = addmod(a[j + 0], V0);
            //         a[j + 1]     = addmod(a[j + 1], V1);
            //     }
            // } else { /// single-butterfly
            //     for (long j = j1; j <= j2; j++) {
            //         uint64_t T = a[j + t];
            //         uint64_t V = mont_redc(a[j + t], W);
            //         a[j + t]   = submod(a[j], V);
            //         a[j]       = addmod(a[j], V);
            //     }
            // }

                /// Way 3: all-pipelined, but without great performance,
                ///        because of more load/store inst.
                // for (long j = j1; j <= j2; j++) {
                //     uint64_t T = a[j + t];
                //     uint64_t V = mont_redc(T, W);
                //     a[j + t]   = V;
                // }
                // for (long j = j1; j <= j2; j++) {
                //     uint64_t V = a[j + t];
                //     a[j + t]   = submod(a[j], V);
                //     a[j]       = addmod(a[j], V);
                // }
    #else
            for (long j = j1; j <= j2; j++) {
                uint64_t T = a[j + t];
                uint64_t V = mont_redc(T, W);
                a[j + t]   = submod(a[j], V);
                a[j]       = addmod(a[j], V);
            }
    #endif
        }
    }
}

static inline void
ext_row_mul_withBar(long rowIdx, long rowSz, long logRowSz, uint64_t *src,
                    uint64_t *wmatrix_pows, uint64_t *barPres, long q,
                    long qInv)
{
    for (int j = 0; j < rowSz; ++j) {
        uint64_t idx = (rowIdx << logRowSz) + j;
        uint64_t T   = src[idx];
        uint64_t W   = wmatrix_pows[idx];
        uint64_t R   = barPres[idx];
        src[idx]     = barrett_modMul_singalVal(T, W, q, R);
    }
}

static inline void
ext_row_mul_withMont(long rowIdx, long rowSz, long logRowSz, uint64_t *src,
                     uint64_t *wmatrix_scalepows, long q, long qInv)
{
    for (int j = 0; j < rowSz; ++j) {
        uint64_t idx = (rowIdx << logRowSz) + j;
        uint64_t T   = src[idx];
        uint64_t W   = wmatrix_scalepows[idx];
        src[idx]     = montgomey_redc(T, W, q, qInv);
    }
}

    #define ROW_MUL_Barrett                                       \
        ext_row_mul_withBar(i, N2, logN2, a, wmatrix_pows[index], \
                            s4ntt_wmatrixBarPres[index], q, qInv);

    #define ROW_MUL_Montgomeny                                             \
        ext_row_mul_withMont(i, N2, logN2, a, wmatrix_scalepows[index], q, \
                             qInv);

    #define ROW_NTT_Barrett(a, s, N)                        \
        {                                                   \
            uint64_t *__a1 = a + (i << log##N);             \
            ext_rowNTTWithBar(__a1, N, log##N, q,           \
                              s4ntt_##s##_qRootPows[index], \
                              s4ntt_##s##BarPres[index]);   \
        }

    #define ROW_NTT_Montgomeny(a, s, N)                            \
        {                                                          \
            uint64_t *__a1 = a + (i << log##N);                    \
            ext_rowNTTWithMont(__a1, N, log##N, q, qInv,           \
                               s4ntt_##s##_qRootScalePows[index]); \
        }

STEP4NTT_TEMPLATE_IMPLEMENT(ext_step4_qiNTTAndEqual_withBar, ROW_MUL_Barrett,
                            ROW_NTT_Barrett, SET_MOD);

STEP4NTT_TEMPLATE_IMPLEMENT(ext_step4_qiNTTAndEqual_withMont,
                            ROW_MUL_Montgomeny, ROW_NTT_Montgomeny, SET_MOD);

#endif
