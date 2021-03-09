#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "log.h"
#include "parse.h"
#include "time_ctrl.h"
#include "xmod.h"

int changeFileMode(command_t *command) {
    struct stat buf;

    if (stat(command->path, &buf) == -1) {
        perror("");
        return 1;
    }

    mode_t mode = buf.st_mode;

    if (command->action == ACTION_REMOVE) mode &= ~(command->mode); // Remove the relevant bits, keeping others
    else if (command->action == ACTION_ADD)
        mode |= command->mode; // Add the relevant bits, keeping others
    else if (command->action == ACTION_SET)
        mode = command->mode; // Set only the relevant bits

    if (chmod(command->path, mode) == -1) {
        perror("");
        return 1;
    }
    return 0;
}

int changeFolderMode(command_t *command) {
    DIR *d = opendir(command->path);

    if (d == NULL) {
        perror("");
        return 1;
    }

    struct dirent *de;
    errno = 0;

    while ((de = readdir(d)) != NULL) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;

        command_t c = *command;
        char n[strlen(command->path) + strlen(de->d_name) + 1];
        // memset(n, '\0', sizeof(n));
        strcpy(n, command->path);
        strcat(n, "/");
        strcat(n, de->d_name);

        c.path = n;
        changeMode(&c);
    }

    if (errno != 0) {
        perror("");
        return 1;
    }

    return 0;
}

int changeMode(command_t *command) {
    struct stat buf;

    if (stat(command->path, &buf) == -1) {
        perror("");
        return 1;
    }

    if (S_ISDIR(buf.st_mode)) return changeFileMode(command) && changeFolderMode(command);
    else
        return changeFileMode(command);
}

int printChangeMessage(mode_t lastMode, command_t *command) {
    char str[] = "---------", lastModeStr[] = "---------";
    parseModeToString(command->mode, str);
    parseModeToString(lastMode, lastModeStr);
    printf("mode of '%s' changed from %#o (%s) to %#o (%s)\n", command->path, lastMode, lastModeStr, command->mode,
           str);
    fflush(stdout);
    return 0;
}

int printRetainMessage(command_t *command) {
    char str[] = "---------";
    parseModeToString(command->mode, str);
    printf("mode of '%s' retained as %#o (%s)\n", command->path, command->mode, str);
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

int printNoPermissionMessage(command_t *command) {
    printf("xmod: changing permissions of '%s': Operation not permitted", command->path);
    fflush(stdout);
    return 0;
}

static bool logFileAvailable;
int main(int argc, char *argv[]) {
    setBegin();

    command_t result;
    if (parseCommand(argc, argv, &result)) return 1;

    logFileAvailable = checkLogFilename();
    if (logFileAvailable) {
        registerEvent(getpid(), FILE_MODF, "some additional info");
    } else {
        printf("File not available. Could not register event.\n");
    }

    return 0;
}
