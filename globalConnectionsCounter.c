//
// Created by Gur Telem on 17/06/2021.
//

#include "globalConnectionsCounter.h"
#include "log.h"

pthread_mutex_t counterMutex;
int mutexInitialized = 0;
int globalConnectionsCounter = 0;

void destroyConnCounter(void) {
    if (mutexInitialized) {
        pthread_mutex_destroy(&counterMutex);
        mutexInitialized = 0;
    }
}

void initConnCounter(void) {
    destroyConnCounter();

    if (pthread_mutex_init(&counterMutex, NULL) != 0) {
        error("Mutex failed to initialize");
        return;
    }
    globalConnectionsCounter = 0;
    mutexInitialized = 1;
}

int getConnCounter(void) {
    return globalConnectionsCounter;
}

void incConnCounter(void) {
    pthread_mutex_lock(&counterMutex);
    globalConnectionsCounter++;
    pthread_mutex_unlock(&counterMutex);
}

void decConnCounter(void) {
    pthread_mutex_lock(&counterMutex);
    globalConnectionsCounter--;
    pthread_mutex_unlock(&counterMutex);
}

