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
// std includes
#include <stdio.h>

// Stm32 includes
#include "stm32h5xx_hal_conf.h"
#include "usart.h"
#include "spi.h"
#include "tim.h"

// Driver includes
#include "params.h"
#include "icm42688.h"
#include "kalman_filter.h"
#include "ros2_communication.h"
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
  .stack_size = 16384 * 4
};
/* Definitions for motorTask */
osThreadId_t motorTaskHandle;
const osThreadAttr_t motorTask_attributes = {
  .name = "motorTask",
  .priority = (osPriority_t) osPriorityRealtime,
  .stack_size = 1000 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void ledTask(void *argument);
/* USER CODE END FunctionPrototypes */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
#ifdef USE_SERIAL_PRINTS
    huart2.Init.BaudRate = 115200;
    
    huart2.hdmatx = NULL; // <--- Disables DMA association so HAL_UART_Transmit works!
    HAL_UART_Init(&huart2);
    printf("tedjfsl\r\n");
#endif
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
#ifdef USE_UROS
    ros2_com_task();
#endif
  for(;;)
  {
    osDelay(1000);
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

  // Kalman filter Initialization
  LOGPRINT("test print\r\n"); 
  kalman_init();
  
  // IMU Initialization
  icm42688_t imu;
  icm42688_init(&imu, &hspi2, cs_imu_GPIO_Port, cs_imu_Pin); // CS: PB10 | SCK:PB2 | MOSI:PC1 | MISO:PC2

  // Encoder Initialization
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL); // PA15 & PB3
  HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_ALL); // PA0 & PA1

  float pose[3] = {0.0f, 0.0f, 0.0f};
  uint8_t enable_encoder = USE_ENCODERS;
  uint8_t enable_imu = USE_IMU;
  int32_t encoder1_count = 0;
  int32_t encoder2_count = 0;
  int32_t encoder1_delta = 0;
  int32_t encoder2_delta = 0;  
  float gz = 0.0f;
  float vr = 0.0f;
  float vl = 0.0f;


 
  /* Infinite loop */
  for(;;)
  {

    if(ulTaskNotifyTake( pdTRUE, osWaitForever )) {} // block until notified by timer interrupt every 1ms

    //* Read encoder counts *//
    if(enable_encoder)
    {
      int32_t old_encoder1_count = encoder1_count;
      int32_t old_encoder2_count = encoder2_count;
      encoder1_count = (int32_t)__HAL_TIM_GET_COUNTER(&htim2); // read htim2->COUNT and cast into int32_t to have center value
      encoder2_count = (int32_t)__HAL_TIM_GET_COUNTER(&htim5);
      encoder1_delta = encoder1_count - old_encoder1_count;
      encoder2_delta = encoder2_count - old_encoder2_count;
    }

    if(enable_imu)
    {
        //* Read IMU data *//
        // Get sensor data
        icm42688_start_dma_read(&imu); // start DMA read of IMU data. Interrupt will trigger when data is ready
        if(ulTaskNotifyTake( pdTRUE, 2 )) { // block until notified by dma interrupt when IMU data is ready
          icm42688_parse_data(&imu);
          int16_t raw_gz = (int16_t)((imu.rx_buf[11] << 8) | imu.rx_buf[12]);
          // LSB sensitivity at ±2000dps full scale = 16.4 LSB/dps
          gz = (float)raw_gz / 16.4f;
        } 
        else { enable_imu = 0;} // no imu data   
    }
    
    if(!enable_encoder && !enable_imu)
    {
      vr = 0.5f;
      vl = 0.0f; // TODO get motor speeds
    }

    if(enable_imu && enable_encoder){
      float d = (encoder1_delta - encoder2_delta) * 3.14f * 0.007f;  // d = 2 * pi * r
      kalman_predict_w_sensor(d, gz);
    }
    else if(enable_encoder){
      float d = (encoder1_delta - encoder2_delta) * 3.14f * 0.007f;
      float w = 2.0f * (encoder2_delta - encoder1_delta) /  0.015 * 3.14f * 0.007f;   
      kalman_predict_w_sensor(d, w);
    }
    else{
      kalman_predict_w_model(vl, vr);
    }

    kalman_get_pose(pose);
  }
  /* USER CODE END motorTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void ledTask(void *argument)
{
  for(;;)
  {
    HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
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

int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

int __io_getchar(void) {
    uint8_t ch = 0;
    HAL_UART_Receive(&huart2, &ch, 1, HAL_MAX_DELAY);
    return ch;
}

/* USER CODE END Application */

