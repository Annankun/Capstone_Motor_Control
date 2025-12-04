#include "PWM.h"
#include "user_defs.h"

void PWM_Init(void) {
    // Enable clock for PORTB, PORTD, TPM0, TPM2
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK;
    SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK | SIM_SCGC6_TPM2_MASK;

    // Select clock source for TPM: MCGFLLCLK or MCGPLLCLK/2
    SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);

    // Configure PTB18 as TPM2_CH0 (Red LED) - MUX ALT3
    PORTB->PCR[18] &= ~PORT_PCR_MUX_MASK;
    PORTB->PCR[18] |= PORT_PCR_MUX(3);

    // Configure PTB19 as TPM2_CH1 (Green LED) - MUX ALT3
    PORTB->PCR[19] &= ~PORT_PCR_MUX_MASK;
    PORTB->PCR[19] |= PORT_PCR_MUX(3);

    // Configure PTD1 as TPM0_CH1 (Blue LED) - MUX ALT4
    PORTD->PCR[1] &= ~PORT_PCR_MUX_MASK;
    PORTD->PCR[1] |= PORT_PCR_MUX(4);

    // Configure TPM2 (Red and Green)
    TPM2->SC = 0;                                    // Disable TPM2 while configuring
    TPM2->MOD = 255;                                 // Set PWM period (8-bit: 0-255)
    TPM2->SC = TPM_SC_CMOD(1) | TPM_SC_PS(3);       // Enable counter, prescaler = 8

    // Configure TPM2 Channel 0 (Red) - Edge-aligned PWM, high-true pulses
    TPM2->CONTROLS[0].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
    TPM2->CONTROLS[0].CnV = 255;  // Start with LED OFF (active-low)

    // Configure TPM2 Channel 1 (Green)
    TPM2->CONTROLS[1].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
    TPM2->CONTROLS[1].CnV = 255;  // Start with LED OFF

    // Configure TPM0 (Blue)
    TPM0->SC = 0;                                    // Disable TPM0 while configuring
    TPM0->MOD = 255;                                 // Set PWM period (8-bit: 0-255)
    TPM0->SC = TPM_SC_CMOD(1) | TPM_SC_PS(3);       // Enable counter, prescaler = 8

    // Configure TPM0 Channel 1 (Blue)
    TPM0->CONTROLS[1].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
    TPM0->CONTROLS[1].CnV = 255;  // Start with LED OFF
}

void Set_Red_Brightness(uint8_t brightness) {
    // brightness: 0 = OFF, 100 = MAX
    if(brightness > 100) brightness = 100;

    // Convert 0-100 to 0-255
    uint8_t pwm_value = (brightness * 255) / 100;

    // Active-low LED: CnV = 255 means OFF, CnV = 0 means fully ON
    TPM2->CONTROLS[0].CnV = 255 - pwm_value;
}

void Set_Green_Brightness(uint8_t brightness) {
    if(brightness > 100) brightness = 100;
    uint8_t pwm_value = (brightness * 255) / 100;
    TPM2->CONTROLS[1].CnV = 255 - pwm_value;
}

void Set_Blue_Brightness(uint8_t brightness) {
    if(brightness > 100) brightness = 100;
    uint8_t pwm_value = (brightness * 255) / 100;
    TPM0->CONTROLS[1].CnV = 255 - pwm_value;
}

void Set_RGB(uint8_t r, uint8_t g, uint8_t b) {
    Set_Red_Brightness(r);
    Set_Green_Brightness(g);
    Set_Blue_Brightness(b);
}
