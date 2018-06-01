#include "mbed.h"

DigitalOut led1(PC_0);

// main() runs in its own thread in the OS
int main() {
    while (true) {
        uint32_t timetowait = 1;
        led1 = !led1;
        wait(timetowait);
    }
}

