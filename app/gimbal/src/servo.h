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
    void writeDegreesNCON(float angleInDegrees);

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
	#define NCONSEV_USDEG_POS         10.44
	#define NCONSEV_USDEG_NEG         10
	#define NCONSEV_MINDEG           -55
	#define NCONSEV_MAXDEG            90
};
#endif