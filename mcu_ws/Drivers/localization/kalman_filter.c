#include "kalman_filter.h"
#include "arm_math.h"
// Standart unit is milimeter [mm] and radian [rad]

float32_t X_f32[5] = {
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f
}; // State vector [x, y, theta, v_lin, v_ang]^T
arm_matrix_instance_f32 X; // Matrix X is state vector 

float32_t P_f32[25] = {
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 1.0f
};
arm_matrix_instance_f32 P; // Matrix P is state covariance matrix

float32_t F_f32[25];
arm_matrix_instance_f32 F; // Matrix F is state transition matrix

float32_t Q_f32[25] = {
    0.1f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.1f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.1f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.1f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.1f
};
arm_matrix_instance_f32 Q; // Matrix Q is process noise covariance matrix
arm_matrix_instance_f32 H; // Matrix H is measurement matrix
arm_matrix_instance_f32 R; // Matrix R is measurement noise covariance matrix   



int init_kalman(kalman_state_t *state, 

    if(state->pF != NULL){
        state->pF(state->x); // update F
    }
    if(state->pQ != NULL){
        state->pQ(state->x); // update Q
    }
    if(state->pf != NULL){
        state->pf(state->x); // update x
    }
    else{
        
    }
int propagation(kalman_state_t *state){

    arm_mat_trans_f32(&state->F, &state->Ft);  
    arm_mat_mult_f32(&state->P, &state->F, &state->tmp);
    arm_mat_mult_f32(&state->Ft, &state->tmp, &state->P);
    arm_mat_add_f32(&state->P, &state->Q, &state->P);
    return 0;
}
int kalman_update(kalman_state_t *state, kalman_measurement_t *measurement);