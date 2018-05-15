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

#define NUM_MODULES 4

Serial pc(USBTX, USBRX);
I2C i2c(I2C_SDA, I2C_SCL);

enum module_ids{
    science_module_pwm_filter   = 400,
    elevator_pwm_filter_ID      = 420,
    drill_pwm_filter_ID         = 421,
    temp_pwm_filter_ID          = 422,
    flap_pwm_filter_ID          = 423,

    elevator_pwm_ID             = 424,
    drill_pwm_ID                = 425,
    temp_pwm_ID                 = 426,
    flap_pwm_ID                 = 427,
    
    temp_humidity_filter_ID     = 501,

    temp_humidity_sensor_read   = 600,
};

static bool can_write_pwm = false;
static bool can_read_temp_sensor = false;
static uint16_t can_id;
static float pwm_duty;

// TODO: fix the pin numbers; current ones are guesses
PwmOut* elevator_pwm_pin        = new PwmOut(PB_13);
PwmOut* drill_pwm_pin           = new PwmOut(PB_14);
PwmOut* temp_pwm_pin            = new PwmOut(PB_15);
PwmOut* flap_pwm_pin            = new PwmOut(PB_15);

DigitalOut* elevator_dir_pin    = new DigitalOut(PC_6);
DigitalOut* drill_dir_pin       = new DigitalOut(PC_7);
DigitalOut* temp_dir_pin        = new DigitalOut(PC_8);
DigitalOut* flap_dir_pin        = new DigitalOut(PC_9);

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

const char am2315_temp_humidity_sensor = 0xB8;

/*
 * Get temp/humidity sensor readings from i2c
 */
uint32_t get_temp_humidity_sensor_reading()
{
    char sensor_cmd[4] = {0};
    uint32_t *sensor_data;
    
    // Host to sensor read temp and humidity data
    sensor_cmd[0] = 0x03;
    sensor_cmd[1] = 0x00;
    sensor_cmd[2] = 0x04;
    i2c.write(am2315_temp_humidity_sensor, sensor_cmd, 3);

    // Tell sensor to prep sensor data
    sensor_cmd[0] = 0x03;
    sensor_cmd[1] = 0x04;
    i2c.write(am2315_temp_humidity_sensor, sensor_cmd, 2);

    wait(0.5);

    // Read data from sensor
    memset(sensor_cmd, 0, sizeof(sensor_cmd)/sizeof(sensor_cmd[0]));
    i2c.read(am2315_temp_humidity_sensor, sensor_cmd, 4);

    sensor_data = (uint32_t*) &sensor_cmd[0];
    return *sensor_data;
}

/*
 *  Receives pwm data from CAN bus
 */
void CANLIB_Rx_OnMessageReceived(void)
{
    can_id = CANLIB_Rx_GetSenderID();
    switch(can_id){
        case elevator_pwm_filter_ID:
        case drill_pwm_filter_ID:
        case temp_pwm_filter_ID:
        case flap_pwm_filter_ID:
            pwm_duty = CANLIB_Rx_GetAsFloat(CANLIB_INDEX_0);
            can_write_pwm = true;
            break;
        case temp_humidity_sensor_read:
            can_read_temp_sensor = true;
            break;
    }
}

int main()
{
    uint32_t temp_humidity_sensor_reading;
    float temp_reading;
    float humidity_reading;
    /*
     * Initialize CAN
     */
    for(int i = 0; i < NUM_MODULES; i++)
    {
        if (CANLIB_Init(elevator_pwm_ID + i, CANLIB_LOOPBACK_OFF) != 0)
        {
            pc.printf("CAN Initialization Failed on PWM ID: %u \r\n", elevator_pwm_ID + i);
        }
        if (CANLIB_AddFilter(elevator_pwm_filter_ID + i) != 0)
        {
            pc.printf("CAN Add Filter Failed on Filter: %u \r\n", elevator_pwm_filter_ID + i);
        } 
    }

    while(1)
    {
        if(can_write_pwm)
        {
            control_modules(pwm_duty, can_id % science_module_pwm_filter);
            can_write_pwm = false;
        }
        if(can_read_temp_sensor)
        {
            temp_humidity_sensor_reading = get_temp_humidity_sensor_reading();
            temp_reading = (float) (temp_humidity_sensor_reading & 0xFF);
            humidity_reading = (float) ((temp_humidity_sensor_reading >> 8) & 0xFF);

            // CAN msg to PC
            CANLIB_ChangeID(temp_humidity_filter_ID);
            CANLIB_Tx_SetFloat(temp_reading, CANLIB_INDEX_0);
            CANLIB_Tx_SetFloat(humidity_reading, CANLIB_INDEX_1);
            CANLIB_Tx_SendData(CANLIB_DLC_ALL_BYTES);
            can_read_temp_sensor = false;
        }
    }
}
#ifdef __cplusplus
}
#endif