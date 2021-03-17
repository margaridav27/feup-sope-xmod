#include "../include/utils.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <limits.h>
#include <string.h>
#include "../include/log.h"

bool isParentProcess(void) {
    return getpid() == getpgrp(); // The group leader has the group id as its process id.
}

void leave(int ret) {
    while (wait(NULL) >= 0); // Wait for any remaining children
    logProcessExit(ret); // Record exit in logfile
    _exit(ret); // Exit, closing all active file descriptors
}

mode_t removePermissions(mode_t old_mode, mode_t new_mode) {
    return old_mode & ~(new_mode); // Remove the relevant bits, keeping others
}

mode_t addPermissions(mode_t old_mode, mode_t new_mode) {
    return old_mode | new_mode; // Add the relevant bits
}

mode_t setPartialPermissions(mode_t old_mode, mode_t new_mode) {
    // Remove the existing permissions for this user, keeping other users intact
    if (new_mode & S_IRWXO) {
        old_mode &= (~S_IRWXO);
    } else if (new_mode & S_IRWXG) {
        old_mode &= (~S_IRWXG);
    } else if (new_mode & S_IRWXU) {
        old_mode &= (~S_IRWXU);
    }
    // Add the requested permissions for this user, keeping other users intact
    return old_mode | new_mode;
}

int openFile(const char *path, struct stat *buf) {
    if (path == NULL || buf == NULL) return 1;
    if (stat(path, buf) == -1) {
        perror("xmod : failed to open file");
        return -1;
    }
    return 0;
}

int concatenateFolderFilenamePath(const char *folder_path, const char *file_name, char *dest) {
    if (folder_path == NULL || file_name == NULL || dest == NULL) return -1;
    snprintf(dest, PATH_MAX, "%s/%s", folder_path, file_name);
    return 0;
}

void convert_integer_to_string(int n, char *dest) {
    int temp = n, i = 0;
    while (temp /= 10) ++i;
    do {
        dest[i--] = (char) ('0' + n % 10);
    } while (n /= 10);
}

void convert_signal_number_to_string(int signo, char *dest) {
    const char *name;
    switch (signo) {
        case SIGHUP: name = "SIGHUP";
            break;
        case SIGINT: name = "SIGINT";
            break;
        case SIGQUIT: name = "SIGQUIT";
            break;
        case SIGILL: name = "SIGILL";
            break;
        case SIGTRAP: name = "SIGTRAP";
            break;
        case SIGABRT: name = "SIGABRT";
            break;
        case SIGFPE: name = "SIGFPE";
            break;
        case SIGKILL: name = "SIGKILL";
            break;
        case SIGUSR1 : name = "SIGUSR1";
            break;
        case SIGSEGV: name = "SIGSEGV";
            break;
        case SIGUSR2 : name = "SIGUSR2";
            break;
        case SIGPIPE: name = "SIGPIPE";
            break;
        case SIGALRM: name = "SIGALRM";
            break;
        case SIGTERM: name = "SIGTERM";
            break;
        case SIGCHLD : name = "SIGCHLD";
            break;
        case SIGCONT : name = "SIGCONT";
            break;
        case SIGSTOP : name = "SIGSTOP";
            break;
        case SIGTSTP : name = "SIGTSTP";
            break;
        case SIGTTIN : name = "SIGTTIN";
            break;
        case SIGTTOU : name = "SIGTTOU";
            break;
        case SIGURG : name = "SIGURG";
            break;
        case SIGXCPU : name = "SIGXCPU";
            break;
        case SIGXFSZ : name = "SIGXFSZ";
            break;
        case SIGVTALRM : name = "SIGVTALRM";
            break;
        case SIGPROF : name = "SIGPROF";
            break;
        case SIGWINCH : name = "SIGWINCH";
            break;
        case SIGPOLL : name = "SIGPOLL";
            break;
        default: name = "UNKNOWN";
            break;
    }
    strncpy(dest, name, strlen(name));
}
