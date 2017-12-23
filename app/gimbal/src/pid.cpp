#include "pid.h"

Pid::Pid(float minControl, float maxControl)
{
    m_prevError = 0;
    m_intError = 0;
    m_minControl = minControl;
    m_maxControl = maxControl;
    b_freshController = true;
}

float Pid::controller(float currentVal, float targetVal)
{
    float error;
    float diffError;
    float control;
    float timeStep;

    // Calculate proportional, integral and differential errors 
    error = currentVal - targetVal;
    if(b_freshController)
    {
        b_freshController = false;
        m_timer.start();
        m_intError += 0;
        diffError = 0;
    } else
    {
        timeStep = m_timer.read_us()/1000000.0;
        m_intError += timeStep*error;
        diffError = (error-m_prevError)/timeStep;
    }
    
    control = s_kp*error + s_ki*m_intError + s_kd*diffError;

    // Limit control to max if > max
    control = (control > m_maxControl) ? m_maxControl : control;
    // Limit control to min if < min
    control = (control < m_minControl) ? m_minControl : control;

    m_prevError = error;

    // Reset timer for next controller call
    m_timer.reset();

    return control;
}