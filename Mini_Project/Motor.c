#include "Motor.h"
#include "MKL25Z4.h"
#include "user_defs.h"

/*----------------------------------------------------------------------------
  Initialize Motor Driver (BTS7960) PWM using TPM1
  - RPWM: PTA13 -> TPM1_CH1 (Forward/Right)
  - LPWM: PTA12 -> TPM1_CH0 (Reverse/Left)
  - R_EN, L_EN: Connected to 3.3V (always enabled)
 *----------------------------------------------------------------------------*/
void Motor_Init(void) {
    // Enable clock to Port A and TPM1
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
    SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;

    // Select TPM clock source: MCGFLLCLK or MCGPLLCLK/2
    SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);

    // Configure PTA13 as TPM1_CH1 (RPWM - Forward)
    PORTA->PCR[MOTOR_RPWM_PIN] &= ~PORT_PCR_MUX_MASK;
    PORTA->PCR[MOTOR_RPWM_PIN] |= PORT_PCR_MUX(3);  // ALT3 = TPM1_CH1

    // Configure PTA12 as TPM1_CH0 (LPWM - Reverse)
    PORTA->PCR[MOTOR_LPWM_PIN] &= ~PORT_PCR_MUX_MASK;
    PORTA->PCR[MOTOR_LPWM_PIN] |= PORT_PCR_MUX(3);  // ALT3 = TPM1_CH0

    // Configure TPM1
    TPM1->SC = 0;                                    // Disable while configuring
    TPM1->MOD = 255;                                 // 8-bit resolution (0-255)
    TPM1->SC = TPM_SC_CMOD(1) | TPM_SC_PS(3);       // Counter mode, prescaler = 8

    // Configure TPM1_CH1 (RPWM - Forward) - Edge-aligned PWM, High-True pulses
    TPM1->CONTROLS[1].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
    TPM1->CONTROLS[1].CnV = 0;  // Start stopped

    // Configure TPM1_CH0 (LPWM - Reverse) - Edge-aligned PWM, High-True pulses
    TPM1->CONTROLS[0].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
    TPM1->CONTROLS[0].CnV = 0;  // Start stopped
}

/*----------------------------------------------------------------------------
  Stop motor
  Sets both RPWM and LPWM to 0
 *----------------------------------------------------------------------------*/
void Motor_Stop(void) {
    TPM1->CONTROLS[1].CnV = 0;  // RPWM = 0
    TPM1->CONTROLS[0].CnV = 0;  // LPWM = 0
}

/*----------------------------------------------------------------------------
  Motor forward rotation
  Parameters: speed (0-100)
    0 = stopped
    100 = maximum forward speed
 *----------------------------------------------------------------------------*/
void Motor_Forward(uint8_t speed) {
    // Limit speed to 0-100
    if (speed > 100) speed = 100;

    // Convert 0-100 to 0-255 PWM value
    uint8_t pwm_value = (speed * 255) / 100;

    // Set forward PWM, ensure reverse is 0
    TPM1->CONTROLS[1].CnV = pwm_value;  // RPWM = speed
    TPM1->CONTROLS[0].CnV = 0;          // LPWM = 0
}

/*----------------------------------------------------------------------------
  Motor reverse rotation
  Parameters: speed (0-100)
    0 = stopped
    100 = maximum reverse speed
 *----------------------------------------------------------------------------*/
void Motor_Reverse(uint8_t speed) {
    // Limit speed to 0-100
    if (speed > 100) speed = 100;

    // Convert 0-100 to 0-255 PWM value
    uint8_t pwm_value = (speed * 255) / 100;

    // Set reverse PWM, ensure forward is 0
    TPM1->CONTROLS[0].CnV = pwm_value;  // LPWM = speed
    TPM1->CONTROLS[1].CnV = 0;          // RPWM = 0
}
