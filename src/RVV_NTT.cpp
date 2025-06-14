#include "Context.h"
#include "ThreadPool.h"
#include <cstddef>
#include <cstdint>

#ifdef CONFIG_RVV
  #include "RVVUtil.h"
  #include "Step4NTTUtil.h"
  #include "multi_threading_acc.h"

static inline void
mth_rowNTTWithBar(uint64_t m, uint64_t *a, long t, long logt1, uint64_t q,
                  uint64_t *qRootPows, uint64_t *barPres)
{
  for (long i = 0; i < m; i++) {
    long j1 = i << logt1;
    long j2 = j1 + t - 1;
    uint64_t W = qRootPows[m + i];
    uint64_t R = barPres[m + i];
    for (long j = j1; j <= j2; j++) {
      uint64_t T = a[j + t];
      uint64_t V = barrett_modMul_singalVal(T, W, q, R);
      a[j + t] = a[j] < V ? a[j] + q - V : a[j] - V;
      a[j] += V;
      if (a[j] > q) a[j] -= q;
    }
  }
}
/**
 * Fit for parallelizing the lay when t >= 8
 * */
static inline void
rvv_mth_op1_rowNTTWithBar(uint64_t m, uint64_t *a, long t, long logt1,
                          uint64_t q, uint64_t *qRootPows, uint64_t *barPres)
{
  // vlen_check();

  long res_vl = vsetvli(t, 64, 4);
  // printf("vl set %ld , get %ld\n", t, res_vl);
  for (long i = 0; i < m; i++) {
    long op_len = t;
    long j1 = i << logt1;
    // long j2 = j1 + t - 1;
    uint64_t W = qRootPows[m + i];
    uint64_t R = barPres[m + i];

    uint64_t *va = a + j1;
    while (op_len > 0) {
      /// load
      vle_v(v4, va + t); // T = a[j + t];
      vle_v(v16, va);    // a[j];
      /// Bar Mod Mul Single Var
      vmul_vx(v8, v4, W);    // z1 = T * W;
      vmulhu_vx(v12, v4, R); // t = ((__uint128_t)T * R) >> 64;
      vmul_vx(v12, v12, q);  // t1 = q * t;
      vsub_vv(v12, v8, v12); // z = z1 - t1;
      /// mask reg
      // vmsgtu_vx(v0, v12, q);   // v0 = z > q;
      // vsub_vx_vm(v12, v12, q); // V = v0 ? z - q : z;
      /// none-mask reg
      vsub_vx(v8, v12, q);    // v0 = z - q;
      vminu_vv(v12, v12, v8); // V = v0 ? z - q : z;

      /// submod
      /// mask reg
      // vmsltu_vv(v0, v16, v12); // v0 = a[j] < V;
      // vsub_vv(v4, v16, v12);   // a[j] - V;
      // vadd_vx_vm(v4, v4, q); // a[j + t]  = v0 ? a[j] - V + q : a[j] -
      // V;
      /// none-mask reg
      vsub_vv(v4, v16, v12); // a[j] - V;
      vadd_vx(v8, v4, q);    // a[j] - V + q;
      vminu_vv(v4, v4, v8);  // a[j + t]  = v0 ? a[j] - V + q : a[j] - V;

      /// addmod
      /// mask reg
      // vadd_vv(v16, v16, v12);  // a[j] += V;
      // vmsgtu_vx(v0, v16, q);   // v0 = a[j] > q;
      // vsub_vx_vm(v16, v16, q); // V = v0 ? a[j] - q : a[j];
      /// none-mask reg
      vadd_vv(v16, v16, v12); // a[j] + V;
      vsub_vx(v8, v16, q);    // a[j] + V - q;
      vminu_vv(v16, v16, v8); // V = v0 ? a[j] - q : a[j];

      /// store
      vse_v(v4, va + t); // a[j + t];
      vse_v(v16, va);    // a[j];

      va += res_vl;
      op_len -= res_vl;
    }
  }
}

