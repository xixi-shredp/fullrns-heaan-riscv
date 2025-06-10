#include "Context.h"

#if defined(CONFIG_RVV) && (CONFIG_FHE_EXT)

  #include "ExtUtil.h"
  #include "RVVUtil.h"
  #include "Step4NTTUtil.h"
  #include "multi_threading_acc.h"

static inline void
ext_mth_rowNTTWithBar(uint64_t m, uint64_t *a, long t, long logt1, uint64_t q,
                      uint64_t *qRootPows, uint64_t *barPres)
{
  for (long i = 0; i < m; i++) {
    long j1 = i << logt1;
    long j2 = j1 + t - 1;
    uint64_t W = qRootPows[m + i];
    uint64_t R = barPres[m + i];
    for (long j = j1; j <= j2; j++) {
      uint64_t T = a[j + t];
      uint64_t V = bar_mulmod_s(T, W, R);
      a[j + t] = submod(a[j], V);
      a[j] = addmod(a[j], V);
    }
  }
}

/**
 * Fit for parallelizing the lay when t >= 8
 * */
static inline void
rvv_ext_mth_op1_rowNTTWithBar(uint64_t m, uint64_t *a, long t, long logt1,
                              uint64_t q, uint64_t *qRootPows,
                              uint64_t *barPres)
{
  long res_vl = vsetvli(t, 64, 4);
  for (long i = 0; i < m; i++) {
    long j1 = i << logt1;
    // long j2 = j1 + t - 1;
    uint64_t W = qRootPows[m + i];
    uint64_t R = barPres[m + i];
    long op_len = t;
    // printf("vl set %ld , get %ld\n", op_len, res_vl);
    uint64_t *va = a + j1;
  #ifdef CONFIG_FHE_EXT_VECTOR_SCHEDULE
    long len = res_vl * 2;
    while (op_len >= len) {
      /// load
      vle_v(v4, va + t); // T = a[j + t];
      vle_v(v8, va + t + res_vl);
      vbarmodmuls_vxx(v4, W, R);
      vbarmodmuls_vxx(v8, W, R);

      vle_v(v12, va); // a[j];
      vle_v(v16, va + res_vl);

      vsubmod_vv(v20, v12, v4);
      vaddmod_vv(v4, v12, v4);
      vse_v(v20, va + t); // a[j + t];
      vse_v(v4, va);      // a[j];

      vsubmod_vv(v24, v16, v8);
      vaddmod_vv(v8, v16, v8);
      vse_v(v24, va + t + res_vl); // a[j + t];
      vse_v(v8, va + res_vl);      // a[j];

      va += len;
      op_len -= len;
    }
    if (op_len == res_vl) {
      /// load
      vle_v(v4, va + t); // T = a[j + t];
      vbarmodmuls_vxx(v4, W, R);
      vle_v(v16, va); // a[j];
      vsubmod_vv(v8, v16, v4);
      vaddmod_vv(v16, v16, v4);
      /// store
      vse_v(v8, va + t); // a[j + t];
      vse_v(v16, va);    // a[j];

      va += res_vl;
      op_len -= res_vl;
    }
  #else
    while (op_len > 0) {
      /// load
      vle_v(v4, va + t); // T = a[j + t];
      vbarmodmuls_vxx(v4, W, R);
      vle_v(v16, va); // a[j];
      vsubmod_vv(v8, v16, v4);
      vaddmod_vv(v16, v16, v4);
      /// store
      vse_v(v8, va + t); // a[j + t];
      vse_v(v16, va);    // a[j];

      va += res_vl;
      op_len -= res_vl;
    }
  #endif
  }
}

/**
 * Fit for parallelizing the lay when m >= 8
 * */
