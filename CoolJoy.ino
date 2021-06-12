//https://www.arduino.cc/en/Tutorial/Foundations/ShiftIn
//https://www.reddit.com/r/hoggit/comments/45j5pk/tm_warthog_stick_circuitry/

#include <Joystick.h>
#include <Wire.h>
#include "MLX90393.h"
#include "arduino-timer.h""

auto timer = timer_create_default();

int latchPin = 8;
int dataPin = 9;
int clockPin = 7;

bool debug = true;

int oldValues;
int newValues;

int buttonCount = 32;

unsigned long oldButtonState = 0;

// MLX90393 I2C Address. Check MLX90393 datasheet. I've found it by trial and error
MLX90393_ MLX90393(0x0F);

#define ReadBit(value, bit) (((value) >> (buttonCount - bit)) & 0x01) == ON_STATE

constexpr auto OFF_STATE = 1;
constexpr auto ON_STATE = 0;

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK,
    17, 4,                  // Button Count, Hat Switch Count
    true, true, true,    // X and Y, but no Z Axis
    false, false, false,    // No Rx, Ry, or Rz
    false, false,           //No slider or dial
    false, false,           // No rudder or throttle
    false, false, false);   // No accelerator, brake, or steering

class SwitchButton {
    long timetoSwitchOff;
    int buttonId;
    Joystick_* joy;

public:
    SwitchButton(Joystick_* joystick, int button, int timeToStop) {
        timetoSwitchOff = millis() + timeToStop;
        buttonId = button;
        joy = joystick;

        joy->pressButton(buttonId);
    }
    void Check() {
        if (millis() >= timetoSwitchOff)
            joy->releaseButton(buttonId);
    }
};

void setup() {
    Serial.begin(9600);

    pinMode(dataPin, INPUT);
    pinMode(latchPin, OUTPUT);
    pinMode(clockPin, OUTPUT);

    digitalWrite(latchPin, HIGH);
    digitalWrite(clockPin, HIGH);

    delayMicroseconds(20);
    digitalWrite(clockPin, LOW);

    Joystick.begin(false);
    Joystick.setXAxisRange(0, 100);
    Joystick.setYAxisRange(0, 100);
    Joystick.setZAxisRange(0, 100);

    MLX90393.begin();
}

int max;

void loop() {
    timer.tick();

    MLX90393.updateAxisValues();
    Joystick.setXAxis(map(MLX90393.X, -22322, 19165, 100, 0));
    Joystick.setYAxis(map(MLX90393.Y, -21791, 17331, 0, 100));

    digitalWrite(latchPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(latchPin, LOW);

    HandleButtonPresses();

    short axisVal = ReadAxis(-20000, 6000);
    Joystick.setZAxis(axisVal);

    Joystick.sendState();
}

void HandleButtonPresses()
{
    unsigned long buttonState = getButtonPresses();

    PressReleaseButton(buttonState, 1, 1);
    PressReleaseButton(buttonState, 2, 14);
    PressReleaseButton(buttonState, 3, 16);
    PressReleaseButton(buttonState, 4, 15);
    PressReleaseButton(buttonState, 5, 5);

    ToggleButtonOn(buttonState, 6, 2);
    ToggleButtonOff(buttonState, 6, 3);

    PressReleaseButton(buttonState, 7, 11);
    PressReleaseButton(buttonState, 8, 0);

    PressReleaseHat(ReadBit(buttonState, 12), ReadBit(buttonState, 11), ReadBit(buttonState, 10), ReadBit(buttonState, 9), false, 1);
    PressReleaseHat(ReadBit(buttonState, 16), ReadBit(buttonState, 15), ReadBit(buttonState, 14), ReadBit(buttonState, 13), true, 0);
    PressReleaseHat(ReadBit(buttonState, 20), ReadBit(buttonState, 19), ReadBit(buttonState, 18), ReadBit(buttonState, 17), false, 3);
    PressReleaseHat(ReadBit(buttonState, 24), ReadBit(buttonState, 23), ReadBit(buttonState, 22), ReadBit(buttonState, 21), false, 2);

    PressReleaseButton(buttonState, 25, 9);
    PressReleaseButton(buttonState, 26, 8);
    PressReleaseButton(buttonState, 27, 10);
    PressReleaseButton(buttonState, 28, 13);
    PressReleaseButton(buttonState, 29, 12);
    PressReleaseButton(buttonState, 30, 6);
    PressReleaseButton(buttonState, 31, 7);
    PressReleaseButton(buttonState, 32, 4);

    oldButtonState = buttonState;
}

void PressReleaseButton(unsigned long buttonState, int buttonIndex, int buttonId)
{
    bool oldValue = ReadBit(oldButtonState, buttonIndex);
    bool value = ReadBit(buttonState, buttonIndex);

    if (value && !oldValue)
        Joystick.pressButton(buttonId);
    else if (!value && oldValue)
        Joystick.releaseButton(buttonId);
}

void ToggleButtonOn(unsigned long buttonState, int buttonIndex, int buttonId) {
    bool oldValue = ReadBit(oldButtonState, buttonIndex);
    bool value = ReadBit(buttonState, buttonIndex);

    if (oldValue != value && value) {
        Joystick.pressButton(buttonId);
        timer.in(100, [](void* btnId) -> bool {
            Joystick.releaseButton((int)btnId);
            return false;
            }, buttonId);
    }
}


void ToggleButtonOff(unsigned long buttonState, int buttonIndex, int buttonId) {
    bool oldValue = ReadBit(oldButtonState, buttonIndex);
    bool value = ReadBit(buttonState, buttonIndex);

    if (oldValue != value && !value) {
        Joystick.pressButton(buttonId);
        timer.in(100, [](void* btnId) -> bool {
            Joystick.releaseButton((int)btnId);
            return false;
            }, buttonId);
    }
}

void PressReleaseHat(bool upVal, bool rightVal, bool downVal, bool leftVal, bool allowDiagonals, int hatId)
{
    if (upVal && !rightVal && !downVal && !leftVal)
        Joystick.setHatSwitch(hatId, 0);
    else if (upVal && rightVal && !downVal && !leftVal && allowDiagonals)
        Joystick.setHatSwitch(hatId, 0 + 45);
    else if (!upVal && rightVal && !downVal && !leftVal)
        Joystick.setHatSwitch(hatId, 90);
    else if (!upVal && rightVal && downVal && !leftVal && allowDiagonals)
        Joystick.setHatSwitch(hatId, 90 + 45);
    else if (!upVal && !rightVal && downVal && !leftVal)
        Joystick.setHatSwitch(hatId, 180);
    else if (!upVal && !rightVal && downVal && leftVal && allowDiagonals)
        Joystick.setHatSwitch(hatId, 180 + 45);
    else if (!upVal && !rightVal && !downVal && leftVal)
        Joystick.setHatSwitch(hatId, 270);
    else if (upVal && !rightVal && !downVal && leftVal && allowDiagonals)
        Joystick.setHatSwitch(hatId, 270 + 45);
    else
        Joystick.setHatSwitch(hatId, -1);
}

unsigned long getButtonPresses()
{
    return (long)flipByte(ReadRegister()) << 24 |
        (long)flipByte(ReadRegister()) << 16 | 
        (long)flipByte(ReadRegister()) << 8 | 
        (long)flipByte(ReadRegister());
}

byte flipByte(byte c) {
    char r = 0;
    for (byte i = 0; i < 8; i++) {
        r <<= 1;
        r |= c & 1;
        c >>= 1;
    }
    return r;
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