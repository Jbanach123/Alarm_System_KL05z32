#include "keyboard.h"

// Global variable definitions
volatile int position = 0;
volatile int pressing_detected = 0;
volatile int was_key_pressed = 0;

volatile uint8_t alarm_on = 0;
volatile uint8_t correct_pass = 0;
volatile uint8_t enter_mode = 0;
volatile uint8_t number = 0;
volatile uint8_t counter = 0;
uint16_t entered = 0;

/**
 * @brief Initializes the keyboard, setting row lines as inputs and columns as outputs.
 */
void Klaw_Init(void)
{
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;

    // Configure rows as inputs with pull-up resistors
    PORTA->PCR[R2] |= PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK; // Row 2
    PORTA->PCR[R3] |= PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK; // Row 3
    PORTA->PCR[R4] |= PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK; // Row 4

    // Set interrupts for rows
    PORTA->PCR[R2] |= PORT_PCR_IRQC(0xa);
    PORTA->PCR[R3] |= PORT_PCR_IRQC(0xa);
    PORTA->PCR[R4] |= PORT_PCR_IRQC(0xa);

    NVIC_SetPriority(PORTA_IRQn, 1);
    NVIC_ClearPendingIRQ(PORTA_IRQn);
    NVIC_EnableIRQ(PORTA_IRQn);

    // Configure columns as outputs, default low state
    PORTA->PCR[C2] |= PORT_PCR_MUX(1);
    PORTA->PCR[C1] |= PORT_PCR_MUX(1);
    PORTA->PCR[C3] |= PORT_PCR_MUX(1);
    PORTA->PCR[C4] |= PORT_PCR_MUX(1);

    PTA->PDDR |= (1 << C1) | (1 << C2) | (1 << C3) | (1 << C4); // Columns 1-4 as outputs
    PTA->PDOR &= ~((1 << C1) | (1 << C2) | (1 << C3) | (1 << C4)); // Set columns to low state
}

/**
 * @brief Detects the active column based on signals from the rows.
 * @return The column number (1-4) that was activated.
 */
int Detect_Column(void)
{
    int column_number = 0;
    // Set all columns to high state
    PTA->PDOR |= (1 << C1);
    PTA->PDOR |= (1 << C2);
    PTA->PDOR |= (1 << C3);
    PTA->PDOR |= (1 << C4);
    
    // Iterate through each column
    for (int i = 0; i < 4; i++)
    {   
        // Set the respective column to low state
        if (i == 3)
            PTA->PDOR &= ~(1 << C1);
        else if (i == 2)
            PTA->PDOR &= ~(1 << C2);
        else if (i == 1)
            PTA->PDOR &= ~(1 << C3);
        else if (i == 0)
            PTA->PDOR &= ~(1 << C4);

        DELAY(20) // Short delay for stabilization

        if ((PTA->PDIR & (1 << R2)) == 0) { column_number = i + 1; break; }
        if ((PTA->PDIR & (1 << R3)) == 0) { column_number = i + 1; break; }
        if ((PTA->PDIR & (1 << R4)) == 0) { column_number = i + 1; break; }

        if (i == 0)
            PTA->PDOR |= (1 << C1);
        else if (i == 1)
            PTA->PDOR |= (1 << C2);
        else if (i == 2)
            PTA->PDOR |= (1 << C3);
        else if (i == 3)
            PTA->PDOR |= (1 << C4);

        DELAY(20) // Short delay for stabilization
    }
    // Restore low state for all columns
    PTA->PDOR &= ~(1 << C1);
    PTA->PDOR &= ~(1 << C2);
    PTA->PDOR &= ~(1 << C3);
    PTA->PDOR &= ~(1 << C4);

    return column_number;
}

/**
 * @brief Handles the interrupt associated with an active row.
 * @param row_bit Flag of the active row.
 * @param row_number The row number (1-3).
 */
void Handle_Row_Interrupt(uint8_t row_bit, int row_number)
{
    NVIC_DisableIRQ(PORTA_IRQn); // Disable interrupts
    int column_number = Detect_Column(); // Detect the column
    DELAY(20);
    NVIC_EnableIRQ(PORTA_IRQn);
    pressing_detected = 1;
    position = (10 * row_number + column_number); // Compute key position
}

/**
 * @brief Handles key press events, performing appropriate actions.
 */
void Keyboard(void)
{
    if (pressing_detected)
    {
        pressing_detected = 0;
        was_key_pressed = 1;
        // Map positions to key numbers
        if (position == 11) number = 1;
        else if (position == 12) number = 2;
        else if (position == 13) number = 3;
        else if (position == 14) number = 4;
        else if (position == 21) number = 5;
        else if (position == 22) number = 6;
        else if (position == 23) number = 7;
        else if (position == 24) number = 8;
        else if (position == 31) number = 9;
        else if (position == 32) number = 0;

        counter++;
        // Handle input mode
        if (position == 33)
        {   
            enter_mode = 1;
            correct_pass = 0;
            counter = 0;
            PTB->PDOR |= LED_G_MASK;
            PTB->PDOR &= ~LED_B_MASK;
            entered = 0;
            LCD1602_ClearAll();
        }
    }
}