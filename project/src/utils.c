#include "../include/utils.h"

#include <limits.h> // PATH_MAX //COMBACK
//  SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGTRAP, SIGABRT, SIGFPE, SIGKILL, SIGUSR1 , SIGSEGV, SIGUSR2 , SIGPIPE, SIGALRM,
//  SIGTERM, SIGCHLD , SIGCONT , SIGSTOP , SIGTSTP , SIGTTIN , SIGTTOU , SIGURG , SIGXCPU , SIGXFSZ , SIGVTALRM,
//  SIGPROF , SIGWINCH , SIGPOLL
#include <signal.h>
#include <stdbool.h> // bool
#include <stdio.h> // perror(), snprintf()
#include <string.h> // strlen()
#include <sys/stat.h> // stat()
#include <sys/wait.h> // wait()
#include <unistd.h> // getpid(), getpgrp(), _exit()

#include "../include/log.h" // logProcessExit()


bool isParentProcess(void) {
    return getpid() == getpgrp(); // The group leader has the group id as its process id.
}

void leave(int ret) {
    while (wait(NULL) >= 0); // Wait for any remaining children
    logProcessExit(ret); // Record exit in logfile
    _exit(ret); // Exit, closing all active file descriptors
}

mode_t modeRemovingPermissions(mode_t old_mode, mode_t new_mode) {
    return old_mode & ~(new_mode); // Remove the relevant bits, keeping others
}

mode_t modeAddingPermissions(mode_t old_mode, mode_t new_mode) {
    return old_mode | new_mode; // Add the relevant bits
}

mode_t modeSettingPartialPermissions(mode_t old_mode, mode_t new_mode) {
    // Remove the existing permissions for this user, keeping other users intact
    if (new_mode & PERMISSIONS_OTHERS) old_mode &= (~PERMISSIONS_OTHERS);
    else if (new_mode & PERMISSIONS_GROUP) old_mode &= (~PERMISSIONS_GROUP);
    else if (new_mode & PERMISSIONS_USER) old_mode &= (~PERMISSIONS_USER);
    // Add the requested permissions for this user, keeping other users intact
    return old_mode | new_mode;
}

int openFile(const char *path, struct stat *buf) {
    if (path == NULL || buf == NULL) return -1;
    // Try to open the file.
    if (stat(path, buf) == -1) {
        perror("xmod : failed to open file");
        return -1;
    }
    return 0;
}

int concatenateFolderFilenamePath(const char *folder_path, const char *file_name, char *dest, unsigned int size) {
    if (folder_path == NULL || file_name == NULL || dest == NULL) return -1;
    if (size < strlen(folder_path) + strlen(file_name) + 1) return -1; // Destination too small for the whole path
    int n = snprintf(dest, size, "%s/%s", folder_path, file_name);
    if (n < 0 || n >= (int) size) return -1; // Failure writing
    return 0;
}

int convertIntegerToString(int n, char *dest, unsigned int size) {
    if (dest == NULL || size == 0) return -1;
    unsigned number_of_digits = 0;
    if (n < 0) {
        // The minus sign is the first character.
        dest[0] = '-';
        ++number_of_digits;
        n *= -1; // Pretend the number is positive from now on.
    }
    int _n = n;
    while (_n /= 10) ++number_of_digits; // Count number of digits.
    if (number_of_digits > size) return -1; // Not enough size to store the whole number
    do {
        char digit = (char) ('0' + (n % 10));    // Extract last digit
        dest[number_of_digits] = digit;         // Place last digit
        --number_of_digits;                     // One less digit to place
        n /= 10;                        // Remove the last digit
    } while (n > 0);                    // While there are digits
    return 0;
}

int convertSignalNumberToString(int sig_no, const char **dest) {
    if (dest == NULL) return -1;
    /* Although this mapping may seem extreme, it is the only option given two things:
     * 1. The signal numbers change with each processor architecture (we can't use an array, as the indexes would not work)
     * 2. The POSIX function strsignal() does not print the actual name, only a description. Plus, it is not async-signal safe.
     */
    switch (sig_no) {
        case SIGHUP: *dest = "SIGHUP";
            return 0;
        case SIGINT: *dest = "SIGINT";
            return 0;
        case SIGQUIT: *dest = "SIGQUIT";
            return 0;
        case SIGILL: *dest = "SIGILL";
            return 0;
        case SIGTRAP: *dest = "SIGTRAP";
            return 0;
        case SIGABRT: *dest = "SIGABRT";
            return 0;
        case SIGFPE: *dest = "SIGFPE";
            return 0;
        case SIGKILL: *dest = "SIGKILL";
            return 0;
        case SIGUSR1 : *dest = "SIGUSR1";
            return 0;
        case SIGSEGV: *dest = "SIGSEGV";
            return 0;
        case SIGUSR2 : *dest = "SIGUSR2";
            return 0;
        case SIGPIPE: *dest = "SIGPIPE";
            return 0;
        case SIGALRM: *dest = "SIGALRM";
            return 0;
        case SIGTERM: *dest = "SIGTERM";
            return 0;
        case SIGCHLD : *dest = "SIGCHLD";
            return 0;
        case SIGCONT : *dest = "SIGCONT";
            return 0;
        case SIGSTOP : *dest = "SIGSTOP";
            return 0;
        case SIGTSTP : *dest = "SIGTSTP";
            return 0;
        case SIGTTIN : *dest = "SIGTTIN";
            return 0;
        case SIGTTOU : *dest = "SIGTTOU";
            return 0;
        case SIGURG : *dest = "SIGURG";
            return 0;
        case SIGXCPU : *dest = "SIGXCPU";
            return 0;
        case SIGXFSZ : *dest = "SIGXFSZ";
            return 0;
        case SIGVTALRM : *dest = "SIGVTALRM";
            return 0;
        case SIGPROF : *dest = "SIGPROF";
            return 0;
        case SIGWINCH : *dest = "SIGWINCH";
            return 0;
        case SIGPOLL : *dest = "SIGPOLL";
            return 0;
        default: *dest = "UNKNOWN";
            return 0;
    }
}
