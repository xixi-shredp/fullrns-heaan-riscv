#include "ThreadPool.h"
#include <pthread.h>

void *worker_function(void *arg);

thread_pool_t *
thread_pool_create(int num_threads)
{
  thread_pool_t *pool = (thread_pool_t *)malloc(sizeof(thread_pool_t));

  pthread_mutex_init(&pool->lock, NULL);
  pthread_cond_init(&pool->cond, NULL);
  pool->shutdown = false;
  pool->queue.head = pool->queue.tail = NULL;
  pool->queue.size = 0;
  pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
  pool->thread_count = num_threads;

  for (int i = 0; i < num_threads; i++) {
    pthread_create(&pool->threads[i], NULL, worker_function, pool);
  }
  return pool;
}

void *
worker_function(void *arg)
{
  thread_pool_t *pool = (thread_pool_t *)arg;
  while (true) {
    pthread_mutex_lock(&pool->lock);
    // 等待条件变量：任务队列非空或关闭
    while (pool->queue.size == 0 && !pool->shutdown) {
      pthread_cond_wait(&pool->cond, &pool->lock);
    }
    // 如果关闭且任务队列为空，退出
    if (pool->shutdown && pool->queue.size == 0) {
      pthread_mutex_unlock(&pool->lock);
      break;
    }
    // 取出任务
    task_t *task = pool->queue.head;
    if (task) {
      pool->queue.head = task->next;
      pool->queue.size--;
      if (pool->queue.head == NULL) {
        pool->queue.tail = NULL;
      }
    }
    pthread_mutex_unlock(&pool->lock);
    // 执行任务
    if (task) {
      task->function(task->arg);
      free(task);
    }
  }
  return NULL;
}

long task_num(thread_pool_t* pool){
    long n = 0;
    pthread_mutex_lock(&pool->lock);
    task_t *task = pool->queue.head;
    while (task) {
      task = task->next;
      n++;
    }
    pthread_mutex_unlock(&pool->lock);
    return n;
}

void
thread_pool_add_task(thread_pool_t *pool, void (*function)(void *), void *arg)
{
  task_t *task = (task_t *)malloc(sizeof(task_t));
  task->function = function;
  task->arg = arg;
  task->next = NULL;
  pthread_mutex_lock(&pool->lock);
  if (pool->queue.tail) {
    pool->queue.tail->next = task;
  } else {
    pool->queue.head = task;
  }
  pool->queue.tail = task;
  pool->queue.size++;
  pthread_cond_signal(&pool->cond); // 通知一个等待的线程
  pthread_mutex_unlock(&pool->lock);
}

void
thread_pool_destroy(thread_pool_t *pool)
{
  pthread_mutex_lock(&pool->lock);
  pool->shutdown = true;
  pthread_cond_broadcast(&pool->cond); // 唤醒所有线程
  pthread_mutex_unlock(&pool->lock);
  // 等待所有线程结束
  for (int i = 0; i < pool->thread_count; i++) {
    pthread_join(pool->threads[i], NULL);
  }
  // 释放资源
  free(pool->threads);
  // 销毁互斥锁和条件变量
  pthread_mutex_destroy(&pool->lock);
  pthread_cond_destroy(&pool->cond);
  // 释放未处理的任务（如果有）
  task_t *current = pool->queue.head;
  while (current) {
    task_t *next = current->next;
    free(current);
    current = next;
  }
  free(pool);
}

void
thread_pool_wait(thread_pool_t *pool)
{
  while (1) {
    pthread_mutex_lock(&pool->lock);
    // pthread_cond_broadcast(&pool->cond); // 唤醒所有线程
    if (pool->queue.size == 0) {
      pthread_mutex_unlock(&pool->lock);
      return;
    }
    pthread_mutex_unlock(&pool->lock);
  }
}