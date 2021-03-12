#include "../include/log.h"

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

#include "../include/time_ctrl.h"

static FILE *logFP;

bool openLogFile() {
    // create env variable LOG_FILENAME "export LOG_FILENAME=./trace.txt"
    char *logFile = getenv("LOG_FILENAME");

    if (logFile == NULL) {
        fprintf(stderr, "WARNING: variable LOG_FILENAME not defined.\n");
        return false;
    }

    if ((logFP = fopen(logFile, "w")) == NULL) {
        fprintf(stderr, "ERROR: failed to open logfile.\n");
        return false;
    }

    fprintf(logFP, "%s\n", logFile);
    return true;
}

void logEvent(int pid, event_t event, char *info) {
    char *action = "";
    switch (event) {
        case PROC_CREAT:
            action = "PROC_CREAT";
            break;
        case PROC_EXIT:
            action = "PROC_EXIT";
            break;
        case SIGNAL_RECV:
            action = "SIGNAL_RECV";
            break;
        case SIGNAL_SENT:
            action = "SIGNAL_SENT";
            break;
        case FILE_MODF:
            action = "FILE_MODF";
            break;
        default:
            break;
    }
    fprintf(logFP, "%d ; %d ; %s ; %s\n", getElapsed(), pid, action, info);
}

int closeLogFile() {
    if (fclose(logFP) != 0) {
        perror("closeLogfile");
        return 1;
    }

    return 0;
}
