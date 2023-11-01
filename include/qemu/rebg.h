#ifndef REBG_H
#define REBG_H

FILE *rebg_log_fd(void);
int rebg_sock_get(void);
char * rebg_savebuf_get();
void reg_savebuf_clear();

// for argument
void rebg_handle_filename(const char * arg);
void rebg_handle_tcp(const char * arg);

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
            write(s, buf, len);                 \
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
};

static void rebg_send_separator() {
    uint8_t kind = (uint8_t)SEPARATOR;
    rebg_write(&kind, 1);
}

static void rebg_send_load(uint64_t address, uint64_t value, uint8_t size) {
    uint8_t kind = (uint8_t)LOAD;
    rebg_write(&kind, 1);

    rebg_write(&size, 1);
    rebg_write(&address, sizeof(address));
    rebg_write(&value, sizeof(value));
}

static void rebg_send_store(uint64_t address, uint64_t value, uint8_t size) {
    uint8_t kind = (uint8_t)STORE;
    rebg_write(&kind, 1);

    rebg_write(&size, 1);
    rebg_write(&address, sizeof(address));
    rebg_write(&value, sizeof(value));
}

static void rebg_send_libload(char * file, uint64_t from, uint64_t to) {
    uint8_t kind = (uint8_t)LIBLOAD;
    rebg_write(&kind, 1);

    uint64_t name_length = strnlen(file, 0x100);
    rebg_write(&name_length, sizeof(uint64_t));
    rebg_write(file, name_length);

    rebg_write(&from, sizeof(uint64_t));
    rebg_write(&to, sizeof(uint64_t));
}

static void rebg_send_address(uint64_t adr) {
    uint8_t kind = (uint8_t)ADDRESS;
    rebg_write(&kind, 1);
    rebg_write(&adr, sizeof(uint64_t));
}

static void rebg_send_code(uint8_t * buf, uint64_t len) {
    uint8_t kind = (uint8_t)CODE;
    rebg_write(&kind, 1);
    rebg_write(&len, sizeof(uint64_t));
    rebg_write(buf, len);
}

static void rebg_send_register_header(uint8_t count, uint64_t flags) {
    uint8_t kind = (uint8_t)REGISTERS;
    rebg_write(&kind, 1);
    rebg_write(&count, 1);

    rebg_write(&flags, sizeof(flags));
}

static void rebg_send_register_value(uint64_t value) {
    rebg_write(&value, sizeof(uint64_t));
}

// static void rebg_send_flags(uint64_t value) {
//     uint8_t kind = (uint8_t)FLAGS;
//     rebg_write(&kind, 1);
//     rebg_write(&value, sizeof(uint64_t));
// }

static void rebg_send_syscall(char * contents) {
    uint8_t kind = (uint8_t)SYSCALL;
    rebg_write(&kind, 1);

    uint64_t content_length = strnlen(contents, 0x100);
    rebg_write(&content_length, sizeof(uint64_t));
    rebg_write(contents, content_length);
}

#endif
