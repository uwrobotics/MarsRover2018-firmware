/*
 * canlib.c
 *
 *  Created on: Jul 6, 2016
 *      Author: ryan
 */

#include "canlib.h"
#include "pins.h"
#include <limits.h>
#include <string.h>

GPIO_InitTypeDef GPIO_InitStruct;

CAN_HandleTypeDef CAN_HandleStruct;
CanTxMsgTypeDef TxMessage;
CanRxMsgTypeDef RxMessage;

return_struct received_message;
uint8_t filterCount = 0;


//
//      Interrupt Service Routines
//

void CEC_CAN_IRQHandler(void)
{
    HAL_CAN_IRQHandler(&CAN_HandleStruct);
}

uint32_t error = HAL_CAN_ERROR_NONE;
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
    error = hcan->ErrorCode;

    __HAL_CAN_CANCEL_TRANSMIT(hcan, CAN_TXMAILBOX_0);
    __HAL_CAN_CANCEL_TRANSMIT(hcan, CAN_TXMAILBOX_1);
    __HAL_CAN_CANCEL_TRANSMIT(hcan, CAN_TXMAILBOX_2);
    hcan->Instance->MSR |= CAN_MCR_RESET;
}

//
//      Initialization and Helper Functions
//

// Function not part of API so declared in here for internal use
void CANLIB_Rx_Decode(void);

