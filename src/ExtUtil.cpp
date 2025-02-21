#include "ExtUtil.h"
#include "Context.h"

#ifdef CONFIG_FHE_EXT
void
Context::ext_qiNTTAndEqual_withBar(uint64_t *a, long index)
{
    long t        = N;
    long logt1    = logN + 1;
    uint64_t q    = qVec[index];
    uint64_t qInv = qInvVec[index];
    set_mod(q, qInv);

    for (long m = 1; m < N; m <<= 1) {
        t >>= 1;
        logt1 -= 1;
        for (long i = 0; i < m; i++) {
            long j1       = i << logt1;
            long j2       = j1 + t - 1;
            uint64_t W    = qRootScalePows[index][m + i];
            uint64_t Wori = qRootPows[index][m + i];
            uint64_t R    = nttBarPres[index][m + i];
    #ifdef CONFIG_FHE_EXT_SCALAR_SCHEDULE
            /// Way 1: 2-butterfly pal
            // if (t % 2 == 0) {
            //     for (long j = j1; j <= j2; j += 2) {
            //         uint64_t V1 =
            //             bar_mulmod_s(a[j + t], Wori,
            //             nttBarPres[index][m + i]);
            //         uint64_t V2  = bar_mulmod_s(a[j + t + 1], Wori,
            //                                     nttBarPres[index][m
            //                                     + i]);
            //         a[j + t]     = submod(a[j], V1);
            //         a[j + t + 1] = submod(a[j + 1], V2);
            //         a[j]         = addmod(a[j], V1);
            //         a[j + 1]     = addmod(a[j + 1], V2);
            //     }
            // } else {
            //     for (long j = j1; j <= j2; j++) {
            //         uint64_t T = a[j + t];
            //         uint64_t V =
            //             bar_mulmod_s(T, Wori, nttBarPres[index][m +
            //             i]);
            //         a[j + t] = submod(a[j], V);
            //         a[j]     = addmod(a[j], V);
            //     }
            // }

            /// Way 2: fixed-butterfly pal
            ///        The Barrett modular multiplier is a 6-stage
            ///        pipeline. To fully utilize its pipeline, the
            ///        maximum parallel granularity should be greater
            ///        than 6.
            if (t % 8 == 0) { /// 8-butterfly pal
                for (long j = j1; j <= j2; j += 8) {
                    uint64_t V0  = bar_mulmod_s(a[j + t], Wori, R);
                    uint64_t V1  = bar_mulmod_s(a[j + t + 1], Wori, R);
                    uint64_t V2  = bar_mulmod_s(a[j + t + 2], Wori, R);
                    uint64_t V3  = bar_mulmod_s(a[j + t + 3], Wori, R);
                    uint64_t V4  = bar_mulmod_s(a[j + t + 4], Wori, R);
                    uint64_t V5  = bar_mulmod_s(a[j + t + 5], Wori, R);
                    uint64_t V6  = bar_mulmod_s(a[j + t + 6], Wori, R);
                    uint64_t V7  = bar_mulmod_s(a[j + t + 7], Wori, R);
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
                    uint64_t V0  = bar_mulmod_s(a[j + t], Wori, R);
                    uint64_t V1  = bar_mulmod_s(a[j + t + 1], Wori, R);
                    uint64_t V2  = bar_mulmod_s(a[j + t + 2], Wori, R);
                    uint64_t V3  = bar_mulmod_s(a[j + t + 3], Wori, R);
                    a[j + t + 0] = submod(a[j + 0], V0);
                    a[j + t + 1] = submod(a[j + 1], V1);
                    a[j + t + 2] = submod(a[j + 2], V2);
                    a[j + t + 3] = submod(a[j + 3], V3);
                    a[j + 0]     = addmod(a[j + 0], V0);
                    a[j + 1]     = addmod(a[j + 1], V1);
                    a[j + 2]     = addmod(a[j + 2], V2);
                    a[j + 3]     = addmod(a[j + 3], V3);
                }
            } else if (t % 2 == 0) { /// 2-butterfly pal
                for (long j = j1; j <= j2; j += 4) {
                    uint64_t V0  = bar_mulmod_s(a[j + t], Wori, R);
                    uint64_t V1  = bar_mulmod_s(a[j + t + 1], Wori, R);
                    a[j + t + 0] = submod(a[j + 0], V0);
                    a[j + t + 1] = submod(a[j + 1], V1);
                    a[j + 0]     = addmod(a[j + 0], V0);
                    a[j + 1]     = addmod(a[j + 1], V1);
                }
            } else { /// single-butterfly
                for (long j = j1; j <= j2; j++) {
                    uint64_t T = a[j + t];
                    uint64_t V = bar_mulmod_s(T, Wori, R);
                    a[j + t]   = submod(a[j], V);
                    a[j]       = addmod(a[j], V);
                }
            }
                /// Way 3: all-pipelined, but without great
                /// performance,
                ///        because of more load/store inst.
                // for (long j = j1; j <= j2; j++) {
                //     uint64_t T = a[j + t];
                //     uint64_t V = bar_mulmod_s(T, Wori,
                //     nttBarPres[index][m + i]); a[j + t]   = V;
                // }
                // for (long j = j1; j <= j2; j++) {
                //     uint64_t V = a[j + t];
                //     a[j + t]   = submod(a[j], V);
                //     a[j]       = addmod(a[j], V);
                // }
    #else
            for (long j = j1; j <= j2; j++) {
                uint64_t T = a[j + t];
                uint64_t V = bar_mulmod_s(T, Wori, nttBarPres[index][m + i]);
                a[j + t]   = submod(a[j], V);
                a[j]       = addmod(a[j], V);
            }
    #endif
        }
    }
}

