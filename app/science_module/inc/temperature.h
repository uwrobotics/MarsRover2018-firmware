#ifndef TEMPERATURE_H
#define TEMPERATURE_H

typedef struct {
  float temperature;
  float humidity;
} temp_hum_reading_t;

bool get_temp_humidity_sensor_reading(temp_hum_reading_t* pReading);


#endif
