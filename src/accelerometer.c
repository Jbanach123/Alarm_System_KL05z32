#include "accelerometer.h"

// Global variable definitions
uint8_t acc_on = 0;           // Accelerometer operation status
uint8_t status;               // Accelerometer status
uint8_t arrayXYZ[6];          // Buffer for XYZ data
uint8_t sens;                 // Accelerometer sensitivity
double X, Y, Z;               // Acceleration values
double X_base = 0, Y_base = 0, Z_base = 0; // Calibration baseline values

/**
 * @brief Accelerometer calibration function.
 */
void calibrate_accelerometer(void) 
{
    double X_sum = 0, Y_sum = 0, Z_sum = 0;
    const int samples = 100;
	
    // Accelerometer configuration
    sens = 0; // Select sensitivity: 0 - 2g; 1 - 4g; 2 - 8g
    I2C_WriteReg(0x1d, 0x2a, 0x0); // ACTIVE=0 - standby mode
    I2C_WriteReg(0x1d, 0xe, sens); // Set sensitivity according to the 'sens' variable
	
    // Accelerometer interrupt configuration
    I2C_WriteReg(0x1d, 0x2d, 0x04); // CTRL_REG4: Enable INT_EN_FF_MT
    I2C_WriteReg(0x1d, 0x2e, 0x00); // CTRL_REG5: Assign INT2 to Freefall/Motion detection
	
    // Motion detection configuration
    I2C_WriteReg(0x1d, 0x15, 0x78); // FF_MT_CFG: Motion detection enabled for XYZ axes
    I2C_WriteReg(0x1d, 0x17, 0x10); // FF_MT_THS: Acceleration threshold set to 1/16 g

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
	
    // Activate the accelerometer
    I2C_WriteReg(0x1d, 0x2a, 0x1); // ACTIVE=1 - active mode
    PORTA->PCR[10] = PORT_PCR_MUX(1) | PORT_PCR_IRQC(0xA); // PTA10 as GPIO interrupt
}