#include "RVVUtil.h"
#include "Context.h"
#ifdef CONFIG_FHE_EXT
#include "FHEUtil.h"
#endif

#include "Step4NTTUtil.h"

#ifdef CONFIG_RVV

static inline void mth_rowNTTWithBar(uint64_t m, uint64_t *a, long t,
                                     long logt1, uint64_t q,
                                     uint64_t *qRootPows, uint64_t *barPres) {
  for (long i = 0; i < m; i++) {
    long j1 = i << logt1;
    long j2 = j1 + t - 1;
    uint64_t W = qRootPows[m + i];
    uint64_t R = barPres[m + i];
    for (long j = j1; j <= j2; j++) {
      uint64_t T = a[j + t];
      // Montgomeny ModMul : V = REDC(x = T, y = W, p = q, p' = qInv)
      // CT butterfly:
      // a[j]   = a[j] + (a[j+t] * W) mod q
      // a[j+t] = a[j] - (a[j+t] * W) mod q
#ifdef CONFIG_FHE_EXT
      uint64_t V = bar_mulmod_s(T, W, R);
      a[j + t] = submod(a[j], V);
      a[j] = addmod(a[j], V);
#else
      uint64_t V = barrett_modMul_singalVal(T, W, q, R);
      a[j + t] = a[j] < V ? a[j] + q - V : a[j] - V;
      a[j] += V;
      if (a[j] > q)
        a[j] -= q;
#endif
    }
  }
}
/**
 * Fit for parallelizing the lay when t >= 8
 * */
static inline void rvv_mth_op1_rowNTTWithBar(uint64_t m, uint64_t *a, long t,
                                             long logt1, uint64_t q,
                                             uint64_t *qRootPows,
                                             uint64_t *barPres) {
  for (long i = 0; i < m; i++) {
    long j1 = i << logt1;
    // long j2 = j1 + t - 1;
    uint64_t W = qRootPows[m + i];
    uint64_t R = barPres[m + i];
    // #ifdef CONFIG_FHE_EXT
    //     asm volatile("mv t0, %0" ::"r"(W));
    //     asm volatile("mv t1, %0" ::"r"(R));
    // #endif
    long op_len = t;
    long res_vl = vsetvli(op_len, 64, 4);
    // printf("vl set %ld , get %ld\n", op_len, res_vl);
    uint64_t *va = a + j1;
    while (op_len > 0) {
      /// load
      vle_v(v4, va + t); // T = a[j + t];
      vle_v(v16, va);    // a[j];
#ifdef CONFIG_FHE_EXT
      vbarmodmuls_vxx(v4, W, R);
      vsubmod_vv(v8, v16, v4);
      vaddmod_vv(v16, v16, v4);
      /// store
      vse_v(v8, va + t); // a[j + t];
      vse_v(v16, va);    // a[j];
#else
      /// Bar Mod Mul Single Var
      vmul_vx(v8, v4, W);      // z1 = T * W;
      vmulhu_vx(v12, v4, R);   // t = ((__uint128_t)T * R) >> 64;
      vmul_vx(v12, v12, q);    // t1 = q * t;
      vsub_vv(v12, v8, v12);   // z = z1 - t1;
      vmsgtu_vx(v0, v12, q);   // v0 = z > q;
      vsub_vx_vm(v12, v12, q); // V = v0 ? z - q : z;
      /// submod
      vmsltu_vv(v0, v16, v12); // v0 = a[j] < V;
      vsub_vv(v4, v16, v12);   // a[j] - V;
      vadd_vx_vm(v4, v4, q);   // a[j + t]  = v0 ? a[j] - V + q : a[j] - V;
      /// addmod
      vadd_vv(v16, v16, v12);  // a[j] += V;
      vmsgtu_vx(v0, v16, q);   // v0 = a[j] > q;
      vsub_vx_vm(v16, v16, q); // V = v0 ? a[j] - q : a[j];
      /// store
      vse_v(v4, va + t); // a[j + t];
      vse_v(v16, va);    // a[j];
#endif

      va += res_vl;
      op_len -= res_vl;
    }
  }
}

