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

#include <AD9851.h>

AD9851::AD9851 (uint8_t reset, uint8_t fq_ud, uint8_t w_clk, uint8_t data)
{
	// init - serial mode
	uint8_t n;

	// get ports, pins & ddr's
	n = digitalPinToPort (reset);
	_RESET_OUT = portOutputRegister (n);
	_RESET_DDR = portModeRegister (n);

	n = digitalPinToPort (fq_ud);
	_FQ_UD_OUT = portOutputRegister (n);
	_FQ_UD_DDR = portModeRegister (n);

	n = digitalPinToPort (w_clk);
	_W_CLK_OUT = portOutputRegister (n);
	_W_CLK_DDR = portModeRegister (n);

	n = digitalPinToPort (data);
	_DATA_OUT = portOutputRegister (n);
	_DATA_DDR = portModeRegister (n);

	// get bitmasks
	_RESET_BIT = digitalPinToBitMask (reset);
	_FQ_UD_BIT = digitalPinToBitMask (fq_ud);
	_W_CLK_BIT = digitalPinToBitMask (w_clk);
	_DATA_BIT  = digitalPinToBitMask (data);

	// set initial pin values all low
	*_RESET_OUT &= ~_RESET_BIT;
	*_FQ_UD_OUT &= ~_FQ_UD_BIT;
	*_W_CLK_OUT &= ~_W_CLK_BIT;
	*_DATA_OUT  &= ~_DATA_BIT;

	// set DDR's to all output
	*_RESET_DDR |= _RESET_BIT;
	*_FQ_UD_DDR |= _FQ_UD_BIT;
	*_W_CLK_DDR |= _W_CLK_BIT;
	*_DATA_DDR  |= _DATA_BIT;

	// to select SERIAL I/O mode, we do the following:
	// See Analog Devices AD9851 datasheet (Jan 2004) REV D, Page 15
	// Tie D0 and D1 (pins 4 and 3) high through a 10K resistor
	// Tie D2 (pin 2) low (to ground)
	// Pulse RESET high, then LOW
	// Pulse W_CLK high, then LOW
	// Pulse FQ_UD high, then LOW
	// Clear all 5 registers (40 bits)
	*_RESET_OUT |= _RESET_BIT; // reset dds
	__asm__ __volatile__ (" nop\n");
	*_RESET_OUT &= ~_RESET_BIT;

	*_W_CLK_OUT |= _W_CLK_BIT;
	*_W_CLK_OUT &= ~_W_CLK_BIT;

	*_FQ_UD_OUT |= _FQ_UD_BIT; // load it into the dds
	*_FQ_UD_OUT &= ~_FQ_UD_BIT;

	_MODE = 0; // flag serial mode

	setPhase (0);
	setFreq (0UL);

	// init complete
}

AD9851::AD9851 (uint8_t reset, uint8_t fq_ud, uint8_t w_clk, volatile uint8_t *port)
{
	// init - parallel mode
	uint8_t n;

	// note: PORT == 0
	// DDR == -1
	// PIN == -2

	_DATA_OUT = (port - 0);
	_DATA_DDR = (port - 1);

	n = digitalPinToPort (reset);
	_RESET_OUT = portOutputRegister (n);
	_RESET_DDR = portModeRegister (n);

	n = digitalPinToPort (w_clk);
	_W_CLK_OUT = portOutputRegister (n);
	_W_CLK_DDR = portModeRegister (n);

	n = digitalPinToPort (fq_ud);
	_FQ_UD_OUT = portOutputRegister (n);
	_FQ_UD_DDR = portModeRegister (n);

	_RESET_BIT = digitalPinToBitMask (reset);
	_W_CLK_BIT = digitalPinToBitMask (w_clk);
	_FQ_UD_BIT = digitalPinToBitMask (fq_ud);

	*_RESET_OUT &= ~_RESET_BIT;
	*_W_CLK_OUT &= ~_W_CLK_BIT;
	*_FQ_UD_DDR &= ~_FQ_UD_BIT;

	*_RESET_DDR |= _RESET_BIT;
	*_W_CLK_DDR |= _W_CLK_BIT;
	*_FQ_UD_DDR |= _FQ_UD_BIT;

	*_DATA_OUT = 0x00; // clear output port
	*_DATA_DDR = 0xFF; // DDR all outputs

	*_RESET_OUT |= _RESET_BIT;
	__asm__ __volatile__ (" nop\n");
	*_RESET_OUT &= ~_RESET_BIT;

	_MODE = 1; // parallel mode

	setPhase (0);
	setFreq (0UL);

	// init complete
}

int AD9851::setPhase (int phase)
{
	// derive phase value
	_phase = ((phase % 360) / P_FACTOR);
	_phase <<= 3;
	_phase |= MULT_EN;
	_update(); // send phase data
	// convert it back to show what is ACTUALLY set
	phase = (_phase & ~MULT_EN);
	phase >>= 3;
	phase *= P_FACTOR;
	return phase;
}

uint32_t AD9851::setFreq (uint32_t freq)
{
	// derive register value
	_freq = (uint32_t) ((freq * F_FACTOR) + 0.5);
	_update(); // send it
	// convert it back to show what is ACTUALLY set
	return (uint32_t) ((_freq / F_FACTOR) + 0.5);
}

void AD9851::_update (void)
{
	uint8_t n;

	if (_MODE) { // parallel mode
		n = 32;

		while (n -= 8) {
			*_W_CLK_OUT |= _W_CLK_BIT;
			*_DATA_OUT = ((_freq >> n) & 0xFF);
			*_W_CLK_OUT &= ~_W_CLK_BIT;
		}

		*_W_CLK_OUT |= _W_CLK_BIT;
		*_DATA_OUT = _phase;
		*_W_CLK_OUT &= ~_W_CLK_BIT;

	} else { // serial mode
		// send 32 bit frequency tuning word
		for (n = 0; n < 32; n++) {
			(_freq & (1UL << n)) ? *_DATA_OUT |= _DATA_BIT : *_DATA_OUT &= ~_DATA_BIT; // send a bit
			*_W_CLK_OUT |= _W_CLK_BIT;
			*_W_CLK_OUT &= ~_W_CLK_BIT;
		}

		// send 8 bit phase data
		// note: bits 7 through 3 are phase data (5 bits, 11.25 degrees per bit)
		// bit 2 is "Power down" control (0=run, 1=power down)
		// bit 1 is reserved and must be 0
		// bit 0 is REF_CLK multiplier enable (0=base ref_clock, 1=multiply by 6)
		for (n = 0; n < 8; n++) {
			(_phase & (1UL << n)) ? *_DATA_OUT |= _DATA_BIT : *_DATA_OUT &= ~_DATA_BIT;
			*_W_CLK_OUT |= _W_CLK_BIT;
			*_W_CLK_OUT &= ~_W_CLK_BIT;
		}
	}

	*_FQ_UD_OUT |= _FQ_UD_BIT; // update all 40 bits
	*_FQ_UD_OUT &= ~_FQ_UD_BIT;

}
// end of ad9851.cpp
