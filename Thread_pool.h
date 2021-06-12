#ifndef TEST__THREAD_POOL_H_
#define TEST__THREAD_POOL_H_
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "request.h"
#include "segel.h"
typedef struct {
  int connfd;
} Task;//TODO: move to Queue.h
typedef struct {} Queue;//TODO: remove when gur pushes Queue.h

#define qValueType Task *
typedef enum { BLOCK, DT, DH, RANDOM } Policy;//TODO: move to Queue.h
qValueType dequeue(Queue *q);//TODO: remove when gur pushes Queue.h
void enqueue(Queue *q, qValueType value);//TODO: remove when gur pushes Queue.h
Queue *queue_create(int size_queue, Policy p);//TODO: remove when gur pushes Queue.h


typedef struct {
  pthread_t *threads_arr;
  Queue *queue;
  int pool_size;
} threadPool;

void thread_requestHandle(Queue *queue);
void threadPool_destroy(threadPool *pool);
threadPool *threadPool_create(Queue *queue, int size);

#endif //TEST__THREAD_POOL_H_
