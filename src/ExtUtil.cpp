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
    #ifdef CONFIG_FHE_EXT_SCALAR_SCHEDULE
            if (t % 2 == 0) {
                for (long j = j1; j <= j2; j += 2) {
                    uint64_t V1 =
                        bar_mulmod_s(a[j + t], Wori, nttBarPres[index][m + i]);
                    uint64_t V2  = bar_mulmod_s(a[j + t + 1], Wori,
                                                nttBarPres[index][m + i]);
                    a[j + t]     = submod(a[j], V1);
                    a[j + t + 1] = submod(a[j + 1], V2);
                    a[j]         = addmod(a[j], V1);
                    a[j + 1]     = addmod(a[j + 1], V2);
                }
            } else {
                for (long j = j1; j <= j2; j++) {
                    uint64_t T = a[j + t];
                    uint64_t V =
                        bar_mulmod_s(T, Wori, nttBarPres[index][m + i]);
                    a[j + t] = submod(a[j], V);
                    a[j]     = addmod(a[j], V);
                }
            }
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
            if (t % 2 == 0) {
                for (long j = j1; j <= j2; j += 2) {
                    uint64_t V1  = mont_redc(a[j + t], W);
                    uint64_t V2  = mont_redc(a[j + t + 1], W);
                    a[j + t]     = submod(a[j], V1);
                    a[j + t + 1] = submod(a[j + 1], V2);
                    a[j]         = addmod(a[j], V1);
                    a[j + 1]     = addmod(a[j + 1], V2);
                }
            } else {
                for (long j = j1; j <= j2; j++) {
                    uint64_t T = a[j + t];
                    uint64_t V = mont_redc(T, W);
                    a[j + t]   = submod(a[j], V);
                    a[j]       = addmod(a[j], V);
                }
            }
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
