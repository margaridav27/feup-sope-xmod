#ifndef PROJECT_INCLUDE_IO_H_
#define PROJECT_INCLUDE_IO_H_

#include <sys/types.h> // mode_t
#include "../include/utils.h" // command_t

int printChangeMessage(const char *path, mode_t previous_mode, mode_t new_mode, char *info, unsigned int size);

int printRetainMessage(const char *path, mode_t mode, char *info, unsigned int size);

int printSymbolicMessage(const char *path, char *info, unsigned int size);

//COMBACK: Will we be using this?
int printFailedMessage(const char *path, mode_t new_mode);

//COMBACK: Will we be using this?
int printNoPermissionMessage(const char *path);

int parseModeToString(mode_t mode, char *str, unsigned int size);

mode_t clearExtraBits(mode_t mode);

int printMessage(mode_t new_mode, mode_t old_mode, const command_t *command, bool isLink);

#endif //PROJECT_INCLUDE_IO_H_
