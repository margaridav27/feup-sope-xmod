#ifndef PARSE
#define PARSE

#include <stdbool.h>

typedef struct {
    bool verbose;
    bool changes;
    bool recursive;

    int user;

    int op;

    int permissions;

    char *path;
} command_t;

int parseCommand(int argc, char *argv[]);

#endif