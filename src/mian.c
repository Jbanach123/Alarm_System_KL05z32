#include "MKL05Z4.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "frdm_bsp.h"
#include "i2c.h"
#include "math.h"
#include "leds.h"
#include "keyboard.h"
#include "TPM.h"
#include "accelerometer.h"

#define MOD_M	40082
#define ZEGAR 1310720

// Variables for the sound alarm
float ampl_v;
float freq;
uint16_t mod_curr=MOD_M;
uint16_t ampl;
uint8_t k_curr=50;
// Variables for the motion sensor
volatile uint32_t d=0;
volatile float wynik=0;
volatile float wynik_f=1.123456;
volatile float tick, tick_head;
volatile float distance;
volatile uint8_t measure_ready = 0;
volatile uint32_t ps_value[] = {1, 2, 4, 8, 16, 32, 64, 128};
// Variables for display
char status_message[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
char numbers_display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};

uint16_t password = 801;

/**
 * @brief Handles interrupts for port A.
 */
void PORTA_IRQHandler(void)
{
    if (PORTA->ISFR & (1 << R2))
    {
        DELAY(20)
	if ((PTA->PDIR & (1 << R2)) == 0) // Check if the interrupt is active
	{	
	    Handle_Row_Interrupt(R2, 3);
	}
	PORTA->ISFR |= (1 << R2); // Clear interrupt flag
    }
    if (PORTA->ISFR & (1 << R3))
    {
	DELAY(20)
	if ((PTA->PDIR & (1 << R3)) == 0) // Check if the interrupt is active
	{	
	    Handle_Row_Interrupt(R3, 2);
	}	
	PORTA->ISFR |= (1 << R3); // Clear interrupt flag
    }
    if (PORTA->ISFR & (1 << R4))
    {
        DELAY(20)
   	if ((PTA->PDIR & (1 << R4)) == 0) // Check if the interrupt is active
	{	
	    Handle_Row_Interrupt(R4, 1);
        }
	PORTA->ISFR |= (1 << R4); // Clear interrupt flag
    }
	
    if (PORTA->ISFR & (INT2_MASK) && correct_pass) // Check if the interrupt comes from PTA10
    {  
	PORTA->ISFR = (INT2_MASK); // Clear interrupt flag
        acc_on = 1;      
    } 
}

/**
 * @brief Handles interrupts for TPM1 module.
 */
void TPM1_IRQHandler(void)
{ 
    if (correct_pass)
    {
	TPM1->SC = 0;	// Stop TPM1
	if((TPM1->STATUS & TPM_STATUS_TOF_MASK))	// Check if the pulse is too long
	{
	    TPM1->SC = 0;
	    wynik=100000;
	    d+=1;		 // Correct reference frequency
	    if(d==8)
	        d=0;
	}
	if(TPM1->STATUS & TPM_STATUS_CH1F_MASK)
	{
	    wynik=TPM1->CONTROLS[1].CnV;	// Update result
	    measure_ready = 1;
	}
		
	TPM1->STATUS |= TPM_STATUS_CH1F_MASK; // Clear interrupt flags
	TPM1->STATUS |= TPM_STATUS_TOF_MASK;
	TPM1->SC = d;		             // Restore PS divider
	TPM1->SC |= TPM_SC_TOIE_MASK;	     // Enable TOF interrupt
	TPM1->SC |= TPM_SC_CMOD(1);		     // Activate TPM1
    }
}

/**
 * @brief Ultrasonic sensor measurement.
 */
void Sensor_Measurement(void) 
{
    PTB->PSOR = (1 << 11); // Set pin - start measurement
    for (int i = 0; i < 400; i++) __NOP(); // Generate pulse
    PTB->PCOR = (1 << 11); // Clear pin - end measurement
}

/**
 * @brief Processes accelerometer data and checks alarm thresholds.
 */