void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan)
{
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin         = CAN_GPIO_RX_PIN | CAN_GPIO_TX_PIN;
    GPIO_InitStruct.Mode        = CAN_GPIO_MODE;
    GPIO_InitStruct.Pull        = CAN_GPIO_PULL;
    GPIO_InitStruct.Speed       = CAN_GPIO_SPEED;
    GPIO_InitStruct.Alternate   = CAN_GPIO_ALTERNATE;

    HAL_GPIO_Init(CAN_GPIO_PORT, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(CEC_CAN_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CEC_CAN_IRQn);
}

static int CANLIB_NetworkInit(uint8_t isLoopbackOn)
{
    CAN_HandleStruct.Instance = CAN_PORT;

    CAN_HandleStruct.Init.Prescaler = CAN_INIT_PRESCALER;
    CAN_HandleStruct.Init.SJW       = CAN_INIT_SJW;
    CAN_HandleStruct.Init.BS1       = CAN_INIT_BS1;
    CAN_HandleStruct.Init.BS2       = CAN_INIT_BS2;
    CAN_HandleStruct.Init.Mode      = (isLoopbackOn) ? CAN_MODE_LOOPBACK : CAN_INIT_MODE;
    CAN_HandleStruct.Init.ABOM      = CAN_INIT_ABOM;
    CAN_HandleStruct.Init.AWUM      = CAN_INIT_AWUM;
    CAN_HandleStruct.Init.NART      = CAN_INIT_NART;
    CAN_HandleStruct.Init.RFLM      = CAN_INIT_RFLM;
    CAN_HandleStruct.Init.TTCM      = CAN_INIT_TTCM;
    CAN_HandleStruct.Init.TXFP      = CAN_INIT_TXFP;

    CAN_HandleStruct.pRxMsg = &RxMessage;
    CAN_HandleStruct.pTxMsg = &TxMessage;


    if (HAL_CAN_Init(&CAN_HandleStruct) != HAL_OK)
    {
        return -1;
    }

    //This following code will allow all CAN messages to pass through the filter
    /*CAN_FilterConfTypeDef sFilterConfig;
    sFilterConfig.FilterNumber = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = 0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.BankNumber = 0x2d;

    if (HAL_CAN_ConfigFilter(&CAN_HandleStruct, &sFilterConfig) != HAL_OK)
    {
        return -2;
    } */


    if (HAL_CAN_Receive_IT(&CAN_HandleStruct, CAN_INIT_FIFO) != HAL_OK)
    {
        return -2;
    }

    return 0;
}

static void CANLIB_NodeInit(uint32_t id)
{
    CAN_HandleStruct.pTxMsg->StdId = id;

    CAN_HandleStruct.pTxMsg->IDE = CAN_IDE_TYPE;
    CAN_HandleStruct.pTxMsg->RTR = CAN_RTR_TYPE;
}

/*
    CANLIB_ChangeID

    Description:
    Changes the Tx message's source ID
*/
void CANLIB_ChangeID(uint32_t node_ID)
{
    CAN_HandleStruct.pTxMsg->StdId = node_ID;
}

/*
    CANLIB_AddFilter

    Description:
    Initializes an Rx filter which will filter for messages matching the specified ID
*/
int CANLIB_AddFilter(uint16_t id)
{
    if (filterCount == 28)
    {
        return 1;
    }

    static CAN_FilterConfTypeDef sFilterConfig;
    sFilterConfig.FilterNumber          = filterCount;
    sFilterConfig.FilterMode            = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale           = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh          = id << 5;
    sFilterConfig.FilterIdLow           = 0x0000;
    sFilterConfig.FilterMaskIdHigh      = ~(id << 5);
    sFilterConfig.FilterMaskIdLow       = 0x0000;
    sFilterConfig.FilterFIFOAssignment  = 0;
    sFilterConfig.FilterActivation      = ENABLE;
    sFilterConfig.BankNumber            = 0x2d;

    if (HAL_CAN_ConfigFilter(&CAN_HandleStruct, &sFilterConfig) != HAL_OK)
    {
        return 2;
    }

    filterCount++;

    return 0;
}

/*
    CANLIB_Init

    Description:
    Initializes HAL, CAN parameters, and CAN networking using the given node ID
    Can also set loopback mode
*/
int CANLIB_Init(uint32_t node_id, uint8_t isLoopbackOn)
{
    if (CANLIB_NetworkInit(isLoopbackOn) != 0)
    {
        return -1;
    }

    CANLIB_NodeInit(node_id);

    return 0;
}


//
//      Tx Message Encoding Functions
//

/*
    CANLIB_ClearDataArray

    Description:
    Clear the 8 byte data array to be sent in CAN frames. May be called automatically by some functions
*/
void CANLIB_ClearDataArray(void)
{
    //Reinitializes an integer array of size 8 to all 0s
    for (int i = 0; i < 8; i++)
    {
        CAN_HandleStruct.pTxMsg->Data[i] = 0;
    }
}

/*
    CANLIB_Tx_SendData

    Description:
    Sends whatever is in the data array with the specified DLC. The node ID will be the current set node ID
*/
void CANLIB_Tx_SendData(uint8_t dlc)
{
    if (dlc < 9)
    {
        CAN_HandleStruct.pTxMsg->DLC = (uint32_t) dlc;
        HAL_CAN_Transmit_IT(&CAN_HandleStruct);
    }
}

/*
    CANLIB_Tx_SetDataWord

    Description:
    Library users should not call this function ever
*/
void CANLIB_Tx_SetDataWord(encoding_union* this_union, uint8_t offset)
{
    if (offset >= CANLIB_FIRST_WORD_OFFSET && offset <= CANLIB_SECOND_WORD_OFFSET)
    {
        CAN_HandleStruct.pTxMsg->Data[0 + offset] = this_union->byte_array[0];
        CAN_HandleStruct.pTxMsg->Data[1 + offset] = this_union->byte_array[1];
        CAN_HandleStruct.pTxMsg->Data[2 + offset] = this_union->byte_array[2];
        CAN_HandleStruct.pTxMsg->Data[3 + offset] = this_union->byte_array[3];
    }
}

/*
    CANLIB_Tx_SetByte

    Description:
    Sets one byte in the data array
*/
void CANLIB_Tx_SetByte(uint8_t byte, uint8_t index)
{
    if (index < 8)
    {
        CAN_HandleStruct.pTxMsg->Data[index] = byte;
    }
}

/*
    CANLIB_Tx_SetBytes

    Description:
    Sets multiple bytes in the data array, starting from the first byte
    8 bytes maximum (size of data array)
    array_size must be 0-8 (although 0 will effectively make the function do nothing)
*/
void CANLIB_Tx_SetBytes(uint8_t* byte_array, uint8_t array_size)
{
    if (array_size > 8)
    {
        return;
    }

    for (uint8_t byte_index = 0; byte_index < array_size; byte_index ++)
    {
        CAN_HandleStruct.pTxMsg->Data[byte_index] = byte_array[byte_index];
    }
}

/*
    CANLIB_Tx_SetChar

    Description:
    Sets one character the data array at the specified index
*/
void CANLIB_Tx_SetChar(char c, uint8_t index)
{
    if (index < 8)
    {
        CAN_HandleStruct.pTxMsg->Data[index] = (uint8_t)c;
    }
}

/*
    CANLIB_Tx_SetChars

    Description:
    Sets array of characters in data array, similar to a byte array
    Note this array of characters need not be a null-terminated one
    8 bytes maximum (size of data array)
*/
void CANLIB_Tx_SetChars(char *string, uint8_t char_count)
{
    CANLIB_Tx_SetBytes((uint8_t*) string, char_count);
}

/*
    CANLIB_Tx_SetUint

    Description:
    Sets 32 bit unsigned int into the data array
    Index can be a value of CANLIB_INDEX_1 or CANLIB_INDEX_0
    See header doc for more details
*/
void CANLIB_Tx_SetUint(uint32_t message, CANLIB_INDEX index)
{
    encoding_union uint_union;
    uint_union.uinteger = message;

    if (index == CANLIB_INDEX_0 || index == CANLIB_INDEX_1)
    {
        CANLIB_Tx_SetDataWord(&uint_union, index * 4);
    }
}

/*
    CANLIB_Tx_SetInt

    Description:
    Sets 32 bit int into the data array
    Index can be a value of CANLIB_INDEX_1 or CANLIB_INDEX_0
    See header doc for more details
*/
void CANLIB_Tx_SetInt(int32_t message, CANLIB_INDEX index)
{
    encoding_union int_union;
    int_union.uinteger = message;

    if (index == CANLIB_INDEX_0 || index == CANLIB_INDEX_1)
    {
        CANLIB_Tx_SetDataWord(&int_union, index * 4);
    }
}

/*
    CANLIB_Tx_SetFloat

    Description:
    Sets 32 bit float into the data array
    Index can be a value of CANLIB_INDEX_1 or CANLIB_INDEX_0
    See header doc for more details
*/
void CANLIB_Tx_SetFloat(float message, CANLIB_INDEX index)
{
    encoding_union float_union;
    float_union.floatingpt = message;

    if (index == CANLIB_INDEX_0 || index == CANLIB_INDEX_1)
    {
        CANLIB_Tx_SetDataWord(&float_union, index * 4);
    }
}

/*
    CANLIB_Tx_SetLongUint

    Description:
    Sets 64 bit unsigned int in the data array
    See header doc for more details
*/
void CANLIB_Tx_SetLongUint(uint64_t message)
{
    encoding_union uint_union;
    uint_union.long_uinteger = message;

    CANLIB_Tx_SetBytes(uint_union.byte_array, 8);
}

/*
    CANLIB_Tx_SetLongInt

    Description:
    Sets 64 bit long int in the data array
    See header doc for more details
*/
void CANLIB_Tx_SetLongInt(int64_t message)
{
    encoding_union int_union;
    int_union.long_integer = message;

    CANLIB_Tx_SetBytes(int_union.byte_array, 8);
}

/*
    CANLIB_Tx_SetDouble

    Description:
    Sets 64 bit float in the data array
    See header doc for more details
*/
void CANLIB_Tx_SetDouble(double message)
{
    encoding_union double_union;
    double_union.dub = message;

    CANLIB_Tx_SetBytes(double_union.byte_array, 8);
}

//
//      Rx Message Decoding Functions
//
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan)
{
    if (HAL_CAN_Receive_IT(&CAN_HandleStruct, CAN_FIFO0) == HAL_OK)
    {
        CANLIB_Rx_Decode();
        CANLIB_Rx_OnMessageReceived();
    }
}

