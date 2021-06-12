#ifndef LOG_H_
#define LOG_H_
#include <stdio.h>

#define log(X, ...)                                                     \
    do {                                                                \
        printf("%s - %s:%d %s> ", __FILE__, __FUNCTION__, __LINE__, X); \
        printf(__VA_ARGS__);                                            \
        printf("\n");                                                   \
    } while (0)

#define DEBUG  // TODO: Comment out

#ifdef DEBUG
#define debug(...)                 \
    do {                           \
        log("DEBUG", __VA_ARGS__); \
    } while (0)
#else
#define DEBUG_PRINT(...) while (0)
#endif

#define error(...)                 \
    do {                           \
        log("ERROR", __VA_ARGS__); \
    } while (0)

#define info(...)                 \
    do {                          \
        log("INFO", __VA_ARGS__); \
    } while (0)

#define warning(...)                 \
    do {                             \
        log("WARNING", __VA_ARGS__); \
    } while (0)

#endif