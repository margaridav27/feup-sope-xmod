#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/time.h>

#include "../include/time_ctrl.h"

static struct timeval startTime;

void setStartTime() {
    // Initialize program execution start time 
    gettimeofday(&startTime, NULL);

    // Set up the environment variable to be accessed by an eventual children process
    char executionStart[1024];
    sprintf(executionStart, "%ld %ld", startTime.tv_sec, startTime.tv_usec); 
    setenv("START_TIME", executionStart, 0);
}

void restoreStartTime() {
    sscanf(getenv("START_TIME"), "%lu %lu", &startTime.tv_sec, &startTime.tv_usec);
}

long getElapsed() {
    struct timeval time;
	gettimeofday(&time, NULL); // Get current time
    
    return (long)(time.tv_usec - startTime.tv_usec) / 1000000 
    + (long)(time.tv_sec - startTime.tv_sec) * 1000;
}
