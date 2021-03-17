#include <errno.h> // errno
#include <stdio.h> // BUFSIZ
#include <string.h> // strlen()
#include <sys/stat.h> // stat()
#include <time.h> // clock_gettime()
#include <unistd.h> // getpgrp()

#include "../include/time_ctrl.h"
#include "../include/utils.h" // convertIntegerToString()

int getStartTime(struct timespec *dest) {
    if (dest == NULL) return -1;
    char path[BUFSIZ] = "/proc/";
    if (convertIntegerToString(getpgrp(), path + strlen(path), sizeof(path) - strlen(path))) return -1;
    struct stat buf;
    errno = 0;
    if (stat(path, &buf) == -1) return -1;
    *dest = buf.st_atim;
    return 0;
}

int getMillisecondsElapsed(void) {
    struct timespec now, then;
    if (getStartTime(&then)) return -1;
    if (clock_gettime(CLOCK_REALTIME, &now) == -1) return -1;
    return (now.tv_sec - then.tv_sec) * 1e3 + (now.tv_nsec - then.tv_nsec) * 1e-6;
}
