#ifndef PROJECT_INCLUDE_IO_H_
#define PROJECT_INCLUDE_IO_H_

#include <sys/types.h>
#include "./parse.h"

int printChangeMessage(const char *path, mode_t previous_mode, mode_t new_mode, char *info, int size);

int printRetainMessage(const char *path, mode_t mode, char *info, int size);

int printSymbolicMessage(const char *path, char *info, int size);

int printFailedMessage(const char *path, mode_t new_mode);

int printNoPermissionMessage(const char *path);

int parseModeToString(mode_t mode, char *str);

mode_t clearExtraBits(mode_t mode);

int printMessage(mode_t new_mode, mode_t old_mode, const command_t *command, bool isLink);
#endif //PROJECT_INCLUDE_IO_H_