static inline void
rvv_ext_mth_op2_rowNTTWithBar(uint64_t m, uint64_t *a, long t, long logt1,
                              uint64_t q, uint64_t *qRootPows,
                              uint64_t *barPres)
{
  long stride = t * 2 * sizeof(uint64_t);

  long op_len = m;
  long res_vl = vsetvli(op_len, 64, 4);

  uint64_t *oa = a;
  long oa_stride = res_vl * t * 2;

  long b_stride = m;
  while (op_len > 0) {
    // printf("op_len:%3ld res_vl:%3ld t:%3ld\n", op_len, res_vl, t);
    vle_v(v4, qRootPows + b_stride); // v4 = W[m + i];
    vle_v(v8, barPres + b_stride);   // v8 = R[m + i];

    uint64_t *va = oa;
    for (int _ = 0; _ < t; _++) {
      vlse_v(v12, va + t, stride); // T = a[j + t];
      vlse_v(v24, va, stride);     // a[j];

      vbarmodmuls_vv(v12, v4, v8);
      vsubmod_vv(v16, v24, v12);
      vaddmod_vv(v24, v24, v12);
      /// store
      vsse_v(v16, va + t, stride); // a[j + t];
      vsse_v(v24, va, stride);     // a[j];

      va++;
    }

    op_len -= res_vl;
    oa += oa_stride;
    b_stride += res_vl;
  }
}
static inline void
ext_mth_rowNTTWithMont(uint64_t m, uint64_t *a, long t, long logt1, uint64_t q,
                       uint64_t qInv, uint64_t *qRootScalePows)
{
  for (long i = 0; i < m; i++) {
    long j1 = i << logt1;
    long j2 = j1 + t - 1;
    uint64_t W = qRootScalePows[m + i];
    for (long j = j1; j <= j2; j++) {
      uint64_t T = a[j + t];
      uint64_t V = mont_redc(T, W);
      a[j + t] = submod(a[j], V);
      a[j] = addmod(a[j], V);
    }
  }
}
/**
 * Fit for parallelizing the lay when t >= 8
 * */
static inline void
rvv_ext_mth_op1_rowNTTWithMont(uint64_t m, uint64_t *a, long t, long logt1,
                               uint64_t q, uint64_t qInv,
                               uint64_t *qRootScalePows)
{
  for (long i = 0; i < m; i++) {
    long j1 = i << logt1;
    uint64_t W = qRootScalePows[m + i];
    long op_len = t;
    long res_vl = vsetvli(op_len, 64, 4);
    // printf("vl set %ld , get %ld\n", op_len, res_vl);
    uint64_t *va = a + j1;
  #ifdef CONFIG_FHE_EXT_VECTOR_SCHEDULE
    long len = res_vl * 2;
    while (op_len >= len) {
      /// load
      vle_v(v4, va + t); // T = a[j + t];
      vle_v(v8, va + t + res_vl);
      vmontredc_vx(v4, v4, W);
      vmontredc_vx(v8, v8, W);

      vle_v(v12, va); // a[j];
      vle_v(v16, va + res_vl);

      vsubmod_vv(v20, v12, v4);
      vaddmod_vv(v4, v12, v4);
      vse_v(v20, va + t); // a[j + t];
      vse_v(v4, va);      // a[j];

      vsubmod_vv(v24, v16, v8);
      vaddmod_vv(v8, v16, v8);
      vse_v(v24, va + t + res_vl); // a[j + t];
      vse_v(v8, va + res_vl);      // a[j];

      va += len;
      op_len -= len;
    }
    if (op_len == res_vl) {
      /// load
      vle_v(v4, va + t); // T = a[j + t];
      vle_v(v16, va);    // a[j];

      vmontredc_vx(v12, v4, W);
      vsubmod_vv(v4, v16, v12);
      vse_v(v4, va + t); // a[j + t];
      vaddmod_vv(v16, v16, v12);
      vse_v(v16, va); // a[j];

      va += res_vl;
      op_len -= res_vl;
    }
  #else
    while (op_len > 0) {
      /// load
      vle_v(v4, va + t); // T = a[j + t];
      vle_v(v16, va);    // a[j];

      vmontredc_vx(v12, v4, W);
      vsubmod_vv(v4, v16, v12);
      vse_v(v4, va + t); // a[j + t];
      vaddmod_vv(v16, v16, v12);
      vse_v(v16, va); // a[j];

      va += res_vl;
      op_len -= res_vl;
    }
  #endif
  }
}

/**
 * Fit for parallelizing the lay when m >= 8
 * */
static inline void
rvv_ext_mth_op2_rowNTTWithMont(uint64_t m, uint64_t *a, long t, long logt1,
                               uint64_t q, uint64_t qInv,
                               uint64_t *qRootScalePows)
{
  long stride = t * 2 * sizeof(uint64_t);

  long op_len = m;
  long res_vl = vsetvli(op_len, 64, 4);

  uint64_t *oa = a;
  long oa_stride = res_vl * t * 2;

  long b_stride = m;
  while (op_len > 0) {
    // printf("op_len:%3ld res_vl:%3ld t:%3ld\n", op_len, res_vl, t);
    vle_v(v4, qRootScalePows + b_stride); // v4 = W[m + i];

    uint64_t *va = oa;
    for (int _ = 0; _ < t; _++) {
      vlse_v(v12, va + t, stride); // T = a[j + t];
      vlse_v(v24, va, stride);     // a[j];

      vmontredc_vv(v16, v12, v4);
      vsubmod_vv(v12, v24, v16);
      vsse_v(v12, va + t, stride); // a[j + t];
      vaddmod_vv(v24, v24, v16);
      vsse_v(v24, va, stride); // a[j];

      va++;
    }

    op_len -= res_vl;
    oa += oa_stride;
    b_stride += res_vl;
  }
}

  #define rvv_ext_bar_diff() \
    rvv_diff(ext_mth_rowNTTWithBar(m, bx, t, logt1, q, qRootPows, barPres))
  #define rvv_ext_mont_diff() \
    rvv_diff(ext_mth_rowNTTWithMont(m, bx, t, logt1, q, qInv, qRootScalePows))
