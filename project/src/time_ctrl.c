#include "../include/time_ctrl.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

static bool start_time_available = false;
static struct timespec startTime;

int getStartTime() {
    char path[2048] = "/proc/";
    int ret = snprintf(path + strlen(path), sizeof(path) - strlen(path), "%d", getpgid(0));
    if (ret < 0) return 1;
    struct stat buf;
    errno = 0;
    ret = stat(path, &buf);
    if (ret == -1) return 1;
    startTime = buf.st_atim;
    start_time_available = true;
    return 0;
}

int getMillisecondsElapsed() {
    if (!start_time_available) return -1;
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return (now.tv_sec - startTime.tv_sec) * 1e3 + (now.tv_nsec - startTime.tv_nsec) * 1e-6;
}