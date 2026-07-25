#include <uxr/client/transport.h>
#include <rmw_microxrcedds_c/config.h>

#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifdef RMW_UXRCE_TRANSPORT_CUSTOM

#define UART_DMA_BUFFER_SIZE 2048

// Align DMA buffer to 4 bytes for Cortex-M33 bus alignment
__attribute__((aligned(4))) static uint8_t dma_buffer[UART_DMA_BUFFER_SIZE];
static size_t dma_head = 0, dma_tail = 0;

extern DMA_HandleTypeDef handle_GPDMA1_Channel0; 

bool cubemx_transport_open(struct uxrCustomTransport * transport){
    UART_HandleTypeDef * uart = (UART_HandleTypeDef*) transport->args;
    
    dma_head = 0;
    dma_tail = 0;
    
    // Stop any existing DMA transfer first to reset channel registers cleanly
    HAL_UART_DMAStop(uart);

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

    if (buf == NULL || len == 0) {
        *err = 1;
        return 0;
    }

    // Safety timeout calculation based on baudrate length
    HAL_StatusTypeDef ret = HAL_UART_Transmit(uart, buf, (uint16_t)len, 100);
    
    if (ret == HAL_OK) {
        *err = 0;
        return len;
    } else {
        *err = 1;
        return 0;
    }
}

size_t cubemx_transport_read(struct uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* err){
    UART_HandleTypeDef * uart = (UART_HandleTypeDef*) transport->args;

    if (buf == NULL || len == 0) {
        *err = 1;
        return 0;
    }

    int ms_used = 0;
    do
    {
        // Check if GPDMA has stopped/completed in Normal Mode and auto-restart
        if (handle_GPDMA1_Channel0.State != HAL_DMA_STATE_BUSY) {
            HAL_UART_Receive_DMA(uart, dma_buffer, UART_DMA_BUFFER_SIZE);
        }

        // Query GPDMA transfer counter safely
        uint32_t remaining = __HAL_DMA_GET_COUNTER(&handle_GPDMA1_Channel0); 
        dma_tail = UART_DMA_BUFFER_SIZE - remaining;

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
    
    *err = 0;
    return wrote;
}

#endif //RMW_UXRCE_TRANSPORT_CUSTOM