static inline void
rvv_ext_rowNTT_withBar(uint64_t *a, long N, long logN, uint64_t q,
                       uint64_t qInv, uint64_t *qRootPows,
                       uint64_t *qRootScalePows, uint64_t *barPres)
{
  long t = N;
  long logt1 = logN + 1;

  for (long m = 1; m < N; m <<= 1) {
    t >>= 1;
    logt1 -= 1;
    // rvv_diff_init();
    //
    // if (t >= 8) {
    //     rvv_ext_mth_op1_rowNTTWithBar(m, a, t, logt1, q, qRootPows,
    //                                   barPres);
    // } else {
    //     rvv_ext_mth_op2_rowNTTWithBar(m, a, t, logt1, q, qRootPows,
    //                                   barPres);
    // }
    // rvv_ext_bar_diff();
    rvv_ext_mth_op1_rowNTTWithBar(m, a, t, logt1, q, qRootPows, barPres);
  }
}
static inline void
rvv_ext_rowNTT_withMont(uint64_t *a, long N, long logN, uint64_t q,
                        uint64_t qInv, uint64_t *qRootPows,
                        uint64_t *qRootScalePows, uint64_t *barPres)
{
  long t = N;
  long logt1 = logN + 1;

  for (long m = 1; m < N; m <<= 1) {
    t >>= 1;
    logt1 -= 1;
    // rvv_diff_init();
    //
    // if (t >= 8) {
    //     rvv_ext_mth_op1_rowNTTWithMont(m, a, t, logt1, q, qInv,
    //                                    qRootScalePows);
    //     // ext_mth_rowNTTWithMont(m, a, t, logt1, q, qInv,
    //     qRootScalePows);
    // } else {
    //     rvv_ext_mth_op2_rowNTTWithMont(m, a, t, logt1, q, qInv,
    //                                    qRootScalePows);
    //     // ext_mth_rowNTTWithMont(m, a, t, logt1, q, qInv,
    //     qRootScalePows);
    // }
    // rvv_ext_mont_diff();
    rvv_ext_mth_op1_rowNTTWithMont(m, a, t, logt1, q, qInv, qRootScalePows);
  }
}

static inline void
rvv_rowMul_withBar(long rowIdx, long rowSz, long logRowSz, uint64_t *src,
                   uint64_t *wmatrix_scalepows, uint64_t *wmatrix_pows,
                   uint64_t *barPres, long q, long qInv)
{
  uint64_t idx = (rowIdx << logRowSz);
  long op_len = rowSz;
  long res_vl = vsetvli(op_len, 64, 4);
  uint64_t *va = src + idx;
  uint64_t *vw = wmatrix_scalepows + idx;
  uint64_t *vwb = wmatrix_pows + idx;
  uint64_t *vb = barPres + idx;
  while (op_len > 0) {
    vle_v(v4, va);  // T
    vle_v(v8, vwb); // W
    vle_v(v12, vb); // R
    vbarmodmuls_vv(v4, v8, v12);
    vse_v(v4, va); // a[idx]

    vwb += res_vl;
    vb += res_vl;

    va += res_vl;
    op_len -= res_vl;
  }
}

