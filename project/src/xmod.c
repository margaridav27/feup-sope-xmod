#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/io.h"
#include "../include/log.h"
#include "../include/parse.h"
#include "../include/time_ctrl.h"
#include "../include/xmod.h"


int changeFileMode(command_t *command) {
    struct stat buf;
    errno = 0;
    if (stat(command->path, &buf) == -1) {
        fprintf(stderr, "xmod: cannot access '%s': %s\n",
                command->path, strerror(errno));
        if (command->verbose) {
            printFailedMessage(command->path, command->mode);
        }
        return 1;
    }

    mode_t mode = buf.st_mode;
    mode_t persistent_bits = __S_IFMT & mode;

    if (command->action == ACTION_REMOVE) {
        mode &= ~(command->mode); // Remove the relevant bits, keeping others
    } else if (command->action == ACTION_ADD) {
        mode |= command->mode; // Add the relevant bits, keeping others
    } else if (command->action == ACTION_SET) {
        mode = persistent_bits | command->mode; // Set only the relevant bits
    } else if (command->action == ACTION_PARTIAL_SET) {
        if (command->mode & S_IRWXO) {
            mode &= (~S_IRWXO);
        } else if (command->mode & S_IRWXG) {
            mode &= (~S_IRWXG);
        } else if (command->mode & S_IRWXU) {
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

    // Logging permissions modification of files/directories - FILE_MODF
    char info[2048];
    sprintf(info, "%s : %o : %o", command->path, buf.st_mode, mode);
    logEvent(getpid(), FILE_MODF, info);

    return 0;
}

int changeMode(command_t *command, int argc, char *argv[]) {

    struct stat buf;
    errno = 0;
    if (stat(command->path, &buf) == -1) {
        fprintf(stderr, "xmod: cannot access '%s': %s\n", command->path,
                strerror(errno));
        if (command->verbose) {
            printFailedMessage(command->path, command->mode);
        }
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
                } else if (pid == 0) { // Child process
                    // Preparing new argv
                    char **new_argv = malloc((argc + 1) * sizeof(*new_argv));
                    for (int i = 0; i < argc; ++i) {
                        new_argv[i] = strdup(argv[i]);
                    }
                    size_t length =
                            strlen(command->path) + 2 + strlen(de->d_name);
                    new_argv[argc - 1] = realloc(new_argv[argc - 1], length);
                    strcpy(new_argv[argc - 1], command->path);
                    strcat(new_argv[argc - 1], "/");
                    strcat(new_argv[argc - 1], de->d_name);
                    new_argv[argc] = NULL;

                    // Setting up the environment variable
                    setenv("IS_FIRST", "0", 0);


                    // Logging process creation - PROC_CREAT
                    char info[1024];
                    strcpy(info, new_argv[0]);
                    for (int i = 1; i < argc; ++i) {
                        strcat(info, " ");
                        strcat(info, new_argv[i]);
                    }
                    logEvent(getpid(), PROC_CREAT, info);

                    execv(new_argv[0], new_argv);

                    for (int i = 0; i <= argc; ++i) { free(new_argv[i]); }

                    free(new_argv);
                } else { // Parent process
                    int childRetval;
                    wait(&childRetval); // Waiting for the child process to finish his execution

                    if (childRetval != 0) {
                        perror("Invalid value return from child");
                    }

                    // Logging process termination - PROC_EXIT
                    char info[1024];
                    sprintf(info, "%d", childRetval);
                    logEvent(getpid(), PROC_EXIT, info);
                }
            } else if (de->d_type == DT_LNK) {
                char *n = malloc(
                        strlen(command->path) + strlen(de->d_name) + 2);
                if (n == NULL)
                    continue; // COMBACK: Insert very special error message
                sprintf(n, "%s/%s", command->path, de->d_name);
                struct stat buf;
                errno = 0;
                if (stat(n, &buf) == -1) {
                    fprintf(stderr, "xmod: cannot access '%s': %s\n",
                            n, strerror(errno));
                    if (command->verbose) {
                        printFailedMessage(n, command->mode);
                    }
                } else if (command->verbose) {
                    printSymbolicMessage(n);
                }
            } else {
                command_t c = *command;

                char *n = malloc(
                        strlen(command->path) + strlen(de->d_name) + 2);
                if (n == NULL)
                    continue; // COMBACK: Insert very special error message

                sprintf(n, "%s/%s", command->path, de->d_name);
                c.path = n;

                changeFileMode(&c);

                free(n);
            }
        }
        closedir(d);
    }

    if (errno != 0) {
        fprintf(stderr, "xmod: error while reading directory '%s': %s\n",
                command->path, strerror(errno));
        return 1;
    }

    return 0;
}


int open_file(const char *path, struct stat *buf) {
    if (buf == NULL) return 1;
    int r = stat(path, buf);
    if (r == -1) {
        perror("chmod: failed to open file");
        return 1;
    }
    return 0;
}

int change_file_mode(const command_t *command) {
    struct stat buf;
    if (open_file(command->path, &buf)) {
        if (command->verbose) fprintf(stderr, "xmod: cannot access %s\n", command->path);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    getStartTime();
    getpid() == getpgid(0) ? openLogFile("w") : openLogFile("a");
    command_t result;
    if (parseCommand(argc, argv, &result)) return 1;
    changeMode(&result, argc, argv);
    closeLogFile();
    return 0;
}
