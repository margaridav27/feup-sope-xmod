#ifndef PROJECT_INCLUDE_LOG_H_
#define PROJECT_INCLUDE_LOG_H_

#include <stdbool.h> // bool
#include <sys/types.h> // pid_t

#include "../include/utils.h" // command_t

typedef enum {
    PROC_CREAT,
    PROC_EXIT,
    SIGNAL_RECV,
    SIGNAL_SENT,
    FILE_MODF
} event_t;

int openLogFile(bool truncate);

int closeLogFile(int fd);

int logEvent(event_t event, char *info);

int logChangePermission(const command_t *command, mode_t old_mode, mode_t new_mode, bool isLink);

int logCurrentStatus(const char *path, int numberOfFiles, int numberOfModifiedFiles);

int logProcessCreation(char **argv, int argc);

int logProcessExit(int ret);

int logSignalReceived(int sig_no);

int logSignalSent(int sig_no, pid_t target);

#endif // PROJECT_INCLUDE_LOG_H_
