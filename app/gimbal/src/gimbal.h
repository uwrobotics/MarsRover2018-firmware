#include "mbed.h"

/**
 * @brief      Class for gimbal.
 */
class Gimbal
{
public:

    /**
     * @brief      { function_description }
     */
    Gimbal();

    /**
     * @brief      { function_description }
     *
     * @param[in]  xShift  The x shift
     * @param[in]  yShift  The y shift
     * @param[in]  speed   The speed
     */
    move(int xShift, int yShift, float speed);
private:
}
