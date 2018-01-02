#ifndef SERVO_H
#define SERVO_H

#include "mbed.h"
#include <memory>

/**
 * @brief Servo class
 * 
 */
class Servo
{
public:

    /**
     * @brief 
     * 
     * @param pwmPin 
     */
    Servo(PinName pwmPin, uint16_t periodInUs);
    /**
     * @brief 
     * 
     * @param pulseWitdh
     */
    void writeMicroseconds(uint16_t pulseWitdhInUs);
    /**
     * @brief 
     * 
     * @param angleInDegrees 
     */
    void writeDegrees(float angleInDegrees);
    /**
     * @brief 
     * 
     * @param angle 
     */
    void setDegreePerUs(float angle);
    /**
     * @brief 
     * 
     * @param periodInUs 
     */
    void setPeriodInUs(uint16_t periodInUs);
private:
    uint16_t m_periodInUs;
    float m_degreePerUs;
    std::unique_ptr<PwmOut> p_servoObj;
    PinName m_pin;
};
#endif