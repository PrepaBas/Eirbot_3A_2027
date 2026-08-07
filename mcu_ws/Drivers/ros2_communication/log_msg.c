// C standard includes.
#include <stdio.h>
#include <unistd.h>

// micro-ROS includes
#include <std_msgs/msg/string.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <rmw_microxrcedds_c/config.h>
#include <rmw_microros/rmw_microros.h>
#include <rosidl_runtime_c/string_functions.h>

#include "log_msg.h"

#include "params.h"

#define DEBUG_BUFFER_SIZE 128

typedef struct {
    std_msgs__msg__String ros_msg;
    rcl_publisher_t publisher;
    int is_initialized;
} log_msg_t;

static log_msg_t log_msg = {0};

rcl_ret_t log_msg_init(rcl_node_t *node) {
    std_msgs__msg__String__init(&log_msg.ros_msg);

    if (!rosidl_runtime_c__String__assignn(&log_msg.ros_msg.data, "", DEBUG_BUFFER_SIZE)) {
        return RCL_RET_ERROR;
    }

    rcl_ret_t ret = rclc_publisher_init_default(
        &log_msg.publisher,
        node, 
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
        "mcu_log"
    );


    if(ret == RCL_RET_OK) {
        log_msg.is_initialized = true;
    }
    return ret;
}

void publish_log_msg(const char * format, ...) {
    if (!log_msg.is_initialized) {
        return;
    }

    va_list args;
    va_start(args, format);
    int len = vsnprintf(log_msg.ros_msg.data.data, DEBUG_BUFFER_SIZE, format, args);
    va_end(args);

    rcl_publish(&log_msg.publisher, &log_msg.ros_msg, NULL);
}

rcl_ret_t log_msg_fini(rcl_node_t *node) {
    log_msg.is_initialized = false;
    std_msgs__msg__String__fini(&log_msg.ros_msg);
    return rcl_publisher_fini(&log_msg.publisher, node);
}