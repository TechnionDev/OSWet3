#include "Thread_pool.h"

void getargs(int *port, Queue **queue, int *pool_size, int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    Policy p;
    if (strcmp(argv[4], "block") == 0) {
        p = BLOCK;
    } else if (strcmp(argv[4], "dt") == 0) {
        p = DT;
    } else if (strcmp(argv[4], "dh") == 0) {
        p = DH;
    } else if (strcmp(argv[4], "random") == 0) {
        p = RANDOM;
    } else {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    *queue = queue_create(atoi(argv[3]), p);
    *pool_size = atoi(argv[2]);
}

int main(int argc, char *argv[]) {

    int listenfd, connfd, port, clientlen, pool_size;
    struct sockaddr_in clientaddr;
    Queue *request_queue;
    getargs(&port, &request_queue, &pool_size, argc, argv);
    threadPool *pool = threadPool_create(request_queue, pool_size);

    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *) &clientaddr, (socklen_t *) &clientlen);
        Task *task = (Task *) malloc(sizeof(Task));
        task->connfd = connfd;
        enqueue(request_queue, task);
    }

    threadPool_destroy(pool);//TODO:: think when the pool is destroyed
}


    


 
