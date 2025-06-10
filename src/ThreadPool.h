#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct task {
  void (*function)(void *);
  void *arg;
  struct task *next;
} task_t;

typedef struct {
  task_t *head;
  task_t *tail;
  int size;
} task_queue_t;

typedef struct {
  pthread_t *threads;
  int thread_count;
  task_queue_t queue;
  pthread_mutex_t lock;
  pthread_cond_t cond;
  bool shutdown;
} thread_pool_t;

thread_pool_t *thread_pool_create(int num_threads);
long task_num(thread_pool_t* pool);
void thread_pool_add_task(thread_pool_t *pool, void (*function)(void *),
                          void *arg);
void thread_pool_destroy(thread_pool_t *pool);
void thread_pool_wait(thread_pool_t *pool);

#endif