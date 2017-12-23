#ifndef PID
#define PID

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
    Servo(PinName pwmPin, int periodInUs);
    /**
     * @brief 
     * 
     * @param onTime 
     */
    void writeMicroseconds(int onTime);
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
    void setPeriodInUs(int periodInUs);
private:
    int m_periodInUs;
    float m_degreePerUs;
    std::unique_ptr<PwmOut> p_servoObj;
    PinName m_pin;
};
#endif