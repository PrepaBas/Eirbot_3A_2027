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
#include <stdint.h>

// Stm32 includes
#include "stm32h5xx_hal_conf.h"
#include "usart.h"
#include "spi.h"
#include "tim.h"

// Driver includes
#include "params.h"
#include "icm42688.h"
#include "kalman_filter.h"
#include "controller.h"
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
/* Definitions for ledTask */
osThreadId_t ledTaskHandle;
const osThreadAttr_t ledTask_attributes = {
  .name = "ledTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
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
  /* creation of ledTask */
  ledTaskHandle = osThreadNew(StartLedTask, NULL, &ledTask_attributes);

  /* creation of motorTask */
  motorTaskHandle = osThreadNew(StartMotorTask, NULL, &motorTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  led_TaskHandle = osThreadNew(ledTask, NULL, &led_Task_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}
/* USER CODE BEGIN Header_StartLedTask */
/**
* @brief Function implementing the ledTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLedTask */
void StartLedTask(void *argument)
{
  /* USER CODE BEGIN ledTask */
  /* Infinite loop */
  for(;;)
  {
	// while(1){osDelay(1000);}
    osDelay(100);
    float pose[3] = {};
	kalman_get_pose(pose);
	long int x = pose [0] * 1000;
	long int y = pose [1] * 1000;
	long int t = pose [2] * 1000;
	LOGPRINT(">x:%ld\n>y:%ld\n>theta:%ld\n>pose:%ld:%ld|xy", x, y, t, x, y);
  }
  /* USER CODE END ledTask */
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
  kalman_init(); // call before uRos start sending pose msg
  osDelay(1000);
  // Kalman filter Initialization
  LOGPRINT("MotorTask Start"); 
  // IMU Initialization
  icm42688_t imu;
  icm42688_init(&imu, &hspi2, cs_imu_GPIO_Port, cs_imu_Pin); // CS: PB10 | SCK:PB2 | MOSI:PC1 | MISO:PC2

  // Encoder Initialization
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_1); // PA15 & PB3
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_2); // PA15 & PB3
  HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_1); // PA0 & PA1
  HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_2); // PA0 & PA1
  HAL_TIM_Base_Start_IT(&htim6);

  float pose[3] = {0.0f, 0.0f, 0.0f};
  uint8_t enable_encoder = USE_ENCODERS;
  uint8_t enable_imu = USE_IMU;
  int32_t encoder1_count = 0;
  int32_t encoder2_count = 0;
  int32_t encoder1_delta = 0;
  int32_t encoder2_delta = 0;  
  float gz = 0.0f;
  float v = 0.0f; 
  float w = 0.0f;

  const int uart_downsample = 100;

  osDelay(1000);
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
    
    if((!enable_encoder) && (!enable_imu))
    {
      //wr = 0.5f;
      //wl = 0.0f; // TODO get motor speeds
    }
    
    if(enable_imu && enable_encoder){
      float d = (encoder1_delta - encoder2_delta) * PI * WHEEL_RADIUS; 
      kalman_predict_w_sensor(d, gz);
    }
    else if(enable_encoder){
    	float dl = 2.0f * (float)encoder1_delta / 4096.0f * PI * WHEEL_RADIUS / 4.0f;
    	float dr = 2.0f * (float)encoder2_delta / 4096.0f * PI * WHEEL_RADIUS / 4.0f;
        kalman_predict_w_encoders(dl, dr);
    }
    else{
      kalman_predict_w_model(v, w);
    }

    kalman_get_pose(pose); 
    //LOGPRINT("Pose: %d %d %d | Speed: %d %d", (int)pose[0], (int)pose[1], (int)pose[2], (int)vl, (int)vr); osDelay(100);
    //LOGPRINT("Pose: %f %f %f | Speed: %f %f", pose[0], pose[1], pose[2], vl, vr); 
    controller_run(pose, &v, &w);
    static int counter = 0;
    //make a square
    if(is_controller_free()){
      LOGPRINT("Free controller");
      switch(counter){
        case 0:
          LOGPRINT("First line");
          float new_goal[3] = {1.8f, 0.0f, PI/2.0f};
          controller_start(CONTROLLER_TYPE_GOTO_P, new_goal);
          break;
        case 1:
          LOGPRINT("First turn");
          float new_goal2[3] = {1.8f, 0.8f, PI};
          controller_start(CONTROLLER_TYPE_GOTO_P, new_goal2);
          break;
        case 2:
          LOGPRINT("Second line");
          float new_goal3[3] = {0.0f, 0.8f, -PI/2.0f};
          controller_start(CONTROLLER_TYPE_GOTO_P, new_goal3);
          break;
        case 3:
          LOGPRINT("Second turn");
          float new_goal4[3] = {0.0f, 0.0f, 0.0f};
          controller_start(CONTROLLER_TYPE_GOTO_P, new_goal4);
          break;
      }
	  counter++;
	  if(counter > 3) counter = 0;
    }

    static int uart_downsample_counter = 0;
    if (uart_downsample_counter++ > uart_downsample) {
    	uart_downsample_counter = 0;

    	int16_t vl_i16 = (int16_t)(100.0f * (v - w * 0.1475f / 2.0f ) / 0.03f / 6.28f);
    	int16_t vr_i16 = (int16_t)(100.0f * (v + w * 0.1475f / 2.0f ) / 0.03f / 6.28);
    	static uint8_t buf[6];
    	buf[0] = 0xAA;
    	buf[1] = vl_i16 & 0xFF;
    	buf[2] = (vl_i16 >> 8) & 0xFF;
    	buf[3] = vr_i16 & 0xFF;
    	buf[4] = (vr_i16 >> 8) & 0xFF;
    	buf[5] = buf[1] ^ buf[2] ^ buf[3] ^ buf[4];

    	// LOGPRINT("vl = %d, vr = %d\r\n", vl_i16, vr_i16);

    	HAL_UART_Transmit(&huart3, buf, sizeof(buf), 1);
    }
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


void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi == &hspi2) 
    { 
        // Notify motorTask that IMU data is ready
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;    
        vTaskNotifyGiveFromISR(motorTaskHandle, &xHigherPriorityTaskWoken); // Notifie la tâche motorTask    
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
}

#ifdef USE_SERIAL_PRINTS
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

int __io_getchar(void) {
    uint8_t ch = 0;
    HAL_UART_Receive(&huart2, &ch, 1, HAL_MAX_DELAY);
    return ch;
}
#endif

/* USER CODE END Application */

