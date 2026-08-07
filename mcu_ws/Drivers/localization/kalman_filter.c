#include "kalman_filter.h"
#include "matrix_functions.h"
#include "params.h"
// Standart unit is milimeter [mm] and radian [rad]


float X_f32[3] = {
    0.0f, 0.0f, 0.0f
}; 
arm_matrix_instance_f32 X; // Matrix X is state vector 

float P_f32[9] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};
arm_matrix_instance_f32 P; // Matrix P is state covariance matrix

float F_f32[9] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};
arm_matrix_instance_f32 F; // Matrix F is state transition matrix

float Q_f32[9] = {
    0.1f, 0.0f, 0.0f,
    0.0f, 0.1f, 0.0f,
    0.0f, 0.0f, 0.1f
};

arm_matrix_instance_f32 Q; // Matrix Q is process noise covariance matrix

float H_f32[9] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};
arm_matrix_instance_f32 H; // Matrix H is measurement matrix

float R_f32[9] = {
    0.1f, 0.0f, 0.0f,
    0.0f, 0.1f, 0.0f,
    0.0f, 0.0f, 0.1f
};
arm_matrix_instance_f32 R; // Matrix R is measurement noise covariance matrix

float Y_f32[3];
arm_matrix_instance_f32 Y;

float temp1_3x1_f32[3];
arm_matrix_instance_f32 temp1_3x1;

float temp1_3x3_f32[9];
arm_matrix_instance_f32 temp1_3x3; 

float temp2_3x3_f32[9];
arm_matrix_instance_f32 temp2_3x3; 



int kalman_init(){
    // Predict Matrix
    arm_mat_init_f32(&X, 3, 1, X_f32);
    arm_mat_init_f32(&P, 3, 3, P_f32);
    arm_mat_init_f32(&F, 3, 3, F_f32);
    arm_mat_init_f32(&Q, 3, 3, Q_f32);

    // LIDAR Matrix
    arm_mat_init_f32(&H, 3, 3, H_f32);
    arm_mat_init_f32(&R, 3, 3, R_f32);
    arm_mat_init_f32(&Y, 3, 1, Y_f32);

    // Workspace Matrix
    arm_mat_init_f32(&temp1_3x3, 3, 3, temp1_3x3_f32);
    arm_mat_init_f32(&temp2_3x3, 3, 3, temp2_3x3_f32);
    arm_mat_init_f32(&temp1_3x1, 3, 1, temp1_3x1_f32);

    return 0;
}

int kalman_set_state(float x, float y, float theta){
    X.pData[0] = x;
    X.pData[1] = y;
    X.pData[2] = theta;

    P.pData[0] = 1.0f;  P.pData[1] = 0.0f;  P.pData[2] = 0.0f;
    P.pData[3] = 0.0f;  P.pData[4] = 1.0f;  P.pData[5] = 0.0f;
    P.pData[6] = 0.0f;  P.pData[7] = 0.0f;  P.pData[8] = 0.01f;
    return 0;
}

// TODO : use linear/angular or wheel speeds?
int kalman_predict_w_model(float vl, float vr){
    F.pData[2] =  -(vl + vr) / 2.0f * sinf(X.pData[2]) * ASSERV_PERIOD;
    F.pData[5] = (vl + vr) / 2.0f * cosf(X.pData[2]) * ASSERV_PERIOD;

    arm_mat_trans_f32(&F, &temp2_3x3); // tmp1 = Ft

    arm_mat_mult_f32(&P, &temp2_3x3, &temp1_3x3); // tmp2 = P * Ft
    arm_mat_mult_f32(&F, &temp1_3x3, &temp2_3x3); // tmp1 = F * P * Ft
    arm_mat_add_f32(&temp2_3x3, &Q, &P); 
    
    X.pData[0] += (vl + vr) / 2.0f * cosf(X.pData[2]) * ASSERV_PERIOD; // x
    X.pData[1] += (vl + vr) / 2.0f * sinf(X.pData[2]) * ASSERV_PERIOD; // y
    X.pData[2] += (vr - vl) / WHEEL_BASE * ASSERV_PERIOD;              // theta

    return 0;
}

