#include <dirent.h> // opendir(), readdir(), DT_DIR, DT_LNK
#include <stdbool.h> // true, false
#include <stdio.h> // perror(),
#include <string.h> // strcmp()
#include <sys/stat.h> // chmod(), struct stat
#include <unistd.h> // execv(),
#include <limits.h> // MAX_PATH

#include "../include/xmod.h"
#include "../include/parse.h" // parseCommand()
#include "../include/io.h" // printMessage()
#include "../include/utils.h"
#include "../include/signals.h" // setUpSignals()
#include "../include/log.h" // logChangePermission(), leave(), modeRemovingPermissions(), modeAddingPermissions(), modeSettingPartialPermissions(), concatenateFolderFilenamePath()

int numberOfFilesFound = 0, numberOfModifiedFiles = 0, numberOfChildren = 0;

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
    ++numberOfFilesFound;
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
    if (new_mode != mode) ++numberOfModifiedFiles;
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
        sleep(1); //TODO: Uncomment me to check signal handling
        // Skip the current and previous directory: those were handled in previous calls
        if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0) continue;
        char new_path[PATH_MAX] = {0};
        command_t new_command = *command;
        if (concatenateFolderFilenamePath(command->path, d->d_name, new_path, sizeof(new_path))) continue;
        new_command.path = new_path;

        struct stat buf;
        if (openFile(new_path, &buf)) continue; // Failed to open this file, next.
        if (d->d_type == DT_DIR) {              // Directory -> new process
            pid_t pid = fork();
            if (pid < 0) {                      // Fork error
                perror("xmod: fork");
                continue;                       // Next file
            } else if (pid == 0) {              // Child process, new invocation with different path.
                //COMBACK: Print error message
                if (executeNewProcess(command, new_path)) {}
            } else {                            // Parent process, keep going.
                ++numberOfChildren;             // Another child
                continue;                       // Next file
            }
        } else if (d->d_type == DT_LNK) {       // Symbolic link when iterating folder: do not change
            //COMBACK: Might need to rethink function sequence
            if (printMessage(0, 0, &new_command, true)) continue; // Failure? Next file.
        } else {                                // File: change
            //COMBACK: Print error message
            if (changeFileMode(&new_command, &buf)) continue;  // Failure? Next file.
        }
    }
    // COMBACK: Check if this can be used as a file descriptor and closed with _exit(). If not, rethink signal handling.
    //COMBACK: Print error message
    if (closedir(dir)) return -1; // Try to close the folder.
    return 0;
}

int changeMode(const command_t *command) {
    if (command == NULL) return -1;
    struct stat buf;
    //COMBACK: Error message
    if (openFile(command->path, &buf)) return -1;           // Try to open the file
    //COMBACK: Error message
    if (changeFileMode(command, &buf)) return -1;           // Try to change the file's mode

    if (S_ISDIR(buf.st_mode) && command->recursive) { // Recursive option + Directory? Change it.
        //COMBACK: Error message
        if (changeFolderMode(command)) return -1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    //COMBACK: Find a better way to structure this
    //COMBACK: Look into error return value
    int fd = openLogFile(isParentProcess()); // Try to open the logfile
    if (isParentProcess() && fd == -1)
        fprintf(stderr, "Variable LOG_FILENAME not defined.\n");
    closeLogFile(fd);
    command_t result;
    if (parseCommand(argc, argv, &result)) leave(-1); // Failure parsing: premature exit
    // COMBACK: Print error message
    if (logProcessCreation(argv, argc)) {}
    // COMBACK: Print error message
    if (setUpSignals(result.path)) {}
    if (changeMode(&result)) leave(-1); // Failure: exit with error code
    leave(0);
}
