#include "ADC.h"
#include "MKL25Z4.h"

/*----------------------------------------------------------------------------
  Initialize ADC0 for potentiometer reading
  - Channel: ADC0_SE8 (PTB0)
  - Resolution: 8-bit (0-255)
  - Clock: Bus clock
 *----------------------------------------------------------------------------*/
void ADC_Init(void) {
    // Enable clock to ADC0 module
    SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;

    // Enable clock to Port B (for PTB0)
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;

    // Configure PTB0 as analog input (MUX = 0)
    PORTB->PCR[0] &= ~PORT_PCR_MUX_MASK;
    PORTB->PCR[0] |= PORT_PCR_MUX(0);

    // Configure ADC0
    // - MODE(0): 8-bit conversion
    // - ADICLK(0): Bus clock
    ADC0->CFG1 = ADC_CFG1_MODE(0) |      // 8-bit mode
                 ADC_CFG1_ADICLK(0);     // Bus clock

    // Disable module initially (ADCH = 31 disables conversion)
    ADC0->SC1[0] = ADC_SC1_ADCH(31);
}

/*----------------------------------------------------------------------------
  Read ADC value from potentiometer
  Returns: 8-bit value (0-255)
  - 0: Potentiometer at minimum position (0V)
  - 255: Potentiometer at maximum position (3.3V)
 *----------------------------------------------------------------------------*/
uint8_t ADC_Read(void) {
    // Start conversion on channel 8 (PTB0)
    ADC0->SC1[0] = ADC_SC1_ADCH(8);

    // Wait for conversion to complete
    // COCO (Conversion Complete) flag will be set when done
    while (!(ADC0->SC1[0] & ADC_SC1_COCO_MASK));

    // Read and return the result (8-bit value in ADC0->R[0])
    return (uint8_t)(ADC0->R[0]);
}
