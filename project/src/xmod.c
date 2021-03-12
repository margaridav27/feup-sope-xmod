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
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>

#include "../include/xmod.h"
#include "../include/log.h"
#include "../include/parse.h"
#include "../include/time_ctrl.h"

static bool logfileUnavailable;

int changeFileMode(command_t *command) {
    // just to test...
    char testPath[1024];
    strcpy(testPath, command->path);
    logEvent(getpid(), PROC_CREAT, testPath);

    struct stat buf;
    errno = 0;
    if (stat(command->path, &buf) == -1) {
        fprintf(stderr, "xmod: cannot access '%s': %s\n",
                    command->path, strerror(errno));
        return 1;
    }

    mode_t mode = buf.st_mode;
    mode_t persistent_bits = __S_IFMT & mode;

    if (command->action == ACTION_REMOVE) {
        mode &= ~(command->mode); // Remove the relevant bits, keeping others
    } else if (command->action == ACTION_ADD) {
        mode |= command->mode; // Add the relevant bits, keeping others
    } else if (command->action == ACTION_SET) {
        mode = persistent_bits | command->mode;  // Set only the relevant bits
    } else if (command->action == ACTION_PARCIAL_SET) {
        if(command->mode & S_IRWXO){ 
            mode &= (~S_IRWXO);
        }else if(command->mode & S_IRWXG){
            mode &= (~S_IRWXG);
        }else if(command->mode & S_IRWXU){
            mode &= (~S_IRWXU);
        }
        mode |= command->mode;
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

int changeMode(command_t *command, int argc, char *argv[]) {

    struct stat buf;
    errno = 0;
    if (stat(command->path, &buf) == -1) {
        fprintf(stderr, "xmod: cannot access '%s': %s\n", command->path, strerror(errno));
        return 1;
    }

    changeFileMode(command);

    if (S_ISDIR(buf.st_mode) && command->recursive) {   
        errno = 0;
        DIR *d = opendir(command->path);

        if (d == NULL) {
            fprintf(stderr, "xmod: cannot read directry '%s': %s\n",
                    command->path, strerror(errno));
            return 1;
        }

        struct dirent *de;
        errno = 0;
        //S_ISLNK
        while ((de = readdir(d)) != NULL) {
            if (strcmp(de->d_name, ".") == 0 ||
                strcmp(de->d_name, "..") == 0)
                continue;

            if (de->d_type == DT_DIR) {
                pid_t pid = fork();

                if (pid == -1) { // Failed to fork
                    perror("Fork Error");
                    return 1;
                } else if (pid == 0) { // Child process

                    // Preparing new argv
                    char **new_argv = malloc((argc + 1) * sizeof(*new_argv));
                    for (int i = 0; i < argc; ++i) { new_argv[i] = strdup(argv[i]); }
                    size_t length = strlen(command->path) + 2 + strlen(de->d_name);
                    new_argv[argc - 1] = realloc(new_argv[argc - 1], length);
                    strcpy(new_argv[argc - 1], command->path);
                    strcat(new_argv[argc - 1], "/");
                    strcat(new_argv[argc - 1], de->d_name);
                    new_argv[argc] = NULL;
                    
                    // Setting up the environment variable                                      
                    setenv("IS_FIRST", "0", 0); 

                    execv(new_argv[0], new_argv);

                    for (int i = 0; i <= argc; ++i) { free(new_argv[i]); }

                    free(new_argv);
                } else { // Parent process
                    int childRetval;
                    wait(&childRetval); // Waiting for the child process to finish his execution
                    if (childRetval != 0) {
                        perror("Invalid value return from child");
                    }
                }
            } else if (de->d_type == DT_LNK) {
                char *n = malloc(strlen(command->path) + strlen(de->d_name) + 1);
                if (n == NULL) continue;  // COMBACK: Insert very special error message
                sprintf(n, "%s/%s", command->path, de->d_name);
                printSymbolicMessage(n);
                free(n);
            } else {
                command_t c = *command;

                char *n = malloc(strlen(command->path) + strlen(de->d_name) + 1);
                if (n == NULL) continue; // COMBACK: Insert very special error message

                sprintf(n, "%s/%s", command->path, de->d_name);
                c.path = n;

                changeFileMode(&c);

                free(n);
            }
        }
        closedir(d);
        
    }

    if (errno != 0) {
        fprintf(stderr, "xmod: error while reading directory '%s': %s\n", command->path, strerror(errno));
        return 1;
    }

    return 0;
}

int printChangeMessage(const char *path, mode_t previous_mode, mode_t new_mode) {
    char new_mode_str[] = "---------", previous_mode_str[] = "---------";
    parseModeToString(new_mode, new_mode_str);
    parseModeToString(previous_mode, previous_mode_str);
    printf("%d - Mode of '%s' changed from 0%o (%s) to 0%o (%s)\n", getpid(), path,
           previous_mode, previous_mode_str, new_mode,
           new_mode_str);
    fflush(stdout);
    return 0;
}

int printRetainMessage(const char *path, mode_t mode) {
    char mode_str[] = "---------";
    parseModeToString(mode, mode_str);
    printf("%d - Mode of '%s' retained as 0%o (%s)\n", getpid(), path, mode, mode_str);
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
    fprintf(stderr, "xmod: changing permissions of '%s': Operation not permitted\n", path);
    fflush(stdout);
    return 0;
}

int printSymbolicMessage(const char *path) {
    printf("neither symbolic link '%s' nor referent has been changed\n", path);
    fflush(stdout);
    return 0;
}

int main(int argc, char *argv[]) {

    printf("\n%d\t%s\n", getpid(), getenv("IS_FIRST"));
    
    if (getenv("IS_FIRST") == NULL) { // Initial process
        setStartTime();
        logfileUnavailable = initLog("w"); // Initially, logfile is supposed be truncated
    } else { // Not initial process
        restoreStartTime();
        logfileUnavailable = initLog("a"); // Child process won't truncate the logfile
    }

    if (logfileUnavailable) {
        fprintf(stderr, "Logfile not available - events won't be registered.\n");
    }

    command_t result;
    if (parseCommand(argc, argv, &result)) {
        fprintf(stderr, "Could not parse command.\n");
        return 1;
    }

    changeMode(&result, argc, argv);

    if (!logfileUnavailable) { closeLog(); }
        
    return 0;
}
