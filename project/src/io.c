#include <stdio.h> // snprintf(), BUFSIZ
#include <string.h> // strncat()
#include <unistd.h> // write()

#include "../include/io.h"

int printChangeMessage(const char *path, mode_t previous_mode, mode_t new_mode, char *info, unsigned int size) {
    if (path == NULL || info == NULL || size < 9) return -1;
    char new_mode_str[] = "---------", previous_mode_str[] = "---------";
    if (parseModeToString(new_mode, new_mode_str)) return -1;
    if (parseModeToString(previous_mode, previous_mode_str)) return -1;
    int n = snprintf(info, size - 1, "mode of '%s' changed from %#o (%s) to %#o (%s)\n", path,
                     previous_mode, previous_mode_str, new_mode,
                     new_mode_str);
    if (n < 0 || n >= (int) size - 1) return -1;
    return 0;
}

int printRetainMessage(const char *path, mode_t mode, char *info, unsigned int size) {
    if (path == NULL || info == NULL || size < 9) return -1;
    char mode_str[] = "---------";
    if (parseModeToString(mode, mode_str)) return -1;
    int n = snprintf(info, size - 1, "mode of '%s' retained as %#o (%s)\n", path, mode, mode_str);
    if (n < 0 || n >= (int) size - 1) return -1;
    return 0;
}

//COMBACK: Will we be using this?
int printFailedMessage(const char *path, mode_t new_mode) {
    if (path == NULL) return -1;
    char new_mode_str[] = "---------";
    char info[BUFSIZ] = {0};
    if (parseModeToString(new_mode, new_mode_str)) return -1;
    int n = snprintf(info, sizeof(info) - strlen(info) - 1, "failed to change mode of '%s' changed to %#o (%s)\n", path,
                     new_mode, new_mode_str);
    if (n != 0) return -1;
    if (write(STDOUT_FILENO, info, strlen(info) == (size_t) -1)) return -1;
    return 0;
}


int parseModeToString(mode_t mode, char *str) {
    // COMBACK: Verify size
    if (str == NULL) return -1;
    if (mode & S_IXOTH) str[8] = 'x';
    if (mode & S_IWOTH) str[7] = 'w';
    if (mode & S_IROTH) str[6] = 'r';
    if (mode & S_IXGRP) str[5] = 'x';
    if (mode & S_IWGRP) str[4] = 'w';
    if (mode & S_IRGRP) str[3] = 'r';
    if (mode & S_IXUSR) str[2] = 'x';
    if (mode & S_IWUSR) str[1] = 'w';
    if (mode & S_IRUSR) str[0] = 'r';
    return 0;
}

//COMBACK: Will we be using this?
int printNoPermissionMessage(const char *path) {
    if (path == NULL) return -1;
    char info[BUFSIZ] = {0};
    strncpy(info, "xmod: changing permissions of '", sizeof(info) - strlen(info) - 1);
    strncat(info, path, sizeof(info) - strlen(info) - 1);
    strncat(info, "': Operation not permitted\n", sizeof(info) - strlen(info) - 1);
    if (write(STDERR_FILENO, info, strlen(info) == (size_t) -1)) return -1;
    return 0;
}

int printSymbolicMessage(const char *path, char *info, unsigned int size) {
    if (path == NULL || info == NULL || size == 0) return -1;
    strncat(info, "neither symbolic link '", size - 1);
    strncat(info, path, size - 1);
    strncat(info, "' nor referent has been changed\n", size - 1);
    return 0;
}

mode_t clearExtraBits(mode_t mode) {
    // Remove bits not related to permissions.
    return mode & ~(S_IFMT);
}

//COMBACK: Maybe simplify this function?
int printMessage(mode_t new_mode, mode_t old_mode, const command_t *command, bool isLink) {
    if (command == NULL) return -1;
    if (!command->verbose && !command->changes) return 0; // No need to log
    char info[BUFSIZ] = {0};
    // Clear these bits for printing
    old_mode = clearExtraBits(old_mode);
    new_mode = clearExtraBits(new_mode);
    if (command->verbose) { // Print all information
        if (new_mode == old_mode) {
            if (isLink && printSymbolicMessage(command->path, info, sizeof(info) - strlen(info) - 1)) return -1;
            else if (printRetainMessage(command->path, old_mode, info, sizeof(info) - strlen(info) - 1)) return -1;
        } else if (printChangeMessage(command->path, old_mode, new_mode, info, sizeof(info) - strlen(info) - 1)) {
            return -1;
        }
    } else if (command->changes && new_mode != old_mode) {
        if (printChangeMessage(command->path, old_mode, new_mode, info, sizeof(info) - strlen(info) - 1)) return -1;
    }
    if (write(STDOUT_FILENO, info, strlen(info) == (size_t) -1)) return -1;
    return 0;
}
