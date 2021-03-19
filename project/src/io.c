#include "../include/io.h"

#include <stdio.h> // snprintf(), BUFSIZ
#include <string.h> // strncat()
#include <unistd.h> // write()

#include "../include/utils.h" // parseModeToString()

int printChangeMessage(const char *path, mode_t previous_mode, mode_t new_mode, char *info, unsigned int size) {
    if (path == NULL || info == NULL) return -1;
    if (size < 9) return -1;
    char new_mode_str[] = "---------", previous_mode_str[] = "---------";
    if (parseModeToString(new_mode, new_mode_str, sizeof(new_mode_str))) return -1;
    if (parseModeToString(previous_mode, previous_mode_str, sizeof(previous_mode_str))) return -1;
    int n = snprintf(info, size - 1, "mode of '%s' changed from %#o (%s) to %#o (%s)\n", path,
                     previous_mode, previous_mode_str, new_mode,
                     new_mode_str);
    if (n < 0 || n >= (int) size - 1) return -1;
    return 0;
}

int printRetainMessage(const char *path, mode_t mode, char *info, unsigned int size) {
    if (path == NULL || info == NULL) return -1;
    if (size < 9) return -1;
    char mode_str[] = "---------";
    if (parseModeToString(mode, mode_str, sizeof(mode_str))) return -1;
    int n = snprintf(info, size - 1, "mode of '%s' retained as %#o (%s)\n", path, mode, mode_str);
    if (n < 0 || n >= (int) size - 1) return -1;
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
int printMessage(mode_t new_mode, mode_t old_mode, const command_t *command, bool is_link) {
    if (command == NULL) return -1;
    if (!command->verbose && !command->changes) return 0; // No need to log
    char buf[BUFSIZ] = {0};
    // Clear these bits for printing
    old_mode = clearExtraBits(old_mode);
    new_mode = clearExtraBits(new_mode);
    if (command->verbose) { // Print all information
        if (new_mode == old_mode) {
            if (is_link) {
                if (printSymbolicMessage(command->path, buf, sizeof(buf) - strlen(buf) - 1))
                    return -1;
            } else if (printRetainMessage(command->path, old_mode, buf,
                                          sizeof(buf) - strlen(buf) - 1)) { return -1; }
        } else if (printChangeMessage(command->path, old_mode, new_mode, buf, sizeof(buf) - strlen(buf) - 1)) {
            return -1;
        }
    } else if (command->changes && new_mode != old_mode) {
        if (printChangeMessage(command->path, old_mode, new_mode, buf, sizeof(buf) - strlen(buf) - 1)) return -1;
    }
    if (write(STDOUT_FILENO, buf, strlen(buf)) == -1) return -1;
    return 0;
}

int printCurrentStatus(const char *path, int numberOfFiles, int numberOfModifiedFiles) {
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