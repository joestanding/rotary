#include "syscall.h"

void _start() {
    const char * msg = "Test";
    write(1, msg, 4);
}
