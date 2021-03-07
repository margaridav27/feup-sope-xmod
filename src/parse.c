#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

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

    //OCTAL NUMBER HANDLER
    if(modeStr[0] == '0'){
            cmd.octalNumber = strtol(&modeStr[0],'\0',10);
            printf("octal number = %d\n", cmd.octalNumber);
    }
    else { //MODE "normal" HANDLER
        switch (modeStr[0]){ // define user
            case 'u':
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
                fprintf(stderr, "Error no user defined\n");
                break;
        }

        switch (modeStr[1]){ // define operation
            case '=':
                cmd.op = '=';
                break;
            case '+':
                cmd.op = '+';
                break;
            case '-':
                cmd.op = '-';
                break;
            default:
                fprintf(stderr, "Error no operator defined\n");
                break;
        }
        for (int j = 2; j < strlen(modeStr); j++) // definde permisions changes
        {
            /*switch (modeStr[1]){
                case 'r':
                    cmd.op = '=';
                    break;
                case 'w':
                    cmd.op = '+';
                    break;
                case 'x':
                    cmd.op = '-';
                    break;
                default:
                    fprintf(stderr, "Error no operator defined\n");
                    break;
            }*/
        }
    }
    

    cmd.path = argv[argc - 1];
}