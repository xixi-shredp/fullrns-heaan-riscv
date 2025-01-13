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
            if (t % 2 == 0)
                for (long j = j1; j <= j2; j += 2) {
                    uint64_t V1  = bar_mulmod_s(a[j + t], W, R);
                    uint64_t V2  = bar_mulmod_s(a[j + t + 1], W, R);
                    a[j + t]     = submod(a[j], V1);
                    a[j]         = addmod(a[j], V1);
                    a[j + t + 1] = submod(a[j + 1], V2);
                    a[j + 1]     = addmod(a[j + 1], V2);
                }
            else
                for (long j = j1; j <= j2; j++) {
                    uint64_t T = a[j + t];
                    uint64_t V = bar_mulmod_s(T, W, R);
                    a[j + t]   = submod(a[j], V);
                    a[j]       = addmod(a[j], V);
                }
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
