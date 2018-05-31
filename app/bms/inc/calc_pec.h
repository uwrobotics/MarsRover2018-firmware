#ifndef _CALC_PEC_H_
#define _CALC_PEC_H_

#include "mbed.h"

// PEC calculation table
extern int16_t pec15Table[256];

bool checkGroupPec(uint8_t *b);
void ltc6804util_initPec(void);
uint16_t ltc6804util_calcPec(uint8_t *data, int len);

#endif //_CALC_PEC_H_