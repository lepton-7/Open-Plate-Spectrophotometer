/**
 * @file leds.c
 * @author Sachit Kshatriya
 * @brief Functions for the LED grid
 * @version 0.1.0
 * @date 2025-06-15
 *
 */

#include <stdint.h>
#include <Wire.h>

#include "leds.h"
#include "../io/expander.h"

#define I2C_EROW 0b0100001 // p-ch
#define I2C_ECOL 0b0100000 // n-ch

static int encol = -1;
static int enrow = -1;

/**
 * @brief Enable power to the selected LED. Does nothing if already enabled.
 *
 * @param col Column ID as int.
 * @param row Row ID as int.
 */
void enable_led(int col, int row)
{
    if (col != encol)
    {
        byte colPorts[2] = {0, 0};          // default to off
        bitSet(colPorts[col / 8], col % 8); // turn col on
        commandIO(I2C_ECOL, OUT0, colPorts[0], colPorts[1]);
        encol = col;
    }

    if (enrow != row)
    {
        byte rowPorts[2] = {0xFF, 0xFF};      // default to off
        bitClear(rowPorts[row / 8], row % 8); // turn row on
        commandIO(I2C_EROW, OUT0, rowPorts[0], rowPorts[1]);
        enrow = row;
    }
}

/**
 * @brief Disable all LEDs in the matrix.
 *
 */
void disable_all_leds(void)
{
    byte colPorts[2] = {0, 0};       // default to off
    byte rowPorts[2] = {0xFF, 0xFF}; // default to off

    // send port values to the devices
    commandIO(I2C_ECOL, OUT0, colPorts[0], colPorts[1]);
    commandIO(I2C_EROW, OUT0, rowPorts[0], rowPorts[1]);

    encol = -1;
    enrow = -1;
}
