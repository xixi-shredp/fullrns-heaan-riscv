#include "../src/TimeUtils.h"

#include "../src/Context.h"
#include "../src/CounterUtils.h"
#include <stdlib.h>

void *
aligned_malloc(size_t size, int alignment)
{
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

template <typename T>
void
aligned_free(T *aligned_ptr)
{
  if (aligned_ptr) {
    free(((T **)aligned_ptr)[-1]);
  }
}

bool
isAligned(void *data, int alignment)
{
  return ((uintptr_t)data & (alignment - 1)) == 0;
}

#define TEST_INIT                                                   \
  TimeUtils timeutils;                                              \
  long k = L;                                                       \
  Context context(logN, logp, L, k);                                \
  CounterUtils counter;                                             \
  uint64_t *ax =                                                    \
      (uint64_t *)aligned_malloc(context.N * sizeof(uint64_t), 64); \
  uint64_t *ori =                                                   \
      (uint64_t *)aligned_malloc(context.N * sizeof(uint64_t), 64); \
  if (!isAligned(ax, 64)) {                                         \
    printf("not aligned\n");                                        \
    exit(1);                                                        \
  }                                                                 \
  srand(time(NULL));                                                \
  for (int i = 0; i < context.N; ++i) {                             \
    ori[i] = rand() % context.pVec[0];                              \
    ax[i] = ori[i];                                                 \
  }

#define TEST(code)         \
  timeutils.start("test"); \
  counter.start_all();     \
  STAT_RESET;              \
  code;                    \
  STAT_RESET;              \
  counter.end_all();       \
  timeutils.stop("test");  \
  counter.print_all();

#define TEST_DIFF()                                                 \
  uint64_t *bx =                                                    \
      (uint64_t *)aligned_malloc(context.N * sizeof(uint64_t), 64); \
  if (!isAligned(bx, 64)) {                                         \
    printf("not aligned\n");                                        \
    exit(1);                                                        \
  }                                                                 \
  for (int i = 0; i < context.N; ++i) {                             \
    bx[i] = ori[i];                                                 \
  }                                                                 \
  context.origin_qiNTTAndEqual_withMont(bx, 0);                     \
  for (int i = 0; i < context.N; ++i) {                             \
    if (ax[i] != bx[i]) {                                           \
      printf("error in %d\n", i);                                   \
      exit(1);                                                      \
    }                                                               \
  }                                                                 \
  printf("pass\n");

#ifdef CONFIG_NATIVE
  #define RVV_CASES(_)
#else
  #define RVV_CASES(_)                                      \
    _(rvv_ori_bar, rvv_ori_qiNTTAndEqual_withBar)           \
    _(rvv_ori_mont, rvv_ori_qiNTTAndEqual_withMont)         \
    _(rvv_step4_bar, rvv_step4_qiNTTAndEqual_withBar)       \
    _(rvv_step4_mont, rvv_step4_qiNTTAndEqual_withMont)     \
    _(mt_rvv_ori_bar, mt_rvv_ori_qiNTTAndEqual_withBar)           \
    _(mt_rvv_step4_bar, mt_rvv_step4_qiNTTAndEqual_withBar) \
    _(mt_rvv_step4_mont, mt_rvv_step4_qiNTTAndEqual_withMont)
#endif

#define FHE_CASES(_)                                              \
  _(ext_ori_bar, ext_qiNTTAndEqual_withBar)                       \
  _(ext_ori_mont, ext_qiNTTAndEqual_withMont)                     \
  _(ext_step4_bar, ext_step4_qiNTTAndEqual_withBar)               \
  _(ext_step4_mont, ext_step4_qiNTTAndEqual_withMont)             \
  _(rvv_ext_ori_bar, rvv_ext_ori_qiNTTAndEqual_withBar)           \
  _(rvv_ext_ori_mont, rvv_ext_ori_qiNTTAndEqual_withMont)         \
  _(rvv_ext_step4_bar, rvv_ext_step4_qiNTTAndEqual_withBar)       \
  _(rvv_ext_step4_mont, rvv_ext_step4_qiNTTAndEqual_withMont)     \
  _(mt_rvv_ext_step4_bar, mt_rvv_ext_step4_qiNTTAndEqual_withBar) \
  _(mt_rvv_ext_step4_mont, mt_rvv_ext_step4_qiNTTAndEqual_withMont)

#define BASE_CASES(_)                         \
  _(ori_bar, origin_qiNTTAndEqual_withBar)    \
  _(ori_mont, origin_qiNTTAndEqual_withMont)  \
  _(step4_bar, step4_qiNTTAndEqual_withBar)   \
  _(step4_mont, step4_qiNTTAndEqual_withMont) \
  RVV_CASES(_)

#ifdef CONFIG_FHE_EXT
  #define CASES(_) BASE_CASES(_) FHE_CASES(_)
#else
  #define CASES(_) BASE_CASES(_)
#endif

#define TEST_FUNC_DECLARE(name, func)                                      \
  void test_##name(bool diff, long logN, long L, long logp, long logSlots) \
  {                                                                        \
    TEST_INIT;                                                             \
    TEST(context.func(ax, 0));                                             \
    if (!diff) return;                                                     \
    TEST_DIFF();                                                           \
    return;                                                                \
  }

#define MAP_DEFINE(name, func) {#name, test_##name},

CASES(TEST_FUNC_DECLARE);

// typedef void (*ntt_func_t)(uint64_t *, long, long, long, long);
typedef decltype(test_ori_bar) ntt_func_t;

void
testCase(bool need_check, long logN, long logp, string case_name,
         long nr_thread)
{

  ntt_func_t *case_func;
  map<string, ntt_func_t *> func_map = {CASES(MAP_DEFINE)};

  long L = 1;
  long logSlots = 0;

  auto cur_case = func_map.find(case_name);
  if (cur_case == func_map.end()) {
    printf("[error]: don't have this test case\n");
    exit(1);
  } else {
    case_func = cur_case->second;
  }
  case_func(need_check, logN, L, logp, logSlots);
}
