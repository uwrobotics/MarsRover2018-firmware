#include "servo.h"

Servo::Servo(PinName pwmPin, uint16_t periodInUs)
{
    m_pin = pwmPin;
    m_periodInUs = periodInUs;
    p_servoObj.reset(new PwmOut(m_pin));
    p_servoObj->period_us(m_periodInUs);
}

void Servo::setDegreePerUs(float degree)
{
    m_degreePerUs = degree;
}

void Servo::setPeriodInUs(uint16_t periodInUs)
{
    m_periodInUs = periodInUs;
}

void Servo::writeDegrees(float angleInDegrees)
{
    float onTime = 1500 + angleInDegrees/m_degreePerUs;
    writeMicroseconds(onTime);
}

void Servo::writeMicroseconds(uint16_t pulseWidthInUs)
{
    p_servoObj->pulsewidth_us(pulseWidthInUs);
}