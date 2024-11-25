#ifndef __PRE_PROCESS_H__
#define __PRE_PROCESS_H__

#include "Numb.h"
#include <stdint.h>

static inline void calBarPre(long N, uint64_t *b, uint64_t *barPres,
                             uint64_t q) {
  for (long j = 0; j < N; ++j) {
    barPres[j] = (static_cast<__uint128_t>(b[j]) << 64) / q;
  }
}

static inline void cal_RootPows(long N, long logN, uint64_t root,
                                uint64_t *rootPows, uint64_t *rootScalePows,
                                uint64_t q) {
  uint64_t power = 1;
  for (long j = 0; j < N; ++j) {
    uint64_t jprime = bitReverse(static_cast<uint32_t>(j)) >> (32 - logN);
    rootPows[jprime] = power;
    unsigned __int128 tmp = (static_cast<unsigned __int128>(power) << 64);
    mulMod(rootScalePows[jprime], rootPows[jprime],
           (static_cast<uint64_t>(1) << 32), q);
    mulMod(rootScalePows[jprime], rootScalePows[jprime],
           (static_cast<uint64_t>(1) << 32), q);

    if (j < N - 1) {
      mulMod(power, power, root, q);
    }
  }
}

#endif
