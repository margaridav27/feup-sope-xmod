#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>

#include "../include/time_ctrl.h"

struct timeval start_time;

void setBegin() {
    gettimeofday(&start_time, NULL); // Initialize start time
}

double getElapsed() {
    struct timeval time;
	gettimeofday(&time, NULL); // Get current time

    double real_time = ((double)(time.tv_usec - start_time.tv_usec) / 1000000 
    + (double)(time.tv_sec - start_time.tv_sec)) * 1000;

    return real_time;
}
