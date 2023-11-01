#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// this fixes warnings of no prototype defined, but im not sure if this is a
// good idea
#include "qemu/rebg.h"

FILE *rebg_fd = NULL;
int rebg_sock = -1;
char save_buf[0x100];

char * rebg_savebuf_get(void) {
    return save_buf;
}

void rebg_savebuf_clear(void) {
    memset(save_buf, 0, 0x100);
}

void rebg_handle_filename(const char * arg) {
    rebg_fd = fopen(arg, "wb");
}

void rebg_handle_tcp(const char * _arg) {
    char * arg = strdup(_arg);

    // get the part before and after :
    char hostname[128], ipstr[128];
    char * token = strtok(arg, ":");
    if(token == NULL) {
        fprintf(stderr, "failed host\n");
        perror("failed to find tcp host when splitting");
        exit(1);
    }
    strncpy(hostname, token, strnlen(token, 128)+1); // +1 for null-terminator

    token = strtok(NULL, ":");
    if(token == NULL) {
        fprintf(stderr, "failed port\n");
        perror("failed to find tcp port when splitting");
        exit(1);
    }
    int port = strtol(token, NULL, 10);
    // fprintf(stderr, "parsed %s:%d\n", hostname, port);

    // resolve hostname (also figures out AF_INET vs AF_INET6)
    struct addrinfo hints, *res, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM; // tcp
    // hints.ai_flags |= AI_CANONNAME; // idk?

    if(getaddrinfo(hostname, NULL, &hints, &result) != 0) {
        perror("getaddrinfo failed");
        exit(1);
    }

    // find the first ipv4 address
    for(res = result; res != NULL; res = res->ai_next) {
        if(res->ai_family == AF_INET) {
            break;
        }
    }
    if(res == NULL) {
        fprintf(stderr, "couldnt find ipv4 address for hostname");
        exit(1);
    }

    // copy the address to the sockaddr_in
    struct sockaddr_in *addr = (struct sockaddr_in*)res->ai_addr;
    strncpy(ipstr, inet_ntoa(addr->sin_addr), sizeof(ipstr)-1);
    freeaddrinfo(result);

    // fprintf(stderr, "resolved %s to to %s\n", hostname, ipstr);


    // connect
    rebg_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(rebg_sock == -1) {
        fprintf(stderr, "failed sock\n");
        perror("failed to create socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ipstr);

    if(connect(rebg_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(rebg_sock);
        exit(1);
    }

    free(arg);
}

void rebg_helper_writeall(int fildes, const void *buf, size_t nbyte) {
    int res;
    do {
        res = write(fildes, buf, nbyte);
        if(res < 0 && errno == EINTR) {
            continue;
        }
        if (res < 0) {
            break;
        }

        nbyte -= res;
        buf += res;
    } while(nbyte > 0 && res != nbyte);
}

// TODO figure out where to call this from
// void rebg_cleanup(void) {
//     if(rebg_fd != NULL) {
//         fflush(rebg_fd);
//         fclose(rebg_fd);
//     }

//     if(rebg_sock != -1) {
//         shutdown(rebg_sock, SHUT_RDWR);
//         close(rebg_sock);
//     }
// }

FILE *rebg_log_fd(void) {
    if(rebg_fd == NULL) {
        rebg_fd = stderr;
    }
    return rebg_fd;
}

int rebg_sock_get(void) {
    return rebg_sock;
}


void rebg_send_separator(void) {
    uint8_t kind = (uint8_t)SEPARATOR;
    rebg_write(&kind, 1);
}

void rebg_send_load(uint64_t address, uint64_t value, uint8_t size) {
    uint8_t kind = (uint8_t)LOAD;
    rebg_write(&kind, 1);

    rebg_write(&size, 1);
    rebg_write(&address, sizeof(address));
    rebg_write(&value, sizeof(value));
}

void rebg_send_store(uint64_t address, uint64_t value, uint8_t size) {
    uint8_t kind = (uint8_t)STORE;
    rebg_write(&kind, 1);

    rebg_write(&size, 1);
    rebg_write(&address, sizeof(address));
    rebg_write(&value, sizeof(value));
}

void rebg_send_libload(const char * file, uint64_t from, uint64_t to) {
    uint8_t kind = (uint8_t)LIBLOAD;
    rebg_write(&kind, 1);

    uint64_t name_length = strnlen(file, 0x100);
    rebg_write(&name_length, sizeof(uint64_t));
    rebg_write(file, name_length);

    rebg_write(&from, sizeof(uint64_t));
    rebg_write(&to, sizeof(uint64_t));
}

void rebg_send_address(uint64_t adr) {
    uint8_t kind = (uint8_t)ADDRESS;
    rebg_write(&kind, 1);
    rebg_write(&adr, sizeof(uint64_t));
}

void rebg_send_code(uint8_t * buf, uint64_t len) {
    uint8_t kind = (uint8_t)CODE;
    rebg_write(&kind, 1);
    rebg_write(&len, sizeof(uint64_t));
    rebg_write(buf, len);
}

void rebg_send_register_header(uint8_t count, uint64_t flags) {
    uint8_t kind = (uint8_t)REGISTERS;
    rebg_write(&kind, 1);
    rebg_write(&count, 1);

    rebg_write(&flags, sizeof(flags));
}

void rebg_send_register_value(uint64_t value) {
    rebg_write(&value, sizeof(uint64_t));
}

void rebg_send_syscall(const char * contents) {
    uint8_t kind = (uint8_t)SYSCALL;
    rebg_write(&kind, 1);

    uint64_t content_length = strnlen(contents, 0x100);
    rebg_write(&content_length, sizeof(uint64_t));
    rebg_write(contents, content_length);
}

void rebg_send_syscall_result(const char * contents) {
    uint8_t kind = (uint8_t)SYSCALL_RESULT;
    rebg_write(&kind, 1);

    uint64_t content_length = strnlen(contents, 0x100);
    rebg_write(&content_length, sizeof(uint64_t));
    rebg_write(contents, content_length);
}
