#include "DAC.h"
#include "frdm_bsp.h"

void DAC_Init(void)
{
	//SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
	SIM->SCGC6 |= SIM_SCGC6_DAC0_MASK;    // Enable clock signal for DAC0
	DAC0->C1 |= DAC_C1_DACBFEN_MASK;      // Enable 2x16-bit buffer
	DAC0->C0 |= (DAC_C0_DACEN_MASK | DAC_C0_DACTRGSEL_MASK);	// Enable DAC0, software triggering
}

uint8_t DAC_Load_Trig(uint16_t load)
{
	uint8_t load_temp, pos=0;
	if(load>0xFFF)	return (1);	      // Check input data range
	load_temp=load&0x0FF;
	pos=(DAC0->C2^0x11)>>4;
	DAC0->DAT[pos].DATL = load_temp;      // Load the lower byte into the DAC
	load_temp=(load>>8);
	DAC0->DAT[pos].DATH = load_temp;      // Load the higher byte into the DAC
	DAC0->C0 |= DAC_C0_DACSWTRG_MASK;     // Switch to the next buffer data (just loaded)
	return (0);
}
