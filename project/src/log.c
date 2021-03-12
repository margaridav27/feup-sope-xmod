#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/log.h"
#include "../include/time_ctrl.h"

static FILE *logFP;

int initLog(char *flag) {
    
    // create env variable LOG_FILENAME "export LOG_FILENAME=./trace.txt"

    if (getenv("LOG_FILENAME") == NULL) {
        fprintf(stderr, "Variable LOG_FILENAME not defined.\n");
        return 1;
    }

    if ((logFP = fopen(getenv("LOG_FILENAME"), flag)) == NULL) {
        fprintf(stderr, "Failed to open logfile.\n");
        return 1;
    }

    if (fclose(logFP)) return 1;
    return 0;
}

int registerEvent(int pid, event_t event, char *info) {

    if ((logFP = fopen(getenv("LOG_FILENAME"), "a")) == NULL) {
        fprintf(stderr, "Failed to open logfile.\n");
        return 1;
    }

    char *action = "";
    switch (event) {
        case PROC_CREAT: action = "PROC_CREAT"; break;
        case PROC_EXIT: action = "PROC_EXIT"; break;
        case SIGNAL_RECV: action = "SIGNAL_RECV"; break;
        case SIGNAL_SENT: action = "SIGNAL_SENT"; break;
        case FILE_MODF: action = "FILE_MODF"; break;
        default: break;
    }

    fprintf(logFP, "%lu ; %d ; %s ; %s\n", getElapsed(), pid, action, info);

    if (fclose(logFP)) return 1;
    return 0;
}
