#include <math.h>
#include "params.h"
#include "controller.h"

enum CONTROLLER_STATE state = CONTROLLER_STATE_INIT;
enum CONTROLLER_STATE next_state = CONTROLLER_STATE_INIT;
enum CONTROLLER_TYPE current_controller = CONTROLLER_TYPE_NONE;
float goal[3] = {0.0f, 0.0f, 0.0f};
int emergency_stop_flag = 0;

int is_initialized = 0;
int is_there_new_goal = 0;
int movement_stage = 0;

//* Declarations *//
int controller_run(float *input_pose, float *output_linear_velocity, float *output_angular_velocity);
float p_controller_command(float err, float x_min, float x_max, float kp);
float p_controller_cap_accel(float command, float prev_command, float max_accel, float dt);
int controller_rot_p_run();
int controller_line_p_run();
int controller_goto_p_run();
int controller_soft_stop();

//* GLOBAL VARIABLES *//
// general
float wheel_base = WHEEL_BASE;
float wheel_radius = WHEEL_RADIUS;
float pose[3] = {0.0f, 0.0f, 0.0f};
float linear_velocity = 0.0f;
float angular_velocity = 0.0f;
float old_linear_velocity = 0.0f;
float old_angular_velocity = 0.0f;
float time_step = 0.001f;

// p rot controller
float kp_rot = 1.2f;
float max_rot_speed = 1.0f;
float max_rot_accel = 1.8f;

// p line controller
float kp_line = 1.5f;
float max_line_linear_speed = 1.0f;
float max_line_linear_accel = 1.5f;


// p goto controller
float kp_goto_linear = 1.0f;
float kp_goto_angular = 0.9f;
float max_goto_linear_speed = 1.0f;
float max_goto_angular_speed = 0.2f;
float max_goto_linear_accel = 1.5f;
float max_goto_angular_accel = 0.3f;

// sorf stop
float max_sstop_linear_accel = 1.5f;
float max_sstop_angular_accel = 0.9f;

void print_state(){
    switch(state){
        case CONTROLLER_STATE_INIT:
            LOGPRINT("CONTROLLER_STATE : INIT");
            break;
        case CONTROLLER_STATE_IDLE:
            LOGPRINT("CONTROLLER_STATE : IDLE");
            break;
        case CONTROLLER_STATE_RUNNING:
            LOGPRINT("CONTROLLER_STATE : RUNNING");
            break;
        case CONTROLLER_STATE_ERROR:
            LOGPRINT("CONTROLLER_STATE : ERROR");
            break;
    }
}

float restrict_angle(float angle){
    while(angle > PI) angle -= 2*PI;
    while(angle < -PI) angle += 2*PI;
    return angle;
}
int is_controller_free(){
    int return_value = 0;
    if((state == CONTROLLER_STATE_IDLE) && (next_state == CONTROLLER_STATE_IDLE)){
        return_value = 1;
    }
    return return_value;    
}

int is_controller_speed_zero(){
    int return_value = 0;
    if((linear_velocity == 0.0f) && (angular_velocity == 0.0f)){
        return_value = 1;
    }
    return return_value;    
}

int init_controller(){
    current_controller = CONTROLLER_TYPE_NONE;
    is_there_new_goal = 0;

    linear_velocity = 0.0f; 
    angular_velocity = 0.0f;
    old_linear_velocity = 0.0f; 
    old_angular_velocity = 0.0f;
    movement_stage = 0;
    return 0;
}

int controller_start(enum CONTROLLER_TYPE controller_type, float *new_goal){
    int error_flag = 0;
    if(state != CONTROLLER_STATE_IDLE)
    {
        error_flag = 1;
        LOGPRINT("Controller is not init or idle, cannot start a new controller.");
    }

    if(new_goal != NULL)
    {
        for(int i = 0; i < 3; i++){
            goal[i] = new_goal[i];
        }
    } 
    else
    {
        error_flag = 1;
        LOGPRINT("Goal is NULL, cannot start a new controller.");
    }

    if(error_flag == 0)
    {
        current_controller = controller_type;
        is_there_new_goal = 1;
        LOGPRINT("Controller started");
    }
    return error_flag;
}

