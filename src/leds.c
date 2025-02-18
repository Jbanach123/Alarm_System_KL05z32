#include "leds.h"

void LED_Init(void)
{
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;      // Enable port B
	// PORTB->PCR[LED_R] |= PORT_PCR_MUX(1);
	PORTB->PCR[LED_G] |= PORT_PCR_MUX(1);	
	PORTB->PCR[LED_B] |= PORT_PCR_MUX(1);
	PTB->PDDR |= LED_G_MASK | LED_B_MASK;	// Set bits 8, 9, and 10 to 1 – configure as outputs
	PTB->PDOR |= LED_G_MASK | LED_B_MASK;	// Turn off all LEDs
}