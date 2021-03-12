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

int initLog(char *flag) {
    // to create env variable: LOG_FILENAME "export LOG_FILENAME=./trace.txt"
    char *logfile = getenv("LOG_FILENAME");

    if (logfile == NULL) {
        fprintf(stderr, "Variable LOG_FILENAME not defined.\n");
        return 1;
    }
    errno = 0;
    if ((logFP = fopen(logFile, "w")) == NULL) {
        perror("Failed to open logfile");
        return 1;
    }

    return 0;
}

void logEvent(int pid, event_t event, char *info) {
    static const char *events[] = {"PROC_CREAT", "PROC_EXIT", "SIGNAL_RECV", "SIGNAL_SENT", "FILE_MODF"};
    const char *action = events[event];
    fprintf(logFP, "%.3f ; %d ; %s ; %s\n", getElapsed(), pid, action, info);
}

int closeLogFile() {
    errno = 0;
    if (fclose(logFP) != 0) {
        perror("Error closing Logfile");
        return 1;
    }

    return 0;
}
