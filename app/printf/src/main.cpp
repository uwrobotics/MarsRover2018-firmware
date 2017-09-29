#include "mbed.h"

Serial pc(USBTX, USBRX);

// main() runs in its own thread in the OS
int main() {
    int i = 0;
    while (true) {
        pc.printf("Hello World! %d\r\n", i);
        i++;
        wait(0.5);
    }
}

