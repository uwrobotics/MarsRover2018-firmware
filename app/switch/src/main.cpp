#include "mbed.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "canlib.h"
#include "pins.h"

#define SW_POS_1     0
#define SW_POS_2     1
#define SW_POS_3     2
#define SW_POS_4     3
#define SW_POS_5     4 
#define SW_POS_6     5
#define SW_POS_7     6
#define SW_POS_8     7
#define SW_POS_9     8
#define SW_POS_10    9

Serial pc(USBTX, USBRX);
const int Number_of_pins=3;
const int nod_ID=200;
const PinName pins_we_want[Number_of_pins]={PC_10, PC_11, PC_12};// Please put pins in array in increasing order basically the ones at the bottom comes first and the top one being last. 
DigitalOut led1(LED2);
static uint16_t switch_status=0; 
// MAX pins for using 1 byte is 32 pins so 0<Number_of_pins<=32;  
InterruptIn *SW [Number_of_pins];
void flip_SW1();void flip_SW2();void flip_SW3();//void flip_SW4();void flip_SW5();void flip_SW6();void flip_SW7();void flip_SW8();void flip_SW9();void flip_SW10();
void intial_status_finder(); 
void (*fun_pointer[Number_of_pins]) ()= {flip_SW1, flip_SW2, flip_SW3};//, flip_SW4, flip_SW5, flip_SW6, flip_SW7, flip_SW8, flip_SW9, flip_SW10};
uint16_t temp;

int main() {
  
   
     //TESTING
	 
    pc.printf("start listening\r\n");
    
    if (CANLIB_Init(nod_ID,CANLIB_LOOPBACK_ON) != 0)
    {
        pc.printf("init failed\r\n");
    }
     
    for(int i=0; i<Number_of_pins; i++){
	 
     	SW[i] = new InterruptIn(pins_we_want[i]);
		   
		SW[i]->rise(fun_pointer[i]); SW[i]->fall(fun_pointer[i]);
	}
   
    intial_status_finder();
	
    // TESTING
    if (CANLIB_AddFilter(nod_ID) != 0)
    {
        pc.printf("add filter failed\r\n");
    }
	
	while(1) {	
		if(temp!=switch_status){
		    CANLIB_Tx_SetInt(switch_status, CANLIB_INDEX_0);
            CANLIB_Tx_SendData(CANLIB_DLC_FIRST_BYTE);
		}
		temp=switch_status;
		wait_ms(100);
    }
 }

void flip_SW1() {	
	switch_status ^= (1<< SW_POS_1);
}

void flip_SW2() {
	switch_status ^= (1<< SW_POS_2);
}

void flip_SW3() {
    switch_status ^= (1<< SW_POS_3);	
}

/*void flip_SW4() {
    switch_status ^= (1<< SW_POS_4);	
}

void flip_SW5() {
    switch_status ^= (1<< SW_POS_5);	
}

void flip_SW6() {
    switch_status ^= (1<< SW_POS_6);	
}

void flip_SW7() {
    switch_status ^= (1<< SW_POS_7);	
}

void flip_SW8() {
    switch_status ^= (1<< SW_POS_8);	
}

void flip_SW9() {
    switch_status ^= (1<< SW_POS_9);	
}

void flip_SW10() {
    switch_status ^= (1<< SW_POS_10);	
}*/

void intial_status_finder(){
	for(int i=0; i<Number_of_pins; i++){
		
		switch_status |= (SW[i]->read() << i);   
   }
}

//TESTING
 void CANLIB_Rx_OnMessageReceived(void)
{
    led1 = 1;
	pc.printf("Switch number %d\r\n ", CANLIB_Rx_GetAsInt(CANLIB_INDEX_0));	
}
#ifdef __cplusplus
}
#endif


