// C standard includes.
#include <stdio.h>
#include <math.h>
#include <unistd.h>

// micro-ROS includes
#include <geometry_msgs/msg/pose_stamped.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <rmw_microxrcedds_c/config.h>
#include <rmw_microros/rmw_microros.h>
#include <rosidl_runtime_c/string_functions.h>

#include "pose_msg.h"

#include "kalman_filter.h"


typedef struct {
    geometry_msgs__msg__PoseStamped ros_msg;
    rcl_publisher_t publisher;
    rcl_timer_t timer;
} pose_msg_t;

pose_msg_t pose_msg;

void pose_timer_callback(rcl_timer_t * timer, int64_t last_call_time){
    (void) last_call_time;
    (void) timer;

    // 1. Synchronized real-world clock timestamping
    int64_t time_ns = rmw_uros_epoch_nanos(); 
    pose_msg.ros_msg.header.stamp.sec = time_ns / 1000000000LL;
    pose_msg.ros_msg.header.stamp.nanosec = time_ns % 1000000000LL;
    
    pose_msg.ros_msg.pose.position.x = kalman_get_x();
    pose_msg.ros_msg.pose.position.y = kalman_get_y();
    pose_msg.ros_msg.pose.position.z = 0;
    
    float yaw = kalman_get_theta();
    pose_msg.ros_msg.pose.orientation.x = 0.0f;
    pose_msg.ros_msg.pose.orientation.y = 0.0f;
    pose_msg.ros_msg.pose.orientation.z = sinf(yaw / 2.0f);
    pose_msg.ros_msg.pose.orientation.w = cosf(yaw / 2.0f);

    // 4. Ship the data over USB
    rcl_publish(&pose_msg.publisher, &pose_msg.ros_msg, NULL);
}



rcl_ret_t pose_msg_init(rcl_node_t *node, rclc_support_t *support, unsigned int period_ms) {
    geometry_msgs__msg__PoseStamped__init(&pose_msg.ros_msg);

    // set frame of msg header
    rosidl_runtime_c__String__assign(&pose_msg.ros_msg.header.frame_id, "odom");

    rclc_publisher_init_default(
        &pose_msg.publisher, 
        node, 
        ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, PoseStamped),
        "robot_pose"
    );

    return rclc_timer_init_default(
        &pose_msg.timer, 
        support, 
        RCL_MS_TO_NS(period_ms), 
        pose_timer_callback
    );
}

rcl_ret_t pose_msg_register_executor(rclc_executor_t *executor) {
    return rclc_executor_add_timer(executor, &pose_msg.timer);
}