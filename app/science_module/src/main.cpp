/**
  ******************************************************************************
  * @file    main.c
  * @author  Kevin, Huo
  * @version V1.0
  * @date    01-Februrary-2018
  * @brief   Default main function.
  *
  *
  * This file serves as a a PWM driver for the science module. It receives data
  * from the CAN bus and generates an appropriate PWM signal for each module.
  ******************************************************************************
*/
#include "mbed.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "canlib.h"
#include "pins.h"

#define NUM_MODULES     4

Serial pc(USBTX, USBRX);

// TODO: make enum or something
static uint32_t elevator_filter_ID  = 420;
static uint32_t drill_filter_ID     = 421;
static uint32_t temp_filter_ID      = 422;
static uint32_t flap_filter_ID      = 423;

static uint32_t elevator_pwm_ID     = 520;
static uint32_t drill_pwm_ID        = 521;
static uint32_t temp_pwm_ID         = 522;  
static uint32_t flap_pwm_ID         = 523;

static bool can_msg_received = false;
static uint16_t can_id;
static float pwm_duty;

// TODO: fix the pin numbers; current ones are guesses
PwmOut* elevator_pwm_pin = new PwmOut(PB_13);
PwmOut* drill_pwm_pin = new PwmOut(PB_14);
PwmOut* temp_pwm_pin = new PwmOut(PB_15);
PwmOut* flap_pwm_pin = new PwmOut(PB_15);

DigitalOut* elevator_dir_pin = new DigitalOut(PC_6);
DigitalOut* drill_dir_pin = new DigitalOut(PC_7);
DigitalOut* temp_dir_pin = new DigitalOut(PC_8);
DigitalOut* flap_dir_pin = new DigitalOut(PC_9);

static PwmOut* pwm_out_vector[NUM_MODULES] = {elevator_pwm_pin,
                                                drill_pwm_pin,
                                                temp_pwm_pin,
                                                flap_pwm_pin};

static DigitalOut* digital_out_vector[NUM_MODULES] = {elevator_dir_pin,
                                                drill_dir_pin,
                                                temp_dir_pin,
                                                flap_dir_pin};

/*
 *  Controls for each instrument.
 *  Positive float is high and negative float is low.
 */
void control_modules(float pwm_duty, uint16_t module_id)
{
    pwm_out_vector[module_id]->write(fabs(pwm_duty));
    *digital_out_vector[module_id] = (pwm_duty >= 0);
}

/*
 *  Receives pwm data from CAN bus
 */
void CANLIB_Rx_OnMessageReceived(void)
{
    pwm_duty = CANLIB_Rx_GetAsFloat(CANLIB_INDEX_0);
    can_id = CANLIB_Rx_GetSenderID();
    can_msg_received = true;
}

int main()
{
    /*
     * Initialize CAN
     */
    for(int i = 0; i < NUM_MODULES; i++)
    {
        if (CANLIB_Init(elevator_pwm_ID + i, CANLIB_LOOPBACK_OFF) != 0)
        {
            pc.printf("CAN Initialization Failed on PWM ID: %u \r\n", elevator_pwm_ID + i);
        }
        if (CANLIB_AddFilter(elevator_filter_ID + i) != 0)
        {
            pc.printf("CAN Add Filter Failed on Filter: %u \r\n", elevator_filter_ID + i);
        } 
    }

    while(1)
    {
        if(can_msg_received)
        {
            can_msg_received = false;
            control_modules(pwm_duty, can_id % 400);
        }
    }
}
#ifdef __cplusplus
}
#endif