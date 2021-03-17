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
        //COMBACK: Look into error return value
        logFileName = getenv("LOG_FILENAME");

        if (logFileName == NULL) {
            return -1;
        }
    }
    int flags = O_WRONLY | O_CLOEXEC;
    flags |= truncate ? O_TRUNC : O_APPEND;
    errno = 0;
    //COMBACK: Look into error return value
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
    //COMBACK: Look into error return value
    if (close(fd) != 0) {
        perror("Error closing Logfile");
        return -1;
    }
    return 0;
}

int logChangePermission(const command_t *command, mode_t old_mode, mode_t new_mode, bool isLink) {
    char info[BUFSIZ] = {0};
    snprintf(info, sizeof(info) - 1, "%s : %o : %o", command->path, old_mode, new_mode);
    if (new_mode != old_mode) log_event(FILE_MODF, info);
    //COMBACK: Properly print this message
    //COMBACK: Look into error return value
    printMessage(new_mode, old_mode, command, isLink);
    return 0;
}

int logProcessCreation(char **argv, int argc) {
    char info[BUFSIZ] = {0};
    strncat(info, argv[0], sizeof(info) - 1);
    for (int i = 1; i < argc; ++i) {
        strncat(info, " ", sizeof(info) - strlen(info) - 1);
        strncat(info, argv[i], sizeof(info) - strlen(info) - 1);
    }
    //COMBACK: Look into error return value
    log_event(PROC_CREAT, info);
    return 0;
}

int logProcessExit(int ret) {
    char info[BUFSIZ] = {0};
    //COMBACK: Look into error return value
    convert_integer_to_string(ret, info, sizeof(info));
    //COMBACK: Look into error return value
    log_event(PROC_EXIT, info);
    return 0;
}

void log_signal_received(int signo) {
    char info[1024] = {0};
    const char *temp;
    //COMBACK: Look into error return value
    convert_signal_number_to_string(signo, &temp);
    strncat(info, temp, sizeof(info) - strlen(info) - 1);
    //COMBACK: Look into error return value
    log_event(SIGNAL_RECV, info);
}

void log_signal_sent(int signo, pid_t target) {
    const char *sep = " : ";
    char info[BUFSIZ] = {0};

    const char *sig_name;
    //COMBACK: Look into error return value
    convert_signal_number_to_string(signo, &sig_name);
    strncat(info, sig_name, sizeof(info) - strlen(info) - 1);
    strncat(info, sep, sizeof(info) - strlen(info) - 1);

    //COMBACK: Look into error return value
    convert_integer_to_string(target, info + strlen(info), sizeof(info) - strlen(info));
    //COMBACK: Look into error return value
    log_event(SIGNAL_SENT, info);
}

void log_current_status(const char *path, int number_of_files, int number_of_modified_files) {
    const char *sep = " ; ";
    char dest[BUFSIZ] = {0};

    pid_t pid = getpid();
    convert_integer_to_string(pid, dest, sizeof(dest));
    strncat(dest, sep, sizeof(dest) - strlen(dest) - 1);

    strncat(dest, path, sizeof(dest) - strlen(dest) - 1);
    strncat(dest, sep, sizeof(dest) - strlen(dest) - 1);

    convert_integer_to_string(number_of_files, dest + strlen(dest), sizeof(dest) - strlen(dest));
    strncat(dest, sep, sizeof(dest) - strlen(dest) - 1);

    convert_integer_to_string(number_of_modified_files, dest + strlen(dest), sizeof(dest) - strlen(dest));

    strncat(dest, "\n", sizeof(dest) - strlen(dest) - 1);

    //COMBACK: Look into error return value
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

    char dest[BUFSIZ] = {0}; // Destination buffer

    //COMBACK: Look into error return value
    int instant = getMillisecondsElapsed();
    //COMBACK: Look into error return value
    convert_integer_to_string(instant, dest, sizeof(dest));
    strncat(dest, sep, sizeof(dest) - strlen(dest) - 1);

    //COMBACK: Look into error return value
    convert_integer_to_string(getpid(), dest + strlen(dest), sizeof(dest) - strlen(dest));
    strncat(dest, sep, sizeof(dest) - strlen(dest) - 1);

    strncat(dest, action, sizeof(dest) - strlen(dest) - 1);
    strncat(dest, sep, sizeof(dest) - strlen(dest) - 1);

    strncat(dest, info, sizeof(dest) - strlen(dest) - 1);
    strncat(dest, "\n", sizeof(dest) - strlen(dest) - 1);

    //COMBACK: Look into error return value
    write(fd, dest, strlen(dest));
    //COMBACK: Look into error return value
    fsync(fd);
    //COMBACK: Look into error return value
    closeLogFile(fd);
}
