#ifndef PROJECT_INCLUDE_UTILS_H_
#define PROJECT_INCLUDE_UTILS_H_

#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PERMISSIONS_OTHERS S_IRWXO
#define PERMISSIONS_GROUP S_IRWXG
#define PERMISSIONS_USER S_IRWXU

typedef enum {
    ACTION_REMOVE = '-',
    ACTION_ADD = '+',
    ACTION_PARTIAL_SET = '=',
    ACTION_SET,
} action_t;

typedef struct {
    bool verbose;
    bool changes;
    bool recursive;

    action_t action;
    mode_t mode;
    const char *path;
    char *const *argv;
    int argc;
} command_t;


bool isParentProcess(void);

void leave(int ret);

int openFile(const char *path, struct stat *buf);

int concatenateFolderFilenamePath(const char *folder_path, const char *file_name, char *dest);

mode_t removePermissions(mode_t old_mode, mode_t new_mode);

mode_t addPermissions(mode_t old_mode, mode_t new_mode);

mode_t setPartialPermissions(mode_t old_mode, mode_t new_mode);

void convert_integer_to_string(int n, char *dest);

void convert_signal_number_to_string(int sig_no, const char **dest);
#endif // PROJECT_INCLUDE_UTILS_H_
