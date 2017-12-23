#include "servo.h"

Servo::Servo(PinName pwmPin, int periodInUs)
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

void Servo::setPeriodInUs(int periodInUs)
{
    m_periodInUs = periodInUs;
}

void Servo::writeDegrees(float angleInDegrees)
{
    float onTime = 1500 + angleInDegrees/m_degreePerUs;
    writeMicroseconds(onTime);
}

void Servo::writeMicroseconds(int onTime)
{
    float dutyCycle = onTime/m_periodInUs;
    p_servoObj->write(dutyCycle);
}