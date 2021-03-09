#ifndef PARSE
#define PARSE

/*
 * A file's mode is of the form: - RWX RWX RWX
 *                                  U   G   O
*/
#include <stdbool.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <stdbool.h>

#define GROUP_POSITION 3
#define USER_POSITION 6

#define EXECUTE_BIT S_IXOTH
#define WRITE_BIT S_IWOTH
#define READ_BIT S_IROTH


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