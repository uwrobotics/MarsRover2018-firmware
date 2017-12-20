#include "pot.h"

Pot::Pot(PinName pin)
{
    m_dev = AnalogIn(pin);
}

uint16_t Pot::readPos()
{
    uint16_t val = m_dev.read_u16();
    return val;
}
