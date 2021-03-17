#include "../include/log.h"

#include <errno.h> // errno
#include <fcntl.h> // O_WRONLY, O_CLOEXEC, O_TRUNC, O_APPEND, open()
#include <signal.h> // sig_atomic_t
#include <stdbool.h> // bool
#include <stdio.h> // snprintf(), perror()
#include <stdlib.h> // getenv()
#include <string.h> // strncat()
#include <unistd.h> // write(), fsync(), close()

#include "../include/time_ctrl.h" // getMillisecondsElapsed()
#include "../include/io.h" // printMessage()

//COMBACK
sig_atomic_t log_file_available = false;
static const char *logFileName;

int openLogFile(bool truncate) {
    if (!log_file_available || logFileName == NULL) {
        logFileName = getenv("LOG_FILENAME");
        if (logFileName == NULL) return -1;
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
    if (close(fd) == -1) {
        perror("Error closing Logfile");
        return -1;
    }
    return 0;
}

int logChangePermission(const command_t *command, mode_t old_mode, mode_t new_mode, bool isLink) {
    if (command == NULL) return -1;
    char info[BUFSIZ] = {0};
    int n = 0;
    snprintf(info, sizeof(info) - 1, "%s : %o : %o", command->path, old_mode, new_mode);
    if (n < 0 || n >= (int) sizeof(info) - 1) return -1;
    if (new_mode != old_mode) log_event(FILE_MODF, info);
    //COMBACK: Properly print this message
    if (printMessage(new_mode, old_mode, command, isLink)) return -1;
    return 0;
}

int logProcessCreation(char **argv, int argc) {
    if (argv == NULL) return -1;
    char info[BUFSIZ] = {0};
    strncat(info, argv[0], sizeof(info) - 1);
    for (int i = 1; i < argc; ++i) {
        strncat(info, " ", sizeof(info) - strlen(info) - 1);
        strncat(info, argv[i], sizeof(info) - strlen(info) - 1);
    }
    if (log_event(PROC_CREAT, info)) return -1;
    return 0;
}

int logProcessExit(int ret) {
    char info[BUFSIZ] = {0};
    if (convert_integer_to_string(ret, info, sizeof(info))) return -1;
    if (log_event(PROC_EXIT, info)) return -1;
    return 0;
}

int log_signal_received(int sig_no) {
    char info[BUFSIZ] = {0};
    const char *temp;
    if (convert_signal_number_to_string(sig_no, &temp)) return -1;
    strncat(info, temp, sizeof(info) - strlen(info) - 1);
    if (log_event(SIGNAL_RECV, info)) return -1;
    return 0;
}

int log_signal_sent(int sig_no, pid_t target) {
    const char *sep = " : ";
    char info[BUFSIZ] = {0};

    const char *sig_name;
    if (convert_signal_number_to_string(sig_no, &sig_name)) return -1;
    strncat(info, sig_name, sizeof(info) - strlen(info) - 1);
    strncat(info, sep, sizeof(info) - strlen(info) - 1);

    if (convert_integer_to_string(target, info + strlen(info), sizeof(info) - strlen(info))) return -1;
    if (log_event(SIGNAL_SENT, info)) return -1;
    return 0;
}

int log_current_status(const char *path, int number_of_files, int number_of_modified_files) {
    if (path == NULL) return -1;
    const char *sep = " ; ";
    char dest[BUFSIZ] = {0};

    pid_t pid = getpid();
    if (convert_integer_to_string(pid, dest, sizeof(dest))) return -1;
    strncat(dest, sep, sizeof(dest) - strlen(dest) - 1);

    strncat(dest, path, sizeof(dest) - strlen(dest) - 1);
    strncat(dest, sep, sizeof(dest) - strlen(dest) - 1);

    if (convert_integer_to_string(number_of_files, dest + strlen(dest), sizeof(dest) - strlen(dest))) return -1;
    strncat(dest, sep, sizeof(dest) - strlen(dest) - 1);

    if (convert_integer_to_string(number_of_modified_files, dest + strlen(dest), sizeof(dest) - strlen(dest)))
        return -1;

    strncat(dest, "\n", sizeof(dest) - strlen(dest) - 1);

    if (write(STDOUT_FILENO, dest, strlen(dest)) == -1) {
        perror("WRITE: ");
        return -1;
    }
    return 0;
}

int log_event(event_t event, char *info) {
    if (info == NULL) return -1;
    int fd = openLogFile(false);
    if (fd == -1) return -1;
    static const char *events[] = {"PROC_CREAT", "PROC_EXIT", "SIGNAL_RECV",
                                   "SIGNAL_SENT", "FILE_MODF"};
    const char *action = events[event];
    const char *sep = " ; ";

    char dest[BUFSIZ] = {0}; // Destination buffer

    int instant = getMillisecondsElapsed();
    if (instant == -1) return -1;
    if (convert_integer_to_string(instant, dest, sizeof(dest)) == -1) return -1;
    strncat(dest, sep, sizeof(dest) - strlen(dest) - 1);

    if (convert_integer_to_string(getpid(), dest + strlen(dest), sizeof(dest) - strlen(dest)) == -1) return -1;
    strncat(dest, sep, sizeof(dest) - strlen(dest) - 1);

    strncat(dest, action, sizeof(dest) - strlen(dest) - 1);
    strncat(dest, sep, sizeof(dest) - strlen(dest) - 1);

    strncat(dest, info, sizeof(dest) - strlen(dest) - 1);
    strncat(dest, "\n", sizeof(dest) - strlen(dest) - 1);

    if (write(fd, dest, strlen(dest)) == -1) return -1;
    if (fsync(fd) == -1) return -1;
    if (closeLogFile(fd)) return -1;
    return 0;
}
