// C standard includes.
#include <stdio.h>
#include <unistd.h>

// STM32 includes
#include "usart.h"

// FreeRtos includes
#include "cmsis_os2.h"

// micro-ROS includes
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <uxr/client/transport.h>
#include <rmw_microxrcedds_c/config.h>
#include <rmw_microros/rmw_microros.h>

// message includes
#include "log_msg.h"
#include "pose_msg.h"
//#include "estop_msg.h"
//#include "srv_path_follow.h"

bool cubemx_transport_open(struct uxrCustomTransport * transport);
bool cubemx_transport_close(struct uxrCustomTransport * transport);
size_t cubemx_transport_write(struct uxrCustomTransport* transport, const uint8_t * buf, size_t len, uint8_t * err);
size_t cubemx_transport_read(struct uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* err);

void * microros_allocate(size_t size, void * state);
void microros_deallocate(void * pointer, void * state);
void * microros_reallocate(void * pointer, size_t size, void * state);
void * microros_zero_allocate(size_t number_of_elements, size_t size_of_element, void * state);

void ros2_com_task(){
// micro-ROS configuration

    rmw_uros_set_custom_transport(
      true,
      (void *) &huart2,
      cubemx_transport_open,
      cubemx_transport_close,
      cubemx_transport_write,
      cubemx_transport_read
    );

    // Retry until successfull connection
    while (rmw_uros_ping_agent(100, 1) != RMW_RET_OK)
    {
        osDelay(100);
    }

    /*
    rcl_allocator_t allocator = rcutils_get_zero_initialized_allocator();
    allocator.allocate = microros_allocate;
    allocator.deallocate = microros_deallocate;
    allocator.reallocate = microros_reallocate;
    allocator.zero_allocate =  microros_zero_allocate;
    */
   rcl_allocator_t allocator = rcl_get_default_allocator();

    rclc_support_t support;
    rcl_node_t node;
    rclc_support_init(&support, 0, NULL, &allocator);
    rclc_node_init_default(&node, "mcu_node", "", &support);

    // sync with PC
    const int sync_timeout_ms = 1500;
    rmw_uros_sync_session(sync_timeout_ms);


    ///* INIT functions of various msg. */// 
    log_msg_init(&node);
    pose_msg_init(&node, &support, 100);

    // Executor definition
    rclc_executor_t executor;
    unsigned int num_handles = 2; 
    rclc_executor_init(&executor, &support.context, num_handles, &allocator);

    
    ///* REGISTER functions of various msg *///
    pose_msg_register_executor(&executor);


    // loop and end
    while(1){
        rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
        osDelay(10);
    }

    // Clean up stack
    rclc_executor_fini(&executor);
    rcl_node_fini(&node);
    rclc_support_fini(&support);
    log_msg_fini(&node);
}