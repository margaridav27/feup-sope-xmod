#include "../include/xmod.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>
#include <sys/resource.h>

#include "../include/parse.h"
#include "../include/io.h"
#include "../include/log.h"
#include "../include/time_ctrl.h"
#include "../include/utils.h"
#include "../include/signals.h"

int number_of_files = 0, number_of_modified_files = 0, number_of_children = 0;
pid_t children[99999];


int createNewProcess(const command_t *command, char *new_path) {
    static char *new_argv[_POSIX_ARG_MAX] = {};
    for (int i = 0; i < command->argc; ++i) new_argv[i] = command->argv[i];
    new_argv[command->argc - 1] = new_path;
    if (execv(new_argv[0], new_argv)) {
        perror("xmod: exec");
        return 1;
    }
    return 0;
}

int changeFileMode(const command_t *command, struct stat *buf, bool isLink) {
    ++number_of_files;
    mode_t mode = buf->st_mode;
    mode_t persistent_bits = mode & S_IFMT;
    mode_t new_mode;
    if (command->action == ACTION_REMOVE) {
        new_mode = removePermissions(mode, command->mode);
    } else if (command->action == ACTION_ADD) {
        new_mode = addPermissions(mode, command->mode);
    } else if (command->action == ACTION_PARTIAL_SET) {
        new_mode = setPartialPermissions(mode, command->mode);
    } else if (command->action == ACTION_SET) {
        new_mode = command->mode | persistent_bits;
    } else {
        return 1;
    }
    if (chmod(command->path, new_mode) == -1) {
        perror("xmod: failed to change permissions");
        return 1;
    }
    if (new_mode != mode) ++number_of_modified_files;
    logChangePermission(command, buf->st_mode, new_mode, isLink);
    return 0;
}

int changeFolderMode(const command_t *command) {
    // Read the folder
    DIR *dir = opendir(command->path);
    if (dir == NULL) {
        perror("xmod: failed to open folder");
        return 1;
    }
    struct dirent *d;
    // bool terminate = false;
    while ((d = readdir(dir)) != NULL) {
        sleep(1);
        // if ((terminate = checkTerminateSignal())) break;
        if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0) continue;
        char new_path[PATH_MAX];
        command_t new_command = *command;
        concatenateFolderFilenamePath(command->path, d->d_name, new_path);
        new_command.path = new_path;

        struct stat buf;
        if (openFile(new_path, &buf)) continue;
        if (d->d_type == DT_DIR) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("xmod: fork");
                continue;
            } else if (pid == 0) {
                createNewProcess(command, new_path);
            } else {
                children[number_of_children] = pid;
                ++number_of_children;
                continue;
            }
        } else {
            bool link = d->d_type == DT_LNK;
            changeFileMode(&new_command, &buf, link);
        }
    }
    closedir(dir);
    // if (terminate || checkTerminateSignal()) terminateProgram();
    return 0;
}

int changeMode(const command_t *command) {
    struct stat buf;
    if (openFile(command->path, &buf)) return 1;
    if (changeFileMode(command, &buf, 0)) return 1;

    if (S_ISDIR(buf.st_mode) && command->recursive) {
        if (changeFolderMode(command)) return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    getStartTime();
    int fd = openLogFile(isParentProcess());
    if (isParentProcess() && fd == -1)
        fprintf(stderr, "Variable LOG_FILENAME not defined.\n");
    closeLogFile(fd);
    command_t result;
    if (parseCommand(argc, argv, &result)) leave(1);
    logProcessCreation(argv, argc);
    setUpSignals(result.path);
    changeMode(&result);
    leave(0);
    return 0;
}
