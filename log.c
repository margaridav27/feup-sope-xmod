#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include "log.h"
#include "time_ctrl.h"

static FILE *logFP;

bool checkLogFilename() {
    char* logFile = getenv("LOG_FILENAME");

    if(logFile == NULL) {
        fprintf(stderr, "WARNING: variable LOG_FILENAME not defined.\n");
        return false;
    }

    if ((logFP = fopen(logFile, "w")) == NULL) {
        fprintf(stderr, "ERROR: failed to open logfile.\n");
        return false;
    }

    return true;
}

void registerEvent(int pid, event_t event, char* info) {
    char *action = "";
    switch(event) {
        case PROC_CREAT: action = "PROC_CREAT"; break;
        case PROC_EXIT: action = "PROC_EXIT"; break;
        case SIGNAL_RECV: action = "SIGNAL_RECV"; break;
        case SIGNAL_SENT: action = "SIGNAL_SENT"; break;
        case FILE_MODF: action = "FILE_MODF"; break;
        default: break;
    }

    fprintf(logFP, "%d ; %d ; %s ; %s\n", getElapsed(), pid, action, info);
}

int closeLogfile() {
    if (fclose(logFP) != 0) {
        perror("closeLogfile");
        return 1;
    }

    return 0;
}