#ifndef XMOD
#define XMOD

#include "parse.h"

int changeMode(command_t *command);

int changeFileMode(command_t *command);

int changeFolderMode(command_t *command);

int printRetainMessage(command_t *command);

int printChangeMessage(mode_t lastMode, command_t *command);

int parseModeToString(mode_t mode, char* str);

#endif