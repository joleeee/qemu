#include <stdio.h>

FILE *rebg_fd;

void rebg_init_fd(void);
void rebg_init_fd(void) {
    rebg_fd = fopen("/tmp/rebg-out", "wb");
}

FILE *rebg_log_fd(void) {
    if(rebg_fd == NULL) {
        rebg_init_fd();
    }
    return rebg_fd;
}
