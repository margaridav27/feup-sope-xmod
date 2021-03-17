#include "../include/io.h"
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int printChangeMessage(const char *path, mode_t previous_mode, mode_t new_mode, char *info, int size) {
    char new_mode_str[] = "---------", previous_mode_str[] = "---------";
    //COMBACK: Look into error return value
    parseModeToString(new_mode, new_mode_str);
    //COMBACK: Look into error return value
    parseModeToString(previous_mode, previous_mode_str);
    //COMBACK: Look into size
    snprintf(info, size, "mode of '%s' changed from %#o (%s) to %#o (%s)\n", path,
             previous_mode, previous_mode_str, new_mode,
             new_mode_str);
    return 0;
}

int printRetainMessage(const char *path, mode_t mode, char *info, int size) {
    char mode_str[] = "---------";
    //COMBACK: Look into error return value
    parseModeToString(mode, mode_str);
    //COMBACK: Look into size
    snprintf(info, size - strlen(info), "mode of '%s' retained as %#o (%s)\n", path, mode, mode_str);
    return 0;
}

//COMBACK: Will we be using this?
int printFailedMessage(const char *path, mode_t new_mode) {
    char new_mode_str[] = "---------";
    //COMBACK: Decide on buffer size
    char info[1024] = {0};
    //COMBACK: Look into error return value
    parseModeToString(new_mode, new_mode_str);
    //COMBACK: Look into size
    snprintf(info, sizeof(info) - strlen(info) - 1, "failed to change mode of '%s' changed to %#o (%s)\n", path,
             new_mode, new_mode_str);
    //COMBACK: Look into size
    //COMBACK: Look into error return value
    write(STDOUT_FILENO, info, strlen(info));
    return 0;
}


int parseModeToString(mode_t mode, char *str) {
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
    //COMBACK: Decide on buffer size
    char info[1024] = {0};
    //COMBACK: Look into size
    strncat(info, "xmod: changing permissions of '", sizeof(info) - strlen(info) - 1);
    //COMBACK: Look into size
    strncat(info, path, sizeof(info) - strlen(info) - 1);
    //COMBACK: Look into size
    strncat(info, "': Operation not permitted\n", sizeof(info) - strlen(info) - 1);
    //COMBACK: Look into size
    //COMBACK: Look into error return value
    write(STDERR_FILENO, info, strlen(info));
    return 0;
}

int printSymbolicMessage(const char *path, char *info, int size) {
    //COMBACK: Look into size
    strncat(info, "neither symbolic link '", size);
    //COMBACK: Look into size
    strncat(info, path, size);
    //COMBACK: Look into size
    strncat(info, "' nor referent has been changed\n", size);
    return 0;
}

mode_t clearExtraBits(mode_t mode) {
    // Remove bits not related to permissions.
    return mode & ~(S_IFMT);
}

int printMessage(mode_t new_mode, mode_t old_mode, const command_t *command, bool isLink) {
    char info[1024] = {0};
    if (!command->verbose && !command->changes) return 0; // No need to log
    // Clear these bits for printing
    old_mode = clearExtraBits(old_mode);
    new_mode = clearExtraBits(new_mode);
    if (command->verbose) { // Print all information
        if (new_mode == old_mode) {
            //COMBACK: Look into error return value
            if (isLink) printSymbolicMessage(command->path, info, sizeof(info));
                //COMBACK: Look into error return value
            else printRetainMessage(command->path, old_mode, info, sizeof(info));
        } else {
            //COMBACK: Look into error return value
            printChangeMessage(command->path, old_mode, new_mode, info, sizeof(info));
        }
    } else if (command->changes && new_mode != old_mode) {
        //COMBACK: Look into error return value
        printChangeMessage(command->path, old_mode, new_mode, info, sizeof(info));
    }
    //COMBACK: Look into size
    //COMBACK: Look into error return value
    write(STDOUT_FILENO, info, strlen(info));
    return 0;
}
