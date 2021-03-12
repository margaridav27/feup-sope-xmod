#ifndef PROJECT_INCLUDE_TIME_CTRL_H_
#define PROJECT_INCLUDE_TIME_CTRL_H_

void setBegin(struct timeval startTime);

void getBegin(struct timeval *time);

/**
 * @brief Get the Elapsed object
 * 
 * @param startTime 
 * @return long 
 */
long getElapsed();

#endif  // PROJECT_INCLUDE_TIME_CTRL_H_
