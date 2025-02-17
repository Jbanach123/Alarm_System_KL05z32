#ifndef ACC_H
#define ACC_H

#include "MKL05Z4.h"
#include "frdm_bsp.h"

// Definicje makr
#define INT2_MASK (1 << 10) // Maska dla INT2
#define ACC_THRESHOLD 0.8     // Próg przyspieszenia
#define ZYXDR_Mask (1 << 3) // Maska bitu ZYXDR w rejestrze STATUS

// Deklaracje zmiennych globalnych
extern uint8_t acc_on;           // Dzia³anie akcelerometru
extern uint8_t status;           // Status akcelerometru
extern uint8_t arrayXYZ[6];      // Bufor na dane XYZ
extern uint8_t sens;             // Czu³oœæ akcelerometru
extern double X, Y, Z;           // Wartoœci przyspieszenia
extern double X_base, Y_base, Z_base; // Wartoœci bazowe kalibracji

// Deklaracja funkcji
void calibrate_accelerometer(void);

#endif /* ACC_H */