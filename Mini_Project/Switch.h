#ifndef SWITCH_H_
#define SWITCH_H_

#include <stdint.h>

// Switch pin definition
// PTB2 is connected to the potentiometer's SW pin
#define SWITCH_PIN    4

// Function prototypes
void Switch_Init(void);              // Initialize switch with interrupt
uint8_t Get_Stop_Status(void);       // Get current stop status (0=run, 1=stop)

#endif /* SWITCH_H_ */
