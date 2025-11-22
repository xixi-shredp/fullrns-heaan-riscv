#ifndef __MULTI_THREADING_ACC_H__
#define __MULTI_THREADING_ACC_H__

#include "Context.h"

typedef struct {
  uint64_t *data;
  long logN;
  long N;
  uint64_t q;
  uint64_t qInv;
  Context *c;
  long index;
} mt_rvv_step_args_t;

typedef struct {
  uint64_t *data;
  long logt;
  long t;
  long m;
  uint64_t q;
  uint64_t *qRootPows;
  uint64_t *barPres;
  long res_vl;
} mt_rvv_ori_args_t;

#define Step4NTTCol_FUNC_Def(case, global, ROWNTT_FUNC, mod)       \
void mt_##case##_rowntt_col_func(void *args){                      \
    mt_rvv_step_args_t *_p = &global; int i = (int)(intptr_t)args; \
    uint64_t *a1 = _p->data + (i << _p->logN); Context *c = _p->c; \
                                                                   \
    ROWNTT_FUNC(a1, _p->N, _p->logN, _p->q, _p->qInv,              \
                c->s4ntt_col_##mod##RootPows[_p->index],           \
                c->s4ntt_col_##mod##RootScalePows[_p->index],      \
                c->s4ntt_##mod##colBarPres[_p->index]);}

#define Step4NTTRowMulNTT_FUNC_Def(case, global, RowMul_Func, ROWNTT_FUNC, mod)             \
void mt_##case##_row_mulntt_row_func(void *args){                                           \
    mt_rvv_step_args_t *_p = &global; int i = (int)(intptr_t)args;                          \
    Context *c = _p->c;                                                                     \
                                                                                            \
    RowMul_Func(i, _p->N, _p->logN, _p->data, c->mod##Wmatrix_scalepows[_p->index],         \
                c->mod##Wmatrix_pows[_p->index], c->s4ntt_##mod##WmatrixBarPres[_p->index], \
                _p->q, _p->qInv);                                                           \
    uint64_t *a1 = _p->data + (i << _p->logN); ROWNTT_FUNC(                                 \
        a1, _p->N, _p->logN, _p->q, _p->qInv, c->s4ntt_row_##mod##RootPows[_p->index],      \
        c->s4ntt_row_##mod##RootScalePows[_p->index],                                       \
        c->s4ntt_##mod##rowBarPres[_p->index]);}

#define MT_STEP4NTT_TEMPLATE_IMPLEMENT(cfunc, case, initcode, global, mod)   \
  void Context::cfunc(uint64_t *a, long index)                               \
  {                                                                          \
    long t = N;                                                              \
    long logt1 = logN + 1;                                                   \
    uint64_t mod = mod##Vec[index];                                          \
    uint64_t mod##Inv = mod##InvVec[index];                                  \
                                                                             \
    uint64_t *temp = nullptr;                                                \
    uint64_t *row_in = a;                                                    \
                                                                             \
    global.q = mod;                                                          \
    global.qInv = mod##Inv;                                                  \
    global.c = this;                                                         \
    global.index = index;                                                    \
                                                                             \
    initcode;                                                                \
                                                                             \
    if (N1 != N2) {                                                          \
      temp = cus_alloc(uint64_t, N);                                         \
      row_in = temp;                                                         \
      non_square_block_mtp(row_in, a, N1, N2, B);                            \
      global.N = N1;                                                         \
      global.logN = logN1;                                                   \
      global.data = row_in;                                                  \
      for (int i = 0; i < N2; i++) {                                         \
        thread_pool_add_task(thread_pool, mt_##case##_rowntt_col_func,       \
                             (void *)(uintptr_t)i);                          \
      }                                                                      \
                                                                             \
      thread_pool_wait(thread_pool);                                         \
                                                                             \
      non_square_block_mtp(a, row_in, N2, N1, B);                            \
      global.N = N2;                                                         \
      global.logN = logN2;                                                   \
      global.data = a;                                                       \
      for (int i = 0; i < N1; i++) {                                         \
        thread_pool_add_task(thread_pool, mt_##case##_row_mulntt_row_func,   \
                             (void *)(uintptr_t)i);                          \
      }                                                                      \
    } else {                                                                 \
      global.N = N1;                                                         \
      global.logN = logN1;                                                   \
      global.data = a;                                                       \
      for (int row = 0; row < N1; row += B) {                                \
        Nth_square_block_mtp(row, a, N1, logN1, B);                          \
        for (int i = row; i < row + B; ++i) {                                \
          thread_pool_add_task(thread_pool, mt_##case##_rowntt_col_func,     \
                               (void *)(uintptr_t)i);                        \
        }                                                                    \
      }                                                                      \
                                                                             \
      thread_pool_wait(thread_pool);                                         \
                                                                             \
      for (int row = 0; row < N2; row += B) {                                \
        Nth_square_block_mtp(row, a, N2, logN2, B);                          \
        for (int i = row; i < row + B; ++i) {                                \
          thread_pool_add_task(thread_pool, mt_##case##_row_mulntt_row_func, \
                               (void *)(uintptr_t)i);                        \
        }                                                                    \
      }                                                                      \
    }                                                                        \
  }
#endif
