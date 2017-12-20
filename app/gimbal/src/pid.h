#ifndef PID
#define PID

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
    float m_prevError;
    float m_intError;
    float m_minControl;
    float m_maxControl;
    static float s_kp = 0.0001;
    static float s_ki = 0;
    static float s_kd = 0;
    static float s_timeStep = 0.02;
};
#endif