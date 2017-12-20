#include "mbed.h"

I2C i2c(I2C_SDA, I2C_SCL);
DigitalOut led1(LED1);

//weird thing I noticed during testing
//you need to bitshift left once since this mbed uses 8 bit i2c addresses
const int i2c_addr = 0x08 << 1;
 
int main() {
    i2c.frequency(100000);
    char cmd[2];
    while (1) {
        led1 = !led1;
        cmd[0] = 0x01;
        cmd[1] = 0x00;
        i2c.write(i2c_addr, cmd, 2);
 
        wait(0.5);
 
        cmd[0] = 0x10;
        i2c.write(i2c_addr, cmd, 1);
    }
}

/* 
I tested this with the following arduino code acting as an I2C slave
#include <Wire.h>

void setup() {
  Wire.begin(0x08);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output
}

void loop() {
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  while (1 < Wire.available()) { // loop through all but the last
    char c = Wire.read(); // receive byte as a character
    Serial.print(c);         // print the character
  }
  int x = Wire.read();    // receive byte as an integer
  Serial.println(x);         // print the integer
}
*/