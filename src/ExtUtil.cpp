#include "ExtUtil.h"
#include "Context.h"

#ifdef CONFIG_FHE_EXT
void Context::ext_qiNTTAndEqual_withBar(uint64_t *a, long index) {
  long t = N;
  long logt1 = logN + 1;
  uint64_t q = qVec[index];
  uint64_t qInv = qInvVec[index];
  set_mod(q, qInv);
  for (long m = 1; m < N; m <<= 1) {
    t >>= 1;
    logt1 -= 1;
    for (long i = 0; i < m; i++) {
      long j1 = i << logt1;
      long j2 = j1 + t - 1;
      uint64_t W = qRootScalePows[index][m + i];
      uint64_t Wori = qRootPows[index][m + i];
      for (long j = j1; j <= j2; j++) {
        uint64_t T = a[j + t];
        uint64_t V = bar_mulmod_s(T, Wori, nttBarPres[index][m + i]);
        a[j + t] = submod(a[j], V);
        a[j] = addmod(a[j], V);
      }
    }
  }
}
void Context::ext_qiNTTAndEqual_withMont(uint64_t *a, long index) {
  long t = N;
  long logt1 = logN + 1;
  uint64_t q = qVec[index];
  uint64_t qInv = qInvVec[index];
  set_mod(q, qInv);
  for (long m = 1; m < N; m <<= 1) {
    t >>= 1;
    logt1 -= 1;
    for (long i = 0; i < m; i++) {
      long j1 = i << logt1;
      long j2 = j1 + t - 1;
      uint64_t W = qRootScalePows[index][m + i];
      uint64_t Wori = qRootPows[index][m + i];
      for (long j = j1; j <= j2; j++) {
        uint64_t T = a[j + t];
        uint64_t V = mont_redc(T, W);
        a[j + t] = submod(a[j], V);
        a[j] = addmod(a[j], V);
      }
    }
  }
}
#endif
