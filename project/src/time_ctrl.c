#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>

#include "../include/time_ctrl.h"

static struct timeval startTime;

void setBegin(struct timeval begin) {
    startTime.tv_sec = begin.tv_sec;
    startTime.tv_usec = begin.tv_usec;
}

long getElapsed(struct timeval startTime) {
    struct timeval time;
	gettimeofday(&time, NULL); // Get current time
    
    return (long)(time.tv_usec - startTime.tv_usec) / 1000000 
    + (long)(time.tv_sec - startTime.tv_sec) * 1000;
}
