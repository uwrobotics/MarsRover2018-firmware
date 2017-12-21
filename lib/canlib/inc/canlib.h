/*
 * canlib.h
 *
 *  Created on: Jul 6, 2016
 *      Initial author: ryan
 *
 *
 *  CAN TRANSMISSION DATA ARRAY VISUALIZATION:
 *
 *  |Byte  1 |Byte  2 |Byte  3 |Byte  4 |||Byte  5 |Byte  6 |Byte  7 |Byte  8 |
 *  |        |        |        |        |||        |        |        |        |
 *  |01001100|10101010|00100110|00000101|||11101010|00000000|11111111|10101010|
 *  |________|________|________|________|||________|________|________|________|
 *
 *  ^--------ZEROTH 32-bit index-------^  ^--------FIRST 32-bit index---------^
 *
 *  Using CANLIB_Tx_SendData() will send these bytes:
 *
 *  CANLIB_DLC_FIRST_BYTE: Byte 1
 *  CANLIB_DLC_TWO_BYTES: Byte 1 and Byte 2
 *  CANLIB_DLC_THREE_BYTES: Byte 1 and Byte 2 and Byte 3
 *  .
 *  .
 *  .
 *  CANLIB_DLC_ALL_BYTES: Byte 1 and Byte 2 and Byte 3 and Byte 4 and
 *                         Byte 5 and Byte 6 and Byte 7 and Byte 8
 *
 *
 *
 *
 *  CANLIB USAGE:
 *
 *  1) Initialize HAL, configure clock and other peripherals as per usual.
 *      Do not bother with initializing any GPIOs or anything relating to CAN
 *  2) Call CANLIB_Init() just once with a node ID as a parameter (this can be changed)
 *      The other paramater, isLoopbackOn, should be set to CANLIB_LOOPBACK_ON only
 *      if testing the CAN network functionality with 1 board, in which case the CAN frames
 *      transmitted by this board will be received by the same board instead of being broadcast
 *      on the network. Otherwise, set this parameter to CAN_LOOPBACK_OFF
 *  3) Add Rx filters to the CAN system by calling CANLIB_AddFilter()
 *      This takes the node ID of the node you want to receive messages from
 *      This should be called only once per different node ID parameter
 *  4) Then, transmit and receive messages at your will!
 *
 *  TRANSMITTING MESSAGES:
 *  Messages can be transmitted in effectively 2 different patterns, so take your pick:
 *
 *  1) If you already have a byte array, use CANLIB_SendBytes() to send that byte array
 *      in one line of code.
 *      This is easy and efficient.
 *      The side effect of this is that the CAN ID of the node will be changed to the
 *       value provided.
 *
 *  2) If you want to send an int, uint, etc..., set the ID, data, and send the data
 *      yourself with CANLIB_ChangeID(), CANLIB_Tx_SetUint() [the Tx set function for
 *      32-bit or less uint data, for example, and CANLIB_Tx_SendData().
 *
 *      - CANLIB_ChangeID() need not be called, if you want to send a CAN frame with
 *         the same ID as set by the last call to this function, or an ID setting function
 *      - Using the appropriate Tx set function for your data, followed by CANLIB_Tx_SendData()
 *         is enough to send the data. Ensure that the DLC parameter supplied to the latter
 *         function is sufficient to send all of the data!
 *
 *  RECEIVING MESSAGES:
 *  Within YOUR OWN CODE, implement CANLIB_Rx_OnMessageReceived().
 *  Within CANLIB_Rx_OnMessageReceived(), all Rx functions can be used. Therefore, a possible
 *  pattern to follow to handle CAN messages is:
 *
 *  - Switch on the ID of the sender (which can be determined by calling CANLIB_Rx_GetSenderID() )
 *     Ideally, this will tell you what the type of the message is
 *  - Get the DLC of the message, if dealing with an incoming message of a byte array of unknown size, for example
 *     This can be done by calling uint8_t CANLIB_Rx_GetDLC()
 *  - Get the message using the appropriate Rx get function, based on the message type
 *     If preferred, copy the value of these variables into global variables so that they may be
 *     accessed outside of this function
 *
 *
 *  ADDITIONAL NOTES:
 *  - If you would like to clear the Tx byte array, CANLIB_ClearDataArray() will set all 8 bytes to 0.
 *  - To use method 2) of transmitting messages, if you are only sending one integer, for example, ensure
 *     that your call to CANLIB_Tx_SetInt() indicates the zeroth index as the index parameter (CANLIB_INDEX_0).
 *     This is because CANLIB_Tx_SendData(uint8_t dlc) sends data with length of the dlc starting from the LSB of the
 *     data array.
 *
 *     If you would like to send 2 32-bit integers at once, however, assign one int to the zeroth, and the
 *     other to the first index. Then calling CANLIB_Tx_SendData(CANLIB_DLC_ALL_BYTES) will send all 8 bytes, or the
 *     entirety of 2 integers.
 *  - CAN IRQ priority is set to 0. Do not set tick priority to 0 or CAN interrupts will probably not be handled
 *
 */

#ifndef CANLIB_H_
#define CANLIB_H_

#include "stm32f0xx.h"


