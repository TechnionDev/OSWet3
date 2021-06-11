#include "segel.h"
#include "request.h"
#include <pthread.h>
// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//
typedef struct {} tasks_queue;

void thread_requestHandle(){
}


typedef struct {
  pthread_t *threads_arr;
  tasks_queue *queue;
  int pool_size;
} threadPool;

void threadPool_destroy(threadPool *pool) {
    if(pool==NULL){
        return;
    }
    for(int i =0;i<pool->pool_size;i++){
        pthread_cancel(pool->threads_arr[i]);
    }
    free(pool->threads_arr);
    free(pool);
}

threadPool *threadPool_create(tasks_queue *queue, int size) {
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
        if (pthread_create(&(pool->threads_arr[i]), NULL, thread_requestHandle, (tasks_queue *) pool->queue) != 0) {
            threadPool_destroy(pool);
            return NULL;
        }
    }
    return pool;
}



// HW3: Parse the new arguments too
void getargs(int *port, int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
}

int main(int argc, char *argv[]) {

    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;

    getargs(&port, argc, argv);

    // 
    // HW3: Create some threads...
    //

    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *) &clientaddr, (socklen_t *) &clientlen);

        //
        // HW3: In general, don't handle the request in the main thread.
        // Save the relevant info in a buffer and have one of the worker threads
        // do the work.
        //
        requestHandle(connfd);

        Close(connfd);
    }

}


    


 
