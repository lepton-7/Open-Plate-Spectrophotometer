/**
 * @file adc.c
 * @author Sachit Kshatriya
 * @brief Interface for the photodiode grid
 * @version 0.1.0
 * @date 2025-06-14
 *
 */

#include <stdint.h>

/**
 * @brief Enable power to the selected detector column. Does nothing if it is
 * already enabled.
 *
 * @param column Column ID as int.
 */
void enable_det(int column);

/**
 * @brief Disable power to all detector columns
 *
 */
void disable_all(void);