#include "Thread_pool.h"

void *thread_requestHandle(void *args_t) {
    Args *args = (Args *) args_t;
    int httpRequestCount = 0;
    int httpRequestStaticCount = 0;
    int httpRequestDynamicCount = 0;
    while (1) {
        Task *task = dequeue(args->queue);
        debug("Start handling %d", task->connfd);
        httpRequestCount++;
        gettimeofday(&task->dispathTime, NULL);
        task->threadId = args->threadId;
        task->threadReqHandledCount = httpRequestCount;
        task->threadReqHandledStaticCount = httpRequestStaticCount;
        task->threadReqHandledDynamicCount = httpRequestDynamicCount;
        requestHandle(task);
        httpRequestStaticCount = task->threadReqHandledStaticCount;
        httpRequestDynamicCount = task->threadReqHandledDynamicCount;
        debug("Finish handling %d", task->connfd);
        close(task->connfd);
        taskDestroy(task);
    }
}

void threadPool_destroy(threadPool *pool) {
    if (pool == NULL) {
        return;
    }
    for (int i = 0; i < pool->poolSize; i++) {
        pthread_cancel(pool->threadsArr[i]);
    }
    free(pool->threadsArr);
    free(pool);
}

threadPool *threadPool_create(Queue *queue, int size) {
    if (size <= 0 || queue == NULL) {
        return NULL;
    }
    threadPool *pool = (threadPool *) malloc(sizeof(threadPool));
    if (pool == NULL) {
        return pool;
    }
    pool->poolSize = size;
    pool->queue = queue;
    pool->threadsArr = (pthread_t *) malloc(sizeof(pthread_t) * size);
    if (pool->threadsArr == NULL) {
        free(pool);
        return NULL;
    }

    for (int i = 0; i < size; i++) {
        Args *args = args_create(pool->queue, i);
        if (pthread_create(&(pool->threadsArr[i]), NULL, thread_requestHandle,
                           args) != 0) {
            threadPool_destroy(pool);
            return NULL;
        }
    }
    return pool;
}

Args *args_create(Queue *queue, int thread_id) {
    Args *args = (Args *) malloc(sizeof(Args));
    args->queue = queue;
    args->threadId = thread_id;
    return args;
}
