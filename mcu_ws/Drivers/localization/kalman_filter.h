#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H
#include "arm_math.h"

#define WHEEL_BASE 0.15f // Distance between the wheels in meters
#define ASSERV_PERIOD 0.001f // Control loop period in seconds