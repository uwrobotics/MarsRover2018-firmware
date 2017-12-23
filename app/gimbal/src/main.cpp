#include "mbed.h"
#include "pot.h"
#include "servo.h"

// DigitalOut led1(LED1);
int period = 20000;

Pot pot(A0);
Servo servo(PWM_OUT, period);
Serial pc(USBTX, USBRX);

void nonContinousTest()
{
    // servo.init();
    servo.setDegreePerUs(0.08);
    float angle = -60;
    float diffAngle = 5;
    while(1)
    {
        servo.writeDegrees(angle);
        pc.printf("Write %.3f", angle);
        
        angle += diffAngle;
        if(angle > 60)
        {
            angle = 60;
            diffAngle = -5;
        } else if (angle < -60)
        {
            angle = -60;
            diffAngle = 5;
        }
        wait_ms(20);
    }
}

void potReadTest()
{
    Pot pot(A0);
    int val;
    while(1)
    {
        val = pot.readPos();
        pc.printf("pot %04x\r\n", val);
        wait(0.01);
    }
}

// main() runs in its own thread in the OS
int main() 
{
    nonContinousTest();
}

