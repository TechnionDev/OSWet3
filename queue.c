#include "queue.h"

struct queue_t
{
    pthread_mutex_t mutex;
    pthread_cond_t enqueueCond;
    pthread_cond_t dequeueCond;
    Task **values;
    size_t startIndex;
    size_t used;
    size_t capacity;
    Policy policy;
};

int queueLock(Queue *q)
{
    if (pthread_mutex_lock(&(q->mutex)) != 0)
    {
        perror("pthread_mutex_lock");
        return 0;
    }
    return 1;
}

int queueUnlock(Queue *q)
{
    if (pthread_mutex_unlock(&(q->mutex)) != 0)
    {
        perror("pthread_mutex_unlock");
        return 0;
    }
    return 1;
}

Task *taskCreate(int connfd)
{
    Task *task = (Task *)malloc(sizeof(Task));
    task->connfd = connfd;
    gettimeofday(&task->time_of_arrival, NULL);
    return task;
}

void task_destroy(Task *task) { free(task); }

Queue *queueCreate(size_t capacity, Policy policy)
{
    srand(time(NULL));
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    if (queue == NULL)
    {
        error("Failed to allocate memory for queue");
        return NULL;
    }

    queue->capacity = capacity;
    queue->policy = policy;
    queue->used = queue->startIndex = 0;
    queue->values = (Task **)calloc(capacity, sizeof(Task *));
    if (queue->values == NULL)
    {
        free(queue);
        error("Failed to allocate memory for values inside the queue");
        return NULL;
    }
    if (pthread_mutex_init(&(queue->mutex), NULL) != 0)
    {
        error("Mutex failed to initialize");
        return NULL;
    }
    if (pthread_cond_init(&(queue->enqueueCond), NULL) != 0)
    {
        error("Enqueue conditional variable failed to initialize");
        return NULL;
    }
    if (pthread_cond_init(&(queue->dequeueCond), NULL) != 0)
    {
        error("Dequeue conditional variable failed to initialize");
        return NULL;
    }

    return queue;
}

void enqueue(Queue *q, Task *task)
{
    if (!queueLock(q))
    {
        return;
    }
    int val_index = 0;
    int toRemove = 0;

    while (q->capacity == q->used)
    {
        // TODO: Handle different policies
        switch (q->policy)
        {
        case BLOCK:
            warning("Queue is full. Blocking");
            pthread_cond_wait(&q->enqueueCond, &q->mutex);
            break;
        case DT:
            warning("Queue is full. Dropping request with fd %d", task->connfd);
            close(task->connfd);
            if (!queueUnlock(q))
            {
                return;
            }
            return; // Ignore the request
        case DH:
            warning("Queue is full. Dropping head with fd %d", q->values[q->startIndex]->connfd);
            close(q->values[q->startIndex]->connfd);
            task_destroy(q->values[q->startIndex]);
            q->values[q->startIndex] = NULL;
            q->startIndex = (q->startIndex + 1) % q->capacity;
            q->used--;
            break;
        case RANDOM:
            warning("Queue is full. Removing 25%% at random");
            // Remove 25% of the queue at random
            toRemove = q->used * 0.25;
            q->used -= toRemove; // Update the new used
            for (int i = q->startIndex, c = q->startIndex, r = rand();
                 i < q->capacity; i = (i + 1) % q->capacity, r = rand())
            {
                if (toRemove / (double)(q->capacity - i) >
                    r / (double)RAND_MAX)
                {
                    toRemove--;
                    debug("Dropping at %d fd %d", i, q->values[i]->connfd);
                    continue;
                }
                q->values[c = (c + 1) % q->capacity] = q->values[i];
            }

            break;
        }
    }
    val_index = (q->startIndex + (q->used++)) % q->capacity;
    debug("Enqueuing fd: %d", task->connfd);
    debug("Enqueue to queue of size: %zu to index: %d", q->used - 1, val_index);

    q->values[val_index] = task;

    if (!queueUnlock(q))
    {
        return;
    }
    pthread_cond_signal(&q->dequeueCond);
}

Task *dequeue(Queue *q)
{
    if (!queueLock(q))
    {
        return NULL;
    }
    while (q->used == 0)
    {
        pthread_cond_wait(&q->dequeueCond, &q->mutex);
    }
    debug("Dequeue from queue of size: %zu from index: %zu", q->used - 1,
          q->startIndex);
    Task *value = q->values[q->startIndex];
    q->values[q->startIndex] = NULL;
    q->used--;
    q->startIndex = (q->startIndex + 1) % q->capacity;
    if (!queueUnlock(q))
    {
        return NULL;
    }
    pthread_cond_signal(&q->enqueueCond);
    return value;
}