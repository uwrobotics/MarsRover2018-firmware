#include "mbed.h"
#include "pot.h"

// DigitalOut led1(LED1);
Pot pot(A0);
PwmOut yawActuator(PWM_OUT);
Serial pc(USBTX, USBRX);

void servoMoveTest()
{
    int period = 20000;
    float dutyCycle = 0.28;
    float ms;
    yawActuator.period_us(period);
    yawActuator.write(dutyCycle);
    while(1)
    {
        ms = (period*1000)*dutyCycle;
       
        // dutyCycle -= 0.3-0.01;

        pc.printf("Write %.3f\r\n", ms);
        wait_ms(10);
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
void inputResponse()
{
    pc.printf("Read data\r\n");
}
// main() runs in its own thread in the OS
int main() 
{
    servoMoveTest();
}

