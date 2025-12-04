#include "MKL25Z4.h"
#include "PWM.h"
#include "ADC.h"
#include "Switch.h"
#include "Motor.h"
#include <stdlib.h>  // For abs()

// Deadzone for potentiometer center position
#define DEADZONE  10

int main(void) {
    // Initialize all modules
    PWM_Init();     // RGB LED control
    ADC_Init();     // Potentiometer input
    Switch_Init();  // Brake button
    Motor_Init();   // Motor driver

    // Turn off LED and motor initially
    Set_RGB(0, 0, 0);
    Motor_Stop();

    // Small startup delay
    for(volatile int i = 0; i < 500000; i++);

    while(1) {
        // Read potentiometer value (0-255)
        uint8_t adc_value = ADC_Read();

        // Check brake status
        if (Get_Stop_Status()) {

            Motor_Stop();
            Set_RGB(100, 0, 0);  // Bright RED = brake active
        }
        else {

            // Calculate offset from center (-128 to +127)
            int16_t offset = (int16_t)adc_value - 128;

            if (offset < -DEADZONE) {
                uint8_t speed = (abs(offset) - DEADZONE) * 100 / (128 - DEADZONE);

                // Control motor - reverse rotation
                Motor_Reverse(speed);

                // LED: RED with brightness = speed
                // Darker red = slower, brighter red = faster
                Set_RGB(speed, 0, 0);
            }
            else if (offset > DEADZONE) {

                uint8_t speed = (offset - DEADZONE) * 100 / (127 - DEADZONE);

                // Control motor - forward rotation
                Motor_Forward(speed);

                // LED: BLUE with brightness = speed
                // Darker blue = slower, brighter blue = faster
                Set_RGB(0, 0, speed);
            }
            else {

                Motor_Stop();

                // LED: Dim YELLOW (stopped indicator)
                Set_RGB(10, 10, 0);
            }
        }

        // Small delay for stability
        for(volatile int i = 0; i < 20000; i++);
    }

    return 0;
}
