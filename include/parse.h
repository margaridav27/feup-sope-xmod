#ifndef PARSE
#define PARSE

#include <stdbool.h>

typedef enum {
    ACTION_REMOVE, ACTION_ADD, ACTION_SET
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