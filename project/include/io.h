#ifndef PROJECT_INCLUDE_IO_H_
#define PROJECT_INCLUDE_IO_H_

#include <sys/types.h> // mode_t

#include "../include/utils.h" // command_t

int printMessage(mode_t new_mode, mode_t old_mode, const command_t *command, bool is_link);

int printChangeMessage(const char *path, mode_t previous_mode, mode_t new_mode, char *info, unsigned int size);

int printRetainMessage(const char *path, mode_t mode, char *info, unsigned int size);

int printSymbolicMessage(const char *path, char *info, unsigned int size);

int printCurrentStatus(const char *path, int numberOfFiles, int numberOfModifiedFiles);

int parseModeToString(mode_t mode, char *str, unsigned int size);

mode_t clearExtraBits(mode_t mode);


#endif //PROJECT_INCLUDE_IO_H_
