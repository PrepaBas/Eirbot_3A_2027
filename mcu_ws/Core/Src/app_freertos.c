/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
// Stm32 includes
#include "stm32h5xx_hal_conf.h"
#include "usart.h"
#include "spi.h"
#include "tim.h"

// micro-ROS includes
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <uxr/client/transport.h>
#include <rmw_microxrcedds_c/config.h>
#include <rmw_microros/rmw_microros.h>

#include <std_msgs/msg/int32.h>

//#include "arm_math.h"

// Driver includes
#include "icm42688.h"
// #include "kalman_filter.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osThreadId_t led_TaskHandle;
const osThreadAttr_t led_Task_attributes = {
  .name = "led_Task",
  .priority = (osPriority_t) 25,
  .stack_size = 400 * 4
};
/* USER CODE END Variables */
/* Definitions for uRosTask */
osThreadId_t uRosTaskHandle;
const osThreadAttr_t uRosTask_attributes = {
  .name = "uRosTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 4000
};
/* Definitions for motorTask */
osThreadId_t motorTaskHandle;
const osThreadAttr_t motorTask_attributes = {
  .name = "motorTask",
  .priority = (osPriority_t) osPriorityRealtime,
  .stack_size = 2000 
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
bool cubemx_transport_open(struct uxrCustomTransport * transport);
bool cubemx_transport_close(struct uxrCustomTransport * transport);
size_t cubemx_transport_write(struct uxrCustomTransport* transport, const uint8_t * buf, size_t len, uint8_t * err);
size_t cubemx_transport_read(struct uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* err);

void * microros_allocate(size_t size, void * state);
void microros_deallocate(void * pointer, void * state);
void * microros_reallocate(void * pointer, size_t size, void * state);
void * microros_zero_allocate(size_t number_of_elements, size_t size_of_element, void * state);


void ledTask(void *argument);
/* USER CODE END FunctionPrototypes */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of uRosTask */
  uRosTaskHandle = osThreadNew(StartURosTask, NULL, &uRosTask_attributes);

  /* creation of motorTask */
  motorTaskHandle = osThreadNew(StartMotorTask, NULL, &motorTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  led_TaskHandle = osThreadNew(ledTask, NULL, &led_Task_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}
/* USER CODE BEGIN Header_StartURosTask */
/**
* @brief Function implementing the uRosTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartURosTask */
void StartURosTask(void *argument)
{
  /* USER CODE BEGIN uRosTask */
  /* Infinite loop */
  // micro-ROS configuration
    rmw_uros_set_custom_transport(
      true,
      (void *) &huart2,
      cubemx_transport_open,
      cubemx_transport_close,
      cubemx_transport_write,
      cubemx_transport_read);

    rcl_allocator_t freeRTOS_allocator = rcutils_get_zero_initialized_allocator();
    freeRTOS_allocator.allocate = microros_allocate;
    freeRTOS_allocator.deallocate = microros_deallocate;
    freeRTOS_allocator.reallocate = microros_reallocate;
    freeRTOS_allocator.zero_allocate =  microros_zero_allocate;

    if (!rcutils_set_default_allocator(&freeRTOS_allocator)) {
        printf("Error on default allocators (line %d)\n", __LINE__);
    }

    // micro-ROS app

    rcl_publisher_t publisher;
    std_msgs__msg__Int32 msg;
    rclc_support_t support;
    rcl_allocator_t allocator;
    rcl_node_t node;

    allocator = rcl_get_default_allocator();

    //create init_options
    rclc_support_init(&support, 0, NULL, &allocator);

    // create node
    rclc_node_init_default(&node, "cubemx_node", "", &support);

    // create publisher
    rclc_publisher_init_default(
      &publisher,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
      "cubemx_publisher");

    msg.data = 0;

    for(;;)
    {
      rcl_ret_t ret = rcl_publish(&publisher, &msg, NULL);
      if (ret != RCL_RET_OK)
      {
        printf("Error publishing (line %d)\n", __LINE__);
      }

      msg.data++;
      osDelay(10);
    }
  /* USER CODE END uRosTask */
}

/* USER CODE BEGIN Header_StartMotorTask */
/**
* @brief Function implementing the motorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMotorTask */
void StartMotorTask(void *argument)
{
  /* USER CODE BEGIN motorTask */

  // IMU Initialization
  //icm42688_t imu;
  //icm42688_init(&imu, &hspi2, cs_imu_GPIO_Port, cs_imu_Pin); // CS: PB10 | SCK:PB2 | MOSI:PC1 | MISO:PC2

  // Encoder Initialization
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL); // PA15 & PB3
  while(1){osDelay(1000);}
  HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_ALL); // PA0 & PA1

  uint8_t enable_encoder = 0;
  uint8_t enable_imu = 0;
  int32_t encoder1_count = 0;
  int32_t encoder2_count = 0;
  float gz = 0.0f;
  float vr = 0.0f;
  float vl = 0.0f;

  float x = 0.0f;
  float y = 0.0f;
  float theta = 0.0f;

 
  /* Infinite loop */
  for(;;)
  {

    //* Read encoder counts *//
    if(enable_encoder)
    {
        encoder1_count = (int32_t)__HAL_TIM_GET_COUNTER(&htim2); // read htim2->COUNT and cast into int32_t to have center value
        encoder2_count = (int32_t)__HAL_TIM_GET_COUNTER(&htim5);
    }

    if(!enable_encoder && !enable_imu)
    {
        vr = 0.0f;
        vl = 0.0f; // TODO get motor speed
    }

    if(enable_imu)
    {
        //* Read IMU data *//
        if(ulTaskNotifyTake( pdTRUE, osWaitForever )) {} // block until notified by timer interrupt every 1ms
        // Get sensor data
        //icm42688_start_dma_read(&imu); // start DMA read of IMU data. Interrupt will trigger when data is ready
        if(ulTaskNotifyTake( pdTRUE, osWaitForever )) {} // block until notified by dma interrupt when IMU data is ready
        //icm42688_parse_data(&imu);
        //int16_t raw_gz = (int16_t)((imu.rx_buf[11] << 8) | imu.rx_buf[12]);
        // LSB sensitivity at ±2000dps full scale = 16.4 LSB/dps
        //gz = (float)raw_gz / 16.4f;
    }


    osDelay(100);
  }
  /* USER CODE END motorTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void ledTask(void *argument)
{
  for(;;)
  {
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    osDelay(200);
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6) 
    {
      // Notify motorTask that timer is elapsed
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;    
      vTaskNotifyGiveFromISR(motorTaskHandle, &xHigherPriorityTaskWoken); // Notifie la tâche motorTask    
      portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi == &hspi2) 
    { 
        // Notify motorTask that IMU data is ready
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;    
        vTaskNotifyGiveFromISR(motorTaskHandle, &xHigherPriorityTaskWoken); // Notifie la tâche motorTask    
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
}

/* USER CODE END Application */

