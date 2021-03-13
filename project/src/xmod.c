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
#include <limits.h>

#include "../include/io.h"
#include "../include/log.h"
#include "../include/parse.h"
#include "../include/time_ctrl.h"
#include "../include/xmod.h"

int create_new_process(const command_t *command, char *new_path) {
    static char *new_argv[_POSIX_ARG_MAX] = {};
    for (int i = 0; i < command->argc; ++i) new_argv[i] = command->argv[i];
    new_argv[command->argc - 1] = new_path;
    if (execv(new_argv[0], new_argv)) {
        perror("xmod: exec");
        return 1;
    }
    return 0;
}

int concatenate_folder_filename_path(const char *folder_path, const char *file_name, char dest[]) {
    if (folder_path == NULL || file_name == NULL || dest == NULL) return 1;
    strcpy(dest, folder_path);
    strcat(dest, "/");
    strcat(dest, file_name);
    return 0;
}

int open_file(const char *path, struct stat *buf) {
    if (path == NULL || buf == NULL) return 1;
    int r = stat(path, buf);
    if (r == -1) {
        perror("xmod: failed to open file");
        return 1;
    }
    return 0;
}

mode_t remove_permissions(mode_t old_mode, mode_t new_mode) {
    return old_mode & ~(new_mode); // Remove the relevant bits, keeping others
}

mode_t add_permissions(mode_t old_mode, mode_t new_mode) {
    return old_mode | new_mode; // Add the relevant bits, keeping others
}


mode_t set_partial_permissions(mode_t old_mode, mode_t new_mode) {
    if (new_mode & S_IRWXO) {
        old_mode &= (~S_IRWXO);
    } else if (new_mode & S_IRWXG) {
        old_mode &= (~S_IRWXG);
    } else if (new_mode & S_IRWXU) {
        old_mode &= (~S_IRWXU);
    }
    return old_mode | new_mode;
}

int log_change_permission(const command_t *command, mode_t old_mode, mode_t new_mode) {
    char info[2048] = {};
    sprintf(info, "%s : %o : %o", command->path, old_mode, new_mode);
    logEvent(getpid(), FILE_MODF, info);
    //COMBACK: Properly print this message
    print_message(new_mode, old_mode, command);
    return 0;
}

int log_process_creation(char **argv, int argc) {
    char info[2048] = {};
    sprintf(info + strlen(info), "%s", argv[0]);
    for (int i = 1; i < argc; ++i) {
        if (i != argc) sprintf(info + strlen(info), " ");
        sprintf(info + strlen(info), "%s", argv[i]);
    }
    logEvent(getpid(), PROC_CREAT, info);
    return 0;
}

int log_process_exit(int ret) {
    char info[2048] = {};
    sprintf(info, "%d", ret);
    logEvent(getpid(), PROC_EXIT, info);
    return 0;
}

int change_file_mode(const command_t *command, struct stat *buf) {
    mode_t mode = buf->st_mode;
    mode_t persistent_bits = mode & S_IFMT;
    mode_t new_mode;
    if (command->action == ACTION_REMOVE)
        new_mode = remove_permissions(mode, command->mode);
    else if (command->action == ACTION_ADD)
        new_mode = add_permissions(mode, command->mode);
    else if (command->action == ACTION_PARTIAL_SET)
        new_mode = set_partial_permissions(mode, command->mode);
    else if (command->action == ACTION_SET) {
        new_mode = command->mode | persistent_bits;
    } else {
        return 1;
    }
    if (chmod(command->path, new_mode) == -1) {
        perror("xmod: failed to change permissions");
        return 1;
    }
    log_change_permission(command, buf->st_mode, new_mode);
    return 0;
}

int change_folder_mode(const command_t *command) {
    // Read the folder
    DIR *dir = opendir(command->path);
    if (dir == NULL) {
        perror("xmod: failed to open folder");
        return 1;
    }
    struct dirent *d;
    while ((d = readdir(dir)) != NULL) {
        //COMBACK: This might be a good place to install our handler (in case there are many subfolders)
        if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0) continue;
        char new_path[PATH_MAX];
        command_t new_command = *command;
        concatenate_folder_filename_path(command->path, d->d_name, new_path);
        new_command.path = new_path;

        struct stat buf;
        if (open_file(new_path, &buf)) continue;
        if (S_ISDIR(buf.st_mode)) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("xmod: fork");
                continue;
            } else if (pid == 0) {
                create_new_process(command, new_path);
            } else {
                continue;
            }
        } else
            change_file_mode(&new_command, &buf);
    }
    //COMBACK: This might also be a good place to install our handler (small folders fork fast)
    //COMBACK: Properly wait for children: this solution won't allow us to receive SIGINT from here on
    int ret;
    while (wait(&ret) > 0) {}
    return 0;
}

int change_mode(const command_t *command) {
    struct stat buf;
    if (open_file(command->path, &buf)) return 1;
    if (change_file_mode(command, &buf)) return 1;

    if (S_ISDIR(buf.st_mode) && command->recursive) {
        if (change_folder_mode(command)) return 1;
    }
    return 0;
}

bool is_parent_process(void) {
    return getpid() == getpgid(0);
}

void leave(int ret) {
    log_process_exit(ret);
    exit(ret);
}

int main(int argc, char *argv[]) {
    getStartTime();
    is_parent_process() ? openLogFile("w") : openLogFile("a");
    command_t result;
    if (parseCommand(argc, argv, &result)) leave(1);
    log_process_creation(argv, argc);
    change_mode(&result);
    leave(0);
    closeLogFile();
    return 0;
}
