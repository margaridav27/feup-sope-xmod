#include "../include/time_ctrl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>


void setBegin() {
    struct timeval start_time;

    if (gettimeofday(&start_time, NULL) != 0) {
        perror("get time of day");
    } // Initialize start time

    char init_sec[64], init_micro[64];

    sprintf(init_sec, "%ld", start_time.tv_sec);
    if (setenv("INITSEC", init_sec, 1)) {
        perror("set env var INITSEC: ");
    }

    sprintf(init_micro, "%ld", start_time.tv_usec);
    if (setenv("INITMICRO", init_micro, 1)) {
        perror("set env var INITMICRO: ");
    }
}

double getElapsed() {

    struct timeval time;
    gettimeofday(&time, NULL); // Get current time

    const char *sec_ptr = getenv("INITSEC");
    if (sec_ptr == NULL)
        perror("SEC getenv: ");

    const char *micro_ptr = getenv("INITMICRO");
    if (sec_ptr == NULL)
        perror("SEC getenv: ");

    long int init_sec = atol(sec_ptr);
    long int init_micro = atol(micro_ptr);

    //double real_time = ((double) (time.tv_usec - start_time.tv_usec) / 1000000 + (double) (time.tv_sec - start_time.tv_sec)) * 1000;
    double real_time = ((double) (time.tv_usec - init_micro) / 1e6 + (double) (time.tv_sec - init_sec)) * 1e3;

    return real_time;
}