//Purpose of this is to take the received data array and put it into our own data structure.
// Can be later expanded such that we can maintain a queue of these messages in the event they come too quickly for the user code
void CANLIB_Rx_Decode(void)
{
    received_message.DLC = (uint8_t) CAN_HandleStruct.pRxMsg->DLC;
    received_message.transmitter_ID = CAN_HandleStruct.pRxMsg->StdId;

    for (uint8_t rxMsgIndex = 0; rxMsgIndex < received_message.DLC; rxMsgIndex ++)
    {
        received_message.whole_byte_array[rxMsgIndex] = CAN_HandleStruct.pRxMsg->Data[rxMsgIndex];
    }
}

/*
    CANLIB_Rx_GetSenderID

    Description:
    Returns the frame's source ID
*/
uint16_t CANLIB_Rx_GetSenderID(void)
{
    return received_message.transmitter_ID;
}

/*
    CANLIB_Rx_GetDLC

    Description:
    Returns the frame's data length code (i.e. number of data bytes)
*/
uint8_t CANLIB_Rx_GetDLC(void)
{
    return received_message.DLC;
}

/*
    CANLIB_Rx_GetSingleByte

    Description:
    Returns the value at the specified index in the data array (0 if invalid index)
    Can be used in OnMessageReceived()
    byte_index is a value from 0-7, corresponding to one whole byte in the CAN frame
    See header docs for more instructions
*/
uint8_t CANLIB_Rx_GetSingleByte(uint8_t byte_index)
{
    if (byte_index < 8)
    {
        return received_message.whole_byte_array[byte_index];
    }

    return 0;
}

