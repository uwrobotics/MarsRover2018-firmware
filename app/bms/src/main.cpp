
#include "mbed.h"
#include "calc_pec.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "canlib.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define NUM_CELLS 8

Serial pc(USBTX, USBRX);

//TODO: figure out the proper pins
//MOSI, MISO, SCLK
SPI ltc_spi(PC_0, PC_1, PC_2); //for some reason enabling SPI disabled USBTX/USBRX for printf
DigitalOut ltc_spi_cs(PC_3);
float cell_voltages[NUM_CELLS];
const uint32_t CAN_CELL_ID[NUM_CELLS] = {110, 111, 112, 113, 114, 115, 116, 117};

void pec_test()
{
    uint8_t data[] = {0x00, 0x06};
    uint16_t pec = ltc6804util_calcPec(data, 2);

    pc.printf("0x%04X", pec);
}

void send_spi(uint8_t* tx, uint8_t* rx, uint8_t tx_len, uint8_t rx_len)
{
    ltc_spi.write(0x00); //dummy byte to wake device
    wait_us(300); //wait for serial interface to start
    ltc_spi.write(0x00);

    ltc_spi_cs = 0;
    for(int i = 0; i < tx_len; i++)
    {
        if(tx != NULL)
            ltc_spi.write(tx[i]);
    }
    pc.printf("\n");

    for(int j = 0; j < rx_len; j++)
    {
        if(rx != NULL);
            rx[j] = ltc_spi.write(0x00); //write dummy byte and read SDO
    }
    ltc_spi_cs = 1;
}

void set_cmd(uint8_t* tx, uint8_t* cmd, uint16_t pec)
{
    tx[0] = cmd[0];
    tx[1] = cmd[1];
    tx[2] = (pec >> 8) & 0xFF;
    tx[3] = pec & 0xFF;
}

float bitarray_to_voltage(uint8_t* bits, int index)
{
    uint16_t val = (bits[2*index+1] << 8) | bits[2*index];
    return (float)val * 0.0001f;
}

void read_cell_voltages()
{
    uint8_t data[2];
    uint16_t pec;
    uint8_t tx[4];
    uint8_t rx[8];

    //send Start Cell Voltage ADC Conversion
    data[0] = 0x03;
    data[1] = 0x70;
    pec = ltc6804util_calcPec(data, 2);
    set_cmd(tx, data, pec);
    send_spi(tx, NULL, 4, 0);

    wait_ms(5); // wait for ADC conversion to finish

    //send Read Cell Voltage Register Group B
    data[0] = 0x80; //using address 0000
    data[1] = 0x06;
    pec = ltc6804util_calcPec(data, 2);
    set_cmd(tx, data, pec);
    send_spi(tx, rx, 4, 8);
    if (!checkGroupPec(rx)) {
        pc.printf("Bad PEC when reading Register Group B.\n");
    }
    cell_voltages[0] = bitarray_to_voltage(rx, 1); //cell 5
    cell_voltages[1] = bitarray_to_voltage(rx, 2); //cell 6

    //send Read Cell Voltage Register Group C
    data[0] = 0x80;
    data[1] = 0x08;
    pec = ltc6804util_calcPec(data, 2);
    set_cmd(tx, data, pec);
    send_spi(tx, rx, 4, 8);
    if (!checkGroupPec(rx)) {
        pc.printf("Bad PEC when reading Register Group C.\n");
    }
    cell_voltages[2] = bitarray_to_voltage(rx, 0); //cell 7
    cell_voltages[3] = bitarray_to_voltage(rx, 1); //cell 8
    cell_voltages[4] = bitarray_to_voltage(rx, 2); //cell 9

    //send Read Cell Voltage Register Group D
    data[0] = 0x80;
    data[1] = 0x0A;
    pec = ltc6804util_calcPec(data, 2);
    set_cmd(tx, data, pec);
    send_spi(tx, rx, 4, 8);
    if (!checkGroupPec(rx)) {
        pc.printf("Bad PEC when reading Register Group D.\n");
    }
    cell_voltages[5] = bitarray_to_voltage(rx, 0); //cell 10
    cell_voltages[6] = bitarray_to_voltage(rx, 1); //cell 11
    cell_voltages[7] = bitarray_to_voltage(rx, 2); //cell 12
}

void send_can()
{
    for (int i = 0; i < NUM_CELLS; i++)
    {
        CANLIB_ChangeID(CAN_CELL_ID[i]);
        CANLIB_Tx_SetFloat(cell_voltages[i], CANLIB_INDEX_0);
        CANLIB_Tx_SendData(CANLIB_DLC_FOUR_BYTES);
    }
}

// main() runs in its own thread in the OS
int main() {
    ltc6804util_initPec();
    pec_test();

    ltc_spi_cs = 1;
    ltc_spi.format(8,3); //LTC 6804 operates in SPI mode 3. CPHA = 1 and CPOL = 1.
    ltc_spi.frequency(1000000); //1MHz SPI speed

    if (CANLIB_Init(CAN_CELL_ID[0], CANLIB_LOOPBACK_OFF) != 0) {
        pc.printf("init failed\r\n");
    }
    Ticker tick;
    tick.attach(send_can, 5.0); //send voltages over CAN every 5 seconds

    const float VOLTAGE_THRESH = 0.2f;
    float lowest_cell_voltage = 10;
    uint16_t drain_cells = 0;
    while(1)
    {
        read_cell_voltages();

        for(int i = 0; i < 8; i++) {
            lowest_cell_voltage = MIN(lowest_cell_voltage, cell_voltages[i]);
        }
        for(int i = 0; i < 8; i++) {
            if(cell_voltages[i] > lowest_cell_voltage + VOLTAGE_THRESH) {
                drain_cells |= (1 << (4+i));
            } else {
                drain_cells &= ~(1 << (4+i));
            }
        }

        uint8_t tx[12];
        tx[0] = 0x00;
        tx[1] = 0x01; //WRCFG command
        tx[2] = 0x3D;
        tx[3] = 0x6E; //PEC of WRCFG

        tx[4] = 0xF8;
        tx[5] = 0xE1;
        tx[6] = 0xC4;
        tx[7] = 0xAF;
        tx[8] = drain_cells & 0xFF;
        tx[9] = (drain_cells >> 8) & 0x0F; 
        uint16_t pec = ltc6804util_calcPec(tx+4, 6);
        tx[10] = (pec >> 8) & 0xFF;
        tx[11] = pec & 0xFF;

        send_spi(tx, NULL, 12, 0);

        wait_ms(100); //run at 10Hz
    }
}

#ifdef __cplusplus
}
#endif