#ifndef REBG_H
#define REBG_H

FILE *rebg_log_fd(void);
int rebg_sock_get(void);

// for argument
void rebg_handle_filename(const char * arg);
void rebg_handle_tcp(const char * arg);

#define rebg_logf(...)                          \
    do {                                        \
        fprintf(rebg_log_fd(), ## __VA_ARGS__); \
        int s = rebg_sock_get();                \
        if(s != -1) {                           \
            dprintf(s, ## __VA_ARGS__);         \
        }                                       \
    } while (0)                                 \

#endif
