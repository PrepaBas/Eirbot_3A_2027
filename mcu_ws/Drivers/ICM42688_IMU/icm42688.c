#include "icm42688.h"

// --- PRIVATE HELPER FUNCTIONS ---

static void CS_Select(icm42688_t *dev) {
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_RESET);
}

static void CS_Deselect(icm42688_t *dev) {
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_SET);
}

// Low-level blocking write used ONLY during initialization setup
static void write_reg(icm42688_t *dev, uint8_t reg, uint8_t val) {
    uint8_t tx[2] = { reg & 0x7F, val }; // Write operation requires MSB = 0
    CS_Select(dev);
    HAL_SPI_Transmit(dev->hspi, tx, 2, HAL_MAX_DELAY);
    CS_Deselect(dev);
    vTaskDelay(pdMS_TO_TICKS(1)); // Yield to FreeRTOS to prevent blocking other tasks
}

// --- PUBLIC DRIVER FUNCTIONS ---

/**
 * @brief  Initializes the ICM42688 hardware configurations.
 * Sets Accel to ±4g (1kHz) and Gyro to ±2000dps (1kHz).
 */
uint8_t icm42688_init(icm42688_t *dev, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin) {
    dev->hspi = hspi;
    dev->cs_port = cs_port;
    dev->cs_pin = cs_pin;
    dev->notified_task_handle = NULL;

    // 1. Reset device settings
    write_reg(dev, REG_DEVICE_CONFIG, 0x01);
    vTaskDelay(pdMS_TO_TICKS(10)); // Time for chip to reboot

    // 2. Enable Accel & Gyro in Low Noise (LN) Mode
    write_reg(dev, REG_PWR_MGMT0, 0x0F); 
    vTaskDelay(pdMS_TO_TICKS(50)); // Time for internal analog components to stabilize

    // 3. Set Scales and ODR (Output Data Rate)
    // Accel Config: 1 kHz ODR (0x06), Full Scale ±4g (0x02 << 5)
    write_reg(dev, REG_ACCEL_CONFIG0, 0x06 | (0x02 << 5)); 
    // Gyro Config: 1 kHz ODR (0x06), Full Scale ±2000dps (0x00 << 5)
    write_reg(dev, REG_GYRO_CONFIG0, 0x06 | (0x00 << 5));

    // 4. Set Hardware Interrupts
    write_reg(dev, REG_INT_CONFIG, 0x00);   // Push-pull, Active Low, Pulsed mode
    write_reg(dev, REG_INT_SOURCE0, 0x08);  // Map Data Ready (DRDY) to physical INT1 pin

    return 0; // Success
}

/**
 * @brief  Triggers an asynchronous SPI DMA read sequence across 12 bytes of sensor data.
 * Returns immediately without wasting CPU instruction cycles.
 */
void icm42688_start_dma_read(icm42688_t *dev) {
    dev->tx_buf[0] = REG_ACCEL_DATA_X1 | 0x80; // Read operation requires MSB = 1
    
    CS_Select(dev);
    // Transmit command and receive data buffer concurrently using DMA hardware channel
    HAL_SPI_TransmitReceive_DMA(dev->hspi, dev->tx_buf, dev->rx_buf, 13);
}

/**
 * @brief  Converts raw binary SPI buffers into true floating point engineering values.
 */
void icm42688_parse_data(icm42688_t *dev) {
    // Reconstruct 16-bit signed integers (offset by 1 due to SPI command phase byte)
    int16_t raw_ax = (int16_t)((dev->rx_buf[1] << 8) | dev->rx_buf[2]);
    int16_t raw_ay = (int16_t)((dev->rx_buf[3] << 8) | dev->rx_buf[4]);
    int16_t raw_az = (int16_t)((dev->rx_buf[5] << 8) | dev->rx_buf[6]);
    
    int16_t raw_gx = (int16_t)((dev->rx_buf[7] << 8) | dev->rx_buf[8]);
    int16_t raw_gy = (int16_t)((dev->rx_buf[9] << 8) | dev->rx_buf[10]);
    int16_t raw_gz = (int16_t)((dev->rx_buf[11] << 8) | dev->rx_buf[12]);

    // Apply standard conversions based on hardware resolution limits:
    // LSB sensitivity at ±4g full scale = 8192 LSB/g
    dev->ax = (float)raw_ax / 8192.0f;
    dev->ay = (float)raw_ay / 8192.0f;
    dev->az = (float)raw_az / 8192.0f;
    
    // LSB sensitivity at ±2000dps full scale = 16.4 LSB/dps
    dev->gx = (float)raw_gx / 16.4f;
    dev->gy = (float)raw_gy / 16.4f;
    dev->gz = (float)raw_gz / 16.4f;
}


// extern icm42688_t imu; // Initialized peripheral structure
QueueHandle_t imu_queue_handle = NULL;

void IMU_Task(void *argument) {
    // 1. Create the FreeRTOS Queue (Holds up to 3 data packets)
    imu_queue_handle = xQueueCreate(3, sizeof(imu_data_t));
    
    // 2. Initialize the physical ICM42688 sensor
    imu.notified_task_handle = xTaskGetCurrentTaskHandle();
    icm42688_init(&imu, &hspi1, imu_CS_GPIO_Port, imu_CS_Pin); 

    // Accumulation variables
    float accel_sum_x = 0, accel_sum_y = 0, accel_sum_z = 0;
    float gyro_sum_x  = 0, gyro_sum_y  = 0, gyro_sum_z  = 0;
    uint8_t sample_count = 0;

    for (;;) {
        // Step A: Wait for IMU Hardware Data Ready Interrupt (INT1)
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        // Step B: Fire off asynchronous SPI DMA transfer (Instantaneous)
        icm42688_start_dma_read(&imu);
        
        // Step C: Sleep while DMA hardware shifts data into RAM
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        // Step D: Parse bytes into floating point values
        icm42688_parse_data(&imu);
        
        // Step E: Lightweight Accumulation (Filtering)
        accel_sum_x += imu.ax;
        accel_sum_y += imu.ay;
        accel_sum_z += imu.az;
        
        // Integration: multiply by dt (1ms = 0.001s)
        gyro_sum_x  += imu.gx * 0.001f; 
        gyro_sum_y  += imu.gy * 0.001f;
        gyro_sum_z  += imu.gz * 0.001f;
        
        sample_count++;
        
        // Step F: Hit 5 samples? Send downsampled 200 Hz package to Kalman Filter
        if (sample_count >= 5) {
            imu_data_t package;
            
            // Average acceleration over 5ms window
            package.ax = accel_sum_x / 5.0f;
            package.ay = accel_sum_y / 5.0f;
            package.az = accel_sum_z / 5.0f;
            
            // Accumulated total delta-angle over 5ms window
            package.delta_theta_x = gyro_sum_x;
            package.delta_theta_y = gyro_sum_y;
            package.delta_theta_z = gyro_sum_z;
            
            // Post to queue without blocking (Timeout = 0)
            xQueueSend(imu_queue_handle, &package, 0);
            
            // Reset accumulators for next 5ms cycle
            accel_sum_x = accel_sum_y = accel_sum_z = 0.0f;
            gyro_sum_x  = gyro_sum_y  = gyro_sum_z  = 0.0f;
            sample_count = 0;
        }
    }
}