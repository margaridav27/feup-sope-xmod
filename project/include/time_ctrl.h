//COMBACK: Explain header usages
#ifndef PROJECT_INCLUDE_TIME_CTRL_H_
#define PROJECT_INCLUDE_TIME_CTRL_H_

#include <time.h>

int getStartTime(struct timespec *dest);

int getMillisecondsElapsed();

#endif // PROJECT_INCLUDE_TIME_CTRL_H_
