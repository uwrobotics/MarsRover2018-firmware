#include "mbed.h"
 
#define CAN_RX PB_8
#define CAN_TX PB_9

Serial pc(USBTX, USBRX);
DigitalOut led1(LED1);
CAN can1(CAN_RX, CAN_TX);
char counter = 5;
 
void send() {
    if(can1.write(CANMessage(0x20, &counter, 1))) {
        pc.printf("send\r\n");
    }
}
 
int main() {
    pc.printf("main()\r\n");
    //ticker.attach(&send, 1);
    CANMessage msg;
    can1.frequency(500000);
    while(1) {
        pc.printf("loop()\r\n");
        //if(can1.read(msg)) {
        //    pc.printf("Message received: %d\r\n", msg.data[0]);
        //    led1 = !led1;
        //}
        send(); 
        wait(0.2);
    }
}