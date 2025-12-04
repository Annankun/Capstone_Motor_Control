#ifndef MOTOR_H_
#define MOTOR_H_

#include <stdint.h>

// Motor driver pin definitions (BTS7960)
// RPWM - PTA13 (TPM1_CH1) - Forward
// LPWM - PTA12 (TPM1_CH0) - Reverse
// R_EN, L_EN - Connected to 3.3V (always enabled)

#define MOTOR_RPWM_PIN      13          // PTA13 - TPM1_CH1 - Forward
#define MOTOR_LPWM_PIN      12          // PTA12 - TPM1_CH0 - Reverse

// Motor control parameters
#define MOTOR_DEADZONE      10          // Potentiometer center deadzone

// Function prototypes
void Motor_Init(void);                  // Initialize motor PWM (TPM1)
void Motor_Stop(void);                  // Stop motor (both PWM = 0)
void Motor_Forward(uint8_t speed);      // Forward rotation (0-100)
void Motor_Reverse(uint8_t speed);      // Reverse rotation (0-100)

#endif /* MOTOR_H_ */
