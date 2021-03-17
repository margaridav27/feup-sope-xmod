#include "../include/parse.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int parseCommand(int argc, char *argv[], command_t *result) {
    memset(result, 0, sizeof(command_t)); // Clear all information
    result->argv = argv;
    result->argc = argc;

    // Parse command line arguments for options
    int opt;
    while ((opt = getopt(argc, argv, "vcR")) != -1) {
        if (opt == 'v') {
            result->verbose = true;
        } else if (opt == 'c') {
            result->changes = true;
        } else if (opt == 'R') {
            result->recursive = true;
        } else {
            return 1;
        }
    }

    // Incorrect number of arguments after flags: command is invalid
    if (argc - optind != 2) {
        fprintf(stderr, "xmod: missing operand\n");
        return 1;
    }

    // After processing options, this is the first argument
    const char *mode_string = argv[optind];
    // Mode has less than three characters: command is invalid
    if (strlen(mode_string) < 3) return 1;

    // End of the string
    const char *string_end = mode_string + strlen(mode_string);
    // Last character parsed
    char *process_end;
    int64_t mode = strtol(mode_string, &process_end, 8);

    if (process_end == string_end) {
        // Processed the whole string successfully
        result->action = ACTION_SET;
    } else {
        // Conversion was unsuccessful: parse the string
        unsigned char user = mode_string[0], change = mode_string[1];

        if (change == '-' || change == '+' || change == '=') {
            result->action = change;
        } else {
            fprintf(stderr, "xmod: invalid mode: '%s'\n", mode_string);
            return 1;
        }

        const char *permissions_string = mode_string + 2;
        for (unsigned int i = 0; i < strlen(permissions_string); ++i) {
            if (permissions_string[i] == 'r') {
                mode |= READ_BIT;
            } else if (permissions_string[i] == 'w') {
                mode |= WRITE_BIT;
            } else if (permissions_string[i] == 'x') {
                mode |= EXECUTE_BIT;
            } else {
                fprintf(stderr, "xmod: invalid mode: '%s'\n", mode_string);
                return 1;
            }
        }

        // Default mode is in the "others position"
        if (user == 'u') {
            mode <<= USER_POSITION;
        } else if (user == 'g') {
            mode <<= GROUP_POSITION;
        } else if (user == 'a') {
            mode |= (mode << USER_POSITION) | (mode << GROUP_POSITION);
            if (result->action == ACTION_PARTIAL_SET) {
                result->action = ACTION_SET;
            }
        } else if (user != 'o') {
            fprintf(stderr, "xmod: invalid mode: '%s'\n", mode_string);
            return 1;
        }
    }
    result->mode = (mode_t) mode;
    result->path = argv[optind + 1];
    return 0;
}
