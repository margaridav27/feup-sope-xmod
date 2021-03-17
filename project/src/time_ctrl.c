#include "../include/time_ctrl.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "../include/utils.h"

int getStartTime(struct timespec *dest) {
    if (dest == NULL) return -1;
    //COMBACK: Decide on buffer size
    char path[2048] = "/proc/";
    //COMBACK: Decide on buffer size
    char temp[32] = {0};
    //COMBACK: Look into error return value
    convert_integer_to_string(getpgrp(), temp);
    //COMBACK: Look into error return value
    strncat(path, temp, strlen(temp));
    int ret;
    struct stat buf;
    errno = 0;
    //COMBACK: Look into error return value
    ret = stat(path, &buf);
    if (ret == -1) return 1;
    *dest = buf.st_atim;
    return 0;
}

int getMillisecondsElapsed() {
    struct timespec now, then;
    //COMBACK: Look into error return value
    if (getStartTime(&then)) return -1;
    //COMBACK: Look into error return value
    clock_gettime(CLOCK_REALTIME, &now);
    return (now.tv_sec - then.tv_sec) * 1e3 + (now.tv_nsec - then.tv_nsec) * 1e-6;
}
