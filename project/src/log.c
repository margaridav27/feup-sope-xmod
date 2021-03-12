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

static FILE *logFile;
static bool log_file_available = false;

int openLogFile(char *flag) {
    char *logFileName = getenv("LOG_FILENAME");

    if (logFileName == NULL) {
        fprintf(stderr, "Variable LOG_FILENAME not defined.\n");
        return 1;
    }
    errno = 0;
    if ((logFile = fopen(logFileName, flag)) == NULL) {
        perror("Failed to open logFileName");
        return 1;
    }
    log_file_available = true;
    return 0;
}

//COMBACK: What is info?
int logEvent(pid_t pid, event_t event, char *info) {
    if (!log_file_available) return 0;
    static const char *events[] = {"PROC_CREAT", "PROC_EXIT", "SIGNAL_RECV",
                                   "SIGNAL_SENT", "FILE_MODF"};
    const char *action = events[event];
    errno = 0;
    fprintf(logFile, "%.3f ; %d ; %s ; %s\n", getMillisecondsElapsed(), pid,
            action, info);
    if (errno != 0) {
        perror("Failed to log");
        return 1;
    }
    return 0;
}

int closeLogFile() {
    if (!log_file_available) return 0;
    errno = 0;
    if (fclose(logFile) != 0) {
        perror("Error closing Logfile");
        return 1;
    }
    return 0;
}
