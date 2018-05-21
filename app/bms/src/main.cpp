#include "mbed.h"
#include "calc_pec.h"

Serial pc(USBTX, USBRX);
//TODO: figure out the proper pins
//MOSI, MISO, SCLK
SPI ltc_spi(PC_0, PC_1, PC_2);
DigitalOut ltc_spi_cs(PC_3);

void pec_test()
{
    uint8_t data[] = {0x00, 0x06};
    uint16_t pec = ltc6804util_calcPec(data, 2);

    pc.printf("0x%04X", pec);
}

// main() runs in its own thread in the OS
int main() {
    ltc6804util_initPec();
    pec_test();
    ltc_spi_cs = 1;


    ltc_spi.format(8,3); //LTC 6804 operates in SPI mode 3. CPHA = 1 and CPOL = 1.
    ltc_spi.frequency(1000000); //1MHz SPI speed
 
    

}

