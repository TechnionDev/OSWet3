#ifndef QUEUE_H_
#define QUEUE_H_

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include "log.h"

#ifndef qValueType
// Defines that should be overloaded according to the type of values stored in the queue
#define qValueType void *
#define qNullValue NULL
#define qFreeValue(x) free(x)
#endif

typedef enum { BLOCK, DT, DH, RANDOM } Policy;

typedef struct {
    pthread_mutex_t mutex;
    qValueType *values;
    size_t start_index;
    size_t used;
    size_t capacity;
    Policy policy;
} Queue;

Queue *queueCreate(size_t capacity, Policy policy) {
    srand(time(NULL));
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    if (queue == NULL) {
        error("Failed to allocate memory for queue");
        return NULL;
    }

    queue->capacity = capacity;
    queue->policy = policy;
    queue->used = queue->start_index = 0;
    queue->values = (qValueType *)calloc(capacity, sizeof(qValueType));
    if (queue->values == NULL) {
        free(queue);
        error("Failed to allocate memory for values inside the queue");
        return NULL;
    }
    if (pthread_mutex_init(&(queue->mutex), NULL) != 0) {
        error("Mutex failed to initialize");
        return NULL;
    }

    return queue;
}

void enqueue(Queue *q, qValueType value) {
    if (pthread_mutex_lock(&(q->mutex)) != 0) {
        perror("pthread_mutex_lock");
        return;
    }
    int val_index = 0;

    if (q->capacity == q->used) {
        // TODO: Handle different policies
        switch (q->policy) {
            case BLOCK:
                // TODO: implement with a semaphore
                break;
            case DT:
                return;  // Ignore the request
            case DH:
                q->start_index++;
                q->used--;
                break;
            case RANDOM:
                // Remove 25% of the queue at random
                int to_remove = q->used * 0.25;
                q->used -= to_remove;  // Update the new used
                for (int i = q->start_index, c = q->start_index, r = rand();
                     i < q->capacity; i = (i + 1) % q->capacity, r = rand()) {
                    if (to_remove / (double)(q->capacity - i) >
                        r / (double)RAND_MAX) {
                        to_remove--;
                        continue;
                    }
                    q->values[c = (c + 1) % q->capacity] = q->values[i];
                }
                break;
        }

    } else {
        val_index = (q->start_index + (++q->used)) % q->capacity;
        debug("Enqueue to queue of size: %d to index: %d", q->used - 1,
              val_index);
    }

    q->values[val_index] = value;

    if (pthread_mutex_unlock(&(q->mutex)) != 0) {
        perror("pthread_mutex_unlock");
        return;
    }
}

qValueType dequeue(Queue *q) {
    if (pthread_mutex_lock(&(q->mutex)) != 0) {
        perror("pthread_mutex_lock");
        return qNullValue;
    }
    debug("Dequeue from queue of size: %d from index: %d", q->used - 1,
          q->start_index);
    if (q->used == 0) {
        return qNullValue;
    }
    qValueType value = q->values[q->start_index];

    q->start_index++;
    q->used--;
    q->start_index %= q->capacity;
    if (pthread_mutex_unlock(&(q->mutex)) != 0) {
        perror("pthread_mutex_unlock");
        return qNullValue;
    }
}

#endif