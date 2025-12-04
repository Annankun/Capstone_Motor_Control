#include "Switch.h"
#include "MKL25Z4.h"
#include "user_defs.h"

// Global variable for stop status
volatile static uint8_t stop_flag = 0;

void Switch_Init(void) {
    // Enable clock to Port D
    SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;

    // Configure PTD4 as GPIO input
    PORTD->PCR[SWITCH_PIN] &= ~PORT_PCR_MUX_MASK;
    PORTD->PCR[SWITCH_PIN] |= PORT_PCR_MUX(1);       // GPIO mode

    // Enable internal pull-up resistor
    PORTD->PCR[SWITCH_PIN] |= PORT_PCR_PE_MASK;      // Pull enable
    PORTD->PCR[SWITCH_PIN] |= PORT_PCR_PS_MASK;      // Pull-up

    // Configure interrupt on FALLING edge only (button press)
    PORTD->PCR[SWITCH_PIN] |= PORT_PCR_IRQC(0x0a);   // 0x0a = falling edge

    // Set PTD4 as input
    PTD->PDDR &= ~MASK(SWITCH_PIN);

    // Configure NVIC for Port D interrupt
    NVIC_SetPriority(PORTD_IRQn, 128);
    NVIC_ClearPendingIRQ(PORTD_IRQn);
    NVIC_EnableIRQ(PORTD_IRQn);

    // Initialize stop flag to 0 (running)
    stop_flag = 0;
}

uint8_t Get_Stop_Status(void) {
    return stop_flag;
}

/*----------------------------------------------------------------------------
  Port D Interrupt Handler - TOGGLE/LATCH mode
  Each button press toggles the stop state
 *----------------------------------------------------------------------------*/
void PORTD_IRQHandler(void) {
    // Clear pending interrupt
    NVIC_ClearPendingIRQ(PORTD_IRQn);

    // Check if interrupt is from PTD4
    if (PORTD->ISFR & MASK(SWITCH_PIN)) {
        // Debounce delay
        for(volatile int i = 0; i < 10000; i++);

        // Check if button is still pressed (to avoid noise)
        if (!(PTD->PDIR & MASK(SWITCH_PIN))) {
            // Toggle stop flag (0 → 1 or 1 → 0)
            stop_flag = !stop_flag;
        }

        // Clear PTD4 interrupt flag
        PORTD->ISFR = MASK(SWITCH_PIN);
    }

    // Clear all Port D flags
    PORTD->ISFR = 0xffffffff;
}