/**
 * Fit for parallelizing the lay when m >= 8
 * */
static inline void
rvv_mth_op2_rowNTTWithBar(uint64_t m, uint64_t *a, long t, long logt1,
                          uint64_t q, uint64_t *qRootPows, uint64_t *barPres)
{
  long op_len = m;
  long res_vl = vsetvli(op_len, 64, 4);

  long stride = t * 2 * sizeof(uint64_t);

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

      /// Bar Mod Mul Single Var
      vmul_vv(v16, v12, v4);   // z1 = T * W;
      vmulhu_vv(v20, v12, v8); // t0 = T * R;
      vmul_vx(v20, v20, q);    // t1 = t0 * q;
      vsub_vv(v16, v16, v20);  // z = z1 - t1;
      // mask reg
      // vmsgtu_vx(v0, v16, q);   // v0 = z > q;
      // vsub_vx_vm(v16, v16, q); // V = v0 ? z - q : z;
      // none-mask reg
      vsub_vx(v12, v16, q);    // z - q;
      vminu_vv(v16, v16, v12); // V = v0 ? z - q : z;

      /// submod
      // mask reg
      // vmsltu_vv(v0, v24, v16); // v0 = a[j] < V;
      // vsub_vv(v12, v24, v16);  // a[j] - V;
      // vadd_vx_vm(v12, v12,
      //            q); // a[j + t]  = v0 ? a[j] - V + q : a[j] - V;
      // none-mask reg
      vsub_vv(v12, v24, v16); // a[j] - V;
      vadd_vx(v20, v12, q);   // a[j] - V + q;
      vminu_vv(v12, v12,
               v20); // a[j + t]  = v0 ? a[j] - V + q : a[j] - V;

      /// addmod
      // mask reg
      // vadd_vv(v24, v24, v16);  // a[j] += V;
      // vmsgtu_vx(v0, v24, q);   // v0 = a[j] > q;
      // vsub_vx_vm(v24, v24, q); // V = v0 ? a[j] - q : a[j];
      // none-mask reg
      vadd_vv(v24, v24, v16);  // a[j] + V;
      vsub_vx(v20, v24, q);    // a[j] + V - q;
      vminu_vv(v24, v24, v20); // V = v0 ? a[j] - q : a[j];

      /// store
      vsse_v(v12, va + t, stride); // a[j + t];
      vsse_v(v24, va, stride);     // a[j];

      va++;
    }

    op_len -= res_vl;
    oa += oa_stride;
    b_stride += res_vl;
  }
}

static inline void
mth_rowNTTWithMont(uint64_t m, uint64_t *a, long t, long logt1, uint64_t q,
                   uint64_t qInv, uint64_t *qRootScalePows)
{
  for (long i = 0; i < m; i++) {
    long j1 = i << logt1;
    long j2 = j1 + t - 1;
    uint64_t W = qRootScalePows[m + i];
    for (long j = j1; j <= j2; j++) {
      uint64_t T = a[j + t];
      uint64_t V = montgomey_redc(T, W, q, qInv);
      // unsigned __int128 U  = static_cast<unsigned __int128>(T) * W;
      // uint64_t U0          = static_cast<uint64_t>(U);
      // uint64_t U1          = static_cast<uint64_t>(U >> 64);
      // uint64_t Q           = U0 * qInv;
      // unsigned __int128 Hx = static_cast<unsigned __int128>(Q) * q;
      // uint64_t H           = static_cast<uint64_t>(Hx >> 64);
      // uint64_t V           = U1 < H ? U1 + q - H : U1 - H;
      a[j + t] = a[j] < V ? a[j] + q - V : a[j] - V;
      a[j] += V;
      if (a[j] > q) a[j] -= q;
    }
  }
}
/**
 * Fit for parallelizing the lay when t >= 8
 * */
