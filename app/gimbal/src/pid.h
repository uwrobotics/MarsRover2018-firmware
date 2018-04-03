#ifndef PID_H
#define PID_H

#include "mbed.h"
#include "pot.h"
#include <math.h>

/**
 * @brief      Class for pid controller
 */
class Pid
{
public:

    /**
     * @brief 
     * 
     * @param minControl 
     * @param maxControl 
     */
    Pid(float minControl, float maxControl);
    /**
     * @brief 
     * 
     * @param currentVal 
     * @param targetVal 
     * @return float 
     */
    float controller(float targetVal, Pot pot);
	
	//~Pid();
private:
    
	bool first_time;
	float m_intergralsum;
    float m_prevdiffError;
	float m_diffError;
    float m_intError;
    float m_pError;
    float m_minControl;
    float m_maxControl;
    const float s_kp = 35.46155556;
    const float s_ki = 0.00;
    const float s_kd = 1.12;
    Timer m_timer;
};
#endif