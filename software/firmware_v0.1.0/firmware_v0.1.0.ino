/**
 * @file firmware_v0.1.0.ino
 * @author Sachit Kshatriya
 * @brief Interface firmware for the 96-well OPSpec
 * @version 0.1
 * @date 2022-11-27
 *
 */

#include <Wire.h>

// ALWAYS GIVE I2C 7-BIT ADDRESSES FUUUUUUUUUUUUUUCK
// I2C devices
#define ADC 0b1001000
#define EROW 0b0100001 // p-ch
#define ECOL 0b0100000 // n-ch
#define DCOL 0b0100011 // p-ch

// I/O expander registers
#define IN0 0x00 // input read
#define IN1 0x01
#define OUT0 0x02 // output set
#define OUT1 0x03
#define INV0 0x04 // polarity inversion
#define INV1 0x05
#define CONFIG0 0x06 // configuration
#define CONFIG1 0x07

// additional parameters
int ledOn = 100;  // in ms
int deadTime = 1; // in ms - time between actions on different functional units

// The ADC channel addresses in single-ended input mode arranged by index (A - H)
byte chAddr[8] = {0b1000, 0b1100, 0b1001, 0b1101,
                  0b1010, 0b1110, 0b1011, 0b1111};

// Converstion buffer for transmission over Serial
unsigned int raw[12][8];

bool oneloop;

void setup()
{
    Serial.begin(115200);

    Wire.begin();
    delay(500);

    // Set I/O expander pins to keep grids inactive on startup
    // Unconnected pins set to high-Z input mode
    commandIO(ECOL, OUT0, 0x00, 0x00);    // output low for n-ch
    commandIO(ECOL, CONFIG0, 0x00, 0xF0); // pin config to output (1 = highZ-in)

    Serial.println("Ecol set");

    commandIO(EROW, OUT0, 255, 255);  // output high for p-ch
    commandIO(EROW, CONFIG0, 0, 255); // pin config to output

    Serial.println("Erow set");

    commandIO(DCOL, OUT0, 255, 255);         // output high for p-ch
    commandIO(DCOL, CONFIG0, 0, 0b11110000); // pin config to output

    Serial.println("Dcol set");

    oneloop = true;

    delay(50);
}

void loop()
{
    if (oneloop)
    {

        // for (int k = 0; k < 12; k++)
        // {
        //     addressDetCol(k, 1);
        //     delay(3000);
        // }
        // addressDetCol(0, 1);

        flushRaw();
        // readSamples();
        for (int k = 0; k < 20; k++)
        {
            readWell(11, 1);
        }
        // sendRaw();
        // blinkLEDs(100);

        // handleSerial();

        // delay(500); // allow serial monitor to open
        oneloop = false;
    }
    // Serial.println();
    // readADCs();
    // readDet();
    // sendRaw();

    // flushRaw();
    // sendRaw();

    // oneLoop = false;

    // blinkLEDs();
}

/**
 * @brief Handles execution control over Serial
 *
 */
void handleSerial()
{
    while (Serial.available() > 0)
    {
        char incomingCharacter = Serial.read();
        switch (incomingCharacter)
        {
        case '0':
            readDark();
            // sendRaw();

        case 'A':
            readSamples();
            sendRaw();

        case 'T':
            blinkLEDs(200);
        }
    }
}

/**
 * @brief Reads raw ADC conversions into the buffer without turning on LEDs
 *
 */
void readDark()
{

    for (int i = 0; i < 12; i++) // col loop
    {
        addressDetCol(i, 1); // enable detector column

        delay(10);

        for (int j = 0; j < 8; j++) // row loop
        {
            delay(ledOn); // let reading settle

            readADCValue(chAddr[j], raw[i], j);

            delay(deadTime);

            // for debugging
            // Serial.print("raw[" + String(i) + "][" + j + "] is: ");
            // Serial.println(raw[i][j]);
        }
    }
    addressDetCol(11, 0); // disable detector column at the end
}

/**
 * @brief Reads raw ADC conversions for well samples into the buffer
 *
 */
