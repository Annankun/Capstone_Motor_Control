#ifndef LEDS_H
#define LEDS_H

#include <stdint.h>

// Freedom KL25Z LEDs
#define RED_LED_POS (18)     // on port B
#define GREEN_LED_POS (19)   // on port B
#define BLUE_LED_POS (1)     // on port D

// PWM control functions (recommended for smooth color control)
void Init_RGB_LEDs_PWM(void);
void Control_RGB_LEDs_Brightness(uint8_t red, uint8_t green, uint8_t blue);

// GPIO control functions (legacy - ON/OFF only)
void Init_RGB_LEDs(void);
void Control_RGB_LEDs(unsigned int red_on, unsigned int green_on, unsigned int blue_on);

#endif
