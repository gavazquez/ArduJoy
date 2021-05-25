// MLX90393.h

#ifndef _MLX90393_h
#define _MLX90393_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#endif

class MLX90393_
{
	private:	
		int			_address;
		bool	    _started;
	public:
		int X;
		int Y;
		MLX90393_(byte address); 
		void begin();
		void updateAxisValues();
};