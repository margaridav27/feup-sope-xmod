#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>

#include "../include/time_ctrl.h"

static struct timeval startTime;

void setBegin(struct timeval begin) {
    startTime.tv_sec = begin.tv_sec;
    startTime.tv_usec = begin.tv_usec;
}

void getBegin(struct timeval *time) {
    time->tv_sec = startTime.tv_sec;
    time->tv_usec = startTime.tv_usec;
}

long getElapsed(struct timeval startTime) {
    struct timeval time;
	gettimeofday(&time, NULL); // Get current time
    
    return (time.tv_usec - startTime.tv_usec) * 1000000 
    + (time.tv_sec - startTime.tv_sec);
}
