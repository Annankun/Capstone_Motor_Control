#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"

#define MASK(n) (1u << (n))

// Global variable for stop switch
volatile static uint8_t stop = 0;

/**  INITIALIZATION FUNCTIONS ----------------------------------------------------------------------------------------- **/

void init_ports(void){
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK; 	// Motor PWM,
	SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK | SIM_SCGC6_TPM2_MASK;							// For RGB LED
	SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;													// For motor
	SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;													// For analog input from potentiometer.
}

// Necessary for timers
void init_clock_source(void){
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);
}

void init_RGB_LED(void){
	// Initialize on-board RGB LEDs. Set as output and set timer channel.
	// Red - B18, Green - B19, Blue - D1
	PORTB->PCR[18] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[18] |= PORT_PCR_MUX(3);			// Alternate function 3: TPM0_CH0

	PORTB->PCR[19] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[19] |= PORT_PCR_MUX(3);  		// Alternate function 3: TMP0_CH1

	PORTD->PCR[1] &= ~PORT_PCR_MUX_MASK;
	PORTD->PCR[1] |= PORT_PCR_MUX(4);			// Alternate function 4: TMP0_CH2

	// Set on-board RGB pins as output
	PTB->PDDR |= MASK(18) | MASK(19);
	PTD->PDDR |= MASK(1);

	// TMP2 used for red and green
	TPM2->MOD = 255;							// Set 8-bit resolution (2^8-1)
	TPM2->SC = TPM_SC_CMOD(1) | TPM_SC_PS(3); 	// Set count mode to true, prescaler of 8 (PS = 3 --> 2^3)

	// TMP0 used for blue (since directly wired)
	TPM0->MOD = 255;							// Set 8-bit resolution
	TPM0->SC = TPM_SC_CMOD(1) | TPM_SC_PS(3);	// Set count mode to true, prescaler of 8 (PS = 3 --> 2^3)

	// Set edge-aligned, high-true
	TPM2->CONTROLS[0].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
	TPM2->CONTROLS[1].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
	TPM0->CONTROLS[1].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;

	TPM2->CONTROLS[0].CnV = 255;				// Set all outputs low
	TPM2->CONTROLS[1].CnV = 255;
	TPM0->CONTROLS[1].CnV = 255;
}

void init_pot(void){
	// Initialize pot GPIO
	PORTB->PCR[0] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[0] |= PORT_PCR_MUX(0);  // Analog mode

	// Configure ADC0
	// Use default clock (bus clock), 8-bit resolution, single-ended mode
	ADC0->CFG1 = ADC_CFG1_MODE(0) |     	// 8-bit mode
				 ADC_CFG1_ADICLK(0);    	// Bus clock

	ADC0->SC1[0] = ADC_SC1_ADCH(31);    	// Disable module initially. 31 disables conversion.
}

void init_switch(void){
	PORTD->PCR[4] &= ~PORT_PCR_MUX_MASK;	// Initialize the motor stop switch GPIO. B2
	PORTD->PCR[4] |= PORT_PCR_MUX(1);    	// GPIO mode
	PORTD->PCR[4] |= PORT_PCR_PE_MASK;   	// Enable internal pull-up/down resistor
	PORTD->PCR[4] |= PORT_PCR_PS_MASK;   	// Set internal resistor as pull-up
	PORTD->PCR[4] |= PORT_PCR_IRQC(0x0a); 	// Interrupt on falling edge (0b1010 = 0x0a)
	PTD->PDDR &= ~MASK(4);					// Toggle pin 2. Setting to 0 initializes it as input pin.
	NVIC_SetPriority(PORTD_IRQn, 128); 		// 0, 64, 128 or 192
	NVIC_ClearPendingIRQ(PORTD_IRQn);  		// Clear interrupts.
	NVIC_EnableIRQ(PORTD_IRQn); 			// Enable interrupts

	stop = 0; // Set stop again in case
}

