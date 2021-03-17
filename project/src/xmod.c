// COMBACK: Comment header uses
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

int number_of_files = 0, number_of_modified_files = 0;
// COMBACK: Check whether this allows us to sync output. If not, remove.
// int number_of_children;
// pid_t children[99999];


int createNewProcess(const command_t *command, char *new_path) {
    static char *new_argv[_POSIX_ARG_MAX] = {0};
    for (int i = 0; i < command->argc; ++i) new_argv[i] = command->argv[i];
    new_argv[command->argc - 1] = new_path;
    //COMBACK: Look into error return value
    if (execv(new_argv[0], new_argv)) {
        perror("xmod: exec");
        return 1;
    }
    return 0;
}

int changeFileMode(const command_t *command, struct stat *buf) {
    ++number_of_files;
    mode_t mode = buf->st_mode;
    mode_t persistent_bits = mode & UNRELATED_BITS;
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
    //COMBACK: Look into error return value
    if (chmod(command->path, new_mode) == -1) {
        perror("xmod: failed to change permissions");
        return 1;
    }
    if (new_mode != mode) ++number_of_modified_files;
    //COMBACK: Look into error return value
    logChangePermission(command, buf->st_mode, new_mode, false);
    return 0;
}

int changeFolderMode(const command_t *command) {
    // Read the folder
    //COMBACK: Look into error return value
    DIR *dir = opendir(command->path);
    if (dir == NULL) {
        perror("xmod: failed to open folder");
        return 1;
    }
    struct dirent *d;
    //COMBACK: Look into error return value
    while ((d = readdir(dir)) != NULL) {
        //TODO: Uncomment me to check signal handling
        // sleep(1);
        if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0) continue;
        char new_path[PATH_MAX];
        command_t new_command = *command;
        //COMBACK: Look into error return value
        concatenateFolderFilenamePath(command->path, d->d_name, new_path);
        new_command.path = new_path;

        struct stat buf;
        //COMBACK: Look into error return value
        if (openFile(new_path, &buf)) continue;
        if (d->d_type == DT_DIR) {
            //COMBACK: Look into error return value
            pid_t pid = fork();
            if (pid < 0) {
                perror("xmod: fork");
                continue;
            } else if (pid == 0) {
                //COMBACK: Look into error return value
                createNewProcess(command, new_path);
            } else {
                // COMBACK: Check whether this allows us to sync output. If not, remove.
                // children[number_of_children] = pid;
                // ++number_of_children;
                continue;
            }
        } else if (d->d_type == DT_LNK) {
            //COMBACK: Might need to rethink function sequence
            //COMBACK: Look into error return value
            printMessage(0, 0, command, true);
        } else {
            //COMBACK: Look into error return value
            changeFileMode(&new_command, &buf);
        }
    }
    // COMBACK: Check if this can be used as a file descriptor and closed with _exit(). If not, rethink signal handling.
    //COMBACK: Look into error return value
    closedir(dir);
    return 0;
}

int changeMode(const command_t *command) {
    struct stat buf;
    //COMBACK: Look into error return value
    if (openFile(command->path, &buf)) return 1;
    //COMBACK: Look into error return value
    if (changeFileMode(command, &buf)) return 1;

    if (S_ISDIR(buf.st_mode) && command->recursive) {
        //COMBACK: Look into error return value
        if (changeFolderMode(command)) return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    //COMBACK: Look into error return value
    //COMBACK: Is this still necessary?
    //getStartTime(NULL);
    //COMBACK: Find a better way to structure this
    //COMBACK: Look into error return value
    int fd = openLogFile(isParentProcess());
    if (isParentProcess() && fd == -1)
        fprintf(stderr, "Variable LOG_FILENAME not defined.\n");
    closeLogFile(fd);
    command_t result;
    //COMBACK: Look into error return value
    if (parseCommand(argc, argv, &result)) leave(1);
    //COMBACK: Look into error return value
    logProcessCreation(argv, argc);
    //COMBACK: Look into error return value
    setUpSignals(result.path);
    //COMBACK: Look into error return value
    changeMode(&result);
    leave(0);
    return 0;
}
