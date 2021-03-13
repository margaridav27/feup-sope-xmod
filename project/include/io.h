#ifndef PROJECT_IO_H_
#define PROJECT_IO_H

#include <sys/types.h>
#include "parse.h"

int printChangeMessage(const char *path, mode_t previous_mode, mode_t new_mode);

int printRetainMessage(const char *path, mode_t mode);

int printSymbolicMessage(const char *path);

int printFailedMessage(const char *path, mode_t new_mode);

int printNoPermissionMessage(const char *path);

int parseModeToString(mode_t mode, char *str);

mode_t clear_extra_bits(mode_t mode);

int print_message(mode_t new_mode, mode_t old_mode, const command_t *command, bool isLink);
#endif //PROJECT_IO_H_