void init_motor(void){
	PORTA->PCR[13] &= ~PORT_PCR_MUX_MASK;
	PORTA->PCR[13] = PORT_PCR_MUX(3); 			// Alternate function 3: TPM1_CH1
	PORTA->PCR[12] &= ~PORT_PCR_MUX_MASK;
	PORTA->PCR[12] = PORT_PCR_MUX(3); 			// Alternate function 3: TPM1_CH0

	// F_pwm = 24,000,000 / [8 * (255 + 1)] = 24M / 2048 = 11,718.75 Hz
	TPM1->SC = 0;								// Disable during configuration
	TPM1->MOD = 255;							// Set 8-bit resolution
	TPM1->SC = TPM_SC_CMOD(1) | TPM_SC_PS(3); 	// Set count mode CMOD and prescaler of 2^3 = 8

	TPM1->CONTROLS[1].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
	TPM1->CONTROLS[0].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;

	TPM1->CONTROLS[1].CnV = 0;				// Set all outputs low
	TPM1->CONTROLS[0].CnV = 0;
}

/**  IRQ HANDLERS -------------------------------------------------------------------------------------------------- **/
void PORTD_IRQHandler(void) {
	NVIC_ClearPendingIRQ(PORTD_IRQn);				// Clear pending interrupts

	if ((PORTD->ISFR & MASK(4))) {					// Check if flag for pin 6 is set
		for(volatile int i = 0; i < 10000; i++); 	// Debounce delay
		if (!(PTD->PDIR & MASK(4))) {		// Check if still pressed
			stop = !stop;							// Set stop state for motor
		}
	}
	PORTD->ISFR = 0xffffffff; 						// Clear all flags (W1TC - write 1 to clear)
}

/**  I/O CONTROL FUNCTIONS ----------------------------------------------------------------------------------------- **/

void control_RGB_brightness(unsigned int r, unsigned int g, unsigned int b) {
	TPM2->CONTROLS[0].CnV = 255 - r; 		// On-board LED is active low. Set control values.
	TPM2->CONTROLS[1].CnV = 255 - g;
	TPM0->CONTROLS[1].CnV = 255 - b;
}

void set_RGB_color(uint8_t adc_input){
	if (adc_input <= 100){
		control_RGB_brightness(0,(100-adc_input) * 255 / 100,(100-adc_input) * 255 / 100);
	}
	// If input is from 155-255, set duty cycle as (input - 155).
	else if (adc_input >= 155){
		control_RGB_brightness(0,0,(adc_input - 155) * 255 / 100);
	}
	// Stopped by pot --> set half bright white.
	else{
		control_RGB_brightness(128,128,128);
	}

}

void control_motor_speed(uint8_t adc_input){
	// ADC gives 0-255
	// If input is from 1-100, set duty cycle as input.
	if (adc_input <= 100){
		TPM1->CONTROLS[1].CnV = (100-adc_input) * 255 / 100;
		TPM1->CONTROLS[0].CnV = 0;				// Disable one direction
	}
	// If input is from 155-255, set duty cycle as (input - 155).
	else if (adc_input >= 155){
		TPM1->CONTROLS[1].CnV = 0;
		TPM1->CONTROLS[0].CnV = (adc_input - 155) * 255 / 100;
	}
	// Otherwise, set as low. Safe default.
	else{
		TPM1->CONTROLS[1].CnV = 0;				// Set all outputs low
		TPM1->CONTROLS[0].CnV = 0;
	}
}

static uint8_t ADC_Read(void){
    // Start conversion on specified channel
    ADC0->SC1[0] = ADC_SC1_ADCH(8);

    // Wait for conversion complete
    while (!(ADC0->SC1[0] & ADC_SC1_COCO_MASK));

    // Return result
    return (uint8_t)ADC0->R[0];
}

/**  MAIN FUNCTION  **/

int main(void) {
	init_ports();					// Enable all ports being used
	init_clock_source();			// Initialize clock source
	init_RGB_LED();					// Initialize on-board RGB LED with PWM
	init_pot();						// Initialize potentiometer analog input with ADC0
	init_switch();					// Initialize switch GPIO and interrupt
	init_motor();					// Initialize motor GPIO and PWM

    while(1) {
        if(stop){
        	control_motor_speed(127);			// Set in middle. Stopped
        	control_RGB_brightness(25,0,0);				// Set LED as red
        }
        else {
        	uint8_t adc_read = ADC_Read();
        	control_motor_speed(adc_read);
        	set_RGB_color(adc_read);
        }

        // Small delay
        for(volatile int i = 20000; i > 0; i--) ;
    }
    return 0 ;
}
