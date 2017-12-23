#ifndef PID
#define PID

#include "mbed.h"

/**
 * @brief      Class for pid controller
 */
class Pid
{
public:

    /**
     * @brief      Constructor
     */
    Pid(float minControl, float maxControl);
    float controller(float currentVal, float targetVal);
private:
    bool b_freshController;
    float m_prevError;
    float m_intError;
    float m_minControl;
    float m_maxControl;
    const float s_kp = 0.0001;
    const float s_ki = 0;
    const float s_kd = 0;
    const float s_timeStep = 0.02;
    Timer m_timer;
};
#endif