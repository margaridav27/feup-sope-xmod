#ifndef PARSE
#define PARSE

/*
 * A file's mode is of the form: - RWX RWX RWX
 *                                  U   G   O
*/

#define GROUP_POSITION 3
#define USER_POSITION 6

#define EXECUTE_BIT 0
#define WRITE_BIT 1
#define READ_BIT 2

#define BIT(n) (1 << (n))

#include <fcntl.h>
#include <stdbool.h>

typedef enum {
    ACTION_REMOVE,
    ACTION_ADD,
    ACTION_SET
} action_t;

typedef struct {
    bool verbose;
    bool changes;
    bool recursive;

    action_t action;
    mode_t mode;
    const char *path;
} command_t;

int parseCommand(int argc, char *argv[], command_t *result);

#endif