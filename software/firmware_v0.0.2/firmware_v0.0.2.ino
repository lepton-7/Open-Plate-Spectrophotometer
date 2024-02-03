/**
 * @file firmware_v0.0.2.ino
 * @author Sachit Kshatriya
 * @brief Interface firmware for the OPSpec linear test
 * @version 0.1
 * @date 2022-07-17
 *
 */

#include <Wire.h>

// ADC address
#define ADC 0b1001000

// Define the LED grid control gate pins
// p-ch
int DetOne = 0;
int DetTwo = 1;

// n-ch
int one = 2;
int two = 3;

// p-ch
int A = 4;
int B = 5;
int C = 6;
int D = 7;
int E = 8;
int F = 9;
int G = 10;
int H = 11;

// additional parameters
int ledOn = 50;  // in ms
int deadTime = 1; // in ms - time between actions on different functional units

// The ADC channel addresses in single-ended input mode arranged by index (A - H)
byte chAddr[8] = {0b1000, 0b1100, 0b1001, 0b1101,
                  0b1010, 0b1110, 0b1011, 0b1111};

bool oneLoop;

void setup()
{

    Serial.begin(115200);

    Wire.begin();

    // Set pin modes to output:
    for (int i = 0; i < 12; i++)
    {
        pinMode(i, OUTPUT);
    }

    // Turn n-FETs off
    digitalWrite(2, LOW);
    digitalWrite(3, LOW);

    // Turn p-FETs off
    digitalWrite(0, HIGH);
    digitalWrite(1, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(5, HIGH);
    digitalWrite(6, HIGH);
    digitalWrite(7, HIGH);
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(10, HIGH);
    digitalWrite(11, HIGH);

    oneLoop = true;
}

int raw[2][8];

void loop()
{

    handleSerial();

    // delay(500); // allow serial monitor to open

    // Serial.println();
    // Serial.println();
    // readADCs();
    // readDet();
    // sendRaw();

    // flushRaw();
    // sendRaw();

    // oneLoop = false;

    // blinkLEDs();
}

int readADCValue(byte channel, int *convData, int idx)
{
    /**
     * @brief Commands a conversion and reads the value for the given
     * ADC channel into the array at the given index after concatenating into an int
     */

    byte commandByte = (channel << 4) + 0b0100; // internal ref off and ADC conv ON

    Wire.beginTransmission(ADC);
    Wire.write(commandByte);
    Wire.endTransmission();

    delay(5); // maybe let the ADC settle idk

    Wire.requestFrom(ADC, 2);

    // Get and store the conversion
    int temp = Wire.read() << 8;
    temp += Wire.read();

    convData[idx] = temp;
    // convData[1] = Wire.read();
}

void blinkLEDs()
{
    /**
     * @brief Tests the LED array by cycling through them
     *
     */

    int rows[] = {A, B, C, D, E, F, G, H};
    int cols[] = {one, two};

    for (int i = 0; i < 2; i++)
    {
        digitalWrite(cols[i], HIGH);

        for (int j = 0; j < 8; j++)
        {
            digitalWrite(rows[j], LOW);
            delay(200);
            digitalWrite(rows[j], HIGH);
        }

        digitalWrite(cols[i], LOW);
    }
}

void readADCs()
{
    /**
     * @brief Returns raw ADC conversions for each well
     *
     */

    int rows[] = {A, B, C, D, E, F, G, H};
    int cols[] = {one, two};
    int detCols[] = {DetOne, DetTwo};

    for (int i = 0; i < 2; i++) // col loop
    {
        digitalWrite(cols[i], HIGH);   // LED col on
        digitalWrite(detCols[i], LOW); // det col on

        delay(10);

        for (int j = 0; j < 8; j++) // row loop
        {
            digitalWrite(rows[j], LOW); // LED row on
            delay(ledOn);               // let reading settle

            readADCValue(chAddr[j], raw[i], j);

            delay(deadTime);

            // for debugging
            // Serial.print("raw[" + String(i) + "][" + j + "] is: ");
            // Serial.println(raw[i][j]);

            digitalWrite(rows[j], HIGH); // LED row off
        }

        digitalWrite(cols[i], LOW);     // LED col off
        digitalWrite(detCols[i], HIGH); // det col off
    }
}

void flushRaw()
{
    /**
     * @brief Flushes the conversion data buffer with -1s
     */

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            raw[i][j] = -1;
        }
    }
}

void sendRaw()
{
    /**
     * @brief Writes contents of the conversion data buffer to serial
     */

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            Serial.println(raw[i][j]);
        }
    }
}

void handleSerial()
{
    /**
     * @brief Handles execution control over Serial
     */

    while (Serial.available() > 0)
    {
        char incomingCharacter = Serial.read();
        switch (incomingCharacter)
        {
        case '0':
            readDet();
            // sendRaw();

        case 'A':
            readADCs();
            sendRaw();
        }
    }
}

void readDet()
{
    /**
     * @brief Reads the ADC outputs into the buffer with no LED activation
     */
    int rows[] = {A, B, C, D, E, F, G, H};
    int cols[] = {one, two};
    int detCols[] = {DetOne, DetTwo};

    for (int i = 0; i < 2; i++) // col loop
    {
        digitalWrite(detCols[i], LOW); // det col on

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

        digitalWrite(detCols[i], HIGH); // det col off
    }
}