static inline void
rvv_mth_op1_rowNTTWithMont(uint64_t m, uint64_t *a, long t, long logt1,
                           uint64_t q, uint64_t qInv, uint64_t *qRootScalePows)
{
  long res_vl = vsetvli(t, 64, 4);
  // printf("vl set %ld , get %ld\n", t, res_vl);
  // vmv_v_x(v20, q);
  for (long i = 0; i < m; i++) {
    long j1 = i << logt1;
    uint64_t W = qRootScalePows[m + i];
    long op_len = t;
    uint64_t *va = a + j1;
    while (op_len > 0) {
      /// load
      vle_v(v4, va + t); // T = a[j + t];
      vle_v(v16, va);    // a[j];

      /// 1. Mont Redc
      vmul_vx(v8, v4, W);    // U0 = low_word(T * W);
      vmulhu_vx(v12, v4, W); // U1 = high_word(T * W);
      vmul_vx(v8, v8, qInv); // Q = U0 * qInv;
      vmulhu_vx(v8, v8, q);  // H = high_word(Q * q);
      // ! mask register usage ! (error in gem5)
      // vmsltu_vv(v0, v12, v8);  // v0 = U1 < H;
      // vsub_vv(v12, v12, v8); // U1 - H;
      // vadd_vx_vm(v12, v12, q); // V = v0 ? U1 - H + q : U1 - H;
      // ! none-mask register usage !
      vsub_vv(v12, v12, v8);   // U1 - H;
      vadd_vx(v24, v12, q);    // U1 - H + q;
      vminu_vv(v12, v24, v12); // V = U1 < H ? U1 + q - H : U1 - H;

      /// 2. submod
      // ! mask register usage ! (error in gem5)
      // vmsltu_vv(v0, v16, v12); // v0 = a[j] < V;
      // vsub_vv(v4, v16, v12);   // a[j] - V;
      // vadd_vx_vm(v4, v4, q); // a[j + t]  = v0 ? a[j] - V + q : a[j] -
      // V;
      // ! none-mask register usage !
      vsub_vv(v4, v16, v12); // a[j] - V;
      vadd_vx(v8, v4, q);    // a[j] - V + q;
      vminu_vv(v4, v4, v8);  // a[j + t]  = v0 ? a[j] - V + q : a[j] - V;
      vse_v(v4, va + t);     // a[j + t];

      /// 3. addmod
      // ! mask register usage ! (error in gem5)
      // vadd_vv(v16, v16, v12);  // a[j] += V;
      // vmsgtu_vx(v0, v16, q);   // v0 = a[j] > q;
      // vsub_vx_vm(v16, v16, q); // V = v0 ? a[j] - q : a[j];
      // ! none-mask register usage !
      vadd_vv(v16, v16, v12); // a[j] + V;
      vsub_vx(v12, v16, q);   // a[j] + V - q;
      vminu_vv(v16, v16,
               v12); // a[j] = a[j] + V > q ? a[j] + V - q : a[j] + V;

      vse_v(v16, va); // a[j];

      va += res_vl;
      op_len -= res_vl;
    }
  }
}

/**
 * Fit for parallelizing the lay when m >= 8
 * */
