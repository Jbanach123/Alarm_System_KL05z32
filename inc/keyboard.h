#ifndef KLAW_H
#define KLAW_H

#include "MKL05Z4.h"
#include "frdm_bsp.h"
#include "leds.h"

// Bit definitions for rows and columns (reversed keyboard - C4 is the first column from the left, etc.)
#define R2 12 // Bit number for row 2
#define R3 7  // Bit number for row 3
#define R4 11 // Bit number for row 4

#define C1 9  // Bit number for column 1
#define C2 8  // Bit number for column 2
#define C3 5  // Bit number for column 3
#define C4 6  // Bit number for column 4

// Global variable declarations
extern volatile int position;
extern volatile int pressing_detected;
extern volatile int was_key_pressed;

extern volatile uint8_t alarm_on;
extern volatile uint8_t correct_pass;
extern volatile uint8_t enter_mode;
extern volatile uint8_t number;
extern volatile uint8_t counter;
extern uint16_t entered;

// Function declarations
void Klaw_Init(void);
int Detect_Column(void);
void Handle_Row_Interrupt(uint8_t row_bit, int row_number);
void Keyboard(void);

#endif  /* KLAW_H */