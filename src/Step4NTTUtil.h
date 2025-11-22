#ifndef __STEP4_NTT_UTIL_HH__
#define __STEP4_NTT_UTIL_HH__

#include "Common.h"
#include "utils.h"

#include "MatrixUtils.h"

const int B = (64 / (sizeof(uint64_t)));

#define SET_MOD(mod) set_mod(mod, mod##Inv)
#define NO_SET_MOD

#define STEP4NTT_TEMPLATE_IMPLEMENT(name, mul_func, row_ntt_func, init_code, mod) \
  void Context::name(uint64_t *a, long index) {                                   \
    long t = N;                                                                   \
    long logt1 = logN + 1;                                                        \
    uint64_t mod = mod##Vec[index];                                               \
    uint64_t mod##Inv = mod##InvVec[index];                                       \
                                                                                  \
    init_code;                                                                    \
                                                                                  \
    uint64_t *temp = nullptr;                                                     \
    uint64_t *row_in = a;                                                         \
                                                                                  \
    if (N1 != N2) {                                                               \
      temp = cus_alloc(uint64_t, N);                                              \
      row_in = temp;                                                              \
      non_square_block_mtp(row_in, a, N1, N2, B);                                 \
      for (int i = 0; i < N2; i++) {                                              \
        row_ntt_func(row_in, col, N1, mod);                                       \
      }                                                                           \
      non_square_block_mtp(a, row_in, N2, N1, B);                                 \
      for (int i = 0; i < N1; i++) {                                              \
        mul_func(mod);                                                            \
        row_ntt_func(a, row, N2, mod);                                            \
      }                                                                           \
    } else {                                                                      \
      for (int row = 0; row < N1; row += B) {                                     \
        Nth_square_block_mtp(row, a, N1, logN1, B);                               \
        for (int i = row; i < row + B; ++i) {                                     \
          row_ntt_func(a, col, N1, mod);                                          \
        }                                                                         \
      }                                                                           \
                                                                                  \
      for (int row = 0; row < N2; row += B) {                                     \
        Nth_square_block_mtp(row, a, N2, logN2, B);                               \
        for (int i = row; i < row + B; ++i) {                                     \
          mul_func(mod);                                                          \
          row_ntt_func(a, row, N2, mod);                                          \
        }                                                                         \
      }                                                                           \
    }                                                                             \
  }

#endif
