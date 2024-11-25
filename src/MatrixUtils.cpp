#include "MatrixUtils.h"

#include <stdio.h>
#include <stdlib.h>

#include <cassert>

#ifdef DISPLAY_DEBUG
#define debug(...) printf(__VA_ARGS__)
#else
#define debug(...)
#endif

// #define r(i, j) (i + (j >> shift1) & logm_mask)
// #define d(i, j) ((r(i, j) + (j << logm)) & logn_mask)
// #define s(i, j) ((j + (i << logn) - (i >> shift2)) & logm_mask)
//
// // buf size: max(logm, logn)
// void dep_mtp(long logm, long logn, uint64_t *in, uint64_t *buf) {
//   // assert(logm <= logn);
//   uint64_t m = 1 << logm;
//   uint64_t n = 1 << logn;
//   uint64_t logm_mask = m - 1;
//   uint64_t logn_mask = n - 1;
//
//   int shift1 = logn - logm;
//   int shift2 = 0;
//   if (shift1 < 0)
//     shift2 = -shift1, shift1 = 0;
//
//   for (int j = 0; j < n; j++) {
//     for (int i = 0; i < m; i++)
//       buf[i] = in[(r(i, j) << logn) + j];
//     for (int i = 0; i < m; i++)
//       in[(i << logn) + j] = buf[i];
//   }
//   for (int i = 0; i < m; i++) {
//     for (int j = 0; j < n; j++)
//       buf[d(i, j)] = in[(i << logn) + j];
//     for (int j = 0; j < n; j++)
//       in[(i << logn) + j] = buf[j];
//   }
//   for (int j = 0; j < n; j++) {
//     for (int i = 0; i < m; i++)
//       buf[i] = in[(s(i, j) << logn) + j];
//     for (int i = 0; i < m; i++)
//       in[(i << logn) + j] = buf[i];
//   }
// }

// void ref_mtp(uint64_t *out, uint64_t *a, long m, long n) {
//   for (long j = 0; j < n; j++)
//     for (long i = 0; i < m; i++)
//       out[i + j * m] = a[i * n + j];
// }

// void block_mtp(uint64_t *out, uint64_t *a, long m, long n, const int B) {
//  for (int row = 0; row < m; row += B) {
//     for (int col = 0; col < n; col += B) {
//
//       // do transpose on this submatrix
//       const int rlimit = row + B;
//       const int climit = col + B;
//
//       for (int i = row; i < rlimit; i++) {
//         for (int j = col; j < climit; j++) {
//           out[j * m + i] = a[i * n + j];
//         }
//       }
//     }
//   }
// }

void diff(uint64_t *dut, uint64_t *ref, long N) {
  for (long i = 0; i < N; i++) {
    if (dut[i] != ref[i]) {
      printf("got the difference in index %ld.\n", i);
      printf("dut : %ld, ref : %ld\n", dut[i], ref[i]);
      return;
    }
  }
}
void mt_display(uint64_t *a, long m, long n) {
  for (long i = 0; i < m; i++) {
    for (long j = 0; j < n; j++) {
      debug("%ld ", a[i * n + j]);
    }
    debug("\n");
  }
}

#include <stddef.h>
#include <stdint.h>
#define REVERSE_VOODOO 1
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))

typedef struct {
  uint64_t work;
  uint64_t span;
} ws;

#define cilk_sync
#define cilk_spawn

