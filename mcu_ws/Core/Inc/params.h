#ifndef PARAMS_H
#define PARAMS_H


#define PI 3.14159265358979323846f
#define WHEEL_RADIUS 0.020244075336 // test on 1.75m straight line
#define WHEEL_BASE 0.1881f// test on 1.75m straight back and forth 
#define LR__ENCOD_IMBALANCE 0.00122f
#define USE_IMU 0
#define USE_ENCODERS 1

//#define USE_UROS
#ifndef USE_UROS
    #include <stdio.h>
    #define LOGPRINT(format, ...) printf(format "\n", ##__VA_ARGS__)
    #define USE_SERIAL_PRINTS
#endif
#endif
