/**
  ******************************************************************************
  * @file    main.c
  * @author  Kevin Huo
  * @version V1.0
  * @date    13-May-2018
  * @brief   Main function for science module.
  *
  *
  * This file serves as the driver for the science module. It receives data
  * from the CAN bus and generates an appropriate PWM signal for each module.
  * Additionally, it reads sensor data and transmits through CAN when the 
  * appropriate message is received.
  ******************************************************************************
*/
#include "mbed.h"
#include "ec.h"
#include "servo.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "canlib.h"
#include "pins.h"

#define FLAP_CLOSED 0.0
#define FLAP_OPEN 0.3

enum ScienceMotors
{
    ELEVATOR = 0,
    DRILL = 1,
    PROBE = 2,
    NUM_MOTORS
};

enum ScienceSensors
{
    EC_SENSOR = 0,
    TEMP_HUMID_SENSOR = 1,
    NUM_SENSORS
};

const uint32_t MOTOR_ID[NUM_MOTORS] = {420, 421, 422};
const uint32_t FLAP_ID = 423;
const uint32_t READ_SENSOR_ID = 510;
const uint32_t SENSOR_OUT_ID[NUM_SENSORS] = {500, 501};

volatile bool can_read_temp_sensor = false;
volatile bool can_read_ec_sensor = false;
volatile float pwm_duty[NUM_MOTORS];
volatile uint32_t flap_set;
volatile bool flap_changed = false;

float sen_temp;
float sen_humid;
float sen_ec;

volatile float elevator_pwm_ready = false; //need this because its a joystick

Serial pc(PC_10, PC_11);
Servo flap_servo(PB_13);
DigitalOut led(PC_0);

PwmOut* elevator_pwm_pin        = new PwmOut(PC_9); //bottom motor controller, pwm4
PwmOut* drill_pwm_pin           = new PwmOut(PC_7); //middle motor controller, pwm3
PwmOut* probe_pwm_pin           = new PwmOut(PB_15); //top motor controller, pwm2

DigitalOut* elevator_dir_pin    = new DigitalOut(PA_8);
DigitalOut* drill_dir_pin       = new DigitalOut(PC_8);
DigitalOut* probe_dir_pin       = new DigitalOut(PC_6);

static PwmOut* pwm_out_vector[NUM_MOTORS] = {
    elevator_pwm_pin,
    drill_pwm_pin,
    probe_pwm_pin};

static DigitalOut* digital_out_vector[NUM_MOTORS] = {
    elevator_dir_pin,
    drill_dir_pin,
    probe_dir_pin};
/*
 *  Controls for each instrument.
 *  Positive float is high and negative float is low.
 */
void control_modules(float pwm_duty, uint16_t module_id)
{
    pwm_out_vector[module_id]->write(fabs(pwm_duty));
    *digital_out_vector[module_id] = (pwm_duty <= 0);
}

/*
 *  Receives pwm data from CAN bus
 */
void CANLIB_Rx_OnMessageReceived(void)
{
    uint16_t can_id = CANLIB_Rx_GetSenderID();

    if(can_id == READ_SENSOR_ID) {
        uint32_t sensor_to_read = CANLIB_Rx_GetAsUint(CANLIB_INDEX_0);
        if(sensor_to_read == EC_SENSOR) can_read_ec_sensor = true;
        else if (sensor_to_read == TEMP_HUMID_SENSOR) can_read_temp_sensor = true;
    }
    if(can_id == MOTOR_ID[ELEVATOR]) {
        elevator_pwm_ready = true;
    }
    if(can_id == MOTOR_ID[ELEVATOR] || can_id == MOTOR_ID[DRILL] || can_id == MOTOR_ID[PROBE]) {
        pwm_duty[can_id - MOTOR_ID[0]] = CANLIB_Rx_GetAsFloat(CANLIB_INDEX_0);
    }
    if (can_id == FLAP_ID){ 
        flap_set = CANLIB_Rx_GetAsUint(CANLIB_INDEX_0);
        flap_changed = true;
    }
}

int main()
{
    led = 1;
    /*
     * Initialize CAN for pwm
     */
    if (CANLIB_Init(SENSOR_OUT_ID[0], CANLIB_LOOPBACK_OFF) != 0)
    {
        pc.printf("CAN Initialization Failed\r\n");
    }

    for(int i = 0; i < NUM_MOTORS; i++)
    {
        if (CANLIB_AddFilter(MOTOR_ID[i]) != 0)
        {
            pc.printf("CAN Add Filter Failed on Filter: %u \r\n", MOTOR_ID[i]);
        }
    }
    if (CANLIB_AddFilter(FLAP_ID) != 0)
    {
        pc.printf("CAN Add Filter Failed on Filter FLAP_ID\r\n");
    }
    if (CANLIB_AddFilter(READ_SENSOR_ID) != 0)
    {
        pc.printf("CAN Add Filter Failed on Filter READ_SENSOR_ID\r\n");
    }

    while(1)
    {
        if(elevator_pwm_ready)
        {
            control_modules(pwm_duty[ELEVATOR], ELEVATOR);
            elevator_pwm_ready = false;
        }

        //buttons will only send once
        control_modules(pwm_duty[DRILL], DRILL);
        control_modules(pwm_duty[PROBE], PROBE);

        if(flap_changed)
        {
            if(flap_set == 0)
            {
                flap_servo = FLAP_CLOSED;
            }
            else
            {
                flap_servo = FLAP_OPEN;
            }
            flap_changed = false;
        }

        if(can_read_ec_sensor)
        {
            CANLIB_ChangeID(SENSOR_OUT_ID[EC_SENSOR]);
            CANLIB_Tx_SetFloat(sen_ec, CANLIB_INDEX_0);
            CANLIB_Tx_SendData(CANLIB_DLC_FOUR_BYTES);
            can_read_ec_sensor = false;
        }
        if(can_read_temp_sensor)
        {
            // CAN msg to PC
            CANLIB_ChangeID(SENSOR_OUT_ID[TEMP_HUMID_SENSOR]);
            CANLIB_Tx_SetFloat(sen_temp, CANLIB_INDEX_0);
            CANLIB_Tx_SetFloat(sen_humid, CANLIB_INDEX_1);
            CANLIB_Tx_SendData(CANLIB_DLC_ALL_BYTES);
            can_read_temp_sensor = false;
        }
        wait_ms(50); //run at 20Hz
    }
}
#ifdef __cplusplus
}
#endif