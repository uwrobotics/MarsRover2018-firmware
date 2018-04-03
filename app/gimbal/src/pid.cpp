#include "pid.h"
// Speeds big 300-1400 or 1700-2100 slow 1440<=x<=1480 or 1660>=x>=1560;
Pid::Pid(float minControl, float maxControl)
{
    m_prevdiffError = 0;
    m_intError = 0;
    m_diffError = 0;
    m_intergralsum = 0;
	m_minControl = minControl;
    m_maxControl = maxControl;
	first_time=false;

}

float Pid::controller(float targetVal, Pot pot)

{
	if(targetVal>350){targetVal=350;}
	if(targetVal<0){targetVal=0;}
    float timenow;
	float Out_control;
	float val = (pot.readPos()/100)/1.87;
	float diff=0;
    if(!first_time){m_timer.start(); first_time=true;}
    // Calculate proportional, integral and differential errors 
    // Proportional Error;
	
	else{
		m_timer.stop();
		timenow=m_timer.read_us();
		timenow/=1000000;
		m_timer.reset();
		
		m_pError = targetVal-val;
		// Integration 
		
		m_intError = targetVal-val;
		m_intergralsum += m_intError *timenow;
		m_intError = m_intergralsum;

		// Derivation
			

		m_diffError = targetVal-val;
		
		diff = m_diffError - m_prevdiffError;
		m_prevdiffError = m_diffError;
			
		m_diffError = diff/timenow; 
			
		
		Out_control = s_kp*m_pError + s_kd*m_diffError + s_ki*m_intError;
		
		if(fabs(Out_control)<=9.482){Out_control=0;}

	 
		Out_control = (Out_control > m_maxControl) ? m_maxControl : Out_control;
	 
		Out_control = (Out_control < m_minControl) ? m_minControl : Out_control;


		// Reset timer for next controller call

        m_timer.start();
		return (Out_control);
	}
}