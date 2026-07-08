
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



