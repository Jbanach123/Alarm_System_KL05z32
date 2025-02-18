#include "TPM.h"

void PWM_Init()
{
	//SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;			// Enable clock signal for port B
	// Select the appropriate role for the port B pin
	PORTB->PCR[8] |= PORT_PCR_MUX(2);		// LED R 	TPM0_CH3
	//PORTB->PCR[9] |= PORT_PCR_MUX(2);		// LED G	TPM0_CH2
	//PORTB->PCR[10] |= PORT_PCR_MUX(2);	// LED B	TPM0_CH1
	
	SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;			// Enable clock signal for TPM0
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);			// Select TPMx clock source MCGFLLCLK=41943040Hz
	
	TPM0->SC &= ~TPM_SC_CPWMS_MASK;				// TPM0 in "up counting" mode
	TPM0->SC |= TPM_SC_PS(5);					// Input clock prescaler set to 32; clock=1310720Hz
	TPM0->MOD = 0xFFFF;							// MODULO register = 65535 - frequency output = 20Hz
	
	TPM0->CONTROLS[3].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSA_MASK;	// TPM0, channel 3 (red LED) in "Edge-aligned PWM Low-true pulses" mode (set Output on match, clear Output on reload)
	TPM0->CONTROLS[3].CnV = 0x0000;				// Initial duty cycle set to 0
	TPM0->SC |= TPM_SC_CMOD(1);					// Enable TPM0 counter
}

void InCap_OutComp_Init()
{
	// Select the appropriate role for the pin
	PORTB->PCR[0] |= PORT_PCR_MUX(2);		// PTB0 - EXTRG_IN
	PORTB->PCR[13] |= PORT_PCR_MUX(2);	    // PTB13 - TPM1_CH1 (Input Capture)
	PORTB->PCR[11] |= PORT_PCR_MUX(1);	    // PTB11 - GPIO
	PORTB->PCR[11] &= (~PORT_PCR_SRE_MASK);
	PTB->PDDR |= (1<<11);					// PTB11 - output
	PTB->PCOR = (1<<11);
	
    // TPM1 configuration
	SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;		// Enable clock signal for TPM1 module
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);		// Select TPMx clock source MCGFLLCLK=41943040Hz
	
	TPM1->SC &= (~TPM_SC_CPWMS_MASK);		// TPM1 in "up counting" mode
	TPM1->SC &= (~TPM_SC_PS_MASK);			// Input clock prescaler set to 1; clock = 41943040Hz
	TPM1->CONF |= (TPM_CONF_CSOT_MASK | TPM_CONF_CROT_MASK | TPM_CONF_CSOO_MASK);	// TPM1: hardware triggering, reload on trigger, stop on OVERFLOW
	TPM1->CONF &= (~TPM_CONF_TRGSEL_MASK);	// Triggering from external signal EXTRG_IN
	TPM1->CONTROLS[1].CnSC = (TPM_CnSC_ELSB_MASK | TPM_CnSC_CHIE_MASK); // Input Capture on channel 1 on falling edge, interrupt enabled for this mode
	
	TPM1->SC |= TPM_SC_TOIE_MASK;			// Enable OVERFLOW interrupt for TPM1
	NVIC_SetPriority(TPM1_IRQn,2);
	NVIC_ClearPendingIRQ(TPM1_IRQn);
    NVIC_EnableIRQ(TPM1_IRQn);
	
	TPM1->SC |= TPM_SC_CMOD(1);		// Enable TPM1
}