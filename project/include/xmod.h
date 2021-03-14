#ifndef PROJECT_INCLUDE_XMOD_H_
#define PROJECT_INCLUDE_XMOD_H_

#include <sys/types.h>
#include "../include/parse.h"

int createNewProcess(const command_t *command, char *new_path);

int concatenateFolderFilenamePath(const char *folder_path, const char *file_name, char *dest);

int openFile(const char *path, struct stat *buf);

mode_t removePermissions(mode_t old_mode, mode_t new_mode);

mode_t addPermissions(mode_t old_mode, mode_t new_mode);

mode_t setPartialPermissions(mode_t old_mode, mode_t new_mode);

int logChangePermission(const command_t *command, mode_t old_mode, mode_t new_mode, bool isLink);

int logProcessCreation(char **argv, int argc);

int logProcessExit(int ret);

int changeFileMode(const command_t *command, struct stat *buf, bool isLink);

int changeFolderMode(const command_t *command);

int changeMode(const command_t *command);

bool isParentProcess(void);

void leave(int ret);

int main(int argc, char *argv[]);

#endif //PROJECT_INCLUDE_XMOD_H_
