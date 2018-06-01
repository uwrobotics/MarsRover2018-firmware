#include "mbed.h"

DigitalOut led(PC_0);
Serial pc(PC_10, PC_11);

// main() runs in its own thread in the OS
int main() {
    int i = 0;
    while (true) {
        pc.printf("Hello World! %d\r\n", i);
        i++;
        wait(0.5);
        led = i % 2;
    }
}