template <typename T> class TRIP {
public:
  static inline void increaseComplexity(ws *const comp) {
#ifdef COMPUTE_COMPLEXITY
    comp->span += 1;
    comp->work += 1;
#endif
  }

  static inline void combineAndAddComplexity4(ws *const comp, ws a, ws b, ws c,
                                              ws d) {
#ifdef COMPUTE_COMPLEXITY
    ws ab = combineComplexity(a, b);
    ws cd = combineComplexity(c, d);
    ws abcd = combineComplexity(ab, cd);
    addComplexity(comp, abcd);
#endif
  }

  static inline ws combineComplexity(ws a, ws b) {
#ifdef COMPUTE_COMPLEXITY
    ws result;
    result.span = MAX(a.span, b.span);
    result.work = a.work + b.work;
    return result;
#endif
    return a;
  }

  static inline void addComplexity(ws *const comp, ws b) {
#ifdef COMPUTE_COMPLEXITY
    comp->work += b.work;
    comp->span += b.span;
#endif
  }

  static inline void combineAndAddComplexity(ws *const comp, ws a, ws b) {
#ifdef COMPUTE_COMPLEXITY
    ws combined = combineComplexity(a, b);
    addComplexity(comp, combined);
#endif
  }

  static inline void swap_single(T *x, T *y) {
    T t = *x;
    *x = *y;
    *y = t;
  }

  static inline ws transpose4(T *a, size_t I0, size_t J0, size_t i0, size_t i1,
                              size_t j0, size_t j1, size_t N) {
    ws comp = {1, 1};
    if (i1 - i0 > 1) {
      size_t im = (i0 + i1) / 2, jm = (j0 + j1) / 2;
      ws tComp1, tComp2, tComp3, tComp4 = {0, 0};
      tComp1 = cilk_spawn transpose4(a, I0, J0, i0, im, j0, jm, N);
      tComp2 = cilk_spawn transpose4(a, I0, J0, i0, im, jm, j1, N);
      tComp3 = cilk_spawn transpose4(a, I0, J0, im, i1, jm, j1, N);
      if (i1 <= j0)
        tComp4 = cilk_spawn transpose4(a, I0, J0, im, i1, j0, jm, N);
      cilk_sync;
      combineAndAddComplexity4(&comp, tComp1, tComp2, tComp3, tComp4);
    } else {
      size_t j;
      for (j = j0; j < j1; j++) {
        swap_single(&a[(I0 + j) * N + J0 + i0], &a[(I0 + i0) * N + J0 + j]);
        increaseComplexity(&comp);
      }
    }
    return comp;
  }

  static inline void next(size_t *i, size_t *count, size_t j0, size_t j1,
                          size_t N, size_t p) {
    if (*count == p - 1) {
      *count = 0;
      *i += (N - j1) + j0 + 1;
    } else {
      *count += 1;
      *i += 1;
    }
  }

  static inline void prev(size_t *i, size_t *count, size_t j0, size_t j1,
                          size_t N, size_t p) {
    if (*count == 0) {
      *count = p - 1;
      *i -= (N - j1) + j0 + 1;
    } else {
      *count -= 1;
      *i -= 1;
    }
  }
  static inline ws reverse_offset(T *a, size_t m0, size_t m1, size_t l,
                                  size_t i0, size_t j0, size_t j1, size_t N) {
    size_t i, next_count;
    size_t j, prev_count;
    size_t m, mm;
    size_t p;
    T tmp;
    ws comp = {0, 0};
    p = j1 - j0;
    // index starting from left ( going right ) ; origin
    // al matrix index
    i = i0 * N + j0 + (m0 / p) * N + (m0 % p);
    next_count = m0 % p;
    // i n d e x s t a r t i n g from r i g h t ( going l e f t ) ; o r i g i n
    // a l m a t r i x i n d e x
    j = i0 * N + j0 + ((m1 - 1) / p) * N + ((m1 - 1) % p);
    prev_count = (m1 - 1) % p;
    mm = m0 + l;
    for (m = m0; m < mm; next(&i, &next_count, j0, j1, N, p),
        prev(&j, &prev_count, j0, j1, N, p), m++) {
      tmp = a[j];
      a[j] = a[i];
      a[i] = tmp;
      increaseComplexity(&comp);
    }
    return comp;
  }
  static inline ws reverse_recursive(T *a, size_t m0, size_t m1, size_t l,
                                     size_t i0, size_t j0, size_t j1,
                                     size_t N) {
    ws comp = {1, 1};
    if (l > REVERSE_VOODOO) {
      size_t lm = l / 2;

      ws rComp1 = cilk_spawn reverse_recursive(a, m0, m1, lm, i0, j0, j1, N);
      ws rComp2 = cilk_spawn reverse_recursive(a, m0 + lm, m1 - lm, l - lm, i0,
                                               j0, j1, N);
      cilk_sync;
      combineAndAddComplexity(&comp, rComp1, rComp2);
    } else {
      ws rComp = reverse_offset(a, m0, m1, l, i0, j0, j1, N);
      addComplexity(&comp, rComp);
    }
    return comp;
  }

  static inline ws reverse(T *a, size_t m0, size_t m1, size_t i0, size_t j0,
                           size_t j1, size_t N) {
    return reverse_recursive(a, m0, m1, (m1 - m0) / 2, i0, j0, j1, N);
  }

  static inline ws splitr(T *a, size_t p, size_t q, size_t i0, size_t i1,
                          size_t j0, size_t j1, size_t s0, size_t s1, size_t k,
                          size_t N) {

    size_t k2 = k >> 1;
    size_t rm, r0, r1;
    size_t sm;
    ws comp = {1, 1};
    if (k == 1)
      return comp;
    // split left and right part
    sm = s0 + k2 * (p + q);
    ws sComp1 = cilk_spawn splitr(a, p, q, i0, i1, j0, j1, s0, sm, k2, N);
    ws sComp2 = cilk_spawn splitr(a, p, q, i0, i1, j0, j1, sm, s1, k - k2, N);
    cilk_sync;
    combineAndAddComplexity(&comp, sComp1, sComp2);
    // rotate middle part
    r0 = s0 + k2 * p;
    r1 = s0 + k * p + k2 * q;
    ws rComp = cilk_spawn reverse(a, r0, r1, i0, j0, j1, N);
    cilk_sync;
    addComplexity(&comp, rComp);
    // rotate left and right part
    rm = s0 + k * p;
    ws rComp1 = cilk_spawn reverse(a, r0, rm, i0, j0, j1, N);
    ws rComp2 = cilk_spawn reverse(a, rm, r1, i0, j0, j1, N);
    cilk_sync;
    combineAndAddComplexity(&comp, rComp1, rComp2);
    return comp;
  }

  static inline ws split(T *a, size_t p, size_t q, size_t i0, size_t i1,
                         size_t j0, size_t j1, size_t N) {
    size_t s0, s1;
    s0 = 0;
    s1 = (j1 - j0) * (i1 - i0);
    return splitr(a, p, q, i0, i1, j0, j1, s0, s1, i1 - i0, N);
  }

  static inline ws merger(T *a, size_t p, size_t q, size_t i0, size_t i1,
                          size_t j0, size_t j1, size_t m0, size_t m1, size_t k,
                          size_t N) {
    size_t k2 = k / 2;
    size_t rm, r0, r1;
    size_t mm;
    ws comp = {1, 1};
    if (k == 1)
      return comp;
    // for rotation first reverse middle part
    // then reverse left and right
    r0 = m0 + k2 * p;
    r1 = m0 + k * p + k2 * q;
    ws rComp = reverse(a, r0, r1, i0, j0, j1, N);
    addComplexity(&comp, rComp);
    // first reverse whole middle part
    // then reverse l e f t and r i g h t o f t h e m i d d l e p a r t
    rm = r0 + k2 * q;
    ws rComp1 = cilk_spawn reverse(a, r0, rm, i0, j0, j1, N);
    ws rComp2 = cilk_spawn reverse(a, rm, r1, i0, j0, j1, N);
    cilk_sync;
    combineAndAddComplexity(&comp, rComp1, rComp2);
    // now merge theresulting sub *arrays
    mm = m0 + k2 * (p + q); // == rm
    ws mComp1 = cilk_spawn merger(a, p, q, i0, i1, j0, j1, m0, mm, k2, N);
    ws mComp2 = cilk_spawn merger(a, p, q, i0, i1, j0, j1, mm, m1, k - k2, N);
    // k*k2 sonot both k2’sare1incaseofe.g.k==3
    cilk_sync;
    combineAndAddComplexity(&comp, mComp1, mComp2);
    return comp;
  }

  static inline ws merge(T *a, size_t p, size_t q, size_t i0, size_t i1,
                         size_t j0, size_t j1, size_t N) {
    size_t m0, m1;
    m0 = 0;
    m1 = (j1 - j0) * (i1 - i0);
    return merger(a, p, q, i0, i1, j0, j1, m0, m1, j1 - j0, N);
  }

  inline ws transpose(T *a, size_t i0, size_t i1, size_t j0, size_t j1,
                      size_t N) {
    size_t m, n;
    ws comp = {1, 1};
    m = i1 - i0;
    n = j1 - j0;
    debug("transpose: %zd x %zd\n", m, n);
    if ((m == 1) || (n == 1)) {
      return comp;
    }
    if (m == n) {
      ws t4Comp = transpose4(a, i0, j0, 0, m, 0, n, N);
      addComplexity(&comp, t4Comp);
      return comp;
    } else if (m > n) {
      size_t im;
      if (m < 2 * n)
        im = i0 + n;
      else
        im = (i1 + i0) / 2;
      ws tComp1 = cilk_spawn transpose(a, i0, im, j0, j1, N);
      ws tComp2 = cilk_spawn transpose(a, im, i1, j0, j1, N);
      cilk_sync;
      combineAndAddComplexity(&comp, tComp1, tComp2);
      ws mComp = merge(a, im - i0, i1 - im, i0, i1, j0, j1, N);
      addComplexity(&comp, mComp);
      return comp;
    } else { // (m < n )
      size_t jm;
      if (2 * m > n)
        jm = j0 + m;
      else
        jm = (j1 + j0) / 2;
      ws tComp1 = cilk_spawn transpose(a, i0, i1, j0, jm, N);
      ws tComp2 = cilk_spawn transpose(a, i0, i1, jm, j1, N);
      cilk_sync;
      combineAndAddComplexity(&comp, tComp1, tComp2);
      ws sComp = split(a, jm - j0, j1 - jm, i0, i1, j0, j1, N);
      addComplexity(&comp, sComp);
      return comp;
    }
    return comp;
  }
};