int kalman_predict_w_encoders(float d_l, float d_r){

    float d = (d_l + d_r) / 2.0f;
    float w = (d_r - d_l) / WHEEL_BASE;
    F.pData[2] = -d * sinf(X.pData[2]);
    F.pData[5] =  d * cosf(X.pData[2]);

    arm_mat_trans_f32(&F, &temp2_3x3); // tmp1 = Ft

    arm_mat_mult_f32(&P, &temp2_3x3, &temp1_3x3); // tmp2 = P * Ft
    arm_mat_mult_f32(&F, &temp1_3x3, &temp2_3x3); // tmp1 = F * P * Ft
    arm_mat_add_f32(&temp2_3x3, &Q, &P); 
    

    X.pData[0] += d * cosf(X.pData[2]); // x
    X.pData[1] += d * sinf(X.pData[2]); // y
    X.pData[2] += w * ASSERV_PERIOD;    // theta

    return 0;
}

int kalman_predict_w_sensor(float d, float w){
    F.pData[2] = -d * sinf(X.pData[2]);
    F.pData[5] =  d * cosf(X.pData[2]);

    arm_mat_trans_f32(&F, &temp2_3x3); // tmp1 = Ft

    arm_mat_mult_f32(&P, &temp2_3x3, &temp1_3x3); // tmp2 = P * Ft
    arm_mat_mult_f32(&F, &temp1_3x3, &temp2_3x3); // tmp1 = F * P * Ft
    arm_mat_add_f32(&temp2_3x3, &Q, &P); 
    

    X.pData[0] += d * cosf(X.pData[2]); // x
    X.pData[1] += d * sinf(X.pData[2]); // y
    X.pData[2] += w * ASSERV_PERIOD;    // theta

    return 0;
}

int kalman_correct_w_lidar(float x_lidar, float y_lidar, float theta_lidar){
    
    // Y = Z - H*X
    Y.pData[0] = x_lidar - X.pData[0]; // x_lidar - x_predict
    Y.pData[1] = y_lidar - X.pData[1]; // y_lidar - y_predict

    float dtheta = theta_lidar - X.pData[2];
    Y.pData[2] = atan2f(sinf(dtheta), cosf(dtheta));
    

    arm_mat_add_f32(&P, &R, &temp1_3x3); // tmp1 = S       -- H * P * Ht = P as H = I
    if (arm_mat_inverse_f32(&temp1_3x3, &temp2_3x3) != ARM_MATH_SUCCESS) {
        return -1; // no inverse
    }
    arm_mat_mult_f32(&P, &temp2_3x3, &temp1_3x3); // tmp3 = K = P * Ht * S^-1    -- but H = I

    arm_mat_mult_f32(&temp1_3x3, &Y, &temp1_3x1); // temp1 = K * Y
    arm_mat_add_f32(&X, &temp1_3x1, &X); // tempZ = X + K * Y    -- inplace op allowed

    X.pData[2] = atan2f(sinf(X.pData[2]), cosf(X.pData[2]));
    
    arm_mat_sub_f32(&H, &temp1_3x3, &temp2_3x3); // temp3 = I - K * H       --  using H as I
    arm_mat_mult_f32(&temp2_3x3, &P, &temp1_3x3); // temp2 = (I - K * H) * P
    
    memcpy(P.pData, temp1_3x3.pData, 9 * sizeof(float));
    return 0;
}

void kalman_get_pose(float* dest_pose_array){
    while(X.pData[2] > PI) X.pData[2] -= 2*PI;
    while(X.pData[2] < -PI) X.pData[2] += 2*PI;
    memcpy(dest_pose_array, X.pData, 3 * sizeof(float));
}

float kalman_get_x(){
    return X.pData[0];
}
float kalman_get_y(){
    return X.pData[1];
}
float kalman_get_theta(){
    while(X.pData[2] > PI) X.pData[2] -= 2*PI;
    while(X.pData[2] < -PI) X.pData[2] += 2*PI;
    return X.pData[2];
}