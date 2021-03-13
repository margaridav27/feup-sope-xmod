#include "../include/io.h"
#include <stdio.h>
#include <sys/stat.h>

int printChangeMessage(const char *path, mode_t previous_mode, mode_t new_mode) {
    char new_mode_str[] = "---------", previous_mode_str[] = "---------";
    parseModeToString(new_mode, new_mode_str);
    parseModeToString(previous_mode, previous_mode_str);
    printf("Mode of '%s' changed from 0%o (%s) to 0%o (%s)\n", path,
           previous_mode, previous_mode_str, new_mode,
           new_mode_str);
    fflush(stdout);
    return 0;
}

int printRetainMessage(const char *path, mode_t mode) {
    char mode_str[] = "---------";
    parseModeToString(mode, mode_str);
    printf("Mode of '%s' retained as 0%o (%s)\n", path, mode, mode_str);
    fflush(stdout);
    return 0;
}

int printFailedMessage(const char *path, mode_t new_mode) {
    char new_mode_str[] = "---------";
    parseModeToString(new_mode, new_mode_str);
    fprintf(stderr, "failed to change mode of '%s' changed to 0%o (%s)\n", path,
            new_mode, new_mode_str);
    fflush(stdout);
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

int printNoPermissionMessage(const char *path) {
    fprintf(stderr,
            "xmod: changing permissions of '%s': Operation not permitted\n",
            path);
    fflush(stdout);
    return 0;
}

int printSymbolicMessage(const char *path) {
    printf("neither symbolic link '%s' nor referent has been changed\n", path);
    fflush(stdout);
    return 0;
}

mode_t clear_extra_bits(mode_t mode) {
    return mode & ~(S_IFMT);
}

int print_message(mode_t new_mode, mode_t old_mode, const command_t *command) {
    // Clear these bits for printing
    mode_t previous_mode = clear_extra_bits(old_mode);
    new_mode = clear_extra_bits(new_mode);

    if (new_mode == previous_mode && command->verbose) {
        if (S_ISLNK(previous_mode))
            printSymbolicMessage(command->path);
        else printRetainMessage(command->path, new_mode);
    } else if (new_mode != previous_mode && (command->changes || command->verbose))
        printChangeMessage(command->path, previous_mode, new_mode);
    return 0;
}