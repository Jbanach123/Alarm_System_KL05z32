#ifndef KLAW_H
#define KLAW_H

#include "MKL05Z4.h"
#include "frdm_bsp.h"
#include "leds.h"

// Definicje bitów dla wierszy i kolumn (odwrotna klawiatura - C4 to pierwsza kolumna od lewej etc.)
#define R2 12 // Numer bitu dla wiersza 2
#define R3 7  // Numer bitu dla wiersza 3
#define R4 11 // Numer bitu dla wiersza 4

#define C1 9  // Numer bitu dla kolumny 1
#define C2 8  // Numer bitu dla kolumny 2
#define C3 5  // Numer bitu dla kolumny 3
#define C4 6  // Numer bitu dla kolumny 4

// Deklaracje zmiennych globalnych
extern volatile int position;
extern volatile int pressing_detected;
extern volatile int was_key_pressed;

extern volatile uint8_t alarm_on;
extern volatile uint8_t correct_pass;
extern volatile uint8_t enter_mode;
extern volatile uint8_t number;
extern volatile uint8_t counter;
extern uint16_t entered;

// Deklaracje funkcji
void Klaw_Init(void);
int Detect_Column(void);
void Handle_Row_Interrupt(uint8_t row_bit, int row_number);
void Keyboard(void);

#endif  /* KLAW_H */