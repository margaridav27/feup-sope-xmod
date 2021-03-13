#ifndef PROJECT_IO_H
#define PROJECT_IO_H

#include <sys/types.h>

int printChangeMessage(const char *path, mode_t previous_mode, mode_t new_mode);

int printRetainMessage(const char *path, mode_t mode);

int printSymbolicMessage(const char *path);

int printFailedMessage(const char *path, mode_t new_mode);

int printNoPermissionMessage(const char *path);

int parseModeToString(mode_t mode, char *str);

#endif //PROJECT_IO_H
