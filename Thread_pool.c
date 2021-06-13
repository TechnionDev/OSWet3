#include "Thread_pool.h"

void *thread_requestHandle(void *args_t) {
    Args *args = (Args *) args_t;
    int http_request_count = 0;
    while (1) {
        Task *task = dequeue(args->queue);
        http_request_count++;
        requestHandle(task->connfd);
        close(task->connfd);
        task_destroy(task);
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
        Args *args = args_create(pool->queue,i);//TODO:: think of when we call the args_destructor
        if (pthread_create(&(pool->threads_arr[i]), NULL, thread_requestHandle, args) != 0) {
            threadPool_destroy(pool);
            return NULL;
        }
    }
    return pool;
}

Task *task_create(int connfd) {
    Task *task = (Task *) malloc(sizeof(Task));
    task->connfd = connfd;
    gettimeofday(&task->time_of_arrival, NULL);
    return task;
}
Args *args_create(Queue *queue, int thread_id) {
    Args *args = (Args *) malloc(sizeof(Args));
    args->queue = queue;
    args->thread_id = thread_id;
    return args;
}