static inline void
rvv_rowMul_withMont(long rowIdx, long rowSz, long logRowSz, uint64_t *src,
                    uint64_t *wmatrix_scalepows, uint64_t *wmatrix_pows,
                    uint64_t *barPres, long q, long qInv)
{
  uint64_t idx = (rowIdx << logRowSz);
  long op_len = rowSz;
  long res_vl = vsetvli(op_len, 64, 4);
  uint64_t *va = src + idx;
  uint64_t *vw = wmatrix_scalepows + idx;
  uint64_t *vwb = wmatrix_pows + idx;
  uint64_t *vb = barPres + idx;
  while (op_len > 0) {
    vle_v(v4, va); // T
    vle_v(v8, vw); // W

    // montgomeny
    vmontredc_vv(v4, v4, v8);

    vse_v(v4, va); // a[idx]
    vw += res_vl;

    va += res_vl;
    op_len -= res_vl;
  }
}

  #define RVV_EXT_ROW_NTT_Barrett(a, s, N)                               \
    {                                                                    \
      uint64_t *a1 = a + (i << log##N);                                  \
      rvv_ext_rowNTT_withBar(                                            \
          a1, N, log##N, q, qInv, s4ntt_##s##_qRootPows[index],          \
          s4ntt_##s##_qRootScalePows[index], s4ntt_##s##BarPres[index]); \
    }
  #define RVV_ROW_MUL_Barrett                                               \
    rvv_rowMul_withBar(i, N2, logN2, a, wmatrix_scalepows[index],           \
                       wmatrix_pows[index], s4ntt_wmatrixBarPres[index], q, \
                       qInv);

  #define RVV_EXT_ROW_NTT_Montgomeny(a, s, N)                              \
    {                                                                      \
      uint64_t *a1 = a + (i << log##N);                                    \
      rvv_ext_rowNTT_withMont(a1, N, log##N, q, qInv,                      \
                              s4ntt_##s##_qRootPows[index],                \
                              s4ntt_##s##_qRootScalePows[index], nullptr); \
    }
  #define RVV_ROW_MUL_Montgomeny                                            \
    rvv_rowMul_withMont(i, N2, logN2, a, wmatrix_scalepows[index], nullptr, \
                        nullptr, q, qInv);

STEP4NTT_TEMPLATE_IMPLEMENT(rvv_ext_step4_qiNTTAndEqual_withBar,
                            RVV_ROW_MUL_Barrett, RVV_EXT_ROW_NTT_Barrett,
                            SET_MOD);
STEP4NTT_TEMPLATE_IMPLEMENT(rvv_ext_step4_qiNTTAndEqual_withMont,
                            RVV_ROW_MUL_Montgomeny, RVV_EXT_ROW_NTT_Montgomeny,
                            SET_MOD);

void
Context::rvv_ext_ori_qiNTTAndEqual_withBar(uint64_t *a, long index)
{
  uint64_t q = qVec[index];
  uint64_t qInv = qInvVec[index];
  SET_MOD;
  rvv_ext_rowNTT_withBar(a, N, logN, q, qInv, qRootPows[index], nullptr,
                         nttBarPres[index]);
}
void
Context::rvv_ext_ori_qiNTTAndEqual_withMont(uint64_t *a, long index)
{
  uint64_t q = qVec[index];
  uint64_t qInv = qInvVec[index];
  SET_MOD;
  rvv_ext_rowNTT_withMont(a, N, logN, q, qInv, nullptr, qRootScalePows[index],
                          nullptr);
}

static mt_rvv_step_args_t global_args;

void
setmod_work_wrap(void *args)
{
  mt_rvv_step_args_t *p = &global_args;
  set_mod(p->q, p->qInv);
}

  #define SETMOD_FORTHREAD                                        \
    SET_MOD;                                                      \
    thread_pool_add_task(thread_pool, setmod_work_wrap, nullptr); \
    thread_pool_add_task(thread_pool, setmod_work_wrap, nullptr); \
    thread_pool_add_task(thread_pool, setmod_work_wrap, nullptr)

// ====== Multi-Threading Accelaration For rvv_ext_step4_bar =======

Step4NTTCol_FUNC_Def(rvv_ext_step4_bar, global_args, rvv_ext_rowNTT_withBar);
Step4NTTRowMulNTT_FUNC_Def(rvv_ext_step4_bar, global_args, rvv_rowMul_withBar,
                           rvv_ext_rowNTT_withBar);
MT_STEP4NTT_TEMPLATE_IMPLEMENT(mt_rvv_ext_step4_qiNTTAndEqual_withBar,
                               rvv_ext_step4_bar, SETMOD_FORTHREAD,
                               global_args);

// ====== Multi-Threading Accelaration For rvv_ext_step4_mont =======
Step4NTTCol_FUNC_Def(rvv_ext_step4_mont, global_args, rvv_ext_rowNTT_withMont);
Step4NTTRowMulNTT_FUNC_Def(rvv_ext_step4_mont, global_args,
                           rvv_rowMul_withMont, rvv_ext_rowNTT_withMont);
MT_STEP4NTT_TEMPLATE_IMPLEMENT(mt_rvv_ext_step4_qiNTTAndEqual_withMont,
                               rvv_ext_step4_mont, SETMOD_FORTHREAD,
                               global_args);

#endif // CONFIG_RVV
