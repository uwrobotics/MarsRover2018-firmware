#include "mbed.h"
#include "pot.h"
#include "servo.h"
#include "pid.h"

// HSR-1425CR
// Define constants
static const uint16_t STOP_PULSE_WIDTH = 1465;
static const uint16_t MIN_PULSE_WIDTH_POS = 1490; 
static const uint16_t MIN_PULSE_WIDTH_NEG = 1430;
static const float PULSE_WIDTH_RANGE = 87.5;
static const float READING_PER_DEGREE = 13.20968;
static const uint16_t PERIOD = 2500; // 2500us seems to give less jitter
static const float DEGREE_PER_US = 0.08; // From manufacturere: 0.08 degrees per us
static const float MIN_PID_CONTROL = 0;
static const float MAX_PID_CONTROL = 1;

Pot pot(A0); //potentiometer
Servo servoA(D5, PERIOD); //non-continuous servo
Servo servoB(D6, PERIOD); //non-continuous servo
Pid pid(MIN_PID_CONTROL, MAX_PID_CONTROL); //pid controller
Serial pc(USBTX, USBRX); //serial object


/**
 * @brief 
 * 
 * @param degrees 
 */
void writeNonContinuous(float degrees)
{
    // Limit angle range between -60 and 60
    degrees = degrees > 60 ? 60 : (degrees < -60 ? -60 : degrees);
    servoA.writeDegrees(degrees); 
}

/**
 * @brief 
 * 
 * @param degrees 
 */
void writeContinuous(float degrees)
{
    // Limit angle range between 5 and 300
    degrees = degrees > 300 ? 300 : (degrees < 5 ? 5 : degrees);
    
    float control;
    uint16_t readIn;
    uint16_t pulseWidth;
    uint16_t target = (uint16_t)(degrees*READING_PER_DEGREE);
    uint16_t error = 0xFFFF;
    
    // Move the servo until close enough to the destination (1 degree)
    while (abs(error)>READING_PER_DEGREE)
    {
        readIn = pot.readPos();
        pc.printf("pot %04x\r\n", readIn);
        control = pid.controller(readIn, target);
        error = target - readIn;
        if(error > 0)
            pulseWidth = (uint16_t)(MIN_PULSE_WIDTH_POS + PULSE_WIDTH_RANGE*control);
        else
            pulseWidth = (uint16_t)(MIN_PULSE_WIDTH_NEG + PULSE_WIDTH_RANGE*control);
        servoB.writeMicroseconds(pulseWidth);
    }

    //Stop servo from moving
    servoB.writeMicroseconds(1465);
}

void continuousTest()
{
    while(1)
    {
        writeContinuous(200);
        wait_ms(100);
        writeContinuous(10);
        wait_ms(100);
    }
}

void nonContinuousTest()
{
    float angle = -60;
    float diffAngle = 1;
    while(1)
    {
        writeNonContinuous(angle);
        
        angle += diffAngle;
        if(angle > 60)
        {
            angle = 60;
            diffAngle = -1;
        } else if (angle < -60)
        {
            angle = -60;
            diffAngle = 1;
        }
        wait_ms(100);
    }
}

void potReadTest()
{
    uint16_t val;
    while(1)
    {
        val = pot.readPos();
        pc.printf("pot %04x\r\n", val);
        wait(0.01);
    }
}

// main() runs in its own thread in the OS
int main() 
{
    servoA.setDegreePerUs(DEGREE_PER_US);
    nonContinuousTest();
}

