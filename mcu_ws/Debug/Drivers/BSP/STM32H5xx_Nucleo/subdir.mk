################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/STM32H5xx_Nucleo/stm32h5xx_nucleo.c \
../Drivers/BSP/STM32H5xx_Nucleo/stm32h5xx_nucleo_usbpd_pwr.c 

OBJS += \
./Drivers/BSP/STM32H5xx_Nucleo/stm32h5xx_nucleo.o \
./Drivers/BSP/STM32H5xx_Nucleo/stm32h5xx_nucleo_usbpd_pwr.o 

C_DEPS += \
./Drivers/BSP/STM32H5xx_Nucleo/stm32h5xx_nucleo.d \
./Drivers/BSP/STM32H5xx_Nucleo/stm32h5xx_nucleo_usbpd_pwr.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/STM32H5xx_Nucleo/%.o Drivers/BSP/STM32H5xx_Nucleo/%.su Drivers/BSP/STM32H5xx_Nucleo/%.cyclo: ../Drivers/BSP/STM32H5xx_Nucleo/%.c Drivers/BSP/STM32H5xx_Nucleo/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H533xx -c -I../Core/Inc -I"/home/bastien/Documents/robot_mcu/Drivers/CMSIS/DSP/Include/dsp" -I../Drivers/CMSIS/DSP/Include/ -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-STM32H5xx_Nucleo

clean-Drivers-2f-BSP-2f-STM32H5xx_Nucleo:
	-$(RM) ./Drivers/BSP/STM32H5xx_Nucleo/stm32h5xx_nucleo.cyclo ./Drivers/BSP/STM32H5xx_Nucleo/stm32h5xx_nucleo.d ./Drivers/BSP/STM32H5xx_Nucleo/stm32h5xx_nucleo.o ./Drivers/BSP/STM32H5xx_Nucleo/stm32h5xx_nucleo.su ./Drivers/BSP/STM32H5xx_Nucleo/stm32h5xx_nucleo_usbpd_pwr.cyclo ./Drivers/BSP/STM32H5xx_Nucleo/stm32h5xx_nucleo_usbpd_pwr.d ./Drivers/BSP/STM32H5xx_Nucleo/stm32h5xx_nucleo_usbpd_pwr.o ./Drivers/BSP/STM32H5xx_Nucleo/stm32h5xx_nucleo_usbpd_pwr.su

.PHONY: clean-Drivers-2f-BSP-2f-STM32H5xx_Nucleo