void
Context::ext_qiNTTAndEqual_withMont(uint64_t *a, long index)
{
    long t        = N;
    long logt1    = logN + 1;
    uint64_t q    = qVec[index];
    uint64_t qInv = qInvVec[index];
    set_mod(q, qInv);
    for (long m = 1; m < N; m <<= 1) {
        t >>= 1;
        logt1 -= 1;
        for (long i = 0; i < m; i++) {
            long j1       = i << logt1;
            long j2       = j1 + t - 1;
            uint64_t W    = qRootScalePows[index][m + i];
            uint64_t Wori = qRootPows[index][m + i];
    #ifdef CONFIG_FHE_EXT_SCALAR_SCHEDULE
            /// Way 1: 2-butterfly pal
            // if (t % 2 == 0) {
            //     for (long j = j1; j <= j2; j += 2) {
            //         uint64_t V1  = mont_redc(a[j + t], W);
            //         uint64_t V2  = mont_redc(a[j + t + 1], W);
            //         a[j + t]     = submod(a[j], V1);
            //         a[j + t + 1] = submod(a[j + 1], V2);
            //         a[j]         = addmod(a[j], V1);
            //         a[j + 1]     = addmod(a[j + 1], V2);
            //     }
            // } else {
            //     for (long j = j1; j <= j2; j++) {
            //         uint64_t T = a[j + t];
            //         uint64_t V = mont_redc(T, W);
            //         a[j + t]   = submod(a[j], V);
            //         a[j]       = addmod(a[j], V);
            //     }
            // }

            /// Way 2: fixed-butterfly pal
            ///        The Montgomeny modular multiplier is a 8-stage
            ///        pipeline. To fully utilize its pipeline, the
            ///        maximum parallel granularity should be greater
            ///        than 8.
            if (t % 8 == 0) { /// 8-butterfly pal
                for (long j = j1; j <= j2; j += 8) {
                    uint64_t V0  = mont_redc(a[j + t + 0], W);
                    uint64_t V1  = mont_redc(a[j + t + 1], W);
                    uint64_t V2  = mont_redc(a[j + t + 2], W);
                    uint64_t V3  = mont_redc(a[j + t + 3], W);
                    uint64_t V4  = mont_redc(a[j + t + 4], W);
                    uint64_t V5  = mont_redc(a[j + t + 5], W);
                    uint64_t V6  = mont_redc(a[j + t + 6], W);
                    uint64_t V7  = mont_redc(a[j + t + 7], W);
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
                    uint64_t V0  = mont_redc(a[j + t + 0], W);
                    uint64_t V1  = mont_redc(a[j + t + 1], W);
                    uint64_t V2  = mont_redc(a[j + t + 2], W);
                    uint64_t V3  = mont_redc(a[j + t + 3], W);
                    a[j + t + 0] = submod(a[j + 0], V0);
                    a[j + t + 1] = submod(a[j + 1], V1);
                    a[j + t + 2] = submod(a[j + 2], V2);
                    a[j + t + 3] = submod(a[j + 3], V3);
                    a[j + 0]     = addmod(a[j + 0], V0);
                    a[j + 1]     = addmod(a[j + 1], V1);
                    a[j + 2]     = addmod(a[j + 2], V2);
                    a[j + 3]     = addmod(a[j + 3], V3);
                }
            } else if (t % 2 == 0) { /// 2-butterfly pal
                for (long j = j1; j <= j2; j += 4) {
                    uint64_t V0  = mont_redc(a[j + t + 0], W);
                    uint64_t V1  = mont_redc(a[j + t + 1], W);
                    a[j + t + 0] = submod(a[j + 0], V0);
                    a[j + t + 1] = submod(a[j + 1], V1);
                    a[j + 0]     = addmod(a[j + 0], V0);
                    a[j + 1]     = addmod(a[j + 1], V1);
                }
            } else { /// single-butterfly
                for (long j = j1; j <= j2; j++) {
                    uint64_t T = a[j + t];
                    uint64_t V = mont_redc(a[j + t], W);
                    a[j + t]   = submod(a[j], V);
                    a[j]       = addmod(a[j], V);
                }
            }
                /// Way 3: all-pipelined, but without great
                /// performance,
                ///        because of more load/store inst.(the physical
                ///        registers can't contain all the middle-result)
                // for (long j = j1; j <= j2; j++) {
                //     uint64_t T = a[j + t];
                //     uint64_t V = bar_mulmod_s(T, Wori,
                //     nttBarPres[index][m + i]); a[j + t]   = V;
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
#endif
