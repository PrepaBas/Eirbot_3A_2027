#ifndef PARAMS_H
#define PARAMS_H


#define PI 3.14159265358979323846f
#define WHEEL_RADIUS 0.020f
#define WHEEL_BASE 0.1875f
#define USE_IMU 0
#define USE_ENCODERS 1

//#define USE_UROS
#ifndef USE_UROS
    #include <stdio.h>
    #define LOGPRINT(format, ...) printf(format "\n", ##__VA_ARGS__)
    #define USE_SERIAL_PRINTS
#endif
#endif
