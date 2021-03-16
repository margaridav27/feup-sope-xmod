#include <unistd.h>
#include "../include/utils.h"
#include "../include/log.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <limits.h>

bool isParentProcess(void) {
    return getpid() == getpgid(0);
}

void leave(int ret) {
    logProcessExit(ret);
    while (wait(NULL) >= 0);
    exit(ret);
}

mode_t removePermissions(mode_t old_mode, mode_t new_mode) {
    return old_mode & ~(new_mode); // Remove the relevant bits, keeping others
}

mode_t addPermissions(mode_t old_mode, mode_t new_mode) {
    return old_mode | new_mode; // Add the relevant bits, keeping others
}

mode_t setPartialPermissions(mode_t old_mode, mode_t new_mode) {
    if (new_mode & S_IRWXO) {
        old_mode &= (~S_IRWXO);
    } else if (new_mode & S_IRWXG) {
        old_mode &= (~S_IRWXG);
    } else if (new_mode & S_IRWXU) {
        old_mode &= (~S_IRWXU);
    }
    return old_mode | new_mode;
}

int openFile(const char *path, struct stat *buf) {
    if (path == NULL || buf == NULL) return 1;
    int r = stat(path, buf);
    if (r == -1) {
        perror("xmod: failed to open file");
        return 1;
    }
    return 0;
}

int concatenateFolderFilenamePath(const char *folder_path, const char *file_name, char *dest) {
    if (folder_path == NULL || file_name == NULL || dest == NULL) return 1;
    snprintf(dest, PATH_MAX, "%s/%s", folder_path, file_name);
    return 0;
}