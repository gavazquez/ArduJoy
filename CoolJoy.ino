//https://www.arduino.cc/en/Tutorial/Foundations/ShiftIn
//https://www.reddit.com/r/hoggit/comments/45j5pk/tm_warthog_stick_circuitry/

#include <Joystick.h>
#include <Wire.h>
#include "MLX90393.h"

int latchPin = 8;
int dataPin = 9;
int clockPin = 7;

bool debug = true;

byte oldValues[4];
byte newValues[4];

// MLX90393 I2C Address. Check MLX90393 datasheet. I've found it by trial and error
MLX90393_ MLX90393(0x0F);

#define READ_REGISTER_PIN( pin, data ) ( ( data & ( 1 << pin ) ) != 0 )

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK,
    16, 4,                  // Button Count, Hat Switch Count
    true, true, true,    // X and Y, but no Z Axis
    false, false, false,    // No Rx, Ry, or Rz
    false, false,           //No slider or dial
    false, false,           // No rudder or throttle
    false, false, false);   // No accelerator, brake, or steering

void setup() {
    Serial.begin(9600);

    pinMode(dataPin, INPUT);
    pinMode(latchPin, OUTPUT);
    pinMode(clockPin, OUTPUT);

    digitalWrite(latchPin, HIGH);
    digitalWrite(clockPin, HIGH);

    delayMicroseconds(20);
    digitalWrite(clockPin, LOW);

    Joystick.begin();
    Joystick.setXAxisRange(0, 100);
    Joystick.setYAxisRange(0, 100);
    Joystick.setZAxisRange(0, 100);

    MLX90393.begin();
}

int max;

void loop() {
    byte c_Result;

    MLX90393.updateAxisValues();
    Joystick.setXAxis(map(MLX90393.X, -22322, 19165, 0, 100));
    Joystick.setYAxis(map(MLX90393.Y, -21791, 17331, 0, 100));

    digitalWrite(latchPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(latchPin, LOW);

    newValues[0] = ReadRegister();
    newValues[1] = ReadRegister();
    newValues[2] = ReadRegister();
    newValues[3] = ReadRegister();

    CompareArrays(oldValues, newValues);

    short axisVal = ReadAxis(-20000, 6000);
    Joystick.setZAxis(axisVal);
    if (axisVal >= 50)
    {
        Joystick.pressButton(0);
        Joystick.setHatSwitch(0, 0);
        Joystick.setHatSwitch(1, 90);
        Joystick.setHatSwitch(2, 180);
        Joystick.setHatSwitch(3, 270);
    }
    if (axisVal <= 10)
    {
        Joystick.releaseButton(0);
        Joystick.setHatSwitch(0, -1);
        Joystick.setHatSwitch(1, -1);
        Joystick.setHatSwitch(2, -1);
        Joystick.setHatSwitch(3, -1);
    }
}

byte ReadRegister()
{
    byte result;
    int temp = 0;

    for (int i = 7; i >= 0; i--)
    {
        digitalWrite(clockPin, LOW);
        delayMicroseconds(2);
        temp = digitalRead(dataPin);

        if (temp)
        {
            result = result | (1 << i);
        }

        digitalWrite(clockPin, HIGH);
    }

    return result;
}

void printByteVals(byte val)
{
    for (int i = 0; i < 8; i++) {
        Serial.print(READ_REGISTER_PIN(i, val));
    }
    Serial.println();
}

void CompareArrays(byte* oldArray, byte* newArray)
{
    int i;

    bool registerHasChanges;
    bool newVal;
    bool oldVal;
    for (i = 0; i < 4; i++)
    {
        registerHasChanges = false;
        for (int j = 0; j < 8; j++) 
        {
            oldVal = READ_REGISTER_PIN(j, oldArray[i]);
            newVal = READ_REGISTER_PIN(j, newArray[i]);

            if (oldVal != newVal)
            {
                registerHasChanges = true;
                if (newVal == 0)
                {
                    Serial.print("Btn: ");
                    Serial.print((8 * i) + (j + 1));
                    Serial.println(" PRESSED");
                }
            }
        }

        if (registerHasChanges)
        {
            oldArray[i] = newArray[i];
        }
    }
}

short ReadAxis(short lowerLimit, short upperLimit)
{
    byte b1 = ReadRegister();
    byte b2 = ReadRegister();

    short rawVal = b1 << 8 | b2;
    short axisVal = map(rawVal, lowerLimit, upperLimit, 0, 100);

    if (axisVal < 0) return 0;
    if (axisVal > 100) return 100;

    return axisVal;
}