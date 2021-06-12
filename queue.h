#ifndef QUEUE_H_
#define QUEUE_H_

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include "log.h"

#ifndef qValueType
#define qValueType void *
#endif

typedef struct {
    pthread_mutex_t mutex;
    qValueType *values;
    size_t start_index;
    size_t used;
    size_t capacity;
} Queue;

void enqueue(Queue *q, qValueType value) {
    pthread_mutex_lock(&(q->mutex));
    int val_index = (q->start_index + (++q->used)) % q->capacity;
    debug("Enqueue to queue of size: %d to index: %d", q->used - 1, val_index);

    // TODO: Handle different policies
    q->values[val_index] = value;

    pthread_mutex_unlock(&(q->mutex));
}

qValueType dequeue(Queue q) {
    pthread_mutex_lock(&(q->mutex));
    debug("Dequeue from queue of size: %d from index: %d", q->used - 1,
          q->start_index);
    if (q->used == 0) {
        return NULL;
    }
    qValueType value = q->values[q->start_index];

    q->start_index++;
    q->used--;
    q->start_index %= q->capacity;
    pthread_mutex_unlock(&(q->mutex));
}

#endif