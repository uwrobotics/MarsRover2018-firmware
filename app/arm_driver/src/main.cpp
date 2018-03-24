/**
  ******************************************************************************
  * @file    main.c
  * @author  TomasBL, Alice, Banky
  * @version V1.0
  * @date    17-Februrary-2018
  * @brief   Default main function.
  *
  *
  *This file consists of the general file for the relative encoder. 
  ******************************************************************************
**/


#include "mbed.h"
#include "encoder/QEI.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "canlib.h"
#include "pins.h"

//#define ABSOLUTE
#define INCREMENTAL
#define NUM_MOTORS 5

Serial pc(USBTX, USBRX);
uint32_t enc_ID[NUM_MOTORS] = {300, 301, 302, 303, 304};
uint32_t motor_ID[NUM_MOTORS] = {400, 401, 402, 403, 404};

uint32_t t_on, t_off;
uint16_t width, position;
float duty_cycle;
volatile uint16_t data[NUM_MOTORS];

volatile uint32_t pwm_duty[NUM_MOTORS];
volatile uint8_t data_ready = 0;

const PinName pwm_pins[NUM_MOTORS] = {PB_13, PB_15, PC_7, PC_9, PA_9}; // PWM_6 is not included (PA_11)
const PinName dir_pins[NUM_MOTORS] = {PB_14, PC_6, PC_8, PA_8, PA_10}; // DIR_6 is not included (PA_12)

PwmOut *pwm_out[NUM_MOTORS];
DigitalOut *dir_out[NUM_MOTORS];

float freq = 50.0; // PWM Frequency [Hz]
float period = 1/freq;
DigitalOut led1(PC_1);

//QEI library used to read from incremental encoders.
//To-Do : define pins A, B and I depending on hardware setup 
QEI wheel_1 (A0, A1, A2, 48);
QEI wheel_2 (A0, A1, A2, 48);
QEI wheel_3 (A0, A1, A2, 48);

// PwmIn abs_enc_1(A4);
// PwmIn abs_enc_2(A5);

/*
 *	Initialize pins
 */
void initPins() {
	for (int i = 0; i < NUM_MOTORS; i++) {
		pwm_out[i] = new PwmOut(pwm_pins[i]);
		dir_out[i] = new DigitalOut(dir_pins[i]);

		// Add Encoder Pin Initialization
	}
}

/*
 *  Generates a PWM signal with a duty cycle of pwm_duty/256
 */
void pwmGen(float pwm_duty, PwmOut pwm_pin)
{
    pwm_pin.period(period);
    if (pwm_duty <= 255)
    {
        pwm_pin.write((float)pwm_duty/255.0);
    }
}

/*
 *  Controls the motor speed. Direction = 1 is clockwise
 */
void motorControl(int16_t speed, PwmOut pwm_pin, DigitalOut dir_pin)
{
    pwmGen(abs(speed), pwm_pin);
    dir_pin = (speed <= 0);
}

/*
 *  Receives pwm data from CAN bus
 */
void CANLIB_Rx_OnMessageReceived(void)
{
	uint16_t sender_ID = CANLIB_Rx_GetSenderID();
    
	for (int i = 0; i < NUM_MOTORS; i++) {
		if (motor_ID[i] == sender_ID) {
			pwm_duty[i] = CANLIB_Rx_GetAsFloat(CANLIB_INDEX_0) * 255.0;
			data_ready = 1;
			led1 = 1;
			break;
		}
	}
}

/*
 *  Handle any errors that occur
 */
void errorHandler() {

}

int main(){
	initPins();

	// Initialize with the first encoder value. Value updated before sending each time
	if (CANLIB_Init(enc_ID[0], CANLIB_LOOPBACK_OFF) != 0) {
		errorHandler();
	}

	for (int i = 0; i < NUM_MOTORS; i++) {
		if (CANLIB_AddFilter(motor_ID[i]) != 0) {
			errorHandler();
		}
	}

	uint8_t count = 0;

	// void reset();

	while (true){

		count++;
		//Read Data from Encoder
		// data[0] = wheel_1.getCurrentState();
		// data[1] = wheel_1.getCurrentState();
		// data[2] = wheel_1.getCurrentState();

		// width = int(abs_enc_1.dutycycle*4098) -1;
		
		// //width = ((t_on *4098)/(t_on + t_off)) -1;
		// if(width <= 4094)
		// 	position = width;
		// if (width == 4096)
		// 	position = 4095;

		// data[3] = position;

		// width = int(abs_enc_2.dutycycle*4098) -1;
		// //width = ((t_on *4098)/(t_on + t_off)) -1;
		// if(width <= 4094)
		// 	position = width;
		// if (width == 4096)
		// 	position = 4095;

		// data[4] = position;
		
		//Write data to CAN BUS
		if (count > 100){
			for (int i =0; i< 5; i++){
				CANLIB_ChangeID(enc_ID[i]);
				// CANLIB_Tx_SendData(data[i]);				
			}

			count = 0;
		}

		if (data_ready) {
			for (int i = 0; i < NUM_MOTORS; i++) {
				motorControl(pwm_duty[i], *pwm_out[i], *dir_out[i]);
			}
			data_ready = 0;
		}
	}
}

#ifdef __cplusplus
}
#endif