/**
 * @file adc.c
 * @author Sachit Kshatriya
 * @brief Functions for the photodiode grid
 * @version 0.1.0
 * @date 2025-06-14
 *
 */

#include "detectors.h"
#include "../io/expander.h"

#define I2C_DCOL 0b0100011 // p-ch

// Tracks which column is currently enabled
static int enabled;

/**
 * @brief Enable power to the selected detector column. Does nothing if it is
 * already enabled.
 *
 * @param column Column ID as int.
 */
void enable_det(int column)
{

    if (enabled == column)
    {
        return;
    }

    uint8_t colPorts[2] = {0xFF, 0xFF};              // default to off
    bitClear(colPorts[int(column / 8)], column % 8); // turn col on

    commandIO(I2C_DCOL, OUT0, colPorts[0], colPorts[1]);
    enabled = column;
}

/**
 * @brief Disable power to all detector columns
 *
 */
void disable_all(void)
{
    if (enabled == -1)
    {
        return;
    }

    uint8_t colPorts[2] = {0xFF, 0xFF}; // default to off
    commandIO(I2C_DCOL, OUT0, colPorts[0], colPorts[1]);

    enabled = -1;
}