void readSamples()
{

    for (int i = 0; i < 12; i++) // col loop
    {

        addressDetCol(i, 1); // det col on
        delay(10);

        for (int j = 0; j < 8; j++) // row loop
        {
            addressLED(i, j, 1); // LED on
            delay(ledOn);        // let LED settle

            readADCValue(chAddr[j], raw[i], j);

            delay(deadTime);

            // for debugging
            Serial.print("raw[" + String(i) + "][" + j + "] is: ");
            Serial.println(raw[i][j]);
        }
    }

    addressDetCol(0, 0); // all detectors off
    addressLED(0, 0, 0); // all LEDs off
}

/**
 * @brief Reads ADC conversion for a well and prints to Serial
 *
 * @param col well column index
 * @param row  well row index
 */
void readWell(int col, int row)
{

    addressDetCol(col, 1); // det col on
    delay(10);

    addressLED(col, row, 1); // LED on
    delay(ledOn);

    readADCValue(chAddr[row], raw[col], row);

    // for debugging
    Serial.print("raw[" + String(col) + "][" + row + "] is: ");
    Serial.println(raw[col][row], DEC);

    // reset LEDs and detector switches to off state
    addressDetCol(0, 0); // all detectors off
    addressLED(0, 0, 0); // all LEDs off
}

/**
 * @brief Flushes the conversion data buffer with -1
 *
 */
void flushRaw()
{
    for (int i = 0; i < 12; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            raw[i][j] = -1;
        }
    }
}

/**
 * @brief Writes contents of the conversion data buffer to serial. Flattens data into groups of columns.
 * Element 0 is well[col][row] = well[0][0], element 1 is well[0][1].
 *
 */
void sendRaw()
{
    for (int i = 0; i < 12; i++) // column loop
    {
        for (int j = 0; j < 8; j++) // row loop
        {
            Serial.println(raw[i][j]);
        }
    }
}

/**
 * @brief Commands a conversion and reads the value for the given
 * ADC channel into the array at the given index after concatenating into an int
 *
 * @param channel ADC channel to read from
 * @param convData pointer to store conversion data
 * @param idx index at which to store the data
 * @return int
 */
int readADCValue(int channel, unsigned int *convData, int idx)
{

    byte commandByte = (chAddr[channel] << 4) + 0b1100; // internal ref on and ADC conv ON

    Wire.beginTransmission(ADC);
    Wire.write(commandByte);
    Wire.endTransmission();

    delay(5); // let the ADC settle idk

    Wire.requestFrom(ADC, 2);

    // Get and store the conversion
    unsigned int temp = Wire.read();
    unsigned int t2 = Wire.read();

    unsigned int tot = (temp << 8) + t2;
    // temp += Wire.read();
    Serial.print(temp, BIN);
    Serial.println(t2, BIN);

    convData[idx] = tot;
    // convData[1] = Wire.read();
}

/**
 * @brief Wrapper function to address individual LEDs in the emitter grid
 *
 * @param col LED column id - left to right
 * @param row LED row id - top to bottom
 * @param action 0 for OFF; 1 for ON
 */
void addressLED(int col, int row, bool action)
{
    byte colPorts[2] = {0, 0};       // default to off
    byte rowPorts[2] = {0xFF, 0xFF}; // default to off

    if (action)
    {
        bitSet(colPorts[int(col / 8)], col % 8);   // turn col on
        bitClear(rowPorts[int(row / 8)], row % 8); // turn row on
    }

    // send port values to the devices
    commandIO(ECOL, OUT0, colPorts[0], colPorts[1]);
    commandIO(EROW, OUT0, rowPorts[0], rowPorts[1]);
}

/**
 * @brief Addresses photodiode column FETs for power delivery
 *
 * @param col photodiode column id - T1:column 0
 * @param action 0 for OFF; 1 for ON
 */
void addressDetCol(int col, bool action)
{
    byte colPorts[2] = {0xFF, 0xFF}; // default to off

    if (action)
        bitClear(colPorts[int(col / 8)], col % 8); // turn col on

    commandIO(DCOL, OUT0, colPorts[0], colPorts[1]);
}

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

/**
 * @brief Cycles through all the LEDs
 * @param msPerLed How long to keep an LED turned on in ms
 */
void blinkLEDs(int msPerled)
{
    for (int i = 0; i < 12; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            addressLED(i, j, 1);
            delay(msPerled);
        }
    }

    addressLED(0, 0, 0);
}
