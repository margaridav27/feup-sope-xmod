#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>

#include "log.h"
#include "time_ctrl.h"
#include "parse.h"

static bool logFileAvailable;
int changeMode(command_t *command);

int changeFileMode(command_t *command) {
    struct stat buf;

    if (stat(command->path, &buf) == -1) {
        perror("");
        return 1;
    }

    mode_t mode = buf.st_mode;

    if (command->action == ACTION_REMOVE) mode &= ~(command->mode); // Remove the relevant bits, keeping others
    else if (command->action == ACTION_ADD) mode |= command->mode; // Add the relevant bits, keeping others
    else if (command->action == ACTION_SET) mode = command->mode; // Set only the relevant bits

    if (chmod(command->path, mode) == -1) {
        perror("");
        return 1;
    }
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
        n[strlen(command->path)] = '/';
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
    else return changeFileMode(command);
}

int main(int argc, char *argv[]) {
    setBegin();

    logFileAvailable = checkLogFilename();
    if (logFileAvailable) registerEvent(getpid(), FILE_MODF, "some additional info");
    else printf("File not available. Could not register event.\n");

    command_t result;
    if (parseCommand(argc, argv, &result)) return 1;
    changeMode(&result);
    return 0;
}
