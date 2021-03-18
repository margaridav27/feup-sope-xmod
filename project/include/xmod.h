#ifndef PROJECT_INCLUDE_XMOD_H_
#define PROJECT_INCLUDE_XMOD_H_

#include <sys/stat.h> // struct stat, S_IFMT
#include "../include/utils.h" // command_t

#define UNRELATED_BITS S_IFMT

int executeNewProcess(const command_t *command, char *new_path);

int changeFileMode(const command_t *command, struct stat *buf);

int changeFolderMode(const command_t *command);

int changeMode(const command_t *command);

void leave(int ret);

int main(int argc, char *argv[]);

#endif //PROJECT_INCLUDE_XMOD_H_
