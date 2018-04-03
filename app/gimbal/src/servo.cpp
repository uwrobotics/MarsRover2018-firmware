#include "servo.h"

Servo::Servo(PinName pwmPin, uint16_t periodInUs)
{
    this-> m_pin = pwmPin;
    this-> m_periodInUs = periodInUs;
    this->p_servoObj.reset(new PwmOut(m_pin));
    this->p_servoObj->period_us(m_periodInUs);
}


void Servo::setPeriodInUs(uint16_t periodInUs)
{
    this->m_periodInUs = periodInUs;
}

void Servo::writeDegreesNCON(float angleInDegrees)
{
  float onTime ;
  if(angleInDegrees > NCONSEV_MAXDEG ){
    angleInDegrees = NCONSEV_MAXDEG ;
  }
  
  if(angleInDegrees < NCONSEV_MINDEG ){
    angleInDegrees = NCONSEV_MINDEG ;
  }
  
  if(angleInDegrees>=0){
	onTime = 1300 + angleInDegrees*NCONSEV_USDEG_POS;
  }
  else{
	onTime = 1300 + angleInDegrees*NCONSEV_USDEG_NEG;
  }
   writeMicroseconds(onTime);
}

void Servo::writeMicroseconds(uint16_t pulseWidthInUs)
{
    this->p_servoObj->pulsewidth_us(pulseWidthInUs);
}