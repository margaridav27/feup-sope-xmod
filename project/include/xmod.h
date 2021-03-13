#ifndef PROJECT_XMOD_H
#define PROJECT_XMOD_H

#include <sys/types.h>
#include "../include/parse.h"

int create_new_process(const command_t *command, char *new_path);

int concatenate_folder_filename_path(const char *folder_path, const char *file_name, char dest[]);

int open_file(const char *path, struct stat *buf);

mode_t remove_permissions(mode_t old_mode, mode_t new_mode);

mode_t add_permissions(mode_t old_mode, mode_t new_mode);

mode_t set_partial_permissions(mode_t old_mode, mode_t new_mode);

int log_change_permission(const command_t *command, mode_t old_mode, mode_t new_mode, bool isLink);

int log_process_creation(char **argv, int argc);

int log_process_exit(int ret);

int change_file_mode(const command_t *command, struct stat *buf, bool isLink);

int change_folder_mode(const command_t *command);

int change_mode(const command_t *command);

bool is_parent_process(void);

void leave(int ret);

int main(int argc, char *argv[]);

#endif //PROJECT_XMOD_H
