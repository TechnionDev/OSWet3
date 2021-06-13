#ifndef LOG_H_
#define LOG_H_

#define LOG(X, ...)                                                     \
    do {                                                                \
        printf("%s - %s:%d %s> ", __FILE__, __FUNCTION__, __LINE__, X); \
        printf(__VA_ARGS__);                                            \
        printf("\n");                                                   \
    } while (0)

#define DEBUG  // TODO: Comment out

#ifdef DEBUG
#define debug(...)                 \
    do {                           \
        LOG("DEBUG", __VA_ARGS__); \
    } while (0)
#else
#define DEBUG_PRINT(...) while (0)
#endif

#define error(...)                 \
    do {                           \
        LOG("ERROR", __VA_ARGS__); \
    } while (0)

#define info(...)                 \
    do {                          \
        LOG("INFO", __VA_ARGS__); \
    } while (0)

#define warning(...)                 \
    do {                             \
        LOG("WARNING", __VA_ARGS__); \
    } while (0)

#endif