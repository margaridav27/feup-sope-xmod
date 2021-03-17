#ifndef PROJECT_INCLUDE_XMOD_H_
#define PROJECT_INCLUDE_XMOD_H_

#include <sys/types.h>
#include <sys/stat.h>
#include "../include/parse.h"

#define UNRELATED_BITS S_IFMT

int createNewProcess(const command_t *command, char *new_path);

int changeFileMode(const command_t *command, struct stat *buf);

int changeFolderMode(const command_t *command);

int changeMode(const command_t *command);

void leave(int ret);

int main(int argc, char *argv[]);

#endif //PROJECT_INCLUDE_XMOD_H_