/**
 * Fit for parallelizing the lay when m >= 8
 * */
static inline void rvv_mth_op2_rowNTTWithBar(uint64_t m, uint64_t *a, long t,
                                             long logt1, uint64_t q,
                                             uint64_t *qRootPows,
                                             uint64_t *barPres) {
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

#ifdef CONFIG_FHE_EXT
      vbarmodmuls_vv(v12, v4, v8); // asm volatile(".word 0xf6440657");
      vsubmod_vv(v16, v24, v12);   // asm volatile(".word 0xef860857");
      vaddmod_vv(v24, v24, v12);   // asm volatile(".word 0xeb860c57");
      /// store
      vsse_v(v16, va + t, stride); // a[j + t];
      vsse_v(v24, va, stride);     // a[j];
#else

      /// Bar Mod Mul Single Var
      vmul_vv(v16, v12, v4);   // z1 = T * W;
      vmulhu_vv(v20, v12, v8); // t0 = T * R;
      vmul_vx(v20, v20, q);    // t1 = t0 * q;
      vsub_vv(v16, v16, v20);  // z = z1 - t1;
      vmsgtu_vx(v0, v16, q);   // v0 = z > q;
      vsub_vx_vm(v16, v16, q); // V = v0 ? z - q : z;

      /// submod
      vmsltu_vv(v0, v24, v16); // v0 = a[j] < V;
      vsub_vv(v12, v24, v16);  // a[j] - V;
      vadd_vx_vm(v12, v12, q); // a[j + t]  = v0 ? a[j] - V + q : a[j] - V;

      /// addmod
      vadd_vv(v24, v24, v16);  // a[j] += V;
      vmsgtu_vx(v0, v24, q);   // v0 = a[j] > q;
      vsub_vx_vm(v24, v24, q); // V = v0 ? a[j] - q : a[j];

      /// store
      vsse_v(v12, va + t, stride); // a[j + t];
      vsse_v(v24, va, stride);     // a[j];

#endif

      va++;
    }

    op_len -= res_vl;
    oa += oa_stride;
    b_stride += res_vl;
  }
}
static inline void mth_rowNTTWithMont(uint64_t m, uint64_t *a, long t,
                                      long logt1, uint64_t q, uint64_t qInv,
                                      uint64_t *qRootScalePows) {
  for (long i = 0; i < m; i++) {
    long j1 = i << logt1;
    long j2 = j1 + t - 1;
    uint64_t W = qRootScalePows[m + i];
    for (long j = j1; j <= j2; j++) {
      uint64_t T = a[j + t];
#ifdef CONFIG_FHE_EXT
      uint64_t V = mont_redc(T, W);
      a[j + t] = submod(a[j], V);
      a[j] = addmod(a[j], V);
#else
      uint64_t V = montgomey_redc(T, W, q, qInv);
      a[j + t] = a[j] < V ? a[j] + q - V : a[j] - V;
      a[j] += V;
      if (a[j] > q)
        a[j] -= q;
#endif
    }
  }
}
/**
 * Fit for parallelizing the lay when t >= 8
 * */
