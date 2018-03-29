/**
  ******************************************************************************
  * @file    main.c
  * @author  TomasBL, Alice, Banky, Jerry
  * @version V1.0
  * @date    17-Februrary-2018
  * @brief   Default main function.
  *
  *
  *This file consists of the general file for the relative encoder. 
  ******************************************************************************
**/

#include "mbed.h"
#include "QEI.h"
#include "PwmIn.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "canlib.h"
#include "pins.h"

enum ArmJoint
{
    TURNTABLE = 0,
    SHOULDER = 1,
    ELBOW = 2,
    WRIST_PITCH = 3,
    WRIST_ROLL = 4,
    NUM_MOTORS
};

Serial pc(USBTX, USBRX);
uint32_t enc_ID[NUM_MOTORS] = {300, 301, 302, 303, 304};
uint32_t motor_ID[NUM_MOTORS] = {400, 401, 402, 403, 404};

volatile uint32_t enc_data[NUM_MOTORS];

volatile float pwm_duty[NUM_MOTORS];
volatile bool data_ready[NUM_MOTORS] = {false};

const PinName pwm_pins[NUM_MOTORS] = {PB_13, PB_15, PC_7, PC_9, PA_9}; // PWM_6 is not included (PA_11)
const PinName dir_pins[NUM_MOTORS] = {PB_14, PC_6, PC_8, PA_8, PA_10}; // DIR_6 is not included (PA_12)

PwmOut *pwm_out[NUM_MOTORS];
DigitalOut *dir_out[NUM_MOTORS];

float freq = 50.0; // PWM Frequency [Hz]
float period = 1 / freq;
DigitalOut led1(PC_1);

//QEI library used to read from incremental encoders.
//To-Do : define pins A, B and I depending on hardware setup
QEI turntable_inc(A0, A1, A2, 48);
QEI wrist_pitch_inc(A0, A1, A2, 48);
PwmIn wrist_roll_abs(A4);
PwmIn shoulder_abs(A4);
PwmIn elbow_abs(A5);

Ticker tick;

/*
 *	Initialize pins
 */
void initPins()
{
    for (int i = 0; i < NUM_MOTORS; i++)
    {
        pwm_out[i] = new PwmOut(pwm_pins[i]);
        dir_out[i] = new DigitalOut(dir_pins[i]);

        pwm_out[i]->period(period);
    }
}


/*
 *  Controls the motor speed. Direction = 1 is clockwise
 */
void motorControl(float speed, PwmOut* pwm_pin, DigitalOut* dir_pin)
{   
    float duty = fabs(speed);
    int dir = (speed <= 0);

    if (duty <= 1.0f)
    {
        pwm_pin->write(duty);
    }
    dir_pin->write(dir);
}

void encoderSend()
{
    for (int i = 0; i < NUM_MOTORS; i++)
    {
        CANLIB_ChangeID(enc_ID[i]);
        CANLIB_Tx_SetUint(enc_data[i], CANLIB_INDEX_0);
        CANLIB_Tx_SendData(CANLIB_DLC_FOUR_BYTES);
    }
}

/*
 *  Receives pwm data from CAN bus
 */
void CANLIB_Rx_OnMessageReceived(void)
{
    uint16_t sender_ID = CANLIB_Rx_GetSenderID() - 400; //400 -> 0, 401 -> 1, etc.

    // Received PWM value will be between -1 and 1
    pwm_duty[sender_ID] = CANLIB_Rx_GetAsFloat(CANLIB_INDEX_0);
    data_ready[sender_ID] = true;
}

/*
 *  Handle any errors that occur
 */
void errorHandler()
{
}

int main()
{
    initPins();

    //send the encoder data through the CAN bus every half second
    tick.attach(&encoderSend,0.5);

    // Initialize with the first encoder value. Value updated before sending each time
    if (CANLIB_Init(enc_ID[0], CANLIB_LOOPBACK_OFF) != 0)
    {
        errorHandler();
    }

    for (int i = 0; i < NUM_MOTORS; i++)
    {
        if (CANLIB_AddFilter(motor_ID[i]) != 0)
        {
            errorHandler();
        }
    }

    while (true)
    {

        //Read Data from Encoders
        enc_data[TURNTABLE] = turntable_inc.getPulses();
        enc_data[WRIST_PITCH] = wrist_pitch_inc.getPulses();
        enc_data[WRIST_ROLL] = wrist_roll_abs.get12BitState();
        enc_data[SHOULDER] = shoulder_abs.get12BitState();
        enc_data[ELBOW] = elbow_abs.get12BitState();

        //if data is ready on any of the motors then we will send
        //WARNING: this will not automatically send 0 to the motors
        //         it relies on CAN bus to end zeros to the unused motors
        for (int i = 0; i < NUM_MOTORS; i++)
        {
            if (data_ready[i])
            {
                motorControl(pwm_duty[i], pwm_out[i], dir_out[i]);
            }
            data_ready[i] = false;
        }
    }
}

#ifdef __cplusplus
}
#endif