#include "ExtUtil.h"
#include "Context.h"
#include <cstdint>

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
            // if (t % 32 == 0) { /// 32-butterfly pal
            //     for (long j = j1; j <= j2; j += 32) {
            //         uint64_t V0   = bar_mulmod_s(a[j + t], Wori, R);
            //         uint64_t V1   = bar_mulmod_s(a[j + t + 1], Wori, R);
            //         uint64_t V2   = bar_mulmod_s(a[j + t + 2], Wori, R);
            //         uint64_t V3   = bar_mulmod_s(a[j + t + 3], Wori, R);
            //         uint64_t V4   = bar_mulmod_s(a[j + t + 4], Wori, R);
            //         uint64_t V5   = bar_mulmod_s(a[j + t + 5], Wori, R);
            //         uint64_t V6   = bar_mulmod_s(a[j + t + 6], Wori, R);
            //         uint64_t V7   = bar_mulmod_s(a[j + t + 7], Wori, R);
            //         uint64_t V8   = bar_mulmod_s(a[j + t + 8], Wori, R);
            //         uint64_t V9   = bar_mulmod_s(a[j + t + 9], Wori, R);
            //         uint64_t V10  = bar_mulmod_s(a[j + t + 10], Wori, R);
            //         uint64_t V11  = bar_mulmod_s(a[j + t + 11], Wori, R);
            //         uint64_t V12  = bar_mulmod_s(a[j + t + 12], Wori, R);
            //         uint64_t V13  = bar_mulmod_s(a[j + t + 13], Wori, R);
            //         uint64_t V14  = bar_mulmod_s(a[j + t + 14], Wori, R);
            //         uint64_t V15  = bar_mulmod_s(a[j + t + 15], Wori, R);
            //         uint64_t V16  = bar_mulmod_s(a[j + t + 16], Wori, R);
            //         uint64_t V17  = bar_mulmod_s(a[j + t + 17], Wori, R);
            //         uint64_t V18  = bar_mulmod_s(a[j + t + 18], Wori, R);
            //         uint64_t V19  = bar_mulmod_s(a[j + t + 19], Wori, R);
            //         uint64_t V20  = bar_mulmod_s(a[j + t + 20], Wori, R);
            //         uint64_t V21  = bar_mulmod_s(a[j + t + 21], Wori, R);
            //         uint64_t V22  = bar_mulmod_s(a[j + t + 22], Wori, R);
            //         uint64_t V23  = bar_mulmod_s(a[j + t + 23], Wori, R);
            //         uint64_t V24  = bar_mulmod_s(a[j + t + 24], Wori, R);
            //         uint64_t V25  = bar_mulmod_s(a[j + t + 25], Wori, R);
            //         uint64_t V26  = bar_mulmod_s(a[j + t + 26], Wori, R);
            //         uint64_t V27  = bar_mulmod_s(a[j + t + 27], Wori, R);
            //         uint64_t V28  = bar_mulmod_s(a[j + t + 28], Wori, R);
            //         uint64_t V29  = bar_mulmod_s(a[j + t + 29], Wori, R);
            //         uint64_t V30  = bar_mulmod_s(a[j + t + 30], Wori, R);
            //         uint64_t V31  = bar_mulmod_s(a[j + t + 31], Wori, R);
            //         a[j + t + 0] = submod(a[j + 0], V0);
            //         a[j + t + 1] = submod(a[j + 1], V1);
            //         a[j + t + 2] = submod(a[j + 2], V2);
            //         a[j + t + 3] = submod(a[j + 3], V3);
            //         a[j + t + 4] = submod(a[j + 4], V4);
            //         a[j + t + 5] = submod(a[j + 5], V5);
            //         a[j + t + 6] = submod(a[j + 6], V6);
            //         a[j + t + 7] = submod(a[j + 7], V7);
            //         a[j + t + 8] = submod(a[j + 8], V8);
            //         a[j + t + 9] = submod(a[j + 9], V9);
            //         a[j + t + 10] = submod(a[j + 10], V10);
            //         a[j + t + 11] = submod(a[j + 11], V11);
            //         a[j + t + 12] = submod(a[j + 12], V12);
            //         a[j + t + 13] = submod(a[j + 13], V13);
            //         a[j + t + 14] = submod(a[j + 14], V14);
            //         a[j + t + 15] = submod(a[j + 15], V15);
            //         a[j + t + 16] = submod(a[j + 16], V16);
            //         a[j + t + 17] = submod(a[j + 17], V17);
            //         a[j + t + 18] = submod(a[j + 18], V18);
            //         a[j + t + 19] = submod(a[j + 19], V19);
            //         a[j + t + 20] = submod(a[j + 20], V20);
            //         a[j + t + 21] = submod(a[j + 21], V21);
            //         a[j + t + 22] = submod(a[j + 22], V22);
            //         a[j + t + 23] = submod(a[j + 23], V23);
            //         a[j + t + 24] = submod(a[j + 24], V24);
            //         a[j + t + 25] = submod(a[j + 25], V25);
            //         a[j + t + 26] = submod(a[j + 26], V26);
            //         a[j + t + 27] = submod(a[j + 27], V27);
            //         a[j + t + 28] = submod(a[j + 28], V28);
            //         a[j + t + 29] = submod(a[j + 29], V29);
            //         a[j + t + 30] = submod(a[j + 30], V30);
            //         a[j + t + 31] = submod(a[j + 31], V31);
            //         a[j + 0] = addmod(a[j + 0], V0);
            //         a[j + 1] = addmod(a[j + 1], V1);
            //         a[j + 2] = addmod(a[j + 2], V2);
            //         a[j + 3] = addmod(a[j + 3], V3);
            //         a[j + 4] = addmod(a[j + 4], V4);
            //         a[j + 5] = addmod(a[j + 5], V5);
            //         a[j + 6] = addmod(a[j + 6], V6);
            //         a[j + 7] = addmod(a[j + 7], V7);
            //         a[j + 8] = addmod(a[j + 8], V8);
            //         a[j + 9] = addmod(a[j + 9], V9);
            //         a[j + 10] = addmod(a[j + 10], V10);
            //         a[j + 11] = addmod(a[j + 11], V11);
            //         a[j + 12] = addmod(a[j + 12], V12);
            //         a[j + 13] = addmod(a[j + 13], V13);
            //         a[j + 14] = addmod(a[j + 14], V14);
            //         a[j + 15] = addmod(a[j + 15], V15);
            //         a[j + 16] = addmod(a[j + 16], V16);
            //         a[j + 17] = addmod(a[j + 17], V17);
            //         a[j + 18] = addmod(a[j + 18], V18);
            //         a[j + 19] = addmod(a[j + 19], V19);
            //         a[j + 20] = addmod(a[j + 20], V20);
            //         a[j + 21] = addmod(a[j + 21], V21);
            //         a[j + 22] = addmod(a[j + 22], V22);
            //         a[j + 23] = addmod(a[j + 23], V23);
            //         a[j + 24] = addmod(a[j + 24], V24);
            //         a[j + 25] = addmod(a[j + 25], V25);
            //         a[j + 26] = addmod(a[j + 26], V26);
            //         a[j + 27] = addmod(a[j + 27], V27);
            //         a[j + 28] = addmod(a[j + 28], V28);
            //         a[j + 29] = addmod(a[j + 29], V29);
            //         a[j + 30] = addmod(a[j + 30], V30);
            //         a[j + 31] = addmod(a[j + 31], V31);
            //     }
            // } else 
            if (t % 16 == 0) { /// 16-butterfly pal
                for (long j = j1; j <= j2; j += 16) {
                    uint64_t V0   = bar_mulmod_s(a[j + t], Wori, R);
                    uint64_t V1   = bar_mulmod_s(a[j + t + 1], Wori, R);
                    uint64_t V2   = bar_mulmod_s(a[j + t + 2], Wori, R);
                    uint64_t V3   = bar_mulmod_s(a[j + t + 3], Wori, R);
                    uint64_t V4   = bar_mulmod_s(a[j + t + 4], Wori, R);
                    uint64_t V5   = bar_mulmod_s(a[j + t + 5], Wori, R);
                    uint64_t V6   = bar_mulmod_s(a[j + t + 6], Wori, R);
                    uint64_t V7   = bar_mulmod_s(a[j + t + 7], Wori, R);
                    uint64_t V8   = bar_mulmod_s(a[j + t + 8], Wori, R);
                    uint64_t V9   = bar_mulmod_s(a[j + t + 9], Wori, R);
                    uint64_t V10  = bar_mulmod_s(a[j + t + 10], Wori, R);
                    uint64_t V11  = bar_mulmod_s(a[j + t + 11], Wori, R);
                    uint64_t V12  = bar_mulmod_s(a[j + t + 12], Wori, R);
                    uint64_t V13  = bar_mulmod_s(a[j + t + 13], Wori, R);
                    uint64_t V14  = bar_mulmod_s(a[j + t + 14], Wori, R);
                    uint64_t V15  = bar_mulmod_s(a[j + t + 15], Wori, R);
                    a[j + t + 0] = submod(a[j + 0], V0);
                    a[j + t + 1] = submod(a[j + 1], V1);
                    a[j + t + 2] = submod(a[j + 2], V2);
                    a[j + t + 3] = submod(a[j + 3], V3);
                    a[j + t + 4] = submod(a[j + 4], V4);
                    a[j + t + 5] = submod(a[j + 5], V5);
                    a[j + t + 6] = submod(a[j + 6], V6);
                    a[j + t + 7] = submod(a[j + 7], V7);
                    a[j + t + 8] = submod(a[j + 8], V8);
                    a[j + t + 9] = submod(a[j + 9], V9);
                    a[j + t + 10] = submod(a[j + 10], V10);
                    a[j + t + 11] = submod(a[j + 11], V11);
                    a[j + t + 12] = submod(a[j + 12], V12);
                    a[j + t + 13] = submod(a[j + 13], V13);
                    a[j + t + 14] = submod(a[j + 14], V14);
                    a[j + t + 15] = submod(a[j + 15], V15);
                    a[j + 0] = addmod(a[j + 0], V0);
                    a[j + 1] = addmod(a[j + 1], V1);
                    a[j + 2] = addmod(a[j + 2], V2);
                    a[j + 3] = addmod(a[j + 3], V3);
                    a[j + 4] = addmod(a[j + 4], V4);
                    a[j + 5] = addmod(a[j + 5], V5);
                    a[j + 6] = addmod(a[j + 6], V6);
                    a[j + 7] = addmod(a[j + 7], V7);
                    a[j + 8] = addmod(a[j + 8], V8);
                    a[j + 9] = addmod(a[j + 9], V9);
                    a[j + 10] = addmod(a[j + 10], V10);
                    a[j + 11] = addmod(a[j + 11], V11);
                    a[j + 12] = addmod(a[j + 12], V12);
                    a[j + 13] = addmod(a[j + 13], V13);
                    a[j + 14] = addmod(a[j + 14], V14);
                    a[j + 15] = addmod(a[j + 15], V15);
                }
            } else 
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
            } else 
            if (t % 4 == 0) { /// 4-butterfly pal
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
            } else 
            if (t % 2 == 0) { /// 2-butterfly pal
                for (long j = j1; j <= j2; j += 2) {
                    uint64_t V0  = bar_mulmod_s(a[j + t], Wori, R);
                    uint64_t V1  = bar_mulmod_s(a[j + t + 1], Wori, R);
                    a[j + t + 0] = submod(a[j + 0], V0);
                    a[j + t + 1] = submod(a[j + 1], V1);
                    a[j + 0]     = addmod(a[j + 0], V0);
                    a[j + 1]     = addmod(a[j + 1], V1);
                }
            } else 
            { /// single-butterfly
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
