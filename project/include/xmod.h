#ifndef PROJECT_INCLUDE_XMOD_H_
#define PROJECT_INCLUDE_XMOD_H_

#include <sys/stat.h> // struct stat, S_IFMT

#include "../include/utils.h" // command_t

#define UNRELATED_BITS S_IFMT

int main(int argc, char *argv[]);

int changeMode(const command_t *command);

int changeFileMode(const command_t *command, struct stat *buf);

int changeFolderMode(const command_t *command);

int executeNewProcess(const command_t *command, char *new_path);

#endif //PROJECT_INCLUDE_XMOD_H_
