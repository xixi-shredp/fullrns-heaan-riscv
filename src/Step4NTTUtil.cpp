#include "Context.h"
#include "Step4NTTUtil.h"

#ifdef CONFIG_STEP4_NTT
void Context::step4_qiNTTAndEqual(uint64_t *a, long index) {
  long t = N;
  long logt1 = logN + 1;
  uint64_t q = qVec[index];
  uint64_t qInv = qInvVec[index];

#ifdef CONFIG_FHE_EXT
  set_mod(q, qInv);
#endif

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
      row_qiNTTAndEqual(a1, N1, logN1, q, qInv, s4ntt_col_qRootPows[index],
                        s4ntt_col_qRootScalePows[index],
                        s4ntt_colBarPres[index]);
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
      row_qiNTTAndEqual(a1, N2, logN2, q, qInv, s4ntt_row_qRootPows[index],
                        s4ntt_row_qRootScalePows[index],
                        s4ntt_rowBarPres[index]);
    }
  });
}
#endif