static inline void
rvv_mth_op2_rowNTTWithMont(uint64_t m, uint64_t *a, long t, long logt1,
                           uint64_t q, uint64_t qInv, uint64_t *qRootScalePows)
{
  long stride = t * 2 * sizeof(uint64_t);

  long op_len = m;
  long res_vl = vsetvli(op_len, 64, 4);
  // printf("vl set %ld , get %ld\n", m, res_vl);

  uint64_t *oa = a;
  long oa_stride = res_vl * t * 2;

  long b_stride = m;
  while (op_len > 0) {
    vle_v(v4, qRootScalePows + b_stride); // v4 = W[m + i];

    uint64_t *va = oa;
    for (int i = 0; i < t; i++) {
      vlse_v(v12, va + t, stride); // T = a[j + t];
      vlse_v(v24, va, stride);     // a[j];

      /// Mont Redc
      vmul_vv(v8, v12, v4);    // U0 = low_word(T * W);
      vmulhu_vv(v16, v12, v4); // U1 = high_word(T * W);
      vmul_vx(v12, v8, qInv);  // Q = U0 * qInv;
      vmulhu_vx(v20, v12, q);  // H = high_word(Q * q);
      // mask register
      vmsltu_vv(v0, v16, v20); // v0 = U1 < H;
      vsub_vv(v16, v16, v20);  // U1 - H;
      vadd_vx_vm(v16, v16, q); // V = v0 ? U1 - H + q : U1 - H;
      // none-mask register
      // vsub_vv(v8, v16, v20);   // U1 - H;
      // vadd_vx(v16, v8, q);    // U1 - H + q;
      // vminu_vv(v12, v8, v16); // V = v0 ? U1 - H + q : U1 - H;

      /// submod
      // mask register
      vmsltu_vv(v0, v24, v16); // v0 = a[j] < V;
      vsub_vv(v12, v24, v16);  // a[j] - V;
      vadd_vx_vm(v12, v12,
                 q); // a[j + t]  = v0 ? a[j] - V + q : a[j] - V;
      // none-mask register
      // vsub_vv(v12, v24, v16); // a[j] - V;
      // vadd_vx(v8, v12, q);    // a[j] - V + q;
      // vminu_vv(v12, v12,
      //          v8); // a[j + t] = v0 ? a[j] - V + q : a[j] - V;
      vsse_v(v12, va + t, stride); // a[j + t];

      /// addmod
      // mask register
      vadd_vv(v24, v24, v16);  // a[j] += V;
      vmsgtu_vx(v0, v24, q);   // v0 = a[j] > q;
      vsub_vx_vm(v24, v24, q); // V = v0 ? a[j] - q : a[j];
      // none-mask register
      // vadd_vv(v12, v24, v16);  // a[j] + V;
      // vsub_vx(v8, v12, q);     // a[j] + V - q
      // vminu_vv(v24, v12, v8);  // a[j] = v0 ? a[j] + V - q : a[j] + V;
      vsse_v(v24, va, stride); // a[j];
      va++;
    }

    op_len -= res_vl;
    oa += oa_stride;
    b_stride += res_vl;
  }
}

static inline void
rvv_rowNTTWithBar(uint64_t *a, long N, long logN, uint64_t q, uint64_t qInv,
                  uint64_t *qRootPows, uint64_t *qRootScalePows,
                  uint64_t *barPres)
{
  long t = N;
  long logt1 = logN + 1;

  for (long m = 1; m < N; m <<= 1) {
    t >>= 1;
    logt1 -= 1;
    // rvv_diff_init();
    //
    // if (t >= 8) {
    //     rvv_mth_op1_rowNTTWithBar(m, a, t, logt1, q, qRootPows,
    //     barPres);
    // } else {
    //     rvv_mth_op2_rowNTTWithBar(m, a, t, logt1, q, qRootPows,
    //     barPres);
    //     // rvv_mth_rowNTTWithBar(m, a, t, logt1, q, qRootPows, barPres);
    // }
    // rvv_diff(mth_rowNTTWithBar(m, bx, t, logt1, q, qRootPows, barPres));
    rvv_mth_op1_rowNTTWithBar(m, a, t, logt1, q, qRootPows, barPres);
  }
}

