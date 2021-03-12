#ifndef PROJECT_INCLUDE_TIME_CTRL_H_
#define PROJECT_INCLUDE_TIME_CTRL_H_

/**
 * @brief Set the Begin object
 * 
 * @param startTime 
 */
void setBegin(struct timeval startTime);

/**
 * @brief Get the Elapsed object
 * 
 * @param startTime 
 * @return long 
 */
long getElapsed();

#endif  // PROJECT_INCLUDE_TIME_CTRL_H_
