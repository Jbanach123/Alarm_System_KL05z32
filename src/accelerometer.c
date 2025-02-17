#include "accelerometer.h"

// Definicje zmiennych globalnych
uint8_t acc_on = 0;           // Dzia³anie akcelerometru
uint8_t status;               // Status akcelerometru
uint8_t arrayXYZ[6];          // Bufor na dane XYZ
uint8_t sens;                 // Czu³oœæ akcelerometru
double X, Y, Z;               // Wartoœci przyspieszenia
double X_base = 0, Y_base = 0, Z_base = 0; // Wartoœci bazowe kalibracji

/**
 * @brief Funkcja kalibracji akcelerometru.
 */
void calibrate_accelerometer(void) 
{
    double X_sum = 0, Y_sum = 0, Z_sum = 0;
    const int samples = 100;
	
	// Konfiguracja akcelerometru
    sens = 0; // Wybór czu³oœci: 0 - 2g; 1 - 4g; 2 - 8g
    I2C_WriteReg(0x1d, 0x2a, 0x0); // ACTIVE=0 - stan czuwania
	  I2C_WriteReg(0x1d, 0xe, sens); // Ustaw czu³oœæ zgodnie ze zmienn¹ sens
	
	// Konfiguracja przerwañ akcelerometru
    I2C_WriteReg(0x1d, 0x2d, 0x04); // CTRL_REG4: W³¹cz INT_EN_FF_MT
    I2C_WriteReg(0x1d, 0x2e, 0x00); // CTRL_REG5: INT2 przypisane do Freefall/Motion
	
	// Konfiguracja Motion Detection
    I2C_WriteReg(0x1d, 0x15, 0x78); // FF_MT_CFG: Wykrywanie ruchu na osiach XYZ
    I2C_WriteReg(0x1d, 0x17, 0x10); // FF_MT_THS: Próg przyspieszenia ustawiony na 1/16 g

    for (int i = 0; i < samples; i++) 
	  {
        I2C_ReadRegBlock(0x1d, 0x1, 6, arrayXYZ);
        X_sum += ((double)((int16_t)((arrayXYZ[0] << 8) | arrayXYZ[1]) >> 2) / (4096 >> sens));
        Y_sum += ((double)((int16_t)((arrayXYZ[2] << 8) | arrayXYZ[3]) >> 2) / (4096 >> sens));
        Z_sum += ((double)((int16_t)((arrayXYZ[4] << 8) | arrayXYZ[5]) >> 2) / (4096 >> sens));
        DELAY(10)
    }

    X_base = X_sum / samples;
    Y_base = Y_sum / samples;
    Z_base = Z_sum / samples;
	
	// Aktywacja akcelerometru
    I2C_WriteReg(0x1d, 0x2a, 0x1); // ACTIVE=1 - stan aktywny
	  PORTA->PCR[10] = PORT_PCR_MUX(1) | PORT_PCR_IRQC(0xA); // PTA10 jako GPIO przerwanie
}