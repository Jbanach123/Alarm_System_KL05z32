#include "MKL05Z4.h"
//#define LED_R_MASK (1<<8)		// Mask for the red LED (R)
#define LED_G_MASK (1<<9)		// Mask for the green LED (G)
#define LED_B_MASK (1<<10)		// Mask for the blue LED (B)
#define LED_R 8					// Bit number for the red LED
#define LED_G 9					// Bit number for the green LED
#define LED_B 10				// Bit number for the blue LED

void LED_Init(void);