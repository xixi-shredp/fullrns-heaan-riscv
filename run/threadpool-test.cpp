#include "../src/ThreadPool.h"
#include <unistd.h>

void
print_hello(void *arg)
{
  int id = *(int *)arg;
  printf("Task %d processed by thread %lu\n", id,
         (unsigned long)pthread_self());
  free(arg);
}

int
main()
{
  thread_pool_t *pool = thread_pool_create(3);
  for (int i = 0; i < 10; i++) {
    int *arg = (int *)malloc(sizeof(int));
    *arg = i;
    thread_pool_add_task(pool, print_hello, arg);
  }
//   sleep(1); // 等待任务完成
  thread_pool_destroy(pool);
  return 0;
}