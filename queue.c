#include "queue.h"

struct queue_t {
    pthread_mutex_t mutex;
    pthread_cond_t enqueueCond;
    pthread_cond_t dequeueCond;
    Task **values;
    size_t startIndex;
    size_t used;
    size_t capacity;
    Policy policy;
};

int queueLock(Queue *q) {
    if (pthread_mutex_lock(&(q->mutex)) != 0) {
        perror("pthread_mutex_lock");
        return 0;
    }
    return 1;
}

int queueUnlock(Queue *q) {
    if (pthread_mutex_unlock(&(q->mutex)) != 0) {
        perror("pthread_mutex_unlock");
        return 0;
    }
    return 1;
}

Task *taskCreate(int connfd) {
    Task *task = (Task *) malloc(sizeof(Task));
    task->connfd = connfd;
    gettimeofday(&task->timeOfArrival, NULL);
    task->threadId = -1;
    task->threadReqHandledCount = -1;
    task->threadReqHandledStaticCount = -1;
    task->threadReqHandledDynamicCount = -1;
    return task;
}

void taskDestroy(Task *task) { free(task); }

Queue *queueCreate(size_t capacity, Policy policy) {
    srand(time(NULL));
    Queue *queue = (Queue *) malloc(sizeof(Queue));
    if (queue == NULL) {
        error("Failed to allocate memory for queue");
        return NULL;
    }

    queue->capacity = capacity;
    queue->policy = policy;
    queue->used = queue->startIndex = 0;
    queue->values = (Task **) calloc(capacity, sizeof(Task *));
    if (queue->values == NULL) {
        free(queue);
        error("Failed to allocate memory for values inside the queue");
        return NULL;
    }
    if (pthread_mutex_init(&(queue->mutex), NULL) != 0) {
        error("Mutex failed to initialize");
        return NULL;
    }
    if (pthread_cond_init(&(queue->enqueueCond), NULL) != 0) {
        error("Enqueue conditional variable failed to initialize");
        return NULL;
    }
    if (pthread_cond_init(&(queue->dequeueCond), NULL) != 0) {
        error("Dequeue conditional variable failed to initialize");
        return NULL;
    }

    return queue;
}

int queuePrint(Queue *q) {
    return 0; // Comment to allow queue prints
    char str[1024] = {0};
    int printed = 0;
    for (int j = 0; j < q->capacity; j++) {
        printed += sprintf(str + printed, "%d ", q->values[j] ? q->values[j]->connfd : 0);
    }
    debug("Start index %zu. Queue: %s", q->startIndex, str);
    return 0;
}

void enqueue(Queue *q, Task *task) {
    if (!queueLock(q)) {
        return;
    }
    int val_index = 0;
    int toRemove = 0;

    while (q->capacity == q->used) {
        // TODO: Handle different policies
        switch (q->policy) {
            case BLOCK:
                warning("Queue is full. Blocking");
                pthread_cond_wait(&q->enqueueCond, &q->mutex);
                break;
            case DT:
                warning("Queue is full. Dropping request with fd %d", task->connfd);
                close(task->connfd);
                if (!queueUnlock(q)) {
                    return;
                }
                return; // Ignore the request
            case DH:
                warning("Queue is full. Dropping head with fd %d", q->values[q->startIndex]->connfd);
                close(q->values[q->startIndex]->connfd);
                taskDestroy(q->values[q->startIndex]);
                q->values[q->startIndex] = NULL;
                q->startIndex = (q->startIndex + 1) % q->capacity;
                q->used--;
                break;
            case RANDOM:
                // Remove 25% of the queue at random
                toRemove = q->used * 0.25;
                warning("Queue is full. Removing 25%% (%d) at random", toRemove);
                q->used -= toRemove; // Update the new used
                for (int i = 0, r = rand() % (q->capacity - i); i < toRemove; i++, r = rand() % (q->capacity - i)) {
                    r = (q->startIndex + r) % q->capacity;
                    close(q->values[r]->connfd);
                    taskDestroy(q->values[r]);
                    q->values[r] = NULL;
                    if (r == q->startIndex) {
                        q->startIndex = (q->startIndex + 1) % q->capacity;
                        continue;
                    }
                    for (int j = r; j != q->startIndex; j = (j + 1) % q->capacity) {
                        if (j == r) {
                            continue;
                        }
                        // Because capacity is unsigned, it's converted to unsigned and causes a weird behaviour with negative numbers
                        q->values[(q->capacity + j - 1) % q->capacity] = q->values[j];
                        q->values[j] = NULL;
                        queuePrint(q);
                    }
                }
                debug("Queue's new capacity: %zu", q->used);

                break;
        }
    }

    val_index = (q->startIndex + q->used) % q->capacity;
    q->used++;
    debug("Enqueuing fd: %d to queue of size: %zu to index: %d", task->connfd, q->used - 1, val_index);

    q->values[val_index] = task;

    if (!queueUnlock(q)) {
        return;
    }
    pthread_cond_signal(&q->dequeueCond);
}

Task *dequeue(Queue *q) {
    if (!queueLock(q)) {
        return NULL;
    }
    while (q->used == 0) {
        pthread_cond_wait(&q->dequeueCond, &q->mutex);
    }
    debug("Dequeue from queue of size: %zu from index: %zu", q->used - 1,
          q->startIndex);
    Task *value = q->values[q->startIndex];
    q->values[q->startIndex] = NULL;
    q->used--;
    q->startIndex = (q->startIndex + 1) % q->capacity;
    if (!queueUnlock(q)) {
        return NULL;
    }
    pthread_cond_signal(&q->enqueueCond);
    return value;
}