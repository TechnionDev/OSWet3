#ifndef QUEUE_H_
#define QUEUE_H_

#include "log.h"
#include "segel.h"

typedef struct {
    int connfd;
    struct timeval timeOfArrival;
    struct timeval dispathTime;
    int threadId;
    int threadReqHandledCount;
    int threadReqHandledStaticCount;
    int threadReqHandledDynamicCount;
} Task;

typedef enum {
    BLOCK, DT, DH, RANDOM
} Policy;

typedef struct queue_t Queue;

Task *taskCreate(int connfd);

void taskDestroy(Task *task);

Queue *queueCreate(size_t capacity, Policy policy);

void enqueue(Queue *q, Task *taks);

Task *dequeue(Queue *q);

#endif