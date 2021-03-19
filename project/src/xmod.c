#include "../include/xmod.h"

#include <dirent.h> // opendir(), readdir(), DT_DIR, DT_LNK
#include <limits.h> // MAX_PATH
#include <stdbool.h> // true, false
#include <stdio.h> // perror(),
#include <string.h> // strcmp()
#include <sys/stat.h> // chmod(), struct stat
#include <unistd.h> // execv(),
#include <errno.h>

#include "../include/io.h" // printMessage()
#include "../include/log.h" // logChangePermission(), leave(), modeRemovingPermissions(), modeAddingPermissions(), modeSettingPartialPermissions(), concatenateFolderFilenamePath()
#include "../include/parse.h" // parseCommand()
#include "../include/signals.h" // setUpSignals()

int number_of_files_found = 0, number_of_modified_files = 0, number_of_children = 0;

int executeNewProcess(const command_t *command, char *new_path) {
    if (command == NULL || new_path == NULL) return -1;
    static char *new_argv[_POSIX_ARG_MAX];
    for (int i = 0; i < command->argc; ++i) new_argv[i] = command->argv[i]; // Copy arguments
    new_argv[command->argc - 1] = new_path;                                     // Set path
    logProcessCreation(new_argv, command->argc);
    if (execv(new_argv[0], new_argv) == -1) {
        perror("xmod: exec");
        return -1;
    }
    return 0;
}

int changeFileMode(const command_t *command, struct stat *buf) {
    if (command == NULL || buf == NULL) return -1;
    ++number_of_files_found;
    mode_t mode = buf->st_mode;
    mode_t persistent_bits = mode & UNRELATED_BITS; // Save bits unrelated to our permissions
    // Calculate a new mode
    mode_t new_mode;
    if (command->action == ACTION_REMOVE) new_mode = modeRemovingPermissions(mode, command->mode);
    else if (command->action == ACTION_ADD) new_mode = modeAddingPermissions(mode, command->mode);
    else if (command->action == ACTION_PARTIAL_SET) new_mode = modeSettingPartialPermissions(mode, command->mode);
    else if (command->action == ACTION_SET) new_mode = command->mode | persistent_bits;
    else return 1; // Invalid action

    // Try to change the permissions
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
        fprintf(stderr, "chmod: cannot read directory '%s': %s\n", command->path, strerror(errno));
        return -1;
    }
    struct dirent *d;
    while ((d = readdir(dir)) != NULL) {
        // Skip the current and previous directory: those were handled in previous calls
        if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0) continue;
        char new_path[PATH_MAX] = {0};
        command_t new_command = *command;
        if (concatenateFolderFilenamePath(command->path, d->d_name, new_path, sizeof(new_path))) continue;
        new_command.path = new_path;

        struct stat buf;
        if (d->d_type == DT_LNK) {       // Symbolic link when iterating folder: do not change
            if (printMessage(0, 0, &new_command, true)) continue; // Failure? Next file.
            continue; // Next.
        }

        if (openFile(new_path, &buf)) continue; // Failed to open this file, next.
        if (d->d_type == DT_DIR) {              // Directory -> new process
            pid_t pid = fork();
            if (pid < 0) {                      // Fork error
                perror("xmod: fork");
                continue;                       // Next file
            } else if (pid == 0) {              // Child process, new invocation with different path.
                if (executeNewProcess(command, new_path)) _exit(-1);
            } else {                            // Parent process, keep going.
                ++number_of_children;             // Another child
                continue;                       // Next file
            }
        } else {                                // File: change
            if (changeFileMode(&new_command, &buf)) continue;  // Failure? Next file.
        }
    }
    // Try to close the folder on normal exit.
    // In the case of a premature exit, there is no need since _exit() also closes the file descriptors.
    if (closedir(dir)) return -1;
    return 0;
}

int changeMode(const command_t *command) {
    if (command == NULL) return -1;
    struct stat buf;
    if (openFile(command->path, &buf)) return -1;           // Try to open the file
    if (changeFileMode(command, &buf)) return -1;           // Try to change the file's mode

    if (S_ISDIR(buf.st_mode) && command->recursive) { // Recursive option + Directory? Change it.
        if (changeFolderMode(command)) return -1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int fd = openLogFile(isParentProcess()); // Try to open the logfile
    if (isParentProcess() && fd == -1)
        fprintf(stderr, "Variable LOG_FILENAME not defined.\n");
    closeLogFile(fd);
    command_t result;
    if (parseCommand(argc, argv, &result)) leave(-1); // Failure parsing: premature exit
    if (logProcessCreation(argv, argc)) {}
    if (setUpSignals(result.path)) {}
    if (changeMode(&result)) leave(-1); // Failure: exit with error code
    leave(0);
}
