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

sig_atomic_t log_file_available = false;
static const char *logFileName;

int openLogFile(bool truncate) {
    if (!log_file_available || logFileName == NULL) {
        logFileName = getenv("LOG_FILENAME");

        if (logFileName == NULL) {
            return -1;
        }
    }
    int flags = O_WRONLY | O_CLOEXEC;
    flags |= truncate ? O_TRUNC : O_APPEND;
    errno = 0;
    int fd = open(logFileName, flags);
    if (fd == -1) {
        perror("Failed to open logFileName");
        return -1;
    }
    log_file_available = true;
    return fd;
}

int closeLogFile(int fd) {
    if (!log_file_available) return 0;
    if (fd == -1) return -1;
    errno = 0;
    if (close(fd) != 0) {
        perror("Error closing Logfile");
        return -1;
    }
    return 0;
}

int logChangePermission(const command_t *command, mode_t old_mode, mode_t new_mode, bool isLink) {
    char info[2048] = {};
    snprintf(info, sizeof(info), "%s : %o : %o", command->path, old_mode, new_mode);
    if (new_mode != old_mode) log_event(FILE_MODF, info);
    //COMBACK: Properly print this message
    printMessage(new_mode, old_mode, command, isLink);
    return 0;
}

int logProcessCreation(char **argv, int argc) {
    char info[2048] = {};

    strncat(info, argv[0],strlen(argv[0]));
    for (int i = 1; i < argc; ++i) {
        strncat(info, " ", strlen(" ") + 1);
        strncat(info, argv[i], strlen(argv[i]) + 1);
    }

    log_event(PROC_CREAT, info);
    return 0;
}

int logProcessExit(int ret) {
    char info[1024] = {};
    char temp[32] = {}; // Temporary buffer for conversion

    convert_integer_to_string(ret, temp);
    strncat(info, temp, strlen(temp) + 1);

    log_event(PROC_EXIT, info);
    return 0;
}

void log_signal_received(int signo) {
    char info[1024] = {};
    char temp[32] = {}; // Temporary buffer for conversion
    convert_signal_number_to_string(signo, temp);
    strncat(info, temp, strlen(temp) + 1);
    log_event(SIGNAL_RECV, info);
}

void log_signal_sent(int signo, pid_t target) {
    const char *sep = " : ";
    char info[1024] = {};
    char temp[32] = {}; // Temporary buffer for conversions

    memset(temp, 0, sizeof(temp));
    convert_signal_number_to_string(signo, temp);
    strncat(info, temp, strlen(temp) + 1);
    strncat(info, sep, strlen(sep) + 1);

    memset(temp, 0, sizeof(temp));
    convert_integer_to_string(target, temp);
    strncat(info, temp, strlen(temp) + 1);
    log_event(SIGNAL_SENT, info);
}

void log_current_status(const char *path, int number_of_files, int number_of_modified_files) {
    const char *sep = " ; ";
    char dest[2048] = {};
    char temp[32] = {};

    pid_t pid = getpid();
    convert_integer_to_string(pid, temp);
    strncat(dest, temp, strlen(temp) + 1);
    strncat(dest, sep, strlen(sep) + 1);

    strncat(dest, path, strlen(path) + 1);
    strncat(dest, sep, strlen(sep) + 1);

    memset(temp, 0, sizeof(temp));
    convert_integer_to_string(number_of_files, temp);
    strncat(dest, temp, strlen(temp) + 1);
    strncat(dest, sep, strlen(sep) + 1);

    memset(temp, 0, sizeof(temp));
    convert_integer_to_string(number_of_modified_files, temp);
    strncat(dest, temp, strlen(temp) + 1);

    strncat(dest, "\n", strlen("\n") + 1);

    //COMBACK check error
    if (write(STDOUT_FILENO, dest, strlen(dest)) == -1)
        perror("WRITE: ");
}

void log_event(event_t event, char *info) {
    int fd = openLogFile(false);
    if (fd == -1) return;
    static const char *events[] = {"PROC_CREAT", "PROC_EXIT", "SIGNAL_RECV",
                                   "SIGNAL_SENT", "FILE_MODF"};
    const char *action = events[event];
    const char *sep = " ; ";

    char dest[2048] = {}; // Destination buffer
    char temp[32] = {}; // Temporary buffer for conversions

    int instant = getMillisecondsElapsed();
    convert_integer_to_string(instant, temp);
    strncat(dest, temp, strlen(temp) + 1);
    strncat(dest, sep, strlen(sep) + 1);

    memset(temp, 0, sizeof(temp));
    convert_integer_to_string(getpid(), temp);
    strncat(dest, temp, strlen(temp) + 1);
    strncat(dest, sep, strlen(sep) + 1);

    strncat(dest, action, strlen(action) + 1);
    strncat(dest, sep, strlen(sep) + 1);

    strncat(dest, info, strlen(info) + 1);

    strncat(dest, "\n", strlen("\n") + 1);

    // //COMBACK check error

    write(fd, dest, strlen(dest));
    fsync(fd);
    closeLogFile(fd);
}
