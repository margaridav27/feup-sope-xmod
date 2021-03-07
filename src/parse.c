#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "parse.h"

int parseCommand(int argc, char *argv[])
{
    command_t cmd;
    cmd.permissions = 0;
    cmd.user = 0;

    int opt;
    while ((opt = getopt(argc, argv, "vcR")) != -1)
    {
        switch (opt)
        {
        case 'v':
            cmd.verbose = true;
            break;
        case 'c':
            cmd.changes = true;
            break;
        case 'R':
            cmd.recursive = true;
            break;
        case '?':
            //error
            break;
        }
    }

    char *modeStr = argv[argc - 2];

    for (int j = 0; j < strlen(modeStr); j++)
    {
        if (j == 0)
        {
            switch (modeStr[j])
            {
            case 'u':
                printf("here1\n");
                cmd.user = 1;
                break;
            case 'g':
                cmd.user = 2;
                break;
            case 'o':
                cmd.user = 3;
                break;
            case 'a':
                cmd.user = 4;
                break;
            default:
                break;
            }
        }
        if (j == 1)
        {
                }
        //COMBACK:
        //return 0;
    }
}