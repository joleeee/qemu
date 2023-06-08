#ifndef REBG_H
#define REBG_H

FILE *rebg_log_fd(void);

#define rebg_logf(...)                      \
    do {                                        \
        fprintf(rebg_log_fd(), ## __VA_ARGS__); \
    } while (0)                                 \

#endif