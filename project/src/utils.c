#include "../include/utils.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <limits.h>
#include <string.h>
#include "../include/log.h"

bool isParentProcess(void) {
    //COMBACK: Look into error return value
    return getpid() == getpgrp(); // The group leader has the group id as its process id.
}

void leave(int ret) {
    while (wait(NULL) >= 0); // Wait for any remaining children
    //COMBACK: Look into error return value
    logProcessExit(ret); // Record exit in logfile
    //COMBACK: Look into error return value
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
    if (new_mode & PERMISSIONS_OTHERS) {
        old_mode &= (~PERMISSIONS_OTHERS);
    } else if (new_mode & PERMISSIONS_GROUP) {
        old_mode &= (~PERMISSIONS_GROUP);
    } else if (new_mode & PERMISSIONS_USER) {
        old_mode &= (~PERMISSIONS_USER);
    }
    // Add the requested permissions for this user, keeping other users intact
    return old_mode | new_mode;
}

int openFile(const char *path, struct stat *buf) {
    if (path == NULL || buf == NULL) return 1;
    // Try to open the file.
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

int convert_integer_to_string(int n, char *dest, unsigned int size) {
    // COMBACK: Maybe make this less LCOM-y?
    int temp = n, i = 0;
    while (temp /= 10) ++i; // Count number of digits.
    if (i > size - 1) return 1;
    do { dest[i--] = (char) ('0' + n % 10); } while (n /= 10); // Convert digits to characters.
    return 0;
}

void convert_signal_number_to_string(int sig_no, const char **dest) {
    // COMBACK: Verify pointer safety
    const char *name;
    // COMBACK: Explain why this isn't an array.
    // COMBACK: Explain why we aren't using strsignal();
    switch (sig_no) {
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
    *dest = name;
}
