#ifndef CONTROLLER_H
#define CONTROLLER_H

enum CONTROLLER_STATE{
    CONTROLLER_STATE_INIT,
    CONTROLLER_STATE_IDLE,
    CONTROLLER_STATE_RUNNING,
    CONTROLLER_STATE_ERROR
};

enum CONTROLLER_TYPE{
    CONTROLLER_TYPE_ROT_P,
    CONTROLLER_TYPE_LINE_P,
    CONTROLLER_TYPE_GOTO_P,
    CONTROLLER_TYPE_NONE
};

int is_controller_free();

int controller_run(float *input_pose, float *output_linear_velocity, float *output_angular_velocity);

int controller_start(enum CONTROLLER_TYPE controller_type, float *new_goal);
#endif // CONTROLLER_H