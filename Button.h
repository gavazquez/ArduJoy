// MLX90393.h

#ifndef _MLX90393_h
#define _MLX90393_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif


#endif

class Button
{
    public:
        int Index;
        bool Value;
    public:
        Button(int index, bool value);
};