// CAN Configuration Parameters - Don't need to modify
#define CAN_GPIO_MODE               GPIO_MODE_AF_PP
#define CAN_GPIO_PULL               GPIO_NOPULL
#define CAN_GPIO_SPEED              GPIO_SPEED_LOW
#define CAN_GPIO_ALTERNATE          GPIO_AF4_CAN
#define CAN_PORT                    CAN1
// See timing_calculations.txt for calculations
#define CAN_INIT_PRESCALER          6 // Set for 500 kbps //6
#define CAN_INIT_MODE               CAN_MODE_NORMAL
#define CAN_INIT_SJW                CAN_SJW_1TQ
#define CAN_INIT_BS1                CAN_BS1_12TQ
#define CAN_INIT_BS2                CAN_BS2_3TQ
#define CAN_INIT_TTCM               DISABLE
#define CAN_INIT_ABOM               DISABLE
#define CAN_INIT_AWUM               DISABLE
#define CAN_INIT_NART               DISABLE
#define CAN_INIT_RFLM               DISABLE
#define CAN_INIT_TXFP               DISABLE
#define CAN_INIT_FIFO               CAN_FIFO0
#define CAN_LOCK                    HAL_UNLOCKED
#define CAN_IDE_TYPE                CAN_ID_STD
#define CAN_RTR_TYPE                CAN_RTR_DATA


// CAN Data Length Parameters
#define CANLIB_DLC_FIRST_BYTE       1
#define CANLIB_DLC_TWO_BYTES        2
#define CANLIB_DLC_THREE_BYTES      3
#define CANLIB_DLC_FOUR_BYTES       4
#define CANLIB_DLC_FIVE_BYTES       5
#define CANLIB_DLC_SIX_BYTES        6
#define CANLIB_DLC_SEVEN_BYTES      7
#define CANLIB_DLC_ALL_BYTES        8

#define CANLIB_INDEX_0              0
#define CANLIB_INDEX_1              1

#define CANLIB_FIRST_WORD_OFFSET    0
#define CANLIB_SECOND_WORD_OFFSET   4

// CAN Loopback Parameters
#define CANLIB_LOOPBACK_OFF         0
#define CANLIB_LOOPBACK_ON          1

//
//      Data Structures
//
/* Union structures used for encoding and decoding various types and byte arrays
 * The code is compiled for little endian processors so this is fine
 */
typedef union
{
    uint8_t byte_array[8];
    int64_t long_integer;
    uint64_t long_uinteger;
    double dub;
    int32_t integer;
    uint32_t uinteger;
    float floatingpt;
} encoding_union;

typedef struct
{
    uint8_t DLC;
    uint32_t transmitter_ID;
    union
    {
        uint64_t long_uint;
        int64_t long_int;
        double dub;
        uint8_t whole_byte_array[8];
        struct
        {
            union
            {
                uint8_t byte_array_1[4];
                int32_t return_int_1;
                uint32_t return_uint_1;
                float return_float_1;
                char return_chars_1[4];
            };
            union
            {
                uint8_t byte_array_2[4];
                int32_t return_int_2;
                uint32_t return_uint_2;
                float return_float_2;
                char return_chars_2[4];
            };
        };
    };
} return_struct;

typedef enum {

    INDEX_0 = 0,
    INDEX_1 = 1

} CANLIB_INDEX;

//
//      Initialization and Helper Functions
//
int CANLIB_Init(uint32_t node_ID, uint8_t isLoopbackOn);
void CANLIB_ChangeID(uint32_t node_ID);
int CANLIB_AddFilter(uint16_t id);

//
//      Tx Message Encoding Functions
//
void CANLIB_ClearDataArray();
void CANLIB_Tx_SendData(uint8_t dlc);
void CANLIB_Tx_SetDataWord(encoding_union* this_union, uint8_t offset);
void CANLIB_Tx_SetByte(uint8_t byte, uint8_t index);
void CANLIB_Tx_SetBytes(uint8_t* byte_array, uint8_t array_size);
void CANLIB_Tx_SetChar(char c, uint8_t index);
void CANLIB_Tx_SetChars(char *string, uint8_t char_count);
void CANLIB_Tx_SetUint(uint32_t message, uint8_t index);
void CANLIB_Tx_SetInt(int32_t message, uint8_t index);
void CANLIB_Tx_SetFloat(float message, uint8_t index);
void CANLIB_Tx_SetDouble(double message);
void CANLIB_Tx_SetLongUint(uint64_t message);
void CANLIB_Tx_SetLongInt(int64_t message);


//
//      Rx Message Decoding Functions
//
//  Called when message is received. Must be user implemented
__weak void CANLIB_Rx_OnMessageReceived();
uint16_t    CANLIB_Rx_GetSenderID();
uint8_t     CANLIB_Rx_GetDLC();
uint8_t     CANLIB_Rx_GetSingleByte(uint8_t byte_index);
uint8_t     CANLIB_Rx_GetSingleChar(uint8_t byte_index);
void        CANLIB_Rx_GetBytes(uint8_t* byte_array);
void        CANLIB_Rx_GetChars(char* char_array);
uint32_t    CANLIB_Rx_GetAsUint(uint8_t uint_num);
int32_t     CANLIB_Rx_GetAsInt(uint8_t int_num);
float       CANLIB_Rx_GetAsFloat(uint8_t float_num);
int64_t     CANLIB_Rx_GetAsLongInt();
uint64_t    CANLIB_Rx_GetAsLongUint();
double      CANLIB_Rx_GetAsDouble();

//
//      Convenience functions
//
//CANLIB_SendBytes: sends a byte array of size "array_size" with CAN node id "id" in one function call
int8_t CANLIB_SendBytes(uint8_t* byte_array, uint8_t array_size, uint32_t id);

#endif
