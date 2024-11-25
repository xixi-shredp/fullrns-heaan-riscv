#ifndef __UTILS_H__
#define __UTILS_H__

#include <cstddef>
#include <sys/mman.h>


#ifdef CONFIG_BIGPAGE
static inline void *big_page_alloc(size_t size) {
  int flags = MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB |
              ((21 & MAP_HUGE_MASK) << MAP_HUGE_SHIFT); // 2^21 == 2M
  int protection = PROT_READ | PROT_WRITE;
  return mmap(NULL, size, protection, flags, -1, 0);
}
#define cus_alloc(type, num)                                                   \
  static_case<type *>(big_page_alloc(sizeof(type) * num))
#else
#define cus_alloc(type, num) new type[num]
#endif

// #define TIME_STAT

#include "TimeUtils.h"
#ifdef TIME_STAT
#define TIME_SPAN(name, code)                                                  \
  {                                                                            \
    TimeUtils timeutils;                                                       \
    timeutils.start(name);                                                     \
    code;                                                                      \
    timeutils.stop(name);                                                      \
  }
#else
#define TIME_SPAN(name, code) code
#endif

static inline uint64_t montgomey_redc(uint64_t T, uint64_t W, uint64_t q,
                                      uint64_t qInv) {
  unsigned __int128 U = static_cast<unsigned __int128>(T) * W;
  uint64_t U0 = static_cast<uint64_t>(U);
  uint64_t U1 = static_cast<uint64_t>(U >> 64);
  uint64_t Q = U0 * qInv;
  unsigned __int128 Hx = static_cast<unsigned __int128>(Q) * q;
  uint64_t H = static_cast<uint64_t>(Hx >> 64);
  uint64_t V = U1 < H ? U1 + q - H : U1 - H;
  return V;
}

static inline uint64_t barrett_modMul_singalVal(uint64_t a, uint64_t b,
                                                uint64_t q, uint64_t r) {
  uint64_t z1 = a * b;
  uint64_t t = (static_cast<__uint128_t>(a) * r) >> 64;
  uint64_t t1 = q * t;
  uint64_t z = z1 - t1;
  return z > q ? z - q : z;
}

#endif
