#ifndef PROJECT_INCLUDE_PARSE_H_
#define PROJECT_INCLUDE_PARSE_H_

/*
 * A file's mode is of the form: - RWX RWX RWX
 *                                  U   G   O
*/
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../include/utils.h"

#define GROUP_POSITION 3
#define USER_POSITION 6

#define EXECUTE_BIT S_IXOTH
#define WRITE_BIT S_IWOTH
#define READ_BIT S_IROTH


int parseCommand(int argc, char *argv[], command_t *result);

#endif // PROJECT_INCLUDE_UTILS_H_
