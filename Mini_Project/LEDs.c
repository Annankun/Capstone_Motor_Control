#include <MKL25Z4.H>
#include "user_defs.h"
#include "LEDs.h"

/*----------------------------------------------------------------------------
  Initialize RGB LEDs using PWM (TPM0)
  - Red LED: PTB18 -> TPM2_CH0 (ALT3)
  - Green LED: PTB19 -> TPM2_CH1 (ALT3)
  - Blue LED: PTD1 -> TPM0_CH1 (ALT4)
 *----------------------------------------------------------------------------*/
void Init_RGB_LEDs_PWM(void) {
    // Enable clock to ports B and D
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK;

    // Enable clock to TPM0 and TPM2 modules
    SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK | SIM_SCGC6_TPM2_MASK;

    // Select TPM clock source: MCGFLLCLK or MCGPLLCLK/2
    SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);

    // Configure pin mux for PWM (ALT3 for PTB, ALT4 for PTD)
    PORTB->PCR[RED_LED_POS] &= ~PORT_PCR_MUX_MASK;
    PORTB->PCR[RED_LED_POS] |= PORT_PCR_MUX(3);     // TPM2_CH0

    PORTB->PCR[GREEN_LED_POS] &= ~PORT_PCR_MUX_MASK;
    PORTB->PCR[GREEN_LED_POS] |= PORT_PCR_MUX(3);   // TPM2_CH1

    PORTD->PCR[BLUE_LED_POS] &= ~PORT_PCR_MUX_MASK;
    PORTD->PCR[BLUE_LED_POS] |= PORT_PCR_MUX(4);    // TPM0_CH1

    // Configure TPM2 for Red and Green LEDs
    TPM2->MOD = 255;                                 // Set 8-bit resolution (0-255)
    TPM2->SC = TPM_SC_CMOD(1) | TPM_SC_PS(3);       // Counter mode, prescaler = 8

    // Configure TPM0 for Blue LED
    TPM0->MOD = 255;                                 // Set 8-bit resolution (0-255)
    TPM0->SC = TPM_SC_CMOD(1) | TPM_SC_PS(3);       // Counter mode, prescaler = 8

    // Set PWM mode: Edge-aligned PWM, High-True pulses
    TPM2->CONTROLS[0].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;  // Red
    TPM2->CONTROLS[1].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;  // Green
    TPM0->CONTROLS[1].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;  // Blue

    // Initialize all LEDs to OFF (255 for active-low LEDs)
    TPM2->CONTROLS[0].CnV = 255;  // Red OFF
    TPM2->CONTROLS[1].CnV = 255;  // Green OFF
    TPM0->CONTROLS[1].CnV = 255;  // Blue OFF
}

/*----------------------------------------------------------------------------
  Control RGB LED brightness using PWM
  Parameters:
    red: 0-255 (0 = OFF, 255 = MAX brightness)
    green: 0-255
    blue: 0-255
  Note: KL25Z LEDs are active-low, so CnV = 255 - brightness
 *----------------------------------------------------------------------------*/
void Control_RGB_LEDs_Brightness(uint8_t red, uint8_t green, uint8_t blue) {
    // KL25Z on-board LEDs are active-low
    // CnV = 255 means LED OFF, CnV = 0 means LED fully ON
    TPM2->CONTROLS[0].CnV = 255 - red;    // Red LED
    TPM2->CONTROLS[1].CnV = 255 - green;  // Green LED
    TPM0->CONTROLS[1].CnV = 255 - blue;   // Blue LED
}

/*----------------------------------------------------------------------------
  Legacy GPIO control function (ON/OFF only, no brightness control)
  Kept for backward compatibility
 *----------------------------------------------------------------------------*/
void Init_RGB_LEDs(void) {
    // Enable clock to ports B and D
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK;

    // Make 3 pins GPIO
    PORTB->PCR[RED_LED_POS] &= ~PORT_PCR_MUX_MASK;
    PORTB->PCR[RED_LED_POS] |= PORT_PCR_MUX(1);
    PORTB->PCR[GREEN_LED_POS] &= ~PORT_PCR_MUX_MASK;
    PORTB->PCR[GREEN_LED_POS] |= PORT_PCR_MUX(1);
    PORTD->PCR[BLUE_LED_POS] &= ~PORT_PCR_MUX_MASK;
    PORTD->PCR[BLUE_LED_POS] |= PORT_PCR_MUX(1);

    // Set ports to outputs
    PTB->PDDR |= MASK(RED_LED_POS) | MASK(GREEN_LED_POS);
    PTD->PDDR |= MASK(BLUE_LED_POS);
}

void Control_RGB_LEDs(unsigned int red_on, unsigned int green_on, unsigned int blue_on) {
    if (red_on) {
        PTB->PCOR = MASK(RED_LED_POS);
    } else {
        PTB->PSOR = MASK(RED_LED_POS);
    }
    if (green_on) {
        PTB->PCOR = MASK(GREEN_LED_POS);
    } else {
        PTB->PSOR = MASK(GREEN_LED_POS);
    }
    if (blue_on) {
        PTD->PCOR = MASK(BLUE_LED_POS);
    } else {
        PTD->PSOR = MASK(BLUE_LED_POS);
    }
}
