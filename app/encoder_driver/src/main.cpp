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
volatile uint8_t data;
//To-Do : define pins A, B and I depending on hardware setup 


QEI wheel (A0, A1, A2, 48);

int main(){

	if (CANLIB_Init(enc_ID, CANLIB_LOOPBACK_OFF) != 0){
		pc.printf("init failed \r\n");
	}
#ifdef INCREMENTAL
	//Reset the relative encoder. #TODO add other calibration
	void reset();

	while (1){
		//Read Data from Encoder
		data = wheel.getCurrentState();

		//Write data to CAN BUS
		CANLIB_Tx_SendData(data);
	}
#endif
#ifdef ABSOLUTE
	while (1){
		//To-Do calculate t_on from pwm output signal from encoder.
		//12 bit PWM
		width = ((t_on *4098)/(t_on + t_off)) -1;
		if(width<=4094)
			position = width;
		if (width == 4096)
			position = 4095;

		CANLIB_Tx_SendData(position);
	}
#endif
}
/*
#ifdef __cplusplus
}
#endif

*/