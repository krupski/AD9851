#include <AD9851.h>

// example for DDS connected to MEGA2560 board
// 8 bit data port d0...d7 connected to PORTK (A8...A15)

#define _RESET 10 // dds reset pin
#define _FQ_UD 11 // dds frequency update pin
#define _W_CLK 12 // dds word load clock
#define _DATA  13 // serial data pin (serial mode only)

// static AD9851 DDS (_RESET, _FQ_UD, _W_CLK, _DATA); // serial mode
static AD9851 DDS (_RESET, _FQ_UD, _W_CLK, &PORTK); // parallel mode

void setup (void)
{
	int phase; // 0 to 359
	uint32_t frequency; // 0 to 72 MHz

	int actual_phase;
	uint32_t actual_frequency;

	phase = 0; // 0 degrees
	frequency = 1000000UL; // 1 MHz.

	actual_phase = DDS.setPhase (phase); // set phase
	actual_frequency = DDS.setFreq (frequency); // set frequency
}

void loop (void)
{
	// nothing
}
