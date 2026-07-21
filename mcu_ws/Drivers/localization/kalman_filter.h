#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

#define WHEEL_BASE 0.15f // Distance between the wheels in meters
#define ASSERV_PERIOD 0.001f // Control loop period in seconds

int kalman_init();
int kalman_set_state(float x, float y, float theta);
int kalman_predict_w_model(float vl, float vr);
int kalman_predict_w_encoders(float d_l, float d_r);
int kalman_predict_w_sensor(float d, float w);
int kalman_correct_w_lidar(float x_lidar, float y_lidar, float theta_lidar);
void kalman_get_pose(float* dest_pose_array);
float kalman_get_x();
float kalman_get_y();
float kalman_get_theta();
#endif