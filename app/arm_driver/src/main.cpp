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

#ifdef __cplusplus
}
#endif

//#define ABSOLUTE
#define INCREMENTAL

Serial pc(USBTX, USBRX);
uint32_t enc_ID[5] = {300, 301, 302, 303, 304};
uint32_t motor_ID[5] = {400, 401, 402, 403, 404};

uint32_t t_on, t_off;
uint16_t width, position;
float duty_cycle;
volatile uint16_t data[5];

volatile uint32_t pwm_duty[5];
volatile uint8_t data_ready = 0;

//QEI library used to read from incremental encoders.
//To-Do : define pins A, B and I depending on hardware setup 
QEI wheel_1 (A0, A1, A2, 48);
QEI wheel_2 (A0, A1, A2, 48);
QEI wheel_3 (A0, A1, A2, 48);

PwmIn abs_enc_1(A4);
PwmIn abs_enc_2(A5);


/*
 *  Generates a PWM signal with a duty cycle of pwm_duty/256
 */
void pwm_gen(uint32_t pwm_duty)
{
    float period = 1/freq;  
    pwm_pin.period(period);
    if (pwm_duty <= 255)
    {
        pwm_pin.write((float)pwm_duty/255.0);
    }
}

/*
 *  Controls the motor speed. Direction = 1 is clockwise
 */
void motor_control(int16_t speed)
{
    pwm_gen(abs(speed));
    dir_pin = (speed <= 0);
}

/*
 *  Receives pwm data from CAN bus
 */
void CANLIB_Rx_OnMessageReceived(void)
{

    switch(CANLIB_Rx_GetSenderID())
    {
        // Expect frame format to be:
        // Bytes 0-3: Azimuth axis PWM input
        // Byte 4-7: Inclination axis PWM input
        case motor_ID[0]:
            pwm_duty[0] = CANLIB_Rx_GetAsFloat(motor_ID[0]);
            data_ready = 1;
            break;


    pwm_duty = CANLIB_Rx_GetAsUint(CANLIB_INDEX_0);
}


int main(){

	if (CANLIB_Init(enc_ID, CANLIB_LOOPBACK_OFF) != 0){
		pc.printf("init failed \r\n");
	}

	if (CANLIB_AddFilter(motor_ID[0]) != 0)
    {
        Error_Handler();
	}

	uint8_t count;
	count =0;
	void reset();

	while (1){

		count ++;
		//Read Data from Encoder
		data[0] = wheel_1.getCurrentState();
		data[1] = wheel_1.getCurrentState();
		data[2] = wheel_1.getCurrentState();

		width = int(abs_enc_1.dutycycle*4098) -1;
		//width = ((t_on *4098)/(t_on + t_off)) -1;
		if(width<=4094)
			position = width;
		if (width == 4096)
			position = 4095;

		data[3] = position;

		width = int(abs_enc_2.dutycycle*4098) -1;
		//width = ((t_on *4098)/(t_on + t_off)) -1;
		if(width<=4094)
			position = width;
		if (width == 4096)
			position = 4095;

		data[4] = position;
		

		//Write data to CAN BUS
		if (count > 100){
			//pc.printf("Test data %d \n", data);
			for (int i =0; i< 5; i++){
				CANLIB_ChangeID(Encoder_ID[i]);
				CANLIB_Tx_SendData(data[i]);				
			}

			count = 0;
		}

		if (data_ready){
			pwm_gen();
		}

	}

}
/*
#ifdef __cplusplus
}
#endif

*/