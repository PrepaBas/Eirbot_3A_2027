#ifndef POSE_MSG_H
#define POSE_MSG_H

#include <geometry_msgs/msg/pose_stamped.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <rmw_microxrcedds_c/config.h>

// void pose_timer_callback(rcl_timer_t * timer, int64_t last_call_time);

rcl_ret_t pose_msg_init(rcl_node_t *node, rclc_support_t *support, unsigned int period_ms);

rcl_ret_t pose_msg_register_executor(rclc_executor_t *executor);


#endif // POSE_MSG_H
