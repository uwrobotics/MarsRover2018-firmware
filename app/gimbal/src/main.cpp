#include "mbed.h"

DigitalOut led1(LED1);
PwmOut yawActuator(PWM_OUT);

// main() runs in its own thread in the OS
int main() {
    yawActuator.period_ms(1);
    yawActuator.write(0.2);
    while (true) {
        led1 = !led1;
        wait(0.01);
    }
}

