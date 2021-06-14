#ifndef TEST__THREAD_POOL_H_
#define TEST__THREAD_POOL_H_

#include "request.h"
#include "segel.h"
#include "queue.h"

typedef struct {
    pthread_t *threadsArr;
    Queue *queue;
    int poolSize;
} threadPool;

void *thread_requestHandle(void *args);

void threadPool_destroy(threadPool *pool);

threadPool *threadPool_create(Queue *queue, int size);

typedef struct {
    Queue *queue;
    int threadId;
} Args;

Args *args_create(Queue *queue, int thread_id);

void args_destroy(Args *args);

#endif  // TEST__THREAD_POOL_H_
