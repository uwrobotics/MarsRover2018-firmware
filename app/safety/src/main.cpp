/**
  ******************************************************************************
  * @file    main.cpp
  * @author  Jerry
  * @version V1.0
  * @date    May 23, 2018
  * @brief   Safety board code to read from current sensors and close relays
  *
  *
  ******************************************************************************
*/
#include "mbed.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "canlib.h"
#include "pins.h"

#define NUM_CELLS 8

DigitalOut motor_relay(PA_11);
DigitalOut computer_relay(PA_9);
DigitalOut module_relay(PA_10);
DigitalOut* relays[] = {&motor_relay, &computer_relay, &module_relay};
Serial pc(USBTX, USBRX);
I2C i2c(PB_7, PB_6);

enum CurrentSensors{
    MOTOR = 0,
    COMPUTER = 1,
    MODULE = 2,
    NUM_SENSORS
};

const int SENSOR_ID[] = {100, 101, 102};
const int BMS_CELL_VOLTAGE_ID[] = {110, 111, 112, 113, 114, 115, 116, 117};

//convert to 8 bit addresses
const int MOTOR_ADDR = 0x52 << 1;
const int COMPUTER_ADDR = 0x55 << 1;
const int MODULE_ADDR = 0x5A << 1; 
const int CURRENT_SENSOR_ADDR[] = {MOTOR_ADDR, COMPUTER_ADDR, MODULE_ADDR};

const float MOTOR_CURRENT_THRESH = 100.0f;
const float COMPUTER_CURRENT_THRESH = 30.0f;
const float MODULE_CURRENT_THRESH = 30.0f;
const float CURRENT_THRESH[] = {MOTOR_CURRENT_THRESH, COMPUTER_CURRENT_THRESH, MODULE_CURRENT_THRESH};
int over_current_counts[NUM_SENSORS] = {0};

const float OVER_VOLTAGE_THRESH = 4.2f;
const float UNDER_VOLTAGE_THRESH = 1.5f;
int over_voltage_counts[NUM_CELLS] = {0};
int under_voltage_counts[NUM_CELLS] = {0};

float current[NUM_SENSORS];
float voltages[NUM_CELLS];
Ticker tick;

void sendCurrent()
{
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        CANLIB_ChangeID(SENSOR_ID[i]);
        CANLIB_Tx_SetFloat(current[i], CANLIB_INDEX_0);
        CANLIB_Tx_SendData(CANLIB_DLC_FOUR_BYTES);
    }
}

int main()
{
    pc.printf("start listening\r\n");
    
    if (CANLIB_Init(100, 0) != 0) {
        pc.printf("init failed\r\n");
    }
    for(int i = 0; i < NUM_CELLS; i++) {
        if (CANLIB_AddFilter(BMS_CELL_VOLTAGE_ID[i]) != 0) {
            pc.printf("add filter failed\r\n");
        }
    }

    motor_relay = 1;
    computer_relay = 1;
    module_relay = 1;
    tick.attach(&sendCurrent, 1.0);
    while(1)
    {
        char data[2];
        for(int i = 0; i < NUM_SENSORS; i++) {
            i2c.read(CURRENT_SENSOR_ADDR[i], data, 2);
            float raw_adc = (((data[0] & 0x0F) * 256) + (data[1] & 0xF0)) / 16.0 ;
            current[i] = float((245- raw_adc)/46.0);

            if(current[i] > CURRENT_THRESH[i]) {
                over_current_counts[i]++;
            } else {
                over_current_counts[i] = 0;
            }

            if(over_current_counts[i] > 20) {
                relays[i]->write(0);
            }
        }

        for(int i = 0; i < NUM_CELLS; i++){
            if(voltages[i] > OVER_VOLTAGE_THRESH){
                over_voltage_counts[i]++;
            } else {
                over_voltage_counts[i] = 0;
            }

            if(voltages[i] < UNDER_VOLTAGE_THRESH) {
                under_voltage_counts[i]++;
            } else{
                under_voltage_counts[i] = 0;
            }

            //CAN messages will arrive every second -> use 3 readings to determine with over or under
            if(over_voltage_counts[i] > 3*10) {
                relays[MOTOR]->write(0);
            }
            if(under_voltage_counts[i] > 3*10) {
                relays[MOTOR]->write(0);
            }
        }

        wait_ms(100); //run at 10Hz
    }
}

void CANLIB_Rx_OnMessageReceived(void)
{
    uint16_t sender_id = CANLIB_Rx_GetSenderID();
    voltages[sender_id - BMS_CELL_VOLTAGE_ID[0]] = CANLIB_Rx_GetAsFloat(CANLIB_INDEX_0);
}
#ifdef __cplusplus
}
#endif
