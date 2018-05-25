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


Serial pc(PC_10, PC_11);
const uint32_t ENC_ID[NUM_MOTORS] = {300, 301, 302, 303, 304};
const uint32_t MOTOR_ID[NUM_MOTORS] = {400, 401, 402, 403, 404};
const uint32_t LIM_SW_ID = 200;

volatile uint32_t enc_data[NUM_MOTORS];

volatile float pwm_duty[NUM_MOTORS];
volatile bool data_ready[NUM_MOTORS] = {false};

//TODO: Determine the PWM and DIR pins actually used
const PinName pwm_pins[NUM_MOTORS] = {PB_13, PB_15, PC_7, PC_9, PA_9}; // PWM_6 is not included (PA_11)
const PinName dir_pins[NUM_MOTORS] = {PB_14, PC_6, PC_8, PA_8, PA_10}; // DIR_6 is not included (PA_12)

PwmOut *pwm_out[NUM_MOTORS];
DigitalOut *dir_out[NUM_MOTORS];

float freq = 50.0; // PWM Frequency [Hz]
float period = 1 / freq;
DigitalOut led1(PC_1);
DigitalOut led2(PC_2);

//QEI library used to read from incremental encoders.
//To-Do : Determine the pins used for the encoders
//Pins and connectors correspond to Arm board rev_2
QEI turntable_inc(PB_0, PB_1, PB_2, 48); //Connector J7
QEI wrist_pitch_inc(PA_7, PC_4, PB_2, 48); // Connector J6
PwmIn wrist_roll_abs(PA_6); // Connector J5
PwmIn shoulder_abs(PA_3); // Connector J4
PwmIn elbow_abs(PA_2); // Connector J3

Ticker tick;

volatile uint32_t switches;

/*	Initialize pins
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
void motorControl(float speed, PwmOut* pwm, DigitalOut* dir)
{   
   	float duty = fabs(speed);
    int set_dir = (speed <= 0);

    if (duty <= 1.0f)
    {
        pwm->write(duty);
    }

    dir->write(set_dir);
}

void encoderSend()
{
    for (int i = 0; i < NUM_MOTORS; i++)
    {
        CANLIB_ChangeID(ENC_ID[i]);
        CANLIB_Tx_SetUint(enc_data[i], CANLIB_INDEX_0);
        CANLIB_Tx_SendData(CANLIB_DLC_FOUR_BYTES);
        //pc.printf("Encoder %d with data %d \n", ENC_ID[i], enc_data[i]);
        led1 = !led1;
    }
}

/*
 *  Receives pwm data from CAN bus
 */
void CANLIB_Rx_OnMessageReceived(void)
{
    uint16_t sender_ID = CANLIB_Rx_GetSenderID();
	//led2 = !led2;

    //limit switch CAN message
	if(sender_ID == LIM_SW_ID){
		switches =  CANLIB_Rx_GetAsUint(CANLIB_INDEX_0);

	}

	else{
		sender_ID -= 400; //400 -> 0, 401 -> 1, etc.

		// Received PWM value will be between -1 and 1
		//switch_status=CANLIB_Rx_GetAsUint(CANLIB_INDEX_1);
		pwm_duty[sender_ID] = CANLIB_Rx_GetAsFloat(CANLIB_INDEX_0);
		//pc.printf("PWM duty is %f from %d \n", pwm_duty[sender_ID], sender_ID); 
        data_ready[sender_ID] = true;
	}
}
/*
 *  Handle any errors that occur
 */
void errorHandler()
{
}

void reset_arm()
{
    for(int i = 0; i < 5; i++)
    {
        bool reset = false;
        while(!reset)
        {
            motorControl(0.1, pwm_out[i], dir_out[i]);
            if((switches << (2*i)) & 1) //top limit switch of each joint is pressed
            {
                reset = true;
                motorControl(0, pwm_out[i], dir_out[i]);
            }
        }
    }    

	turntable_inc.reset();
	wrist_pitch_inc.reset();
}

int main()
{
    initPins();

    //send the encoder data through the CAN bus every half second
    tick.attach(&encoderSend,0.5);

    // Initialize with the first encoder value. Value updated before sending each time
    if (CANLIB_Init(ENC_ID[0], CANLIB_LOOPBACK_OFF) != 0)
    {
        errorHandler();
    }

    for (int i = 0; i < NUM_MOTORS; i++)
    {
        if (CANLIB_AddFilter(MOTOR_ID[i]) != 0)
        {
            errorHandler();
        }
    }
	
	if (CANLIB_AddFilter(LIM_SW_ID) != 0)
    {
        pc.printf("add filter failed for limit switches\r\n");
	}
	
	//reset_arm();
	
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
        //         it relies on CAN bus to send zeros to the unused motors		
		for (int i = 0; i < NUM_MOTORS; i++)
        {
            if (data_ready[i])
            {
			    motorControl(pwm_duty[i], pwm_out[i], dir_out[i]);
            }
            data_ready[i] = false;
        }
        wait_ms(200); //run at 20Hz
    }
}
#ifdef __cplusplus
}
#endif