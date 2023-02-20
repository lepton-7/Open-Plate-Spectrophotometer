# 1 "c:\\School stuff\\Bagby Lab\\Projects\\OPSpec\\software\\firmware_v0.0.2\\firmware_v0.0.2.ino"
/**

 * @file firmware_v0.0.2.ino

 * @author Sachit Kshatriya

 * @brief Interface firmware for the OPSpec linear test

 * @version 0.1

 * @date 2022-07-17

 *

 */
# 10 "c:\\School stuff\\Bagby Lab\\Projects\\OPSpec\\software\\firmware_v0.0.2\\firmware_v0.0.2.ino"
# 11 "c:\\School stuff\\Bagby Lab\\Projects\\OPSpec\\software\\firmware_v0.0.2\\firmware_v0.0.2.ino" 2

// ADC address


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
int ledOn = 50; // in ms
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
        pinMode(i, 0x1);
    }

    // Turn n-FETs off
    digitalWrite(2, 0x0);
    digitalWrite(3, 0x0);

    // Turn p-FETs off
    digitalWrite(0, 0x1);
    digitalWrite(1, 0x1);
    digitalWrite(4, 0x1);
    digitalWrite(5, 0x1);
    digitalWrite(6, 0x1);
    digitalWrite(7, 0x1);
    digitalWrite(8, 0x1);
    digitalWrite(9, 0x1);
    digitalWrite(10, 0x1);
    digitalWrite(11, 0x1);

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

int readADCValue(int channel, int *convData, int idx)
{
    /**

     * @brief Commands a conversion and reads the value for the given

     * ADC channel into the array at the given index after concatenating into an int

     */
# 106 "c:\\School stuff\\Bagby Lab\\Projects\\OPSpec\\software\\firmware_v0.0.2\\firmware_v0.0.2.ino"
    byte commandByte = (chAddr[channel] << 4) + 0b0100; // internal ref off and ADC conv ON

    Wire.beginTransmission(0b1001000);
    Wire.write(commandByte);
    Wire.endTransmission();

    delay(5); // maybe let the ADC settle idk

    Wire.requestFrom(0b1001000, 2);

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
# 131 "c:\\School stuff\\Bagby Lab\\Projects\\OPSpec\\software\\firmware_v0.0.2\\firmware_v0.0.2.ino"
    int rows[] = {A, B, C, D, E, F, G, H};
    int cols[] = {one, two};

    for (int i = 0; i < 2; i++)
    {
        digitalWrite(cols[i], 0x1);

        for (int j = 0; j < 8; j++)
        {
            digitalWrite(rows[j], 0x0);
            delay(200);
            digitalWrite(rows[j], 0x1);
        }

        digitalWrite(cols[i], 0x0);
    }
}

void readADCs()
{
    /**

     * @brief Returns raw ADC conversions for each well

     *

     */
# 156 "c:\\School stuff\\Bagby Lab\\Projects\\OPSpec\\software\\firmware_v0.0.2\\firmware_v0.0.2.ino"
    int rows[] = {A, B, C, D, E, F, G, H};
    int cols[] = {one, two};
    int detCols[] = {DetOne, DetTwo};

    for (int i = 0; i < 2; i++) // col loop
    {
        digitalWrite(cols[i], 0x1); // LED col on
        digitalWrite(detCols[i], 0x0); // det col on

        delay(10);

        for (int j = 0; j < 8; j++) // row loop
        {
            digitalWrite(rows[j], 0x0); // LED row on
            delay(ledOn); // let reading settle

            readADCValue(chAddr[j], raw[i], j);

            delay(deadTime);

            // for debugging
            // Serial.print("raw[" + String(i) + "][" + j + "] is: ");
            // Serial.println(raw[i][j]);

            digitalWrite(rows[j], 0x1); // LED row off
        }

        digitalWrite(cols[i], 0x0); // LED col off
        digitalWrite(detCols[i], 0x1); // det col off
    }
}

void flushRaw()
{
    /**

     * @brief Flushes the conversion data buffer with -1s

     */
# 194 "c:\\School stuff\\Bagby Lab\\Projects\\OPSpec\\software\\firmware_v0.0.2\\firmware_v0.0.2.ino"
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
# 209 "c:\\School stuff\\Bagby Lab\\Projects\\OPSpec\\software\\firmware_v0.0.2\\firmware_v0.0.2.ino"
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
# 224 "c:\\School stuff\\Bagby Lab\\Projects\\OPSpec\\software\\firmware_v0.0.2\\firmware_v0.0.2.ino"
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
# 245 "c:\\School stuff\\Bagby Lab\\Projects\\OPSpec\\software\\firmware_v0.0.2\\firmware_v0.0.2.ino"
    int rows[] = {A, B, C, D, E, F, G, H};
    int cols[] = {one, two};
    int detCols[] = {DetOne, DetTwo};

    for (int i = 0; i < 2; i++) // col loop
    {
        digitalWrite(detCols[i], 0x0); // det col on

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

        digitalWrite(detCols[i], 0x1); // det col off
    }
}
