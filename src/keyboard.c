#include "keyboard.h"

// Definicje zmiennych globalnych
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
 * @brief Inicjalizuje klawiaturê, ustawiaj¹c linie wierszy jako wejœcia i kolumny jako wyjœcia.
 */
void Klaw_Init(void)
{
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;

    // Konfiguracja wierszy jako wejœcia z pull-up
    PORTA->PCR[R2] |= PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK; // Wiersz 2
    PORTA->PCR[R3] |= PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK; // Wiersz 3
    PORTA->PCR[R4] |= PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK; // Wiersz 4

    // Ustawienia przerwañ dla danych wierszy
    PORTA->PCR[R2] |= PORT_PCR_IRQC(0xa);
    PORTA->PCR[R3] |= PORT_PCR_IRQC(0xa);
    PORTA->PCR[R4] |= PORT_PCR_IRQC(0xa);

    NVIC_SetPriority(PORTA_IRQn, 1);
    NVIC_ClearPendingIRQ(PORTA_IRQn);
    NVIC_EnableIRQ(PORTA_IRQn);

    // Konfiguracja kolumn jako wyjœcia, domyœlnie stan niski
    PORTA->PCR[C2] |= PORT_PCR_MUX(1);
    PORTA->PCR[C1] |= PORT_PCR_MUX(1);
    PORTA->PCR[C3] |= PORT_PCR_MUX(1);
    PORTA->PCR[C4] |= PORT_PCR_MUX(1);

    PTA->PDDR |= (1 << C1) | (1 << C2) | (1 << C3) | (1 << C4); // Kolumny 1-4 jako wyjœcia
    PTA->PDOR &= ~((1 << C1) | (1 << C2) | (1 << C3) | (1 << C4)); // Stan niski dla kolumn
}

/**
 * @brief Wykrywa aktywn¹ kolumnê na podstawie sygna³ów z wierszy.
 * @return Numer kolumny (1-4), która zosta³a aktywowana.
 */
int Detect_Column(void)
{
    int column_number = 0;
    // Ustawienie wszystkich kolumn w stan wysoki
    PTA->PDOR |= (1 << C1);
    PTA->PDOR |= (1 << C2);
    PTA->PDOR |= (1 << C3);
    PTA->PDOR |= (1 << C4);
    
	  // Iteracja przez ka¿d¹ kolumnê
    for (int i = 0; i < 4; i++)
    {   
			  // Ustawienie odpowiedniej kolumny w stan niski
        if (i == 3)
            PTA->PDOR &= ~(1 << C1);
        else if (i == 2)
            PTA->PDOR &= ~(1 << C2);
        else if (i == 1)
            PTA->PDOR &= ~(1 << C3);
        else if (i == 0)
            PTA->PDOR &= ~(1 << C4);

        DELAY(20) // Krótkie opóŸnienie na ustabilizowanie

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

        DELAY(20) // Krótkie opóŸnienie na ustabilizowanie
    }
    // Przywrócenie stanu niskiego dla wszystkich kolumn
    PTA->PDOR &= ~(1 << C1);
    PTA->PDOR &= ~(1 << C2);
    PTA->PDOR &= ~(1 << C3);
    PTA->PDOR &= ~(1 << C4);

    return column_number;
}

/**
 * @brief Obs³uguje przerwanie zwi¹zane z aktywnym wierszem.
 * @param row_bit Flaga aktywnego wiersza.
 * @param row_number Numer wiersza (1-3).
 */
void Handle_Row_Interrupt(uint8_t row_bit, int row_number)
{
    NVIC_DisableIRQ(PORTA_IRQn);// Wy³¹czenie przerwañ
    int column_number = Detect_Column();// Wykrycie kolumny
    DELAY(20);
    NVIC_EnableIRQ(PORTA_IRQn);
    pressing_detected = 1;
    position = (10 * row_number + column_number);// Obliczenie pozycji klawisza
}
/**
 * @brief Obs³uguje zdarzenia naciœniêcia klawisza, wykonuj¹c odpowiednie akcje.
 */
void Keyboard(void)
{
    if (pressing_detected)
    {
        pressing_detected = 0;
			  was_key_pressed = 1;
			  // Mapa pozycji do numerów klawiszy
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
        // Obs³uga trybu wprowadzania
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