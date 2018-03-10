///////////////////////////////////////////////////////////////////////////////
//
//  Analog Devices AD9851 DDS Library for Arduino
//  Copyright (c) 2017, 2018 Roger A. Krupski <rakrupski@verizon.net>
//
//  Last update: 09 March 2018
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef AD9851_H
#define AD9851_H

#if ARDUINO < 100
#include "WProgram.h"
#else
#include "Arduino.h"
#endif

// defines
#define CAL_VAL (-339UL) // trim this with WWV
#define MULT_EN (1 << 0) // enable multiplier bit
#define REF_CLK (30000000UL+CAL_VAL) // reference clock 30 MHz plus calibration
#define F_FACTOR (4294967296.0/(REF_CLK*6.0)) // frequency to binary factor
#define P_FACTOR (360.0/32.0) // phase to binary factor

class AD9851 {
	public:
		AD9851 (uint8_t, uint8_t, uint8_t, uint8_t); // serial mode
		AD9851 (uint8_t, uint8_t, uint8_t, volatile uint8_t *); // parallel mode
		int setPhase (int);
		uint32_t setFreq (uint32_t);
	private:
		int _phase = 0;
		uint32_t _freq = 0;
		void _update (void);
		// flag serial or parallel
		uint8_t _MODE;
		// bitmasks
		uint8_t _RESET_BIT;
		uint8_t _FQ_UD_BIT;
		uint8_t _W_CLK_BIT;
		uint8_t _DATA_BIT;
		// control and data pins
		volatile uint8_t *_RESET_OUT;
		volatile uint8_t *_RESET_DDR;
		volatile uint8_t *_FQ_UD_OUT;
		volatile uint8_t *_FQ_UD_DDR;
		volatile uint8_t *_W_CLK_OUT;
		volatile uint8_t *_W_CLK_DDR;
		volatile uint8_t *_DATA_OUT;
		volatile uint8_t *_DATA_DDR;
};

#endif // #ifndef AD9851_H
