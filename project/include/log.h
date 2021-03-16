#ifndef PROJECT_INCLUDE_LOG_H_
#define PROJECT_INCLUDE_LOG_H_

#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include "../include/parse.h"

typedef enum {
    PROC_CREAT,
    PROC_EXIT,
    SIGNAL_RECV,
    SIGNAL_SENT,
    FILE_MODF
} event_t;

int openLogFile(char *flag);

int logEvent(pid_t pid, event_t event, char *info);

int closeLogFile();

int logChangePermission(const command_t *command, mode_t old_mode, mode_t new_mode, bool isLink);

int logProcessCreation(char **argv, int argc);

int logProcessExit(int ret);

#endif // PROJECT_INCLUDE_LOG_H_
