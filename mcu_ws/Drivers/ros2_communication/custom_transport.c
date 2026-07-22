#include <uxr/client/transport.h>
#include <rmw_microxrcedds_c/config.h>

#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifdef RMW_UXRCE_TRANSPORT_CUSTOM

// --- micro-ROS Transports ---
#define UART_DMA_BUFFER_SIZE 2048

static uint8_t dma_buffer[UART_DMA_BUFFER_SIZE];
static size_t dma_head = 0, dma_tail = 0;

// Export the GPDMA handles initialized in main.c / gpdma.c
// Check your main.c file to ensure your RX channel matches Channel 0 or Channel 1!
extern DMA_HandleTypeDef handle_GPDMA1_Channel0; 

bool cubemx_transport_open(struct uxrCustomTransport * transport){
    UART_HandleTypeDef * uart = (UART_HandleTypeDef*) transport->args;
    
    // Clear head and tail tracking pointers on start
    dma_head = 0;
    dma_tail = 0;
    
    // Start GPDMA reception using the modern HAL driver layout
    HAL_StatusTypeDef ret = HAL_UART_Receive_DMA(uart, dma_buffer, UART_DMA_BUFFER_SIZE);
    return (ret == HAL_OK);
}

bool cubemx_transport_close(struct uxrCustomTransport * transport){
    UART_HandleTypeDef * uart = (UART_HandleTypeDef*) transport->args;
    HAL_UART_DMAStop(uart);
    return true;
}

size_t cubemx_transport_write(struct uxrCustomTransport* transport, uint8_t * buf, size_t len, uint8_t * err){
    UART_HandleTypeDef * uart = (UART_HandleTypeDef*) transport->args;

    // For writing, using polling mode is dramatically safer for micro-ROS 
    // serialization loops because it eliminates DMA resource locks.
    // 100ms timeout is plenty for serial arrays.
    HAL_StatusTypeDef ret = HAL_UART_Transmit(uart, buf, len, 100);
    
    return (ret == HAL_OK) ? len : 0;
}

size_t cubemx_transport_read(struct uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* err){
    UART_HandleTypeDef * uart = (UART_HandleTypeDef*) transport->args;

    int ms_used = 0;
    do
    {
        // STM32H5 GPDMA FIX: 
        // We query the actual global GPDMA handle instead of the legacy uart->hdmarx wrapper structure.
        // __HAL_DMA_GET_COUNTER returns the remaining bytes to transfer in the current block (BNDTR).
        __disable_irq();
        uint32_t remaining = __HAL_DMA_GET_COUNTER(&handle_GPDMA1_Channel0); 
        dma_tail = UART_DMA_BUFFER_SIZE - remaining;
        __enable_irq();

        if (dma_head != dma_tail) {
            break;
        }

        osDelay(1);
        ms_used++;
    } while (ms_used < timeout);
    
    size_t wrote = 0;
    while ((dma_head != dma_tail) && (wrote < len)){
        buf[wrote] = dma_buffer[dma_head];
        dma_head = (dma_head + 1) % UART_DMA_BUFFER_SIZE;
        wrote++;
    }
    
    return wrote;
}

#endif //RMW_UXRCE_TRANSPORT_CUSTOM