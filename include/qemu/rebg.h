#ifndef REBG_H
#define REBG_H

FILE *rebg_log_fd(void);

// for argument
void rebg_handle_filename(const char * arg);

#define rebg_logf(...)                          \
    do {                                        \
        fprintf(rebg_log_fd(), ## __VA_ARGS__); \
    } while (0)                                 \

#endif
