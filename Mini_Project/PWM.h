#ifndef PWM_H_
#define PWM_H_

#include "MKL25Z4.h"
#include <stdint.h>

// RGB LED pins
// Red LED - PTB18 (TPM2_CH0)
// Green LED - PTB19 (TPM2_CH1)
// Blue LED - PTD1 (TPM0_CH1)

void PWM_Init(void);
void Set_Red_Brightness(uint8_t brightness);    // 0-255
void Set_Green_Brightness(uint8_t brightness);  // 0-255
void Set_Blue_Brightness(uint8_t brightness);   // 0-255
void Set_RGB(uint8_t r, uint8_t g, uint8_t b); // 0-255 for each

#endif /* PWM_H_ */
