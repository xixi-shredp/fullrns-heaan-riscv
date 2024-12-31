#ifndef __MATRIX_UTILS_HH
#define __MATRIX_UTILS_HH

#include <stdint.h>
#include <utility>

// #define DISPLAY_DEBUG

// #define COMPUTE_COMPLEXITY

/**
 * the most common matrix transposion
 * required space (out size): m*n
 */
static inline void ref_mtp(uint64_t *out, uint64_t *a, long m, long n) {
  for (long j = 0; j < n; j++)
    for (long i = 0; i < m; i++)
      out[i + j * m] = a[i * n + j];
}

static inline void Nth_non_square_block_mtp(int rowIdx, uint64_t *out,
                                            uint64_t *a, long m, long n,
                                            const int B) {
  for (int col = 0; col < n; col += B) {

    // do transpose on this submatrix
    const int rlimit = rowIdx + B;
    const int climit = col + B;

    for (int i = rowIdx; i < rlimit; i++) {
      for (int j = col; j < climit; j++) {
        out[j * m + i] = a[i * n + j];
      }
    }
  }
}
/**
 * block-divisin for the common matrix transposion
 * required space (out size): m*n
 */
static inline void non_square_block_mtp(uint64_t *out, uint64_t *a, long m,
                                        long n, const int B) {
  for (int row = 0; row < m; row += B) {
    Nth_non_square_block_mtp(row, out, a, m, n, B);
  }
}

static inline void Nth_square_block_mtp(int rowIdx, uint64_t *src, long N1,
                                        long logN1, const int l) {
  for (int i = rowIdx; i < rowIdx + l; ++i) {
    uint64_t idx_pre = i << logN1;
    for (int j = i + 1; j < rowIdx + l; ++j) {
      std::swap<uint64_t>(src[idx_pre + j], src[(j << logN1) + i]);
    }
  }
  for (int col = rowIdx + l; col < N1; col += l) {
    // do transpose on this submatrix
    const int rlimit = rowIdx + l;
    const int climit = col + l;
    for (int i = rowIdx; i < rlimit; ++i) {
      uint64_t idx_pre = i << logN1;
      for (int j = col; j < climit; ++j) {
        std::swap<uint64_t>(src[idx_pre + j], src[(j << logN1) + i]);
      }
    }
  }
}

static inline void square_block_mtp(uint64_t *src, long N1, long logN1,
                                    const int l) {
  for (int row = 0; row < N1; row += l) {
    Nth_square_block_mtp(row, src, N1, logN1, l);
  }
}

static void block_mtp(uint64_t *out, uint64_t *in, long N1, long N2, long logN1,
                      const int blk) {
  if (N1 == N2)
    square_block_mtp(in, N1, logN1, blk);
  else
    non_square_block_mtp(out, in, N1, N2, blk);
}

/**
 * matrix transposion based on desposion
 * required space (buf size): max(logm, logn)
 */
static inline void dep_mtp(long logm, long logn, uint64_t *in, uint64_t *buf) {

#define r(i, j) ((i + (j >> shift1)) & logm_mask)
#define d(i, j) ((r(i, j) + (j << logm)) & logn_mask)
#define s(i, j) ((j + (i << logn) - (i >> shift2)) & logm_mask)

  uint64_t m = 1 << logm;
  uint64_t n = 1 << logn;
  uint64_t logm_mask = m - 1;
  uint64_t logn_mask = n - 1;

  int shift1 = logn - logm;
  int shift2 = 0;
  if (shift1 < 0)
    shift2 = -shift1, shift1 = 0;

  long addr;
  for (int j = 0; j < n; j++) {
    long tem1 = j >> shift1;
    for (int i = 0; i < m; i++) {
      addr = (((i + tem1) & logm_mask) << logn) + j;
      buf[i] = in[addr];
    }
    for (int i = 0; i < m; i++)
      in[(i << logn) + j] = buf[i];
  }
  for (int i = 0; i < m; i++) {
    long tem1 = i << logn;
    for (int j = 0; j < n; j++) {
      addr = (r(i, j) + (j << logm)) & logn_mask;
      buf[addr] = in[tem1 + j];
    }
    for (int j = 0; j < n; j++)
      in[tem1 + j] = buf[j];
  }
  for (int j = 0; j < n; j++) {
    for (int i = 0; i < m; i++)
      buf[i] = in[(s(i, j) << logn) + j];
    for (int i = 0; i < m; i++)
      in[(i << logn) + j] = buf[i];
  }
}

/** in-place matrix transposion */
void trip_mtp(long m, long n, uint64_t *in);

/** check the difference between two array */
void diff(uint64_t *dut, uint64_t *ref, long N);

/** display a matrix */
void mt_display(uint64_t *a, long m, long n);

#endif
