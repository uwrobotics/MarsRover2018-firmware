/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  *
  *
  * This file exists to demonstrate basic use of the CAN library for Waterloo
  * Mars Rover. It's a very contrived and simple 'blink' example, basically.
  * This is meant to run in loopback mode (on one dev board).
  ******************************************************************************
*/
#include "mbed.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "canlib.h"
#include "pins.h"

DigitalOut led1(PC_0);
Serial pc(USBTX, USBRX);

int main()
{
    pc.printf("start listening\r\n");
    
    if (CANLIB_Init(20, 0) != 0)
    {
        pc.printf("init failed\r\n");
    }
    if (CANLIB_AddFilter(500) != 0)
    {
        pc.printf("add filter failed\r\n");
    }

int counter = 0;
    while(1)
    {
        CANLIB_ChangeID(300);
        CANLIB_Tx_SetInt(counter, CANLIB_INDEX_0);
        CANLIB_Tx_SendData(CANLIB_DLC_ALL_BYTES);
        
        CANLIB_ChangeID(301);
        CANLIB_Tx_SetInt(counter, CANLIB_INDEX_0);
        CANLIB_Tx_SendData(CANLIB_DLC_ALL_BYTES);
        
        CANLIB_ChangeID(302);
        CANLIB_Tx_SetInt(counter, CANLIB_INDEX_0);
        CANLIB_Tx_SendData(CANLIB_DLC_ALL_BYTES);
        wait(0.2);
        counter++;
    }
}

void CANLIB_Rx_OnMessageReceived(void)
{
    led1 = 1;
    pc.printf("got message %d\r\n", CANLIB_Rx_GetAsInt(CANLIB_INDEX_0));
}
#ifdef __cplusplus
}
#endif