static inline void
rvv_rowNTTWithMont(uint64_t *a, long N, long logN, uint64_t q, uint64_t qInv,
                   uint64_t *qRootPows, uint64_t *qRootScalePows,
                   uint64_t *barPres)
{
  long t = N;
  long logt1 = logN + 1;

  for (long m = 1; m < N; m <<= 1) {
    t >>= 1;
    logt1 -= 1;
    // rvv_diff_init();

    // if (t >= 8) {
    //     rvv_mth_op1_rowNTTWithMont(m, a, t, logt1, q, qInv,
    //                                qRootScalePows);
    //     // mth_rowNTTWithMont(m, a, t, logt1, q, qInv, qRootScalePows);
    // } else {
    //     rvv_mth_op2_rowNTTWithMont(m, a, t, logt1, q, qInv,
    //                                qRootScalePows);
    //     // mth_rowNTTWithMont(m, a, t, logt1, q, qInv, qRootScalePows);
    // }
    rvv_mth_op1_rowNTTWithMont(m, a, t, logt1, q, qInv, qRootScalePows);
    /// FIXME: Gem5 can not run op2,but qemu and xuantie-910 can.
    // rvv_diff(mth_rowNTTWithMont(m, bx, t, logt1, q, qInv,
    // qRootScalePows));
  }
}

static inline void
rvv_rowMulWithBar(long rowIdx, long rowSz, long logRowSz, uint64_t *src,
                  uint64_t *wmatrix_scalepows, uint64_t *wmatrix_pows,
                  uint64_t *barPres, long q, long qInv)
{
  uint64_t idx = (rowIdx << logRowSz);
  long op_len = rowSz;
  long res_vl = vsetvli(op_len, 64, 4);
  uint64_t *va = src + idx;
  uint64_t *vwb = wmatrix_pows + idx;
  uint64_t *vb = barPres + idx;
  while (op_len > 0) {
    vle_v(v4, va);  // T
    vle_v(v8, vwb); // W
    vle_v(v12, vb); // R

    vmul_vv(v8, v4, v8);     // z1 = T * W;
    vmulhu_vv(v12, v4, v12); // t = ((__uint128_t)T * R) >> 64;
    vmul_vx(v12, v12, q);    // t1 = q * t;
    vsub_vv(v12, v8, v12);   // z = z1 - t1;
    /// mask reg
    // vmsgtu_vx(v0, v12, q);   // v0 = z > q;
    // vsub_vx_vm(v12, v12, q); // V = v0 ? z - q : z;
    /// none-mask reg
    vsub_vx(v4, v12, q);    // v0 = z - q;
    vminu_vv(v12, v12, v4); // V = v0 ? z - q : z;
    vse_v(v12, va);         // a[idx]

    vwb += res_vl;
    vb += res_vl;

    va += res_vl;
    op_len -= res_vl;
  }
}

