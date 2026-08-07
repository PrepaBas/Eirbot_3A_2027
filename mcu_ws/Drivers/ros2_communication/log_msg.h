#ifndef LOG_MSG_H
#define LOG_MSG_H

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/string.h>

/**
 * @brief Initializes the log string publisher.
 * 
 * @param node Pointer to initialized rcl_node_t.
 * @return rcl_ret_t RCL_RET_OK on success.
 */
rcl_ret_t log_msg_init(rcl_node_t *node);

/**
 * @brief Cleans up message memory and destroys the LOG publisher.
 * 
 * @param node Pointer to rcl_node_t used during initialization.
 * @return rcl_ret_t RCL_RET_OK on success.
 */
rcl_ret_t log_msg_fini(rcl_node_t *node);

/**
 * @brief Thread-safe function to format and publish a string on topic "mcu_log".
 * 
 * @param format Printf style format string.
 * @param ... Variable arguments.
 */
void publish_log_msg(const char * format, ...);



#endif // LOG_MSG_H