#include "kalman_filter.h"
#include "arm_math.h"
// Standart unit is milimeter [mm] and radian [rad]

float32_t X_f32[3] = {
    0.0f, 0.0f, 0.0f
}; 
arm_matrix_instance_f32 X; // Matrix X is state vector 

float32_t P_f32[9] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};
arm_matrix_instance_f32 P; // Matrix P is state covariance matrix

float32_t F_f32[9] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};
arm_matrix_instance_f32 F; // Matrix F is state transition matrix

float32_t Q_f32[9] = {
    0.1f, 0.0f, 0.0f,
    0.0f, 0.1f, 0.0f,
    0.0f, 0.0f, 0.1f
};

arm_matrix_instance_f32 Q; // Matrix Q is process noise covariance matrix

float32_t H_f32[9] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};
arm_matrix_instance_f32 H; // Matrix H is measurement matrix

float32_t R_f32[9] = {
    0.1f, 0.0f, 0.0f,
    0.0f, 0.1f, 0.0f,
    0.0f, 0.0f, 0.1f
};
arm_matrix_instance_f32 R; // Matrix R is measurement noise covariance matrix


float32_t Z_f32[3];
arm_matrix_instance_f32 Z;

float32_t Y_f32[3];
arm_matrix_instance_f32 Y;

float32_t temp1_3x3_f32[9];
arm_matrix_instance_f32 temp1_3x3; 

float32_t temp2_3x3_f32[9];
arm_matrix_instance_f32 temp2_3x3; 

float32_t temp3_3x3_f32[9];
arm_matrix_instance_f32 temp3_3x3; 


int kalman_init(){
    // Predict Matrix
    arm_mat_init_f32(&X, 3, 1, X_f32);
    arm_mat_init_f32(&P, 3, 3, P_f32);
    arm_mat_init_f32(&F, 3, 3, F_f32);
    arm_mat_init_f32(&Q, 3, 3, Q_f32);

    // LIDAR Matrix
    arm_mat_init_f32(&H, 2, 3, H_f32);
    arm_mat_init_f32(&R, 2, 2, R_f32);
    arm_mat_init_f32(&Z, 2, 1, Z_f32);
    arm_mat_init_f32(&Y, 2, 1, Y_f32);

    // Workspace Matrix
    arm_mat_init_f32(&temp1_3x3, 3, 3, temp1_3x3_f32);
    arm_mat_init_f32(&temp2_3x3, 3, 3, temp2_3x3_f32);
    arm_mat_init_f32(&temp3_3x3, 3, 3, temp3_3x3_f32);

    return 0;
}

int kalman_set_state(float32_t x, float32_t y, float32_t theta){
    X.pData[0] = x;
    X.pData[1] = y;
    X.pData[2] = theta;

    P.pData[0] = 1.0f;  P.pData[1] = 0.0f;  P.pData[2] = 0.0f;
    P.pData[3] = 0.0f;  P.pData[4] = 1.0f;  P.pData[5] = 0.0f;
    P.pData[6] = 0.0f;  P.pData[7] = 0.0f;  P.pData[8] = 0.01f;
    return 0;
}

int kalman_predict_w_model(float32_t vl, float32_t vr){
    F.pData[2] =  -(vl + vr) / 2.0f * sinf(X.pData[2]) * ASSERV_PERIOD;
    F.pData[5] = (vl + vr) / 2.0f * cosf(X.pData[2]) * ASSERV_PERIOD;

    arm_mat_trans_f32(&F, &temp2_3x3); // tmp1 = Ft²

    arm_mat_mult_f32(&P, &temp2_3x3, &temp1_3x3); // tmp2 = P * Ft
    arm_mat_mult_f32(&F, &temp1_3x3, &temp2_3x3); // tmp1 = F * P * Ft
    arm_mat_add_f32(&temp2_3x3, &Q, &P); 
    

    X.pData[0] += (vl + vr) / 2.0f * cosf(X.pData[2]) * ASSERV_PERIOD; // x
    X.pData[1] += (vl + vr) / 2.0f * sinf(X.pData[2]) * ASSERV_PERIOD; // y
    X.pData[2] += (vr - vl) / WHEEL_BASE * ASSERV_PERIOD;              // theta

    return 0;
}

int kalman_predict_w_sensor(float32_t d, float32_t w){
    F.pData[2] = -d * sinf(X.pData[2]);
    F.pData[5] =  d * cosf(X.pData[2]);

    arm_mat_trans_f32(&F, &temp2_3x3); // tmp1 = Ft²

    arm_mat_mult_f32(&P, &temp2_3x3, &temp1_3x3); // tmp2 = P * Ft
    arm_mat_mult_f32(&F, &temp1_3x3, &temp2_3x3); // tmp1 = F * P * Ft
    arm_mat_add_f32(&temp2_3x3, &Q, &P); 
    

    X.pData[0] += d * cosf(X.pData[2]); // x
    X.pData[1] += d * sinf(X.pData[2]); // y
    X.pData[2] += w * ASSERV_PERIOD;    // theta

    return 0;
}

int kalman_correct_w_lidar(float x_lidar, float y_lidar, float theta_lidar){
    
    Z.pData[0] = x_lidar;
    Z.pData[1] = y_lidar;
    Z.pData[2] = theta_lidar;

    arm_mat_trans_f32(&H, &temp3_3x3); // tmp3 = Ht
    arm_mat_mult_f32(&P, &temp3_3x3, &temp1_3x3); // tmp1 = P * Ht
    arm_mat_mult_f32(&H, &temp1_3x3, &temp2_3x3); // tmp2 = H * P * Ht
    arm_mat_add_f32(&temp2_3x3, &R, &temp1_3x3); // tmp1 = S
    arm_mat_inverse_f32(&temp1_3x3, &temp2_3x3); // tmp2 = S^-1

    arm_mat_mult_f32(&temp3_3x3, &temp2_3x3, &temp1_3x3); // tmp1 = Ht * S^-1
    arm_mat_mult_f32(&H, &temp1_3x3, &temp2_3x3); // tmp2 = K = H* P * Ht * S^-1

    arm_mat_mult_f32(&H, &X, &Y); 
    arm_mat_sub_f32(&Z, &Y, &Y); 

    return 0;
}