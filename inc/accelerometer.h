#ifndef ACC_H
#define ACC_H

#include "MKL05Z4.h"
#include "frdm_bsp.h"

// Macro definitions
#define INT2_MASK (1 << 10) // Mask for INT2
#define ACC_THRESHOLD 0.8   // Acceleration threshold
#define ZYXDR_Mask (1 << 3) // ZYXDR bit mask in STATUS register

// Global variable declarations
extern uint8_t acc_on;           // Accelerometer operation status
extern uint8_t status;           // Accelerometer status
extern uint8_t arrayXYZ[6];      // Buffer for XYZ data
extern uint8_t sens;             // Accelerometer sensitivity
extern double X, Y, Z;           // Acceleration values
extern double X_base, Y_base, Z_base; // Calibration base values

// Function declaration
void calibrate_accelerometer(void);

#endif /* ACC_H */