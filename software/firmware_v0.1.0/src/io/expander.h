/**
 * @file expander.h
 * @author Sachit Kshatriya
 * @brief Interface for the I/O expanders
 * @version 0.1.0
 * @date 2025-06-14
 *
 */

// I/O expander registers
#define IN0 0x00 // input read
#define IN1 0x01
#define OUT0 0x02 // output set
#define OUT1 0x03
#define INV0 0x04 // polarity inversion
#define INV1 0x05
#define CONFIG0 0x06 // configuration
#define CONFIG1 0x07

/**
 * @brief Sets a register pair for the given I/O expander
 *
 * @param device I2C address
 * @param port0Reg command byte to port0 of a register pair
 * @param port0 port0 data
 * @param port1 port1 data
 */
void commandIO(byte device, byte port0Reg, byte port0, byte port1);