static inline void
rvv_rowMulWithMont(long rowIdx, long rowSz, long logRowSz, uint64_t *src,
                   uint64_t *wmatrix_scalepows, uint64_t *wmatrix_pows,
                   uint64_t *barPres, long q, long qInv)
{
  uint64_t idx = (rowIdx << logRowSz);
  long op_len = rowSz;
  long res_vl = vsetvli(op_len, 64, 4);
  uint64_t *va = src + idx;
  uint64_t *vw = wmatrix_scalepows + idx;
  uint64_t *vb = barPres + idx;
  while (op_len > 0) {
    vle_v(v4, va); // T
    vle_v(v8, vw); // W

    // montgomeny
    vmul_vv(v12, v4, v8);    // U0 = low_word(T * W);
    vmulhu_vv(v16, v4, v8);  // U1 = high_word(T * W);
    vmul_vx(v12, v12, qInv); // Q = U0 * qInv;
    vmulhu_vx(v12, v12, q);  // H = high_word(Q * q);
    vmsltu_vv(v0, v16, v12); // v0 = U1 < H;
    vsub_vv(v12, v16, v12);  // U1 - H;
    vadd_vx_vm(v12, v12, q); // V = v0 ? U1 - H + q : U1 - H;

    // TODO: why the v0 can be used here in gem5?

    vse_v(v12, va); // a[idx]
    vw += res_vl;

    va += res_vl;
    op_len -= res_vl;
  }
}

  #define RVV_ROW_NTT_Barrett(a, s, N)                                        \
    {                                                                         \
      uint64_t *a1 = a + (i << log##N);                                       \
      rvv_rowNTTWithBar(a1, N, log##N, q, qInv, s4ntt_##s##_qRootPows[index], \
                        s4ntt_##s##_qRootScalePows[index],                    \
                        s4ntt_##s##BarPres[index]);                           \
    }
  #define RVV_ROW_MUL_Barrett                                              \
    rvv_rowMulWithBar(i, N2, logN2, a, wmatrix_scalepows[index],           \
                      wmatrix_pows[index], s4ntt_wmatrixBarPres[index], q, \
                      qInv);

  #define RVV_ROW_NTT_Montgomeny(a, s, N)                             \
    {                                                                 \
      uint64_t *a1 = a + (i << log##N);                               \
      rvv_rowNTTWithMont(a1, N, log##N, q, qInv,                      \
                         s4ntt_##s##_qRootPows[index],                \
                         s4ntt_##s##_qRootScalePows[index], nullptr); \
    }
  #define RVV_ROW_MUL_Montgomeny                                           \
    rvv_rowMulWithMont(i, N2, logN2, a, wmatrix_scalepows[index], nullptr, \
                       nullptr, q, qInv);

STEP4NTT_TEMPLATE_IMPLEMENT(rvv_step4_qiNTTAndEqual_withBar,
                            RVV_ROW_MUL_Barrett, RVV_ROW_NTT_Barrett,
                            NO_SET_MOD);
STEP4NTT_TEMPLATE_IMPLEMENT(rvv_step4_qiNTTAndEqual_withMont,
                            RVV_ROW_MUL_Montgomeny, RVV_ROW_NTT_Montgomeny,
                            NO_SET_MOD);

void
Context::rvv_ori_qiNTTAndEqual_withBar(uint64_t *a, long index)
{
  uint64_t q = qVec[index];
  uint64_t qInv = qInvVec[index];
  rvv_rowNTTWithBar(a, N, logN, q, qInv, qRootPows[index], nullptr,
                    nttBarPres[index]);
}

void
Context::rvv_ori_qiNTTAndEqual_withMont(uint64_t *a, long index)
{
  uint64_t q = qVec[index];
  uint64_t qInv = qInvVec[index];
  rvv_rowNTTWithMont(a, N, logN, q, qInv, nullptr, qRootScalePows[index],
                     nullptr);
}

static mt_rvv_step_args_t global_args;

// ====== Multi-Threading Accelaration For rvv_step4_bar =======

Step4NTTCol_FUNC_Def(rvv_step4_bar, global_args, rvv_rowNTTWithBar);
Step4NTTRowMulNTT_FUNC_Def(rvv_step4_bar, global_args, rvv_rowMulWithBar,
                           rvv_rowNTTWithBar);
MT_STEP4NTT_TEMPLATE_IMPLEMENT(mt_rvv_step4_qiNTTAndEqual_withBar,
                               rvv_step4_bar, NO_SET_MOD, global_args);

// ====== Multi-Threading Accelaration For rvv_step4_mont =======
Step4NTTCol_FUNC_Def(rvv_step4_mont, global_args, rvv_rowNTTWithMont);
Step4NTTRowMulNTT_FUNC_Def(rvv_step4_mont, global_args, rvv_rowMulWithMont,
                           rvv_rowNTTWithMont);
MT_STEP4NTT_TEMPLATE_IMPLEMENT(mt_rvv_step4_qiNTTAndEqual_withMont,
                               rvv_step4_mont, NO_SET_MOD, global_args);

/// ======= Used to accelerate the rowNTT for rvv_ori_bar =======

mt_rvv_ori_args_t ori_global;

void
mt_rvv_rowNTTWithBar_work_wrapper(void *args)
{
  long t = ori_global.t;
  long res_vl = vsetvli(t, 64, 4);

  long m = ori_global.m;
  uint64_t *a = ori_global.data;
  long logt1 = ori_global.logt;
  long q = ori_global.q;
  uint64_t *qRootPows = ori_global.qRootPows;
  uint64_t *barPres = ori_global.barPres;
  long i = (long)(intptr_t)args;

  // printf("i:%ld\n",i);

  // printf("vl set %ld , get %ld\n", m, res_vl);

  long op_len = t;
  long j1 = i << logt1;
  // long j2 = j1 + t - 1;
  uint64_t W = qRootPows[m + i];
  uint64_t R = barPres[m + i];

  uint64_t *va = a + j1;
  while (op_len > 0) {
    /// load
    vle_v(v4, va + t); // T = a[j + t];
    vle_v(v16, va);    // a[j];
    /// Bar Mod Mul Single Var
    vmul_vx(v8, v4, W);    // z1 = T * W;
    vmulhu_vx(v12, v4, R); // t = ((__uint128_t)T * R) >> 64;
    vmul_vx(v12, v12, q);  // t1 = q * t;
    vsub_vv(v12, v8, v12); // z = z1 - t1;
    /// mask reg
    // vmsgtu_vx(v0, v12, q);   // v0 = z > q;
    // vsub_vx_vm(v12, v12, q); // V = v0 ? z - q : z;
    /// none-mask reg
    vsub_vx(v8, v12, q);    // v0 = z - q;
    vminu_vv(v12, v12, v8); // V = v0 ? z - q : z;

    /// submod
    /// mask reg
    // vmsltu_vv(v0, v16, v12); // v0 = a[j] < V;
    // vsub_vv(v4, v16, v12);   // a[j] - V;
    // vadd_vx_vm(v4, v4, q); // a[j + t]  = v0 ? a[j] - V + q : a[j] -
    // V;
    /// none-mask reg
    vsub_vv(v4, v16, v12); // a[j] - V;
    vadd_vx(v8, v4, q);    // a[j] - V + q;
    vminu_vv(v4, v4, v8);  // a[j + t]  = v0 ? a[j] - V + q : a[j] - V;

    /// addmod
    /// mask reg
    // vadd_vv(v16, v16, v12);  // a[j] += V;
    // vmsgtu_vx(v0, v16, q);   // v0 = a[j] > q;
    // vsub_vx_vm(v16, v16, q); // V = v0 ? a[j] - q : a[j];
    /// none-mask reg
    vadd_vv(v16, v16, v12); // a[j] + V;
    vsub_vx(v8, v16, q);    // a[j] + V - q;
    vminu_vv(v16, v16, v8); // V = v0 ? a[j] - q : a[j];

    /// store
    vse_v(v4, va + t); // a[j + t];
    vse_v(v16, va);    // a[j];

    va += res_vl;
    op_len -= res_vl;
  }
}

static inline void
mt_rvv_mth_rowNTTWithBar(thread_pool_t *thread_pool, uint64_t m, uint64_t *a,
                         long t, long logt1)
{
  ori_global.m = m;
  ori_global.t = t;
  ori_global.logt = logt1;

  for (long i = 0; i < m; i++) {
    // for (long i = m-1; i >= 0; i--) {
    // mt_rvv_rowNTTWithBar_work_wrapper((void *)i);
    thread_pool_add_task(thread_pool, mt_rvv_rowNTTWithBar_work_wrapper,
                         (void *)i);
    // printf("bn: %ld\n", task_num(thread_pool));
  }
  thread_pool_wait(thread_pool);
  volatile long nr = task_num(thread_pool);
  printf("rem n: %ld\n", nr);
  // printf("once\n");
}

void
Context::mt_rvv_ori_qiNTTAndEqual_withBar(uint64_t *a, long index)
{
  uint64_t q = qVec[index];
  long t = N;
  long logt1 = logN + 1;

  ori_global.q = q;
  ori_global.data = a;
  ori_global.qRootPows = qRootPows[index];
  ori_global.barPres = nttBarPres[index];

  for (long m = 1; m < N; m <<= 1) {
    t >>= 1;
    logt1 -= 1;
    mt_rvv_mth_rowNTTWithBar(thread_pool, m, a, t, logt1);
  }
}

#endif // CONFIG_RVV
