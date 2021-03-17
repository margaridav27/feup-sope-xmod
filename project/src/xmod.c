#include "../include/xmod.h"
#include <dirent.h> // opendir(), readdir(), DT_DIR, DT_LNK
#include <stdbool.h> // true, false
#include <stdio.h> // perror(),
#include <string.h> // strcmp()
#include <sys/stat.h> // chmod(), struct stat
#include <unistd.h> // execv(),
#include <limits.h> // MAX_PATH
#include "../include/parse.h" // parseCommand()
#include "../include/io.h" // printMessage()
#include "../include/log.h" // logChangePermission(),
// leave(), modeRemovingPermissions(), modeAddingPermissions(), modeSettingPartialPermissions(), concatenateFolderFilenamePath()
#include "../include/utils.h"
#include "../include/signals.h" // setUpSignals()

int number_of_files = 0, number_of_modified_files = 0;

int createNewProcess(const command_t *command, char *new_path) {
    if (command == NULL || new_path == NULL) return -1;
    static char *new_argv[_POSIX_ARG_MAX] = {0};
    for (int i = 0; i < command->argc; ++i) new_argv[i] = command->argv[i];
    logProcessCreation(new_argv, command->argc);
    new_argv[command->argc - 1] = new_path;
    if (execv(new_argv[0], new_argv) == -1) {
        perror("xmod: exec");
        return -1;
    }
    return 0;
}

int changeFileMode(const command_t *command, struct stat *buf) {
    if (command == NULL || buf == NULL) return -1;
    ++number_of_files;
    mode_t mode = buf->st_mode;
    mode_t persistent_bits = mode & UNRELATED_BITS;
    mode_t new_mode;
    if (command->action == ACTION_REMOVE) {
        new_mode = modeRemovingPermissions(mode, command->mode);
    } else if (command->action == ACTION_ADD) {
        new_mode = modeAddingPermissions(mode, command->mode);
    } else if (command->action == ACTION_PARTIAL_SET) {
        new_mode = modeSettingPartialPermissions(mode, command->mode);
    } else if (command->action == ACTION_SET) {
        new_mode = command->mode | persistent_bits;
    } else {
        return 1;
    }
    if (chmod(command->path, new_mode) == -1) {
        perror("xmod: failed to change permissions");
        return -1;
    }
    if (new_mode != mode) ++number_of_modified_files;
    if (logChangePermission(command, buf->st_mode, new_mode, false)) return -1;
    return 0;
}

int changeFolderMode(const command_t *command) {
    if (command == NULL) return -1;
    // Read the folder
    DIR *dir = opendir(command->path);
    if (dir == NULL) {
        perror("xmod: failed to open folder");
        return -1;
    }
    struct dirent *d;
    while ((d = readdir(dir)) != NULL) {
        // sleep(1); //TODO: Uncomment me to check signal handling
        if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0) continue;
        char new_path[PATH_MAX] = {0};
        command_t new_command = *command;
        if (concatenateFolderFilenamePath(command->path, d->d_name, new_path, sizeof(new_path))) continue;
        new_command.path = new_path;

        struct stat buf;
        if (openFile(new_path, &buf)) continue;
        if (d->d_type == DT_DIR) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("xmod: fork");
                continue;
            } else if (pid == 0) {
                //COMBACK: Print error message
                if (createNewProcess(command, new_path)) {}
                _exit(1);
            } else {
                continue;
            }
        } else if (d->d_type == DT_LNK) {
            //COMBACK: Might need to rethink function sequence
            if (printMessage(0, 0, command, true)) continue;
        } else {
            //COMBACK: Print error message
            if (changeFileMode(&new_command, &buf)) continue;
        }
    }
    // COMBACK: Check if this can be used as a file descriptor and closed with _exit(). If not, rethink signal handling.
    //COMBACK: Print error message
    if (closedir(dir)) return -1;
    return 0;
}

int changeMode(const command_t *command) {
    if (command == NULL) return -1;
    struct stat buf;
    //COMBACK: Error message
    if (openFile(command->path, &buf)) return -1;
    //COMBACK: Error message
    if (changeFileMode(command, &buf)) return -1;

    if (S_ISDIR(buf.st_mode) && command->recursive) {
        //COMBACK: Error message
        if (changeFolderMode(command)) return -1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    //COMBACK: Find a better way to structure this
    //COMBACK: Look into error return value
    int fd = openLogFile(isParentProcess());
    if (isParentProcess() && fd == -1)
        fprintf(stderr, "Variable LOG_FILENAME not defined.\n");
    closeLogFile(fd);
    command_t result;
    if (parseCommand(argc, argv, &result)) leave(-1);
    // COMBACK: Print error message
    if (logProcessCreation(argv, argc)) {}
    // COMBACK: Print error message
    if (setUpSignals(result.path)) {}
    if (changeMode(&result)) leave(-1);
    leave(0);
}
