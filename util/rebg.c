#include <stdio.h>

// this fixes warnings of no prototype defined, but im not sure if this is a
// good idea
#include "qemu/rebg.h"

FILE *rebg_fd = NULL;

void rebg_handle_filename(const char * arg) {
    rebg_fd = fopen(arg, "wb");
}

FILE *rebg_log_fd(void) {
    if(rebg_fd == NULL) {
        rebg_fd = stderr;
    }
    return rebg_fd;
}
