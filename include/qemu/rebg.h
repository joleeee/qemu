#ifndef REBG_H
#define REBG_H

#include <errno.h>

FILE *rebg_log_fd(void);
int rebg_sock_get(void);
char * rebg_savebuf_get(void);
void rebg_savebuf_clear(void);

// for argument
void rebg_handle_filename(const char * arg);
void rebg_handle_tcp(const char * arg);
void rebg_helper_writeall(int fildes, const void *buf, size_t nbyte);

#define rebg_logf(...)                          \
    do {                                        \
        fprintf(rebg_log_fd(), ## __VA_ARGS__); \
        int s = rebg_sock_get();                \
        if(s != -1 && false) {                  \
            dprintf(s, ## __VA_ARGS__);         \
        }                                       \
    } while (0)                                 \

#define rebg_save_logf(...)                     \
    do {                                        \
        char * _buf = rebg_savebuf_get();       \
        int _curlen = strlen(_buf);             \
        snprintf(_buf + _curlen, 0x100-_curlen, ## __VA_ARGS__); \
    } while (0)                                 \

// analogous to write() but for both log and socket
#define rebg_write(buf, len)                    \
    do {                                        \
        fwrite(buf, 1, len, rebg_log_fd());     \
        int s = rebg_sock_get();                \
        if(s != -1) {                           \
            rebg_helper_writeall(s, buf, len);  \
        }                                       \
    } while (0)                                 \

enum MESSAGE {
    // ()
    SEPARATOR = 0x55,
    // (u64, string), u64, u64
    LIBLOAD = 0xee,
    // u64
    ADDRESS = 0xaa,
    // (len, bytes),
    CODE = 0xff,
    // size u8, adr u64, value u64
    LOAD = 0x33,
    STORE = 0x44,
    // 
    REGISTERS = 0x77,
    // FLAGS = 0x78,
    //
    SYSCALL = 0x99,
    SYSCALL_RESULT = 0x9a,
};

void rebg_send_separator(void);
void rebg_send_load(uint64_t address, uint64_t value, uint8_t size);
void rebg_send_store(uint64_t address, uint64_t value, uint8_t size);
void rebg_send_libload(const char * file, uint64_t from, uint64_t to);
void rebg_send_address(uint64_t adr);
void rebg_send_code(uint8_t * buf, uint64_t len);
void rebg_send_register_header(uint8_t count, uint64_t flags);
void rebg_send_register_value(uint64_t value);
void rebg_send_syscall(const char * contents);
void rebg_send_syscall_result(const char * contents);

#endif
