/*
 * Copyright (c) by CryptoLab inc.
 * This program is licensed under a
 * Creative Commons Attribution-NonCommercial 3.0 Unported License.
 * You should have received a copy of the license along with this
 * work.  If not, see <http://creativecommons.org/licenses/by-nc/3.0/>.
 */

#include "../src/TestScheme.h"
#include "../src/TimeUtils.h"

#include "../src/Context.h"
#include "../src/CounterUtils.h"
#include "../src/utils.h"
#include <stdlib.h>

#ifdef CONFIG_GEM5_RISCV
#include <gem5/m5ops.h>
#define STAT_RESET m5_dump_reset_stats(0, 0)
#else
#define STAT_RESET
#endif

void *aligned_malloc(size_t size, int alignment) {
  const int pointerSize = sizeof(void *);

  const int requestedSize = size + alignment - 1 + pointerSize;

#ifdef CONFIG_BIGPAGE
  void *raw = big_page_alloc(requestedSize);
#else
  void *raw = malloc(requestedSize);
#endif

  uintptr_t start = (uintptr_t)raw + pointerSize;
  void *aligned = (void *)((start + alignment - 1) & ~(alignment - 1));

  *(void **)((uintptr_t)aligned - pointerSize) = raw;

  return aligned;
}

template <typename T> void aligned_free(T *aligned_ptr) {
  if (aligned_ptr) {
    free(((T **)aligned_ptr)[-1]);
  }
}

bool isAligned(void *data, int alignment) {
  return ((uintptr_t)data & (alignment - 1)) == 0;
}

#define TEST_INIT                                                              \
  TimeUtils timeutils;                                                         \
  long k = L;                                                                  \
  Context context(logN, logp, L, k);                                           \
  CounterUtils counter;                                                        \
  uint64_t *ax = (uint64_t *)aligned_malloc(context.N * sizeof(uint64_t), 64); \
  uint64_t *ori =                                                              \
      (uint64_t *)aligned_malloc(context.N * sizeof(uint64_t), 64);            \
  if (!isAligned(ax, 64)) {                                                    \
    printf("not aligned\n");                                                   \
    exit(1);                                                                   \
  }                                                                            \
  srand(time(NULL));                                                           \
  for (int i = 0; i < context.N; ++i) {                                        \
    ori[i] = rand() % context.pVec[0];                                         \
    ax[i] = ori[i];                                                            \
  }

#define TEST_SPAN(name, code)                                                  \
  printf("---------------" name "---------------\n");                          \
  timeutils.start(name);                                                       \
  counter.start_all();                                                         \
  STAT_RESET;                                                                  \
  code;                                                                        \
  STAT_RESET;                                                                  \
  counter.end_all();                                                           \
  timeutils.stop(name);                                                        \
  counter.print_all();                                                         \
  printf("---------------" name "---------------\n");

#define TEST_WITH_DIFF(name, code)                                             \
  TEST_SPAN(name, code);                                                       \
  uint64_t *bx = (uint64_t *)aligned_malloc(context.N * sizeof(uint64_t), 64); \
  if (!isAligned(bx, 64)) {                                                    \
    printf("not aligned\n");                                                   \
    exit(1);                                                                   \
  }                                                                            \
  for (int i = 0; i < context.N; ++i) {                                        \
    bx[i] = ori[i];                                                            \
  }                                                                            \
  context.origin_qiNTTAndEqual(bx, 0);                                         \
  for (int i = 0; i < context.N; ++i) {                                        \
    if (ax[i] != bx[i]) {                                                      \
      printf(name " error in %d\n", i);                                        \
      exit(1);                                                                 \
    }                                                                          \
  }                                                                            \
  printf(name " pass\n");

#ifdef CONFIG_STEP4_NTT
void testStepNTT(long logN, long L, long logp, long logSlots, bool step_ntt) {
  TEST_INIT;
  if (step_ntt) {
    TEST_WITH_DIFF("step4-ntt", context.step4_qiNTTAndEqual(ax, 0););
  } else {
    TEST_SPAN("ori-ntt", context.origin_qiNTTAndEqual(ax, 0););
  }
  return;
}
#endif

#if defined(CONFIG_STEP4_NTT) && defined(CONFIG_RVV)
void testRVVNTT(long logN, long L, long logp, long logSlots, bool rvv) {
  TEST_INIT;
  if (rvv) {
    TEST_WITH_DIFF("rvv-step4-ntt", context.rvv_step4_qiNTTAndEqual(ax, 0););
  } else {
    TEST_WITH_DIFF("step4-ntt", context.step4_qiNTTAndEqual(ax, 0););
  }
}
#endif

void testWithRefNTT(long logN, long L, long logp, long logSlots) {
  TEST_INIT;
  TEST_WITH_DIFF("ref-ntt", context.ref_qiNTTAndEqual(ax, 0););
}

#include "../src/RVVUtil.h"
int main(int argc, char **argv) {

  // set_mod(100, 0);
  // vsetvli(8, 64, 4);
  // vbarmodmuls_vxx(v4, 1, 1);
  // exit(0);

  // TimeUtils timeutils;
  //  timeutils.start("total");
  long logN = strtol(argv[2], nullptr, 10);

  if (logN < 1) {
    printf("too small logN: %ld \n", logN);
    exit(1);
  }
  // testWithRefNTT(logN, 1, 55, 0);

#ifdef CONFIG_RVV
  if (argv[1] && argv[1][0] == '1')
    testRVVNTT(logN, 1, 55, 0, true);
  else
    testRVVNTT(logN, 1, 55, 0, false);
#else
#ifdef CONFIG_STEP4_NTT
  if (argv[1] && argv[1][0] == '1')
    testStepNTT(logN, 1, 55, 0, true);
  else
    testStepNTT(logN, 1, 55, 0, false);
#endif
#endif

  // TestScheme::testEncodeSingle(14, 1, 55);

  // TestScheme::testEncodeBatch(15, 6, 55, 3);

  // TestScheme::testBasic(15, 11, 55, 3);

  //	TestScheme::testConjugateBatch(15, 6, 55, 1);

  //	TestScheme::testRotateByPo2Batch(16, 26, 40, 1, 4, false);

  //	TestScheme::testRotateBatch(15, 6, 55, 3, 4, true);

  //	TestScheme::testimultBatch(16, 16, 55, 2);

  //	TestScheme::testPowerOf2Batch(16, 15, 50, 2, 3);

  // TestScheme::testInverseBatch(14, 5, 55, 4, 3);

  // TestScheme::testExponentBatch(14, 5, 55, 7, 3);

  // TestScheme::testSigmoidBatch(16, 15, 55, 3, 3);

  //	TestScheme::testSlotsSum(16, 15, 40, 3);

  //	TestScheme::testMeanVariance(14, 3, 55, 13);

  //	TestScheme::testHEML("data/uis.txt", 0, 5);

  // timeutils.stop("total");

  // unsigned vl = -1;
  // asm volatile("vsetvli %0, %1, e16, m8;" : "=r"(vl) : "r"(1 << 9));
  // printf("vl = 0x%x\n", vl);
  // unsigned vlenb = -1;
  // asm volatile("csrr %0, vlenb;" : "=r"(vlenb) :);
  // printf("vlenb = 0x%x\n", vlenb);
  // uint64_t vtype = -1;
  // asm volatile("csrr %0, vtype;" : "=r"(vtype) :);
  // printf("vtype = 0x%lx\n", vtype);

  return 0;
}
