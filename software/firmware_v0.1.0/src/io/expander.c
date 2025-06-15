/**
 * @file expander.c
 * @author Sachit Kshatriya
 * @brief Functions for the I/O expanders
 * @version 0.1.0
 * @date 2025-06-14
 *
 */

#include <stdint.h>
#include <Wire.h>
#include "expander.h"

/**
 * @brief Sets a register pair for the given I/O expander
 *
 * @param device I2C address
 * @param port0Reg command byte to port0 of a register pair
 * @param port0 port0 data
 * @param port1 port1 data
 */
void commandIO(byte device, byte port0Reg, byte port0, byte port1)
{
    Wire.beginTransmission(device);

    // send command byte to write to port0 registers
    Wire.write(port0Reg);

    // since port0 element of the register pair is
    // accessed first the data write can be sequential
    Wire.write(port0);
    Wire.write(port1);

    Wire.endTransmission();
    // Serial.println(Wire.endTransmission());
}