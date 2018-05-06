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

const unsigned int nod_ID_switches=200; 
volatile uint32_t switches_status;
const int number_of_swtiches=10;
volatile int switch_status[number_of_swtiches];
const int number_of_incremental_encoders = 2;
const int number_of_incremental_encoders_switches = 4;
int incremental_encoders_switches[number_of_incremental_encoders_switches] ={0,1,2,3};// format {Upper_limit(turntable), Lower_limit(turntable), Upper_limit(wrist_pitch), Lower_limit(wrist_pitch)}
int incremental_coders_pins_index[number_of_incremental_encoders] = {0,1};// format {turntable, wrist_pitch}
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
    uint16_t sender_ID = CANLIB_Rx_GetSenderID();
	
	if(sender_ID == nod_ID_switches){
		switches_status =  CANLIB_Rx_GetAsUint(CANLIB_INDEX_0);	
	}
	
	else{
		sender_ID -= 400; //400 -> 0, 401 -> 1, etc.

		// Received PWM value will be between -1 and 1
		//switch_status=CANLIB_Rx_GetAsUint(CANLIB_INDEX_1);
		pwm_duty[sender_ID] = CANLIB_Rx_GetAsFloat(CANLIB_INDEX_0);
		data_ready[sender_ID] = true;
	}
}
/*
 *  Handle any errors that occur
 */
void errorHandler()
{
}

void find_switch_status()
{
	for(int i=0; i < number_of_swtiches; i++)
	{
	   	switch_status[i]= (switches_status >> i) & 1;
	}
}

void intializer()
{
	for(int i=0; i < number_of_incremental_encoders; i++)
	{
		bool intializer_successful=false;
	    while(!intializer_successful)
		{
		    //move the encoder/motor;
		    motorControl(0.02,pwm_out[incremental_coders_pins_index[i]],dir_out[incremental_coders_pins_index[i]]);
  		    
			find_switch_status();
			
		    if (switch_status[incremental_encoders_switches[i+1]])
			{
				intializer_successful= true;
			    motorControl(0,pwm_out[incremental_coders_pins_index[i]],dir_out[incremental_coders_pins_index[i]]);
			}
		}
	}
	turntable_inc.reset();
	wrist_pitch_inc.reset();
}

//void incremental_encoders_motor_check(int index){}

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
	
	if (CANLIB_AddFilter(nod_ID_switches) != 0)
    {
        pc.printf("add filter failed for limit switches\r\n");
	}
	
	intializer();
	
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
				find_switch_status();
				
				if (switch_status[i*2]){ // top-limit
				    if(pwm_duty[i]>0)
					    motorControl(0, pwm_out[i], dir_out[i]);//basically stop.	
					else{
						motorControl(pwm_duty[i], pwm_out[i], dir_out[i]);
					}
				}
				if (switch_status[(i*2)+1]){ // lower-limit
					if(pwm_duty[i]<0)
					    motorControl(0, pwm_out[i], dir_out[i]);//basically stop.
 	                else{
						motorControl(pwm_duty[i], pwm_out[i], dir_out[i]);
					}					
				}
                else{
			        motorControl(pwm_duty[i], pwm_out[i], dir_out[i]);
			   }
            }
            data_ready[i] = false;
        }	
    }
}
#ifdef __cplusplus
}
#endif