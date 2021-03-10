#include "../include/xmod.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../include/log.h"
#include "../include/parse.h"
#include "../include/time_ctrl.h"

int changeFileMode(command_t *command) {
    struct stat buf;

    if (stat(command->path, &buf) == -1) {
        perror("");
        return 1;
    }

    mode_t mode = buf.st_mode;
    mode_t persistent_bits = __S_IFMT & mode;

    if (command->action == ACTION_REMOVE) {
        mode &= ~(command->mode);  // Remove the relevant bits, keeping others
    } else if (command->action == ACTION_ADD) {
        mode |= command->mode;  // Add the relevant bits, keeping others
    } else if (command->action == ACTION_SET) {
        mode = persistent_bits | command->mode;  // Set only the relevant bits
    }

    if (chmod(command->path, mode) == -1) {
        perror("");
        return 1;
    }

    // Remove additional bits for printing
    buf.st_mode &= ~persistent_bits;
    mode &= ~persistent_bits;

    if (command->verbose) {
        if (mode == buf.st_mode) {
            printRetainMessage(command->path, mode);
        } else {
            printChangeMessage(command->path, buf.st_mode, mode);
        }
    } else if (command->changes && mode != buf.st_mode) {
        printChangeMessage(command->path, buf.st_mode, mode);
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
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;

        command_t c = *command;
        // COMBACK: WTF
        char *n = malloc(strlen(command->path) + strlen(de->d_name) + 1);
        if (n == NULL) continue;  // COMBACK: Insert very special error message
        sprintf(n, "%s/%s", command->path, de->d_name);
       
        c.path = n;
        changeMode(&c);
        free(n);
    }
    closedir(d);

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

    if (S_ISDIR(buf.st_mode) && command->recursive) {
        pid_t pid = fork();

        if (pid == -1) { // Failed to fork
            perror("");
            return 1;
        }
        else if (pid == 0) { // Child process
            changeFileMode(command);
            changeFolderMode(command);
            exit(0);
        }
        else { // Parent process
            wait(NULL); // Waiting for the child process to finish processing the subfolder 
            return 0;
        }
    } else {
        return changeFileMode(command);
    }
}

int printChangeMessage(const char *path, mode_t previous_mode, mode_t new_mode) {
    char new_mode_str[] = "---------", previous_mode_str[] = "---------";
    parseModeToString(new_mode, new_mode_str);
    parseModeToString(previous_mode, previous_mode_str);
    printf("%d - mode of '%s' changed from 0%o (%s) to 0%o (%s)\n", getpid(), path,
           previous_mode, previous_mode_str, new_mode,
           new_mode_str);
    fflush(stdout);
    return 0;
}

int printRetainMessage(const char *path, mode_t mode) {
    char mode_str[] = "---------";
    parseModeToString(mode, mode_str);
    printf("%d - mode of '%s' retained as 0%o (%s)\n", getpid(), path, mode, mode_str);
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
            "xmod: changing permissions of '%s': Operation not permitted",
            path);
    fflush(stdout);
    return 0;
}

//static bool logFileAvailable;

int main(int argc, char *argv[]) {
    /*setBegin();

    logFileAvailable = checkLogFilename();
    if (logFileAvailable) {
        registerEvent(getpid(), FILE_MODF, "some additional info");
    } else {
        fprintf(stderr, "File not available. Could not register event.\n");
    }*/

    command_t result;
    if (parseCommand(argc, argv, &result)) return 1;
    changeMode(&result);
    return 0;
}
