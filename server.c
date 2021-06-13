#include "Thread_pool.h"

#define USAGE_TEXT "Usage: %s <port> <workers> <buffer_size> <policy>\n"

void getargs(int *port, Queue **queue, int *pool_size, int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, USAGE_TEXT, argv[0]);
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
        fprintf(stderr, USAGE_TEXT,
                argv[0]);
        exit(1);
    }
    debug("Policy is %d", p);
    *queue = queueCreate(atoi(argv[3]), p);
    *pool_size = atoi(argv[2]);
}

int main(int argc, char *argv[]) {
    int listenfd, connfd, port, clientlen, pool_size;
    struct sockaddr_in clientaddr;
    Queue *request_queue;
    getargs(&port, &request_queue, &pool_size, argc, argv);
    threadPool_create(request_queue, pool_size);

    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
        Task *task = taskCreate(connfd);
        enqueue(request_queue, task);
    }
}
