#ifndef POT_H
#define POT_H
#include "mbed.h"

/**
 * @brief      Class for potentiometer
 */
class Pot
{
public:

    /**
     * @brief      Constructor
     */
    Pot(PinName pin);
    uint16_t readPos();
private:
    AnalogIn m_dev = AnalogIn(A0);
};
#endif
