#ifndef PROJECT_INCLUDE_TIME_CTRL_H_
#define PROJECT_INCLUDE_TIME_CTRL_H_

#include <time.h> // struct timespec

int getStartTime(struct timespec *dest);

int getMillisecondsElapsed(void);

#endif // PROJECT_INCLUDE_TIME_CTRL_H_
