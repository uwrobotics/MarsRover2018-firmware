// Adapted from https://www.dfrobot.com/wiki/index.php/Analog_EC_Meter_SKU:DFR0300

#include "mbed.h"
#include "DS1820.h"
//#include "temphumditysensor"

#define RESISTOR_1_VALUE 300
#define RESISTOR_2_VALUE 300
#define VOLTAGE_DIVIDER (float)(RESISTOR_2_VALUE / (RESISTOR_1_VALUE + RESISTOR_2_VALUE))

AnalogIn ecSensor(A0); // EC Sensor Pin

// Returns the electrical conductivity in ms/cm
float getEC() { // TODO: Move numbers into constants
	float voltage = ecSensor.read() * 3300 / VOLTAGE_DIVIDER; //scale to 5V
    float temperature = 25.0; //read from other sensor
    
	float temperatureCoefficient = 1.0 + 0.0185 * (temperature - 25.0);
	float coefficientVoltage = voltage/temperatureCoefficient;
	if (coefficientVoltage < 150 || coefficientVoltage > 3300) {
		printf("The voltage is out of the sensor's accuracy range");
	} else {
		float electricalConductivity;
		if (coefficientVoltage <= 448) {
			electricalConductivity = 6.84 * coefficientVoltage - 64.32; //1ms/cm<EC<=3ms/cm
		} else if (coefficientVoltage <= 1457) {
			electricalConductivity = 6.98 * coefficientVoltage - 127; //3ms/cm<EC<=10ms/cm
		} else {
			electricalConductivity = 5.3 * coefficientVoltage + 2278; //10ms/cm<EC<20ms/cm
		}
		electricalConductivity /= 1000; //convert us/cm to ms/cm
		return electricalConductivity;
	}
	return 0;
}