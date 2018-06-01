#include "mbed.h"

Serial pc(USBTX, USBRX);
AnalogIn ec(A0);

// main() runs in its own thread in the OS
int main() {
    while (true) {

        float averageVoltage = ec.read()*3300.0f;
        float temperature = 25.0;
        float TempCoefficient=1.0+0.0185*(temperature-25.0);
        float CoefficientVoltage=(float)averageVoltage/TempCoefficient;   
        pc.printf("coeff voltage: %f\r\n",CoefficientVoltage);

        float ECcurrent;
        if(CoefficientVoltage<=448)ECcurrent=6.84*CoefficientVoltage-64.32;   //1ms/cm<EC<=3ms/cm
        else if(CoefficientVoltage<=1457)ECcurrent=6.98*CoefficientVoltage-127;  //3ms/cm<EC<=10ms/cm
        else ECcurrent=5.3*CoefficientVoltage+2278;                           //10ms/cm<EC<20ms/cm
        ECcurrent/=1000;    //convert us/cm to ms/cm
        pc.printf("EC: %f ms/cm\r\n",ECcurrent);

        wait(0.5);
    }
}

