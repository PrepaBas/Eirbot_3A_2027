################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/localisation/icm42688.c \
../Drivers/localisation/kalman_filter.c \
../Drivers/localisation/localisation.c 

OBJS += \
./Drivers/localisation/icm42688.o \
./Drivers/localisation/kalman_filter.o \
./Drivers/localisation/localisation.o 

C_DEPS += \
./Drivers/localisation/icm42688.d \
./Drivers/localisation/kalman_filter.d \
./Drivers/localisation/localisation.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/localisation/%.o Drivers/localisation/%.su Drivers/localisation/%.cyclo: ../Drivers/localisation/%.c Drivers/localisation/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H533xx -DSTM32_THREAD_SAFE_STRATEGY=2 -c -I../Core/Inc -I"/home/bastien/Documents/Eirbot_3A_2027/mcu_ws/Core/Startup" -I"/home/bastien/Documents/Eirbot_3A_2027/mcu_ws/Drivers/controller" -I"/home/bastien/Documents/Eirbot_3A_2027/mcu_ws/Drivers/localisation" -I"/home/bastien/Documents/Eirbot_3A_2027/mcu_ws/Drivers/arm_math/src" -I../Drivers/STM32H5xx_HAL_Driver/Inc -I"/home/bastien/Documents/Eirbot_3A_2027/mcu_ws" -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I../Core/ThreadSafe -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-localisation

clean-Drivers-2f-localisation:
	-$(RM) ./Drivers/localisation/icm42688.cyclo ./Drivers/localisation/icm42688.d ./Drivers/localisation/icm42688.o ./Drivers/localisation/icm42688.su ./Drivers/localisation/kalman_filter.cyclo ./Drivers/localisation/kalman_filter.d ./Drivers/localisation/kalman_filter.o ./Drivers/localisation/kalman_filter.su ./Drivers/localisation/localisation.cyclo ./Drivers/localisation/localisation.d ./Drivers/localisation/localisation.o ./Drivers/localisation/localisation.su

.PHONY: clean-Drivers-2f-localisation

