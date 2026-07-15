#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H
#include "arm_math.h"
typedef struct {
    arm_matrix_instance_f32 x;
    void (*pf)(arm_matrix_instance_f32);
    arm_matrix_instance_f32 F;
    void (*pF)(arm_matrix_instance_f32);
    arm_matrix_instance_f32 P;
    arm_matrix_instance_f32 Q;
    void (*pQ)(arm_matrix_instance_f32);
    arm_matrix_instance_f32 Ft;
    arm_matrix_instance_f32 tmp;
} kalman_handle_t;




typedef struct {
    arm_matrix_instance_f32 z;
    arm_matrix_instance_f32 H;
    arm_matrix_instance_f32 R;
} kalman_measurement_t;

/*
typedef struct {
    arm_matrix_instance_f32 z;
    arm_matrix_instance_f32 (*ph)(arm_matrix_instance_f32);
    arm_matrix_instance_f32 H;
    arm_matrix_instance_f32 R;
} ekalman_measurement_t;
*/

int kalman_predict(kalman_workspace_t *workspace);
int kalman_update(kalman_workspace_t *workspace, kalman_measurement_t *measurement);