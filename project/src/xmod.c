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

#include "xmod.h"
#include "log.h"
#include "time_ctrl.h"

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
    else return changeFileMode(command);
}

int printChangeMessage(mode_t lastMode, command_t *command){
    char str[] = "---------", lastModeStr[] = "---------";
    parseModeToString(command->mode,str);
    parseModeToString(lastMode,lastModeStr);
    printf("mode of '%s' changed from 0%o (%s) to 0%o (%s)\n",command->path,lastMode,lastModeStr,command->mode,str);
    fflush(stdout);
    return 0;
}

int printRetainMessage(command_t *command){
    char str[] = "---------";
    parseModeToString(command->mode,str);
    printf("mode of '%s' retained as 0%o (%s)\n",command->path,command->mode,str);
    fflush(stdout);
    return 0;
}

int parseModeToString(mode_t mode, char* str){
    if(mode & 1)str[8] = 'x';
    if(mode>>1 & 1)str[7] = 'w';
    if(mode>>2 & 1)str[6] = 'r';
    if(mode>>3 & 1)str[5] = 'x';
    if(mode>>4 & 1)str[4] = 'w';
    if(mode>>5 & 1)str[3] = 'r';
    if(mode>>6 & 1)str[2] = 'x';
    if(mode>>7 & 1)str[1] = 'w';
    if(mode>>8 & 1)str[0] = 'r';
    return 0;
}