int controller_run(float *input_pose, float *output_linear_velocity, float *output_angular_velocity){
    state = next_state;
    print_state();
    for(int i = 0; i<3; i++)
    {
        pose[i] = input_pose[i];
    }
    switch(state){
        case CONTROLLER_STATE_INIT:
            init_controller();
            next_state = CONTROLLER_STATE_IDLE;
            break;
        case CONTROLLER_STATE_IDLE:
            if(current_controller != CONTROLLER_TYPE_NONE)
            {
                next_state = CONTROLLER_STATE_RUNNING;
            }
            if(emergency_stop_flag)
            {
                next_state = CONTROLLER_STATE_ERROR;
            }
            break;
        case CONTROLLER_STATE_RUNNING:
            switch(current_controller){

                case CONTROLLER_TYPE_NONE:
                    LOGPRINT("No controller is running! Falling back to initialization.");
                    next_state = CONTROLLER_STATE_IDLE;
                    break;

                case CONTROLLER_TYPE_ROT_P:
                    if(controller_rot_p_run() != 0)
                    {
                        next_state = CONTROLLER_STATE_INIT;
                    }
                    break;

                case CONTROLLER_TYPE_LINE_P:
                    if(controller_line_p_run() != 0)
                    {
                        next_state = CONTROLLER_STATE_INIT;
                    }
                    break;
                    
                case CONTROLLER_TYPE_GOTO_P:
                    if(controller_goto_p_run() != 0)
                    {
                        next_state = CONTROLLER_STATE_INIT;
                    }
                    break;

                default:
                    LOGPRINT("Unknown controller type. Falling back to initialization.");
                    next_state = CONTROLLER_STATE_INIT;
                    break;
            }
            break;
        case CONTROLLER_STATE_ERROR:
            if(is_controller_speed_zero()){
                LOGPRINT("Controller speed is zero after emergency stop. Falling back to initialization.");
                next_state = CONTROLLER_STATE_INIT;
            }
            else{
                controller_soft_stop();
            }
            break;
        default:
            LOGPRINT("Unknown controller state. Falling back to initialization.");
            next_state = CONTROLLER_STATE_INIT;
            break;
    }
    
    *output_linear_velocity = linear_velocity;
    *output_angular_velocity = angular_velocity;
    return 0;
}

float p_controller_command(float err, float x_min, float x_max, float kp){
    float u = kp * err;
    if(u > x_max){
        u = x_max;
    }
    else if(u < x_min){
        u = x_min;
    }
    return u;
}


float p_controller_cap_accel(float command, float prev_command, float max_accel, float dt){
    float delta = command - prev_command;
    float max_delta = max_accel * dt;
    if(delta > max_delta){
        command = prev_command + max_delta;
    }
    else if(delta < -max_delta){
        command = prev_command - max_delta;
    }
    return command;
}

int controller_rot_p_run(){
    float error_theta = restrict_angle(goal[2] - pose[2]);
    if((error_theta < 0.01f) && (error_theta > -0.01f)) return 1;
    float rot_command = p_controller_command(error_theta, -max_rot_speed, max_rot_speed, kp_rot);
    angular_velocity = p_controller_cap_accel(rot_command, old_angular_velocity, max_rot_accel, time_step);
    old_angular_velocity = angular_velocity;
    linear_velocity = 0.0f;
    return 0;
}

int controller_line_p_run(){
    float d = sqrt(pow(pose[0] - goal[0], 2) + pow(pose[1] - goal[1], 2));;
    if((d < 0.1f) && (d > -0.1f)) return 1;
    float line_command = p_controller_command(d, 0, max_line_linear_speed, kp_line);
    linear_velocity = p_controller_cap_accel(line_command, old_linear_velocity, max_line_linear_accel, time_step);
    old_linear_velocity = linear_velocity;
    angular_velocity = 0.0f;
    return 0;
}

int controller_goto_p_run(){
    float d = sqrt(pow(pose[0] - goal[0], 2) + pow(pose[1] - goal[1], 2));
    if(d > 0.005f)
    {
      float u_d = p_controller_command(d, -max_goto_linear_speed, max_goto_linear_speed, kp_goto_linear);
      u_d = p_controller_cap_accel(u_d, old_linear_velocity, max_goto_linear_accel, time_step);
      old_linear_velocity = u_d;
      float u_theta = p_controller_command(restrict_angle(atan2f(goal[1] - pose[1], goal[0] - pose[0]) - pose[2]), -max_goto_angular_speed, max_goto_angular_speed, kp_goto_angular);
      u_theta = p_controller_cap_accel(u_theta, old_angular_velocity, max_goto_angular_accel, time_step);
      old_angular_velocity = u_theta;

      linear_velocity = u_d;
      angular_velocity = u_theta;
    }
    else if(d > 1000)//0.050f)
    {
      float u_d = p_controller_command(d, -max_goto_linear_speed, max_goto_linear_speed, 0.2f);
      u_d = p_controller_cap_accel(u_d, old_linear_velocity, max_goto_linear_accel, time_step);
      old_linear_velocity = u_d;
    }
    else return 1;

    return 0;
}

int controller_soft_stop(){
    linear_velocity = p_controller_cap_accel(0.0f, old_linear_velocity, max_sstop_linear_accel, time_step);
    angular_velocity = p_controller_cap_accel(0.0f, old_angular_velocity, max_sstop_angular_accel, time_step);

    old_linear_velocity = linear_velocity;
    old_angular_velocity = angular_velocity;

    return 0;
}