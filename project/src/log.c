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
#include "../include/io.h"

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
        fprintf(logFile, "%.3f\t;\t%d\t;\t%s\t;\t%s\n", getMillisecondsElapsed(), pid,
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

int logChangePermission(const command_t *command, mode_t old_mode, mode_t new_mode, bool isLink) {
    char info[2048] = {};
    snprintf(info, sizeof(info), "%s : %o : %o", command->path, old_mode, new_mode);
    if (new_mode != old_mode) logEvent(getpid(), FILE_MODF, info);
    //COMBACK: Properly print this message
    printMessage(new_mode, old_mode, command, isLink);
    return 0;
}

int logProcessCreation(char **argv, int argc) {
    char info[2048] = {};
    snprintf(info, sizeof(info), "%s", argv[0]);
    for (int i = 1; i < argc; ++i) {
        if (i != argc) snprintf(info + strlen(info), sizeof(info) - strlen(info), " ");
        snprintf(info + strlen(info), sizeof(info) - strlen(info), "%s", argv[i]);
    }
    logEvent(getpid(), PROC_CREAT, info);
    return 0;
}

int logProcessExit(int ret) {
    char info[2048] = {};
    snprintf(info, sizeof(info), "%d", ret);
    logEvent(getpid(), PROC_EXIT, info);
    return 0;
}
