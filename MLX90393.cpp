// 
//https://microcontrollerslab.com/mlx90393-digital-hall-sensor-module-pinout-interfacing-with-arduino/
//

#include "MLX90393.h"
#include <Wire.h>

MLX90393_::MLX90393_(byte address)
{
    _address = address;
}

void MLX90393_::begin()
{
    Wire.begin();
    Wire.beginTransmission(_address);
    Wire.write(0x60); // Select Write register command    
    Wire.write(0x00); // Set AH = 0x00, BIST disabled    
    Wire.write(0x5C); // Set AL = 0x5C, Hall plate spinning rate = DEFAULT, GAIN_SEL = 5    
    Wire.write(0x00); // Select address register, (0x00 << 2)    
    Wire.endTransmission();

    // Request and read status byte (1 byte)
    Wire.requestFrom(_address, 1);
    if (Wire.available() == 1)
    {
        unsigned int c = Wire.read();
    }

    Wire.beginTransmission(_address);
    Wire.write(0x60); // Select Write register command
    Wire.write(0x02); // Set AH = 0x02
    Wire.write(0xB4); // Set AL = 0xB4, RES for magnetic measurement = 0
    Wire.write(0x08); // Select address register, (0x02 << 2)
    Wire.endTransmission();

    // Request and read status byte (1 byte)
    Wire.requestFrom(_address, 1);
    if (Wire.available() == 1)
    {
        unsigned int c = Wire.read();
    }
    delay(100);

    _started = true;
}

void MLX90393_::updateAxisValues()
{
    if (!_started) return;

	unsigned int data[7];

    // Start single meaurement mode, YX enabled
    Wire.beginTransmission(_address);
    Wire.write(0x36);
    Wire.endTransmission();

    // Request and read status byte (1 byte)
    Wire.requestFrom(_address, 1);
    if (Wire.available() == 1)
    {
        unsigned int c = Wire.read();
    }
    delay(5);

    // Send read measurement command (RM), YX enabled
	Wire.beginTransmission(_address);
	Wire.write(0x46);
	Wire.endTransmission();

	// Request 7 bytes of data
	Wire.requestFrom(_address, 7);

	// Read 7 bytes of data
	// status, xMag msb, xMag lsb, yMag msb, yMag lsb, zMag msb, zMag lsb
	if (Wire.available() == 7);
	{
		data[0] = Wire.read(); //Status
		data[1] = Wire.read(); //X
		data[2] = Wire.read(); //X
		data[3] = Wire.read(); //Y
		data[4] = Wire.read(); //Y
	}

	// Convert the data
    X = data[1] * 256 + data[2];
    Y = data[3] * 256 + data[4];
}
