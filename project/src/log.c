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
static const char *logFileName;

int openLogFile(char *flag) {
    if (!log_file_available || logFileName == NULL) {
        logFileName = getenv("LOG_FILENAME");

        if (logFileName == NULL) {
            //COMBACK: Find a better error message
            if (strcmp(flag, "w") == 0) fprintf(stderr, "Variable LOG_FILENAME not defined.\n");
            return 1;
        }
    }
    errno = 0;
    logFile = fopen(logFileName, flag);
    if (logFile == NULL) {
        perror("Failed to open logFileName");
        return 1;
    }
    log_file_available = true;
    return 0;
}

int logEvent(pid_t pid, event_t event, char *info) {
    if (!log_file_available) return 0;
    openLogFile("a");
    static const char *events[] = {"PROC_CREAT", "PROC_EXIT", "SIGNAL_RECV",
                                   "SIGNAL_SENT", "FILE_MODF"};
    const char *action = events[event];
    errno = 0;
    if (logFile != NULL)
        fprintf(logFile, "%.3f ; %d ; %s ; %s\n", getMillisecondsElapsed(), pid,
                action, info);
    errno = 0;
    fflush(logFile);
    closeLogFile();
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
