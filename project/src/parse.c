#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "parse.h"

int parseCommand(int argc, char *argv[], command_t *result) {
    if (argc < 3) return 1; // Mandatory arguments: program_name (octal)mode file/dir
    memset(result, 0, sizeof(command_t)); // Clear all information

    // Parse command line arguments for options
    int opt;
    while ((opt = getopt(argc, argv, "vcR")) != -1) {
        switch (opt) {
            case 'v':
                result->verbose = true;
                break;
            case 'c':
                result->changes = true;
                break;
            case 'R':
                result->recursive = true;
                break;
            case '?':
                return 1; // Unknown option: command is invalid
        }
    }

    if (argc - optind < 2) return 1; // Not enough arguments after flags: command is invalid

    const char *mode_string = argv[optind]; // After processing options, this is the first argument
    if (strlen(mode_string) < 3) return 1; //Mode has less than three characters: command is invalid

    const char *string_end = mode_string + strlen(mode_string);
    char *process_end;
    long int mode = strtol(mode_string, &process_end, 8);

    if (process_end == string_end) result->action = ACTION_SET;
    else { // Conversion was unsuccessful: parse the string
        char user = mode_string[0], change = mode_string[1];

        if (change == '-') result->action = ACTION_REMOVE;
        else if (change == '+') result->action = ACTION_ADD;
        else if (change == '=') result->action = ACTION_SET;
        else return 1;

        const char *permissions_string = mode_string + 2;
        for (int i = 0; i < strlen(permissions_string); ++i) {
            if (permissions_string[i] == 'r') mode |= BIT(READ_BIT);
            else if (permissions_string[i] == 'w') mode |= BIT(WRITE_BIT);
            else if (permissions_string[i] == 'x') mode |= BIT(EXECUTE_BIT);
            else return 1;
        }

        // Default mode is in the "others position"
        if (user == 'u') mode <<= USER_POSITION;
        else if (user == 'g') mode <<= GROUP_POSITION;
        else if (user == 'a') mode |= (mode << USER_POSITION) | (mode << GROUP_POSITION);
        else if (user != 'o') return 1;
    }
    result->mode = (mode_t) mode;
    result->path = argv[optind + 1];
    return 0;
}