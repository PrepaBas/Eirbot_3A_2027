// IA Generatedd
#ifndef ICM42688_H_
#define ICM42688_H_

#include "stm32h5xx_hal.h" // Change this to your specific series (e.g., hal_g4xx, hal_h7xx)
#include "FreeRTOS.h"
#include "task.h"

// --- ICM42688 REGISTER MAP ---
#define REG_DEVICE_CONFIG  0x11
#define REG_PWR_MGMT0      0x4E
#define REG_ACCEL_CONFIG0  0x50
#define REG_GYRO_CONFIG0   0x4F
#define REG_INT_CONFIG     0x14
#define REG_INT_SOURCE0    0x65
#define REG_ACCEL_DATA_X1  0x1F // First byte of data (total 12 bytes sequential)

typedef struct {
    SPI_HandleTypeDef *hspi;    // Pointer to CubeMX configured SPI (e.g., &hspi1)
    GPIO_TypeDef      *cs_port; // CS Pin GPIO Port (e.g., GPIOB)
    uint16_t           cs_pin;  // CS Pin Number (e.g., GPIO_PIN_0)
    TaskHandle_t       notified_task_handle; // FreeRTOS task hook for DMA/EXTI
    
    // DMA Data Buffers (1 byte command + 12 bytes IMU data)
    uint8_t            rx_buf[13]; 
    uint8_t            tx_buf[13];
    
    // Parsed engineering units
    float ax, ay, az; // Accelerations in g
    float gx, gy, gz; // Angular velocities in dps (degrees per second)
} icm42688_t;

// --- CORE FUNCTION PROTOTYPES ---
uint8_t icm42688_init(icm42688_t *dev, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin);
void icm42688_start_dma_read(icm42688_t *dev);
void icm42688_parse_data(icm42688_t *dev);
/*
 *
 * use "imu_data_t imu_packet;
        if (xQueueReceive(imu_queue_handle, &imu_packet, portMAX_DELAY) == pdPASS) {
        " to receive data 
 *
 *
 * and use "extern icm42688_t imu;

    // 1. This triggers when the ICM42688 pin asserts (Data is Ready)
    void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
        if (GPIO_Pin == IMU_INT1_PIN) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            if (imu.notified_task_handle != NULL) {
                // Notify the IMU processing task that data is ready to be fetched
                vTaskNotifyGiveFromISR(imu.notified_task_handle, &xHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }
    }

    // 2. This triggers when the SPI DMA hardware finishes reading the 12 bytes
    void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
        if (hspi == imu.hspi) {
            // Deselect CS immediately
            HAL_GPIO_WritePin(imu.cs_port, imu.cs_pin, GPIO_PIN_SET);
            
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            if (imu.notified_task_handle != NULL) {
                // Notify the processing task that DMA transfer is complete
                vTaskNotifyGiveFromISR(imu.notified_task_handle, &xHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }
}" as ISR
*/
#endif