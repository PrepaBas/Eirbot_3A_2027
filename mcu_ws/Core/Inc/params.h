#ifndef PARAMS_H
#define PARAMS_H
#include "logging_macros.h"

#define USE_IMU 0
#define USE_ENCODERS 0

//#define USE_UROS
#ifndef USE_UROS
    #define LOGPRINT(...) printf(__VA_ARGS__)
    #define USE_SERIAL_PRINTS
#else
    #define LOGPRINT(...) RCUTILS_LOG_INFO(__VA_ARGS__);
#endif

#endif