static inline void rvv_mth_op1_rowNTTWithMont(uint64_t m, uint64_t *a, long t,
                                              long logt1, uint64_t q,
                                              uint64_t qInv,
                                              uint64_t *qRootScalePows) {
  for (long i = 0; i < m; i++) {
    long j1 = i << logt1;
    uint64_t W = qRootScalePows[m + i];
    long op_len = t;
    long res_vl = vsetvli(op_len, 64, 4);
    // printf("vl set %ld , get %ld\n", op_len, res_vl);
    uint64_t *va = a + j1;
    while (op_len > 0) {
      /// load
      vle_v(v4, va + t); // T = a[j + t];
      vle_v(v16, va);    // a[j];
#ifdef CONFIG_FHE_EXT
      vmontredc_vx(v12, v4, W);
      vsubmod_vv(v4, v16, v12);
      vaddmod_vv(v16, v16, v12);
#else
      /// Mont Redc
      vmul_vx(v8, v4, W);      // U0 = low_word(T * W);
      vmulhu_vx(v12, v4, W);   // U1 = high_word(T * W);
      vmul_vx(v8, v8, qInv);   // Q = U0 * qInv;
      vmulhu_vx(v8, v8, q);    // H = high_word(Q * q);
      vmsltu_vv(v0, v12, v8);  // v0 = U1 < H;
      vsub_vv(v12, v12, v8);   // U1 - H;
      vadd_vx_vm(v12, v12, q); // V = v0 ? U1 - H + q : U1 - H;
      /// submod
      vmsltu_vv(v0, v16, v12); // v0 = a[j] < V;
      vsub_vv(v4, v16, v12);   // a[j] - V;
      vadd_vx_vm(v4, v4, q);   // a[j + t]  = v0 ? a[j] - V + q : a[j] - V;
      /// addmod
      vadd_vv(v16, v16, v12);  // a[j] += V;
      vmsgtu_vx(v0, v16, q);   // v0 = a[j] > q;
      vsub_vx_vm(v16, v16, q); // V = v0 ? a[j] - q : a[j];
#endif
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
static inline void rvv_mth_op2_rowNTTWithMont(uint64_t m, uint64_t *a, long t,
                                              long logt1, uint64_t q,
                                              uint64_t qInv,
                                              uint64_t *qRootScalePows) {
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

#ifdef CONFIG_FHE_EXT
      vmontredc_vv(v16, v12, v4);
      vsubmod_vv(v12, v24, v16);
      vaddmod_vv(v24, v24, v16);
#else

      /// Mont Redc
      vmul_vv(v8, v12, v4);    // U0 = low_word(T * W);
      vmulhu_vv(v12, v12, v4); // U1 = high_word(T * W);
      vmul_vx(v8, v8, qInv);   // Q = U0 * qInv;
      vmulhu_vx(v8, v8, q);    // H = high_word(Q * q);
      vmsltu_vv(v0, v12, v8);  // v0 = U1 < H;
      vsub_vv(v16, v12, v8);   // U1 - H;
      vadd_vx_vm(v16, v16, q); // V = v0 ? U1 - H + q : U1 - H;

      /// submod
      vmsltu_vv(v0, v24, v16); // v0 = a[j] < V;
      vsub_vv(v12, v24, v16);  // a[j] - V;
      vadd_vx_vm(v12, v12, q); // a[j + t]  = v0 ? a[j] - V + q : a[j] - V;

      /// addmod
      vadd_vv(v24, v24, v16);  // a[j] += V;
      vmsgtu_vx(v0, v24, q);   // v0 = a[j] > q;
      vsub_vx_vm(v24, v24, q); // V = v0 ? a[j] - q : a[j];

#endif
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

static inline void rvv_rowNTT(uint64_t *a, long N, long logN, uint64_t q,
                              uint64_t qInv, uint64_t *qRootPows,
                              uint64_t *qRootScalePows, uint64_t *barPres) {
  long t = N;
  long logt1 = logN + 1;

#ifdef CONFIG_FHE_EXT
  set_mod(q, qInv);
#endif

  for (long m = 1; m < N; m <<= 1) {
    t >>= 1;
    logt1 -= 1;
    // rvv_diff_init();

#if defined CONFIG_NTT_BARRETT
    if (t >= 8) {
      rvv_mth_op1_rowNTTWithBar(m, a, t, logt1, q, qRootPows, barPres);
    } else {
      rvv_mth_op2_rowNTTWithBar(m, a, t, logt1, q, qRootPows, barPres);
      // rvv_mth_rowNTTWithBar(m, a, t, logt1, q, qRootPows, barPres);
    }
#elif defined CONFIG_NTT_MONTGOMENY
    if (t >= 8) {
      rvv_mth_op1_rowNTTWithMont(m, a, t, logt1, q, qInv, qRootScalePows);
    } else {
      rvv_mth_op2_rowNTTWithMont(m, a, t, logt1, q, qInv, qRootScalePows);
      // mth_rowNTTWithMont(m, a, t, logt1, q, qInv, qRootScalePows);
    }
#endif
    // rvv_diff();
  }
}

#if defined CONFIG_NTT_BARRETT
#define RVV_ROW_NTT(s)                                                         \
  rvv_rowNTT(a1, N1, logN1, q, qInv, s4ntt_##s##_qRootPows[index],             \
             s4ntt_##s##_qRootScalePows[index], s4ntt_##s##BarPres[index])
#elif defined CONFIG_NTT_MONTGOMENY
#define RVV_ROW_NTT(s, N)                                                      \
  rvv_rowNTT(a1, N, log##N, q, qInv, s4ntt_##s##_qRootPows[index],             \
             s4ntt_##s##_qRootScalePows[index], nullptr)
#endif

#ifdef CONFIG_STEP4_NTT
void Context::rvv_step4_qiNTTAndEqual(uint64_t *a, long index) {
  long t = N;
  long logt1 = logN + 1;
  uint64_t q = qVec[index];
  uint64_t qInv = qInvVec[index];

  uint64_t *temp = cus_alloc(uint64_t, N);
  uint64_t *row_in = a;

  TIME_SPAN("matrix transpose 1", {
  // 1. Transpose the N1 × N2 input matrix and get a new N2 × N1 matrix.
  //    Perform NTT on each row of the N2 × N1 matrix

#if defined(CONFIG_SIMPLE_MT)
    row_in = temp;
    ref_mtp(row_in, a, N1, N2);
#elif defined(CONFIG_BLOCKED_MT)
      row_in = temp;
      block_mtp(row_in, a, N1, N2, B);
#elif defined(CONFIG_DEP_MT)
      dep_mtp(logN1, logN2, a, temp);
#elif defined(CONFIG_TRIP_MT)
      trip_mtp(N1, N2, a);
#endif
  });

  TIME_SPAN("row ntt 1", {
    for (long row = 0; row < N2; row++) {
      uint64_t *a1 = row_in + (row << logN1);
      RVV_ROW_NTT(col, N1);
    }
  });

  TIME_SPAN("matrix transpose 2", {
// 2. Transpose matrix
#if defined(CONFIG_SIMPLE_MT)
    ref_mtp(a, row_in, N2, N1);
#elif defined(CONFIG_BLOCKED_MT)
     block_mtp(a, row_in, N2, N1, B);
#elif defined(CONFIG_DEP_MT)
     dep_mtp(logN2, logN1, a, temp);
#elif defined(CONFIG_TRIP_MT)
     trip_mtp(N2, N1, a);
#endif
  });

  TIME_SPAN("row ntt 2", {
    // 3. Generate a N1 × N2 twisting factor matrix and perform dyadic
    // multiplication between matrixs
    // for (int i = N1 - 1; i >= 0; --i) {
    for (int i = 0; i < N1; ++i) {
      for (int j = 0; j < N2; ++j) {
        uint64_t idx = (i << logN2) + j;
        uint64_t T = a[idx];
        // uint64_t W = wmatrix_scalepows[index][i][j];
        uint64_t W = wmatrix_scalepows[index][idx];
#ifdef CONFIG_NTT_BARRETT
        uint64_t R = s4ntt_wmatrixBarPres[index][idx];
        a[idx] = barrett_modMul_singalVal(T, W, q, R);
#endif
        a[idx] = montgomey_redc(T, W, q, qInv);
      }
      // 4. Perform NTT on each row of the N1 × N2 matrix
      uint64_t *a1 = a + (i << logN2);
      RVV_ROW_NTT(row, N2);
    }
  });
}
#endif
#endif
