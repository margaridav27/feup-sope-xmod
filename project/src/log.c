#include "../include/log.h"

#include <errno.h> // errno
#include <fcntl.h> // O_WRONLY, O_CLOEXEC, O_TRUNC, O_APPEND, O_CREAT, open()
#include <stdbool.h> // bool
#include <stdio.h> // snprintf(), perror()
#include <stdlib.h> // getenv()
#include <signal.h> // sig_atomic_t
#include <string.h> // strncat()
#include <unistd.h> // write(), fsync(), close()

#include "../include/io.h" // printMessage()
#include "../include/time_ctrl.h" // getMillisecondsElapsed()

sig_atomic_t logfile_available = false;
static const char *logfile_name;

int openLogFile(bool truncate) {
    if (!logfile_available || logfile_name == NULL) {
        logfile_name = getenv("LOG_FILENAME");
        if (logfile_name == NULL) return -1;
    }
    int flags = O_WRONLY | O_CLOEXEC; // Common flags
    flags |= truncate ? (O_TRUNC | O_CREAT) : O_APPEND; // First process/others flags
    errno = 0;
    int fd = open(logfile_name, flags, 0666); // Try to open the logfile
    if (fd == -1) {
        perror("Failed to open logfile");
        return -1;
    }
    logfile_available = true; // Success
    return fd;
}

int closeLogFile(int fd) {
    if (!logfile_available) return 0;
    if (fd == -1) return -1;
    errno = 0;
    if (close(fd) == -1) { // Try to close the logfile
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
    if (new_mode != old_mode) logEvent(FILE_MODF, info);
    //COMBACK: Properly print this message
    if (printMessage(new_mode, old_mode, command, isLink)) return -1;
    return 0;
}

int logProcessCreation(char **argv, int argc) {
    if (argv == NULL) return -1;
    char info[BUFSIZ] = {0};
    // Copy argv into buffer
    strncat(info, argv[0], sizeof(info) - 1);
    for (int i = 1; i < argc; ++i) {
        strncat(info, " ", sizeof(info) - strlen(info) - 1);
        strncat(info, argv[i], sizeof(info) - strlen(info) - 1);
    }
    if (logEvent(PROC_CREAT, info)) return -1;
    return 0;
}

int logProcessExit(int ret) {
    char info[BUFSIZ] = {0};
    if (convertIntegerToString(ret, info, sizeof(info))) return -1;
    if (logEvent(PROC_EXIT, info)) return -1;
    return 0;
}

int logSignalReceived(int sig_no) {
    char info[BUFSIZ] = {0};
    const char *temp;
    if (convertSignalNumberToString(sig_no, &temp)) return -1;
    strncat(info, temp, sizeof(info) - strlen(info) - 1);
    if (logEvent(SIGNAL_RECV, info)) return -1;
    return 0;
}

int logSignalSent(int sig_no, pid_t target) {
    const char *sep = " : ";
    char info[BUFSIZ] = {0};

    const char *sig_name;
    if (convertSignalNumberToString(sig_no, &sig_name)) return -1;
    strncat(info, sig_name, sizeof(info) - strlen(info) - 1);
    strncat(info, sep, sizeof(info) - strlen(info) - 1);

    if (convertIntegerToString(target, info + strlen(info), sizeof(info) - strlen(info))) return -1;
    if (logEvent(SIGNAL_SENT, info)) return -1;
    return 0;
}

int logCurrentStatus(const char *path, int numberOfFiles, int numberOfModifiedFiles) {
    if (path == NULL) return -1;
    const char *sep = " ; ";
    char dest[BUFSIZ] = {0};

    pid_t pid = getpid();
    if (convertIntegerToString(pid, dest, sizeof(dest))) return -1;
    strncat(dest, sep, sizeof(dest) - strlen(dest) - 1);

    strncat(dest, path, sizeof(dest) - strlen(dest) - 1);
    strncat(dest, sep, sizeof(dest) - strlen(dest) - 1);

    if (convertIntegerToString(numberOfFiles, dest + strlen(dest), sizeof(dest) - strlen(dest))) return -1;
    strncat(dest, sep, sizeof(dest) - strlen(dest) - 1);

    if (convertIntegerToString(numberOfModifiedFiles, dest + strlen(dest), sizeof(dest) - strlen(dest)))
        return -1;

    strncat(dest, "\n", sizeof(dest) - strlen(dest) - 1);

    if (write(STDOUT_FILENO, dest, strlen(dest)) == -1) {
        perror("WRITE: ");
        return -1;
    }
    return 0;
}

int logEvent(event_t event, char *info) {
    if (info == NULL) return -1;
    if (!logfile_available) return 0;
    int fd = openLogFile(false);
    if (fd == -1) return -1;
    static const char *events[] = {"PROC_CREAT", "PROC_EXIT", "SIGNAL_RECV",
                                   "SIGNAL_SENT", "FILE_MODF"};
    const char *action = events[event];
    const char *sep = " ; ";

    char dest[BUFSIZ] = {0}; // Destination buffer

    int instant = getMillisecondsElapsed();
    if (instant == -1) {
        closeLogFile(fd);
        return -1;
    }
    if (convertIntegerToString(instant, dest, sizeof(dest)) == -1) {
        closeLogFile(fd);
        return -1;
    }
    strncat(dest, sep, sizeof(dest) - strlen(dest) - 1);

    if (convertIntegerToString(getpid(), dest + strlen(dest), sizeof(dest) - strlen(dest)) == -1) {
        closeLogFile(fd);
        return -1;
    }
    strncat(dest, sep, sizeof(dest) - strlen(dest) - 1);

    strncat(dest, action, sizeof(dest) - strlen(dest) - 1);
    strncat(dest, sep, sizeof(dest) - strlen(dest) - 1);

    strncat(dest, info, sizeof(dest) - strlen(dest) - 1);
    strncat(dest, "\n", sizeof(dest) - strlen(dest) - 1);

    if (write(fd, dest, strlen(dest)) == -1) {
        closeLogFile(fd);
        return -1;
    }

    if (fsync(fd) == -1) {
        closeLogFile(fd);
        return -1;
    }

    if (closeLogFile(fd)) return -1;
    return 0;
}