void Accelerometer_Measurement()
{
    if(acc_on)
    {
	I2C_ReadReg(0x1d, 0x0, &status);
        status &= ZYXDR_Mask;
        if (status)  // Are data ready to read?
        {    
	    uint8_t int_source;
            I2C_ReadReg(0x1d, 0x0C, &int_source); // Read INT_SOURCE
            if (int_source & 0x04)  // Check if source is FF_MT (bit 2)
            {   
							  
		I2C_ReadRegBlock(0x1d, 0x1, 6, arrayXYZ); // Read XYZ data

                // Convert acceleration values
                X = ((double)((int16_t)((arrayXYZ[0] << 8) | arrayXYZ[1]) >> 2) / (4096 >> sens)) - X_base;
                Y = ((double)((int16_t)((arrayXYZ[2] << 8) | arrayXYZ[3]) >> 2) / (4096 >> sens)) - Y_base;
                Z = ((double)((int16_t)((arrayXYZ[4] << 8) | arrayXYZ[5]) >> 2) / (4096 >> sens)) - Z_base;
                    
	        acc_on = 0;								
									
                // Check if acceleration exceeds threshold
                if (fabs(X) > ACC_THRESHOLD || fabs(Y) > ACC_THRESHOLD || fabs(Z) > ACC_THRESHOLD) 
		{
                    alarm_on = 1; // Activate alarm
		    sprintf(status_message,"Alarm  On");
		    PTB->PDOR |= LED_B_MASK;   // Turn off blue LED
                }     
            }  
        }
    }						
}

/**
 * @brief Handles turning the sound alarm and LED on/off.
 */
void Alarm()
{
    if (alarm_on) // Alarm activated
    {  
        ampl_v=3.0*(100.0-k_curr)/100.0;
	mod_curr=31813/8; // Set sound
	TPM0->MOD = mod_curr;
	ampl=((int)mod_curr*k_curr)/100;
	TPM0->CONTROLS[3].CnV = ampl;	// Set PWM duty cycle     
    }
    else  
    {
	TPM0->CONTROLS[3].CnV = 0;	// Disable PWM (no output signal)  
    }
}

/**
 * @brief Checks password correctness.
 */
void Check_Password(void)
{
    if (enter_mode)
    {
        if (counter ==1 && was_key_pressed)
        {
	    entered += number;
	    was_key_pressed = 0;
	}
	else if(counter == 2 && was_key_pressed)
        {
	    entered = entered * 10 + number;
	    was_key_pressed = 0;
	}
	else if(counter >= 3  && was_key_pressed)
        {   
	    enter_mode = 0;       // Exit password mode
	    PTB->PDOR |= LED_B_MASK;   // Turn off blue LED
            counter = 0;
	    entered = entered * 10 + number;
	    if (entered==password)
            {
		correct_pass = 1;
		PTB->PDOR |= LED_G_MASK;   // Turn off green LED
		LCD1602_SetCursor(0,0);
                sprintf(numbers_display,"%d",entered);
		LCD1602_Print(numbers_display);
		DELAY(5000)
		if (alarm_on)
		{
		    correct_pass = 0;
		    alarm_on = 0;
		    sprintf(status_message,"Alarm Off");
		    PTB->PDOR &= ~LED_G_MASK;   // Zapal zielon� diod�
		}
	    }
	    else if(!alarm_on)
		PTB->PDOR &= ~LED_G_MASK;   // Zapal zielon� diod�

	    LCD1602_ClearAll();
	    entered = 0;
	    was_key_pressed = 0;
	}
    }
}

int main(void)
{   
    // Initializations
    LCD1602_Init();           
    LCD1602_Backlight(TRUE);      
    LCD1602_ClearAll();
    LED_Init();
    I2C_Init();
    Klaw_Init();
    PWM_Init();				
    calibrate_accelerometer(); 
    InCap_OutComp_Init();               
		
    sprintf(status_message,"Alarm Off");
    PTB->PDOR &= ~LED_G_MASK;   // Turn on green LED
    tick_head = 1000.0 / SystemCoreClock;			
    while (1)
    {   
        if(correct_pass)
	{
	    Sensor_Measurement();
            if (measure_ready) 
            { 
                measure_ready = 0;
                tick = tick_head * ps_value[d];
                wynik *= tick;
                distance = wynik / 148 * 1000; // Convert to centimeters
            }
            if (distance > 0 && distance < 15) 
            {
                alarm_on = 1;
		sprintf(status_message,"Alarm  On");
            }		
	}

        LCD1602_SetCursor(0,0);
        sprintf(numbers_display,"%d",entered);
        LCD1602_Print(numbers_display);
	LCD1602_SetCursor(0,1);
        LCD1602_Print(status_message);
	Accelerometer_Measurement();
	Keyboard();
        Check_Password();
        Alarm();       
    }
}
