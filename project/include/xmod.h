#ifndef PROJECT_INCLUDE_XMOD_H_
#define PROJECT_INCLUDE_XMOD_H_

#include "../include/parse.h"

int changeMode(command_t *command, int argc, char *argv[]);

int changeFileMode(command_t *command);

int printRetainMessage(const char *path, mode_t mode);

int printChangeMessage(const char *path, mode_t previous_mode, mode_t new_mode);

int parseModeToString(mode_t mode, char *str);

int printNoPermissionMessage(const char *path);

int printSymbolicMessage(const char *path);

int printFailedMessage(const char *path, mode_t new_mode);

#endif // PROJECT_INCLUDE_XMOD_H_
