/**
 * @file adc.c
 * @author Sachit Kshatriya
 * @brief Functions for the ADC
 * @version 0.1.0
 * @date 2025-06-14
 *
 */

#include <stdint.h>
#include <Wire.h>

#include "adc.h"
#include "../det_grid/detectors.h"

#define I2C_ADC 0b1001000

// The ADC channel addresses in single-ended input mode arranged by index (A - H)
static uint8_t chAddr[8] = {0b1000, 0b1100, 0b1001, 0b1101,
                            0b1010, 0b1110, 0b1011, 0b1111};

static uint16_t read;

uint16_t read_well_value(int col, int row)
{
    enable_det(col);

    byte commandByte = (chAddr[row] << 4) + 0b1100; // internal ref on and ADC conv ON

    Wire.beginTransmission(I2C_ADC);
    Wire.write(commandByte);
    Wire.endTransmission();

    Wire.requestFrom(I2C_ADC, 2);

    // Get and store the conversion
    uint16_t temp = Wire.read();
    uint16_t t2 = Wire.read();

    uint16_t tot = (temp << 8) + t2;
    return tot;
}
