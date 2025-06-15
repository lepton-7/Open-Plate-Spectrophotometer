/**
 * @file leds.h
 * @author Sachit Kshatriya
 * @brief Interface for the LED grid
 * @version 0.1.0
 * @date 2025-06-15
 *
 */

#include <stdint.h>

/**
 * @brief Enable power to the selected LED. Does nothing if already enabled.
 *
 * @param col Column ID as int.
 * @param row Row ID as int.
 */
void enable_led(int col, int row);

/**
 * @brief Disable all LEDs in the matrix.
 *
 */
void disable_all_leds(void);