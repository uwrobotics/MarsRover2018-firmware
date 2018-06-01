/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  *
  ******************************************************************************
*/
#include "mbed.h"
#include "QEI.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "canlib.h"

Serial pc(PC_10, PC_11);

PwmOut claw_pwm(PA_9);
DigitalOut claw_dir(PA_8);
QEI claw_enc(PB_0, PB_1, PB_2, 600);
DigitalOut led(PC_0);

Ticker tick;
const uint32_t MOTOR_ID = 405;
const uint32_t ENC_ID = 305;
const uint32_t LIM_SW_ID = 200;

//TODO: determine position of claw limit switch
const uint32_t CLAW_LIM_SW_POS = (1 << 10);

volatile uint32_t switches;
volatile uint32_t enc_data;
volatile float pwm_duty;
volatile bool data_ready = false;

void CANLIB_Rx_OnMessageReceived(void)
{
    uint16_t sender_id = CANLIB_Rx_GetSenderID();
    if(MOTOR_ID == sender_id) {
        pwm_duty = CANLIB_Rx_GetAsFloat(CANLIB_INDEX_0);
        data_ready = true;
    }
    else if(LIM_SW_ID == sender_id) {
        switches = CANLIB_Rx_GetAsUint(CANLIB_INDEX_0);
    }
}

void encoderSend()
{
    CANLIB_Tx_SetUint(enc_data, CANLIB_INDEX_0);
    CANLIB_Tx_SendData(CANLIB_DLC_FOUR_BYTES);
}

/*
 *  Controls the motor speed. Direction = 1 is clockwise
 */
void motorControl(float speed)
{   
   	float duty = fabs(speed);
    int set_dir = (speed <= 0);

    if (duty <= 1.0f)
    {
        claw_pwm.write(duty);
    }

    claw_dir.write(set_dir);
}

//reset claw to known position
void reset_claw()
{
    bool reset = false;
    while(!reset)
    {
        motorControl(0.2);
        if((CLAW_LIM_SW_POS & switches) == CLAW_LIM_SW_POS)
        {
            reset = true;
            motorControl(0);
        }
    }
    claw_enc.reset();
}


int main()
{    
    if (CANLIB_Init(ENC_ID, 0) != 0)
    {
        pc.printf("init failed\r\n");
    }
    if (CANLIB_AddFilter(MOTOR_ID) != 0)
    {
        pc.printf("add filter failed\r\n");
    }
    if (CANLIB_AddFilter(LIM_SW_ID) != 0)
    {
        pc.printf("add filter failed\r\n");
    }
    led = 1;
    //reset_claw();
    tick.attach(&encoderSend, 2.0);

    while(1)
    {
        enc_data = claw_enc.getPulses();
        if(data_ready) {
            motorControl(pwm_duty);
            data_ready = false;
        }
        wait_ms(50); // run at 20 Hz
    }
}

#ifdef __cplusplus
}
#endif