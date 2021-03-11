#ifndef PROJECT_INCLUDE_TIME_CTRL_H_
#define PROJECT_INCLUDE_TIME_CTRL_H_

/**
 * @brief Marks the beginning of the program execution
 * 
 */
void setBegin();

/**
 * @brief Calculates the milliseconds elapsed since the beginning of the program execution
 * 
 * @return seconds elapsed 
 */
double getElapsed();

#endif  // PROJECT_INCLUDE_TIME_CTRL_H_