/*
    CANLIB_Rx_GetSingleChar

    Description:
    Returns the value as a char at the specified index in the data array (0 if invalid index)
    Can be used in OnMessageReceived()
    byte_index is a value from 0-7, corresponding to one whole byte in the CAN frame
    See header docs for more instructions
*/
uint8_t CANLIB_Rx_GetSingleChar(uint8_t byte_index)
{
    if (byte_index < 8)
    {
        return (char)received_message.whole_byte_array[byte_index];
    }

    return 0;
}

/*
    CANLIB_Rx_GetBytes

    Description:
    Stores the data array in a byte array
    Can be used in OnMessageReceived()
    byte_array and char_array must have as many bytes as required according to the DLC of the frame (size of 8 is thus always a safe size)
    DO NOT USE AN ARRAY WITH SIZE SMALLER THAN THE DLC OF THE FRAME
    See header docs for more instructions
*/
void CANLIB_Rx_GetBytes(uint8_t* byte_array)
{
    for (uint8_t rxMsgIndex = 0; rxMsgIndex < received_message.DLC; rxMsgIndex ++)
    {
        byte_array[rxMsgIndex] = received_message.whole_byte_array[rxMsgIndex];
    }
}

/*
    CANLIB_Rx_GetChars

    Description:
    Stores the data array as chars in an array
    Can be used in OnMessageReceived()
    byte_array and char_array must have as many bytes as required according to the DLC of the frame (size of 8 is thus always a safe size)
    DO NOT USE AN ARRAY WITH SIZE SMALLER THAN THE DLC OF THE FRAME
    See header docs for more instructions
*/
void CANLIB_Rx_GetChars(char* char_array)
{
    for (uint8_t rxMsgIndex = 0; rxMsgIndex < received_message.DLC; rxMsgIndex ++)
    {
        char_array[rxMsgIndex] = (char)received_message.whole_byte_array[rxMsgIndex];
    }
}

/*
    CANLIB_Rx_GetAsUint

    Description:
    Returns the value as an unsigned int at the specified index in the data array
    Can be used in OnMessageReceived()
    uint_num can be 1 or 0. This corresponds to the last or first 32 bits in the frame, respectively
    See header docs for more instructions
*/
uint32_t CANLIB_Rx_GetAsUint(uint8_t uint_num)
{
    return (uint_num == 0) ? received_message.return_uint_1 : received_message.return_uint_2;
}

/*
    CANLIB_Rx_GetAsInt

    Description:
    Returns the value as an int at the specified index in the data array
    Can be used in OnMessageReceived()
    See header docs for more instructions
*/
int32_t CANLIB_Rx_GetAsInt(uint8_t int_num)
{
    return (int_num == 0) ? received_message.return_int_1 : received_message.return_int_2;
}

/*
    CANLIB_Rx_GetAsFloat

    Description:
    Returns the value as an float at the specified index in the data array
    Can be used in OnMessageReceived()
    float_num can be 1 or 0. This corresponds to the last or first 32 bits in the frame, respectively
    See header docs for more instructions
*/
float CANLIB_Rx_GetAsFloat(uint8_t float_num)
{
    return (float_num == 0) ? received_message.return_float_1 : received_message.return_float_2;
}

/*
    CANLIB_Rx_GetAsLongInt

    Description:
    Returns the value as a 64 bit int at the specified index in the data array
    Can be used in OnMessageReceived()
    See header docs for more instructions
*/
int64_t CANLIB_Rx_GetAsLongInt(void)
{
    return received_message.long_int;
}

/*
    CANLIB_Rx_GetAsLongUint

    Description:
    Returns the value as a 64 bit unsigned int at the specified index in the data array
    Can be used in OnMessageReceived()
    See header docs for more instructions
*/
uint64_t CANLIB_Rx_GetAsLongUint(void)
{
    return received_message.long_uint;
}

/*
    CANLIB_Rx_GetAsDouble

    Description:
    Returns the value as a 64 bit double at the specified index in the data array
    Can be used in OnMessageReceived()
    See header docs for more instructions
*/
double CANLIB_Rx_GetAsDouble(void)
{
    return received_message.dub;
}

//Convenience functions
int8_t CANLIB_SendBytes(uint8_t* byte_array, uint8_t array_size, uint32_t id)
{
    if (array_size > 8)
    {
        return -2;
    }

    CANLIB_ClearDataArray();
    CAN_HandleStruct.pTxMsg->DLC = array_size;
    CANLIB_Tx_SetBytes(byte_array, array_size);
    CAN_HandleStruct.pTxMsg->StdId = id;

    if (HAL_CAN_Transmit_IT(&CAN_HandleStruct) != HAL_OK)
    {
        return -1;
    }

    return 0;
}

