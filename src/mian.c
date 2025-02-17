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

//Zmienne dla alarmu d¿wiêkowego
float	ampl_v;
float freq;
uint16_t	mod_curr=MOD_M;
uint16_t	ampl;
uint8_t k_curr=50;
// Zmienne dla czujnika ruchu
volatile uint32_t d=0;
volatile float wynik=0;
volatile float wynik_f=1.123456;
volatile float tick, tick_head;
volatile float distance;
volatile uint8_t measure_ready = 0;
volatile uint32_t ps_value[] = {1, 2, 4, 8, 16, 32, 64, 128};
// Zmienne do wyœwietlania
char status_message[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
char numbers_display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};

uint16_t password = 801;

/**
 * @brief Obs³uguje przerwania dla portu A.
 */
void PORTA_IRQHandler(void)
{
	
 	 	if (PORTA->ISFR & (1 << R2))
		{
		 	  DELAY(20)
				if ((PTA->PDIR & (1 << R2)) == 0) // Sprawdzenie, czy przerwanie jest aktywne
				{	
				 	  Handle_Row_Interrupt(R2, 3);
				}
				PORTA->ISFR |= (1 << R2); // Czyszczenie flagi przerwania
		}
		if (PORTA->ISFR & (1 << R3))
		{
		    DELAY(20)
				if ((PTA->PDIR & (1 << R3)) == 0) // Sprawdzenie, czy przerwanie jest aktywne
				{	
				    Handle_Row_Interrupt(R3, 2);
				}	
				PORTA->ISFR |= (1 << R3); // Czyszczenie flagi przerwania
		}
		if (PORTA->ISFR & (1 << R4))
    {
        DELAY(20)
   			if ((PTA->PDIR & (1 << R4)) == 0) // Sprawdzenie, czy przerwanie jest aktywne
				{	
				    Handle_Row_Interrupt(R4, 1);
        }
				PORTA->ISFR |= (1 << R4); // Czyszczenie flagi przerwania
    }
	
		if (PORTA->ISFR & (INT2_MASK) && correct_pass) // SprawdŸ, czy przerwanie pochodzi z PTA10
    {  
		    PORTA->ISFR = (INT2_MASK); // Wyczyœæ flagê przerwania
        acc_on = 1;      
    } 
}

/**
 * @brief Obs³uguje przerwania modu³u TPM1.
 */
void TPM1_IRQHandler(void)
{ 
    if (correct_pass)
		{
	      TPM1->SC = 0;		// TPM1 stop
	      if((TPM1->STATUS & TPM_STATUS_TOF_MASK))		// Sprawdzenie, czy impuls nie jest za d³ugi
	      {
		 	      TPM1->SC = 0;
			      wynik=100000;
			      d+=1;						// Korekcja czêstotliwoœci wzorcowej
			      if(d==8)
			          d=0;
	      }
	      if(TPM1->STATUS & TPM_STATUS_CH1F_MASK)
	      {
		  	    wynik=TPM1->CONTROLS[1].CnV;		// Aktualizacja wyniku
			      measure_ready = 1;
	      }
		
	      TPM1->STATUS |= TPM_STATUS_CH1F_MASK;		// Kasowanie flag przerwania
	      TPM1->STATUS |= TPM_STATUS_TOF_MASK;
	      TPM1->SC = d;		// Odtworzenie dzielnika PS
	      TPM1->SC |= TPM_SC_TOIE_MASK;		// Odblokowanie przerwania od TOF
	      TPM1->SC |= TPM_SC_CMOD(1);		// TMP1 aktywny
    }
}

/**
 * @brief Pomiar czujnika ultradŸwiêkowego.
 */
void Sensor_Measurement(void) 
{
    PTB->PSOR = (1 << 11); // Ustawienie pinu - pocz¹tek pomiaru
	  for (int i = 0; i < 400; i++) __NOP(); // generowanie impulsu
    PTB->PCOR = (1 << 11); // Wyczyszczenie pinu - koniec pomiaru
}

/**
 * @brief Obs³uguje dane z akcelerometru i sprawdza progi alarmowe.
 */
