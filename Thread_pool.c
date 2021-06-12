#include "Thread_pool.h"

void thread_requestHandle(Queue *queue) {
    while (1) {
        Task *task = dequeue(queue);
        requestHandle(task->connfd);
        close(task->connfd);
        free(task);
    }
}

void threadPool_destroy(threadPool *pool) {
    if (pool == NULL) {
        return;
    }
    for (int i = 0; i < pool->pool_size; i++) {
        pthread_cancel(pool->threads_arr[i]);
    }
    free(pool->threads_arr);
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
    pool->pool_size = size;
    pool->queue = queue;
    pool->threads_arr = (pthread_t *) malloc(sizeof(pthread_t) * size);
    if (pool->threads_arr == NULL) {
        free(pool);
        return NULL;
    }

    for (int i = 0; i < size; i++) {
        if (pthread_create(&(pool->threads_arr[i]), NULL, thread_requestHandle, (Queue *) pool->queue) != 0) {
            threadPool_destroy(pool);
            return NULL;
        }
    }
    return pool;
}
