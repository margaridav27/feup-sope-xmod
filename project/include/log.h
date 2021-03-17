//COMBACK: Explain header usages
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

int openLogFile(bool truncate);

int closeLogFile(int fd);

int logChangePermission(const command_t *command, mode_t old_mode, mode_t new_mode, bool isLink);

int logProcessCreation(char **argv, int argc);

int logProcessExit(int ret);

void log_signal_received(int signo);

void log_signal_sent(int signo, pid_t target);

void log_current_status(const char *path, int number_of_files, int number_of_modified_files);

void log_event(event_t event, char *info);

#endif // PROJECT_INCLUDE_LOG_H_
