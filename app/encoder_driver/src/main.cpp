/**
  ******************************************************************************
  * @file    main.c
  * @author  TomasBL
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
uint32_t enc_ID = 400;
uint32_t t_on, t_off;
uint16_t width, position;
float duty_cycle;
volatile uint8_t data;

//To-Do : define pins A, B and I depending on hardware setup 


QEI wheel (A0, A1, A2, 48);
//PwmIn abs_enc (A4);

int main(){



	if (CANLIB_Init(enc_ID, CANLIB_LOOPBACK_OFF) != 0){
		pc.printf("init failed \r\n");
	}
#ifdef INCREMENTAL
	//Reset the relative encoder. #TODO add other calibration

	uint8_t count;
	count =0;
	void reset();

	while (1){
		pc.printf("While loop is looping");
		//Read Data from Encoder
		position = wheel.getCurrentState();
		count ++;

		//Write data to CAN BUS
		if (count > 100){
			//pc.printf("Test data %d \n", data);
			CANLIB_Tx_SendData(data);
			count = 0;
		}

	}
#endif
#ifdef ABSOLUTE
	while (1){
		//To-Do calculate t_on from pwm output signal from encoder.
		//12 bit PWM
		//250Hz freq -> Period T = 0.4 ms
		width = int(abs_enc.dutycycle*4098) -1;
		//width = ((t_on *4098)/(t_on + t_off)) -1;
		if(width<=4094)
			position = width;
		if (width == 4096)
			position = 4095;

		if (count > 100){
			CANLIB_Tx_SendData(position);
			count =0;
		}
	}
#endif
}
/*
#ifdef __cplusplus
}
#endif

*/