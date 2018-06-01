
#include "mbed.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "pins.h"
#include "temperature.h"


I2C i2c(I2C_SDA, I2C_SCL);
Serial pc1(USBTX, USBRX);
static const char am2315_temp_humidity_sensor = 0xB8;


/*
 * Get temp/humidity sensor readings from i2c
 */
bool get_temp_humidity_sensor_reading(temp_hum_reading_t* pReading)
{
	if (!pReading)
	{
		return false;
	}
    char sensor_cmd[4] = {0};
    char sensor_reply[8] = {0};
    
    // Host to sensor read temp and humidity data
    sensor_cmd[0] = 0x03;
    sensor_cmd[1] = 0x00;
    sensor_cmd[2] = 0x04;

    //wake up sensor
    i2c.write(am2315_temp_humidity_sensor, sensor_cmd, 1);
    wait_ms(2);

    // actual transmission
    i2c.write(am2315_temp_humidity_sensor, sensor_cmd, 3);

    wait_ms(10);

    // Tell sensor to prep sensor data
    //sensor_cmd[0] = 0x03;
    //sensor_cmd[1] = 0x04;
    //i2c.write(am2315_temp_humidity_sensor, sensor_cmd, 2);

    //wait(0.5);

    // Read data from sensor
    memset(sensor_reply, 0, sizeof(sensor_reply)/sizeof(sensor_reply[0]));
    i2c.read(am2315_temp_humidity_sensor, sensor_reply, 8);


// pc1.printf("received: ");
// for (int i = 0; i < 8; i++)
// {
// 	pc1.printf("%d, ", sensor_reply[i]);
// }
// pc1.printf("\n\r");



	float temp, humidity;
	if (sensor_reply[0] != 3) 
	{
		return false;
	}
  	if (sensor_reply[1] != 4) {
  		return false; // bytes req'd
  	}

  	humidity = sensor_reply[2];
  	humidity *= 256;
  	humidity += sensor_reply[3];
  	humidity /= 10;

  	temp = sensor_reply[4] & 0x7F;
  	temp *= 256;
  	temp += sensor_reply[5];
  	temp /= 10;


	// change sign
	if (sensor_reply[4] >> 7) {
		temp = -temp;
	}


    pReading->temperature = temp;
    pReading->humidity = humidity;
    return true;
}


#ifdef __cplusplus
}
#endif