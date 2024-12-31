#include "OpRecorder.h"
#include "Context.h"
#include "Step4NTTUtil.h"

// #define ENBALE_OPRECORD
#ifdef ENBALE_OPRECORD
static OpRecorder opr = OpRecorder(CONFIG_OP_RECORDER_PATH);
    #define addmod_record \
        opr.rd_addmod(a[j], V, q, ((a[j] + V > q) ? a[j] + V - q : a[j] + V));
    #define submod_record   opr.rd_submod(a[j], V, q, a[j + t]);
    #define barmul_record   opr.rd_barmulmod(T, W, q, R, V);
    #define montredc_record opr.rd_montredc(T, W, q, qInv, V);
#else
    #define addmod_record
    #define submod_record
    #define barmul_record
    #define montredc_record
#endif

static inline void
rowNTTWithBar(uint64_t *a, long N, long logN, uint64_t q, uint64_t *qRootPows,
              uint64_t *barPres)
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
            for (long j = j1; j <= j2; j++) {
                uint64_t T = a[j + t];
                uint64_t V = barrett_modMul_singalVal(T, W, q, R);
                barmul_record;
                a[j + t] = a[j] < V ? a[j] + q - V : a[j] - V;
                submod_record;
                addmod_record;
                a[j] += V;
                if (a[j] > q) a[j] -= q;
            }
        }
    }
}

static inline void
rowNTTWithMont(uint64_t *a, long N, long logN, uint64_t q, uint64_t qInv,
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
            for (long j = j1; j <= j2; j++) {
                uint64_t T = a[j + t];
                uint64_t V = montgomey_redc(T, W, q, qInv);
                montredc_record;
                a[j + t] = a[j] < V ? a[j] + q - V : a[j] - V;
                submod_record;
                addmod_record;
                a[j] += V;
                if (a[j] > q) a[j] -= q;
            }
        }
    }
}

static inline void
row_mul_withBar(long rowIdx, long rowSz, long logRowSz, uint64_t *src,
                uint64_t *wmatrix_pows, uint64_t *barPres, long q, long qInv)
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
row_mul_withMont(long rowIdx, long rowSz, long logRowSz, uint64_t *src,
                 uint64_t *wmatrix_scalepows, long q, long qInv)
{
    for (int j = 0; j < rowSz; ++j) {
        uint64_t idx = (rowIdx << logRowSz) + j;
        uint64_t T   = src[idx];
        uint64_t W   = wmatrix_scalepows[idx];
        src[idx]     = montgomey_redc(T, W, q, qInv);
    }
}

#define ROW_MUL_Barrett                                   \
    row_mul_withBar(i, N2, logN2, a, wmatrix_pows[index], \
                    s4ntt_wmatrixBarPres[index], q, qInv);

#define ROW_MUL_Montgomeny \
    row_mul_withMont(i, N2, logN2, a, wmatrix_scalepows[index], q, qInv);

#define ROW_NTT_Barrett(a, s, N)                                        \
    {                                                                   \
        uint64_t *__a1 = a + (i << log##N);                             \
        rowNTTWithBar(__a1, N, log##N, q, s4ntt_##s##_qRootPows[index], \
                      s4ntt_##s##BarPres[index]);                       \
    }

#define ROW_NTT_Montgomeny(a, s, N)                        \
    {                                                      \
        uint64_t *__a1 = a + (i << log##N);                \
        rowNTTWithMont(__a1, N, log##N, q, qInv,           \
                       s4ntt_##s##_qRootScalePows[index]); \
    }

STEP4NTT_TEMPLATE_IMPLEMENT(step4_qiNTTAndEqual_withBar, ROW_MUL_Barrett,
                            ROW_NTT_Barrett, NO_SET_MOD);

STEP4NTT_TEMPLATE_IMPLEMENT(step4_qiNTTAndEqual_withMont, ROW_MUL_Montgomeny,
                            ROW_NTT_Montgomeny, NO_SET_MOD);
