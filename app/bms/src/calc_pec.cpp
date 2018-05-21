#include "calc_pec.h"

int16_t pec15Table[256];

/*! Check PEC for a register group.
 *  \param b Buffer containing received data, 6 data bytes + 2 PEC bytes.
 *  \return return TRUE if OK, return FALSE if FAIL
 *  PEC received in 7th and 8th byte. Function returns PEC_ERROR on mis-match.
 */
bool checkGroupPec(uint8_t *b) {
    
    uint16_t pec = ltc6804util_calcPec(b, 6);              // PEC of data bytes
    uint16_t rxPec = ((uint16_t)b[6]) << 8 | b[7];        // received PEC
    if (pec != rxPec) {
        return false;
    }
    return true;
}

/*! Pre-compute PEC15 checksum table for fast checksumming.
 *  Function is based on code supplied in the LTC6804 datasheet.
 */
void ltc6804util_initPec(void)
{
    const int16_t CRC15_POLY = 0x4599;
    int i;
    for (i = 0; i < 256; i++) {
        int remainder = i << 7;
        int bit;
        for (bit = 8; bit > 0; bit--) {
            if (remainder & 0x4000) {
                    remainder <<= 1;
                    remainder ^= CRC15_POLY;
            } else {
                    remainder <<= 1;
            }
        }
        pec15Table[i] = remainder & 0xFFFF;
    }
}

/*! Perform PEC15 checksum calculation based on pre-initialized table.
 *  \param data Pointer to data which should be checksummed.
 *  \param len Length in bytes of data to be checksummed.
 *  \return Function returns PEC15 checksum of data, << by 1 bit.
 */
uint16_t ltc6804util_calcPec(uint8_t *data, int len)
{
    uint16_t remainder, address;
    int i;
    remainder = 16;                     // PEC seed
    for (i = 0; i < len; i++) {
        //calculate PEC table address
        address = ((remainder >> 7) ^ data[i]) & 0xff;
	    remainder = (remainder << 8 ) ^ pec15Table[address];
    }
    // The CRC15 has a 0 in the LSB so the final value must be << by 1.
    return remainder << 1;
}