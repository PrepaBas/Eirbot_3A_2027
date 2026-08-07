#ifndef PARAMS_H
#define PARAMS_H


#define PI 3.14159265358979323846f
#define WHEEL_RADIUS 0.035f
#define WHEEL_BASE 0.25f
#define USE_IMU 0
#define USE_ENCODERS 0

#define USE_UROS
#ifndef USE_UROS
    #include <stdio.h>
    #define LOGPRINT(format, ...) printf(format "\n", ##__VA_ARGS__)
    #define USE_SERIAL_PRINTS
#else
    #include "log_msg.h"
    #define LOGPRINT(format, ...) //publish_log_msg(format , ##__VA_ARGS__)
#endif
#endif
