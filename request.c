//
// request.c: Does the bulk of the work for the web server.
// 

#include "segel.h"
#include "request.h"


void addStatsHeaders(char *buf, Task *task) {
    long long arrival, dispatch;
    arrival = task->timeOfArrival.tv_sec * 1000 + task->timeOfArrival.tv_usec / 1000;
    dispatch = (task->dispathTime.tv_sec * 1000 + task->dispathTime.tv_usec/ 1000) - arrival;
    sprintf(buf, "%sStat-req-arrival: %lld\r\n", buf, arrival);
    sprintf(buf, "%sStat-req-dispatch: %lld\r\n", buf, dispatch);
    sprintf(buf, "%sStat-thread-id: %d\r\n", buf, task->threadId);
    sprintf(buf, "%sStat-thread-count: %d\r\n", buf, task->threadReqHandledCount);
    sprintf(buf, "%sStat-thread-static: %d\r\n", buf, task->threadReqHandledStaticCount);
    sprintf(buf, "%sStat-thread-dynamic: %d\r\n", buf, task->threadReqHandledDynamicCount);
}

// requestError(      fd,    filename,        "404",    "Not found", "OS-HW3 Server could not find this file");
void requestError(Task *task, char *cause, char *errnum, char *shortmsg, char *longmsg) {
    char buf[MAXLINE], body[MAXBUF];

    // Create the body of the error message
    sprintf(body, "<html><title>OS-HW3 Error</title>");
    sprintf(body, "%s<body bgcolor=""fffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr>OS-HW3 Web Server\r\n", body);

    // Write out the header information for this response
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(task->connfd, buf, strlen(buf));
    printf("%s", buf);

    sprintf(buf, "Content-Type: text/html\r\n");
    Rio_writen(task->connfd, buf, strlen(buf));
    printf("%s", buf);

    sprintf(buf, "Content-Length: %lu\r\n\r\n", strlen(body));
    Rio_writen(task->connfd, buf, strlen(buf));
    printf("%s", buf);

    // Add stats headers
    buf[0] = '\0';
    addStatsHeaders(buf, task);
    Rio_writen(task->connfd, buf, strlen(buf));
    printf("%s", buf);

    // Write out the content
    Rio_writen(task->connfd, body, strlen(body));
    printf("%s", body);

}


//
// Reads and discards everything up to an empty text line
//
void requestReadhdrs(rio_t *rp) {
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    while (strcmp(buf, "\r\n")) {
        Rio_readlineb(rp, buf, MAXLINE);
    }
}

//
// Return 1 if static, 0 if dynamic content
// Calculates filename (and cgiargs, for dynamic) from uri
//
int requestParseURI(char *uri, char *filename, char *cgiargs) {
    char *ptr;

    if (strstr(uri, "..")) {
        sprintf(filename, "./public/home.html");
        return 1;
    }

    if (!strstr(uri, "cgi")) {
        // static
        strcpy(cgiargs, "");
        sprintf(filename, "./public/%s", uri);
        if (uri[strlen(uri) - 1] == '/') {
            strcat(filename, "home.html");
        }
        return 1;
    } else {
        // dynamic
        ptr = index(uri, '?');
        if (ptr) {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        } else {
            strcpy(cgiargs, "");
        }
        sprintf(filename, "./public/%s", uri);
        return 0;
    }
}

//
// Fills in the filetype given the filename
//
void requestGetFiletype(char *filename, char *filetype) {
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}

void requestServeDynamic(Task *task, char *filename, char *cgiargs) {
    char buf[MAXLINE], *emptylist[] = {NULL};

    // The server does only a little bit of the header.
    // The CGI script has to finish writing out the header.
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: OS-HW3 Web Server\r\n", buf);
    addStatsHeaders(buf, task);

    Rio_writen(task->connfd, buf, strlen(buf));

    if (Fork() == 0) {
        /* Child process */
        Setenv("QUERY_STRING", cgiargs, 1);
        /* When the CGI process writes to stdout, it will instead go to the socket */
        Dup2(task->connfd, STDOUT_FILENO);
        Execve(filename, emptylist, environ);
    }
    Wait(NULL);
}


void requestServeStatic(Task *task, char *filename, int filesize) {
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    requestGetFiletype(filename, filetype);

    srcfd = Open(filename, O_RDONLY, 0);

    // Rather than call read() to read the file into memory,
    // which would require that we allocate a buffer, we memory-map the file
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);

    // put together response
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: OS-HW3 Web Server\r\n", buf);
    addStatsHeaders(buf, task);
    sprintf(buf, "%sContent-Length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-Type: %s\r\n\r\n", buf, filetype);


    Rio_writen(task->connfd, buf, strlen(buf));

    //  Writes out to the client socket the memory-mapped file
    Rio_writen(task->connfd, srcp, filesize);
    Munmap(srcp, filesize);

}

// handle a request
void requestHandle(Task *task) {

    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, task->connfd);
    Rio_readlineb(&rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, uri, version);

    printf("%s %s %s\n", method, uri, version);

    if (strcasecmp(method, "GET")) {
        requestError(task, method, "501", "Not Implemented", "OS-HW3 Server does not implement this method");
        return;
    }
    requestReadhdrs(&rio);

    is_static = requestParseURI(uri, filename, cgiargs);
    if (stat(filename, &sbuf) < 0) {
        requestError(task, filename, "404", "Not found", "OS-HW3 Server could not find this file");
        return;
    }

    if (is_static) {
        task->threadReqHandledStaticCount++;
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            requestError(task, filename, "403", "Forbidden", "OS-HW3 Server could not read this file");
            return;
        }
        requestServeStatic(task, filename, sbuf.st_size);
    } else {
        task->threadReqHandledDynamicCount++;
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
            requestError(task, filename, "403", "Forbidden", "OS-HW3 Server could not run this CGI program");
            return;
        }
        requestServeDynamic(task, filename, cgiargs);
    }
}