static TRIP<uint64_t> trip;
void trip_mtp(long m, long n, uint64_t *in) {
  ws comp = trip.transpose(in, 0, m, 0, n, n);
}

int test_main() {
#define MAX_VAL 10
  long logN = 17;
  long N = 1 << logN;

  long logN1 = logN / 2;
  long logN2 = logN - logN1;
  long N1 = 1 << logN1;
  long N2 = 1 << logN2;

  uint64_t *a = new uint64_t[N];
  for (int i = 0; i < N; i++) {
    // a[i] = rand() % MAX_VAL;
    a[i] = rand();
  }

  debug("a:\n");
  mt_display(a, N1, N2);

  uint64_t *ref = new uint64_t[N];
  ref_mtp(ref, a, N1, N2);

  TRIP<uint64_t> dut;
  ws comp = dut.transpose(a, 0, N1, 0, N2, N2);
  printf("work: %ld\nspan:%ld", comp.work, comp.span);

  // uint64_t *buf = new uint64_t[std::max(N1, N2)];
  // dep_mtp(logN1, logN2, a, buf);
  debug("\ndut:\n");
  mt_display(a, N2, N1);
  debug("\nref:\n");
  mt_display(ref, N2, N1);

  diff(a, ref, N);

  delete[] a;
  delete[] ref;
  return 0;
}
