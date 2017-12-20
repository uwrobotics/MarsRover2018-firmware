#include "pid.h"

Pid::Pid(float minControl, float maxControl)
{
    m_prevError = 0;
    m_intError = 0;
    m_minControl = minControl;
    m_maxControl = maxControl;
}

float Pid::controller(float currentVal, float targetVal)
{
    float error;
    float diffError;
    float control;
    
    // Calculate proportional, integral and differential errors 
    error = currentVal - targetVal;
    m_intError += s_timeStep*error;
    diffError = (error-m_prevError)/s_timeStep;

    control = s_kp*error + s_ki*m_intError + s_kd*diffError;

    // Limit control to max if > max
    control = (control > m_maxControl) ? m_maxControl : control;
    // Limit control to min if < min
    control = (control <     m_minControl) ? m_minControl : control;

    m_prevError = error;

    return control;
}