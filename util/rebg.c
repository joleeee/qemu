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

char * rebg_savebuf_get() {
    return save_buf;
}

void rebg_savebuf_clear() {
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
