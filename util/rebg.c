#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// this fixes warnings of no prototype defined, but im not sure if this is a
// good idea
#include "qemu/rebg.h"

FILE *rebg_fd = NULL;
int rebg_sock = -1;

void rebg_handle_filename(const char * arg) {
    rebg_fd = fopen(arg, "wb");
}

void rebg_handle_tcp(const char * _arg) {
    fprintf(stderr, "hi\n");

    char * arg = strdup(_arg);

    // get the part before and after :
    char host[128];
    char * token = strtok(arg, ":");
    if(token == NULL) {
        fprintf(stderr, "failed host\n");
        perror("failed to find tcp host when splitting");
        exit(1);
    }
    strncpy(host, token, strnlen(token, 128));

    token = strtok(NULL, ":");
    if(token == NULL) {
        fprintf(stderr, "failed port\n");
        perror("failed to find tcp port when splitting");
        exit(1);
    }
    int port = strtol(token, NULL, 10);
    fprintf(stderr, "greata sucess %s:%d\n", host, port);




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
    server_addr.sin_addr.s_addr = inet_addr(host); // TODO resolve the name

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
