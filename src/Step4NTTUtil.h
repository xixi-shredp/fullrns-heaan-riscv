#ifndef __STEP4_NTT_UTIL_HH__
#define __STEP4_NTT_UTIL_HH__

#include "Common.h"
#include "utils.h"

static inline void MatrixTransposeAndRowNtt(uint64_t *a, long N, long logN,
                                            uint64_t q, uint64_t qInv,
                                            uint64_t *qRootScalePows) {}

#include "MatrixUtils.h"

#define ref_mtp_and_rowNtt(out, a, m, n)                                       \
  do {                                                                         \
    for (long j = 0; j < n; j++)                                               \
      for (long i = 0; i < m; i++)                                             \
        out[i + j * m] = a[i * n + j];                                         \
  } while (0)

#define ref_mtp_and_mul_rowNtt(out, a, m, n, logm)                             \
  do {                                                                         \
    for (long j = 0; j < n; j++) {                                             \
      for (long i = 0; i < m; i++) {                                           \
        uint64_t W = wmatrix_scalepows[index][j][i];                           \
        out[j * m + i] = montgomey_redc(a[i * n + j], W, q, qInv);             \
      }                                                                        \
      uint64_t *a1 = out + m * j;                                              \
      row_qiNTTAndEqual(a1, m, logm, q, qInv, qRootScalePows[index]);          \
    }                                                                          \
  } while (0)

// #define B 2
#define B (64 / (sizeof(uint64_t)))

#define block_mtp_and_rowNtt(out, a, m, n, logm)                               \
  do {                                                                         \
    for (int col = 0; col < n; col += B) {                                     \
      const int climit = col + B;                                              \
      for (int row = 0; row < m; row += B) {                                   \
        const int rlimit = row + B;                                            \
        for (int i = row; i < rlimit; i++) {                                   \
          for (int j = col; j < climit; j++) {                                 \
            out[j * m + i] = a[i * n + j];                                     \
          }                                                                    \
        }                                                                      \
      }                                                                        \
      for (int i = col; i < climit; i++) {                                     \
        uint64_t *a1 = out + m * i;                                            \
        row_qiNTTAndEqual(a1, m, logm, q, qInv, qRootScalePows[index]);        \
      }                                                                        \
    }                                                                          \
  } while (0)

#define block_mtp_and_mul_rowNtt(out, a, m, n, logm)                           \
  do {                                                                         \
    for (int col = 0; col < n; col += B) {                                     \
      const int climit = col + B;                                              \
      for (int row = 0; row < m; row += B) {                                   \
        const int rlimit = row + B;                                            \
        for (int i = row; i < rlimit; i++) {                                   \
          for (int j = col; j < climit; j++) {                                 \
            uint64_t W = wmatrix_scalepows[index][j][i];                       \
            out[j * m + i] = montgomey_redc(a[i * n + j], W, q, qInv);         \
          }                                                                    \
        }                                                                      \
      }                                                                        \
      for (int i = col; i < climit; i++) {                                     \
        uint64_t *a1 = out + m * i;                                            \
        row_qiNTTAndEqual(a1, m, logm, q, qInv, qRootScalePows[index]);        \
      }                                                                        \
    }                                                                          \
  } while (0)

#if defined CONFIG_NTT_BARRETT
static inline void rowNTTWithBar(uint64_t *a, long N, long logN, uint64_t q,
                                 uint64_t *qRootPows, uint64_t *barPres) {
  long t = N;
  long logt1 = logN + 1;

  for (long m = 1; m < N; m <<= 1) {
    t >>= 1;
    logt1 -= 1;
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
}
#define row_qiNTTAndEqual(a, N, logN, q, qInv, qRootPows, qRootScalePows,      \
                          barPres)                                             \
  rowNTTWithBar(a, N, logN, q, qRootPows, barPres)

#elif defined CONFIG_NTT_MONTGOMENY
static inline void rowNTTWithMont(uint64_t *a, long N, long logN, uint64_t q,
                                  uint64_t qInv, uint64_t *qRootScalePows) {
  long t = N;
  long logt1 = logN + 1;

  for (long m = 1; m < N; m <<= 1) {
    t >>= 1;
    logt1 -= 1;
    for (long i = 0; i < m; i++) {
      long j1 = i << logt1;
      long j2 = j1 + t - 1;
      uint64_t W = qRootScalePows[m + i];
      for (long j = j1; j <= j2; j++) {
        uint64_t T = a[j + t];
        // Montgomeny ModMul : V = REDC(x = T, y = W, p = q, p' = qInv)
        // CT butterfly:
        // a[j]   = a[j] + (a[j+t] * W) mod q
        // a[j+t] = a[j] - (a[j+t] * W) mod q
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
}
#define row_qiNTTAndEqual(a, N, logN, q, qInv, qRootPows, qRootScalePows,      \
                          barPres)                                             \
  rowNTTWithMont(a, N, logN, q, qInv, qRootScalePows)
#endif

#endif