void Accelerometer_Measurement()
{
 	  if(acc_on)
    {
	      I2C_ReadReg(0x1d, 0x0, &status);
        status &= ZYXDR_Mask;
        if (status)  // Czy dane gotowe do odczytu?
        {    
				    uint8_t int_source;
            I2C_ReadReg(0x1d, 0x0C, &int_source); // Odczyt INT_SOURCE
            if (int_source & 0x04)  // SprawdŸ, czy Ÿród³em jest FF_MT (bit 2)
            {   
							  // Odczyt danych XYZ
				        I2C_ReadRegBlock(0x1d, 0x1, 6, arrayXYZ);

                // Przeliczenie wartoœci przyspieszenia
                X = ((double)((int16_t)((arrayXYZ[0] << 8) | arrayXYZ[1]) >> 2) / (4096 >> sens)) - X_base;
                Y = ((double)((int16_t)((arrayXYZ[2] << 8) | arrayXYZ[3]) >> 2) / (4096 >> sens)) - Y_base;
                Z = ((double)((int16_t)((arrayXYZ[4] << 8) | arrayXYZ[5]) >> 2) / (4096 >> sens)) - Z_base;
                    
	              acc_on = 0;								
									
                // Sprawdzenie, czy przyspieszenie przekracza próg
                if (fabs(X) > ACC_THRESHOLD || fabs(Y) > ACC_THRESHOLD || fabs(Z) > ACC_THRESHOLD) 
					      {
                    alarm_on = 1; // W³¹cz alarm
						        sprintf(status_message,"Alarm  On");
						        PTB->PDOR |= LED_B_MASK;   // Zgaœ niebiesk¹ diodê
                }     
            }  
        }
 	  }						
}

/**
 * @brief Obs³uguje w³¹czanie/wy³¹czanie alarmu dŸwiêkowego oraz diody Led.
 */
void Alarm()
{
    if (alarm_on) // Alarm w³¹czony
    {  
		    ampl_v=3.0*(100.0-k_curr)/100.0;
				mod_curr=31813/8; //ustawienie dŸwiêku
		    TPM0->MOD = mod_curr;
	      ampl=((int)mod_curr*k_curr)/100;
	      TPM0->CONTROLS[3].CnV = ampl;	// Nowa wartoœæ kreuj¹ca wspó³czynnik wype³nienia PWM    
    }
    else  
    {
				TPM0->CONTROLS[3].CnV = 0;	// Wy³¹czenie PWM (brak sygna³u na wyjœciu)  
    }
}

/**
 * @brief Sprawdza poprawnoœæ has³a wprowadzania.
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
					  enter_mode = 0;       // Wyjœcie z trybu wpisywania
					  PTB->PDOR |= LED_B_MASK;   // Zgaœ niebiesk¹ diodê
            counter = 0;
				    entered = entered * 10 + number;
					  if (entered==password)
            {
						    correct_pass = 1;
					      PTB->PDOR |= LED_G_MASK;   // Zgaœ zielon¹ diodê
							  LCD1602_SetCursor(0,0);
                sprintf(numbers_display,"%d",entered);
							  LCD1602_Print(numbers_display);
							  DELAY(5000)
							  if (alarm_on)
						    {
							      correct_pass = 0;
							      alarm_on = 0;
							      sprintf(status_message,"Alarm Off");
							      PTB->PDOR &= ~LED_G_MASK;   // Zapal zielon¹ diodê
						    }
						}
						else if(!alarm_on)
						    PTB->PDOR &= ~LED_G_MASK;   // Zapal zielon¹ diodê
						LCD1602_ClearAll();
						entered = 0;
						was_key_pressed = 0;
				}
    }
}

int main(void)
{   
    LCD1602_Init();                             // Inicjalizacja LCD
    LCD1602_Backlight(TRUE);      // W??czenie pod?wietlenia
    LCD1602_ClearAll();
    LED_Init();
	  I2C_Init();
    Klaw_Init();
		PWM_Init();				// Inicjalizacja licznika TPM0 (PWM „Low-true pulses”)
    calibrate_accelerometer(); // Kalibracja akcelerometru
    InCap_OutComp_Init();               // Inicjalizacja TPM1 - Input Capture
		
		sprintf(status_message,"Alarm Off");
    PTB->PDOR &= ~LED_G_MASK;    // Zapalenie zielonej diody
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
                distance = wynik / 148 * 1000; // przeliczenie na centymetry
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
