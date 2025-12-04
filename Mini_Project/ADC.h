#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>

void ADC_Init(void);        // Initialize ADC for potentiometer
uint8_t ADC_Read(void);     // Read potentiometer value (0-255)

#endif /* ADC_H_ */
