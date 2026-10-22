################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/CMSIS/DSP/Examples/ARM/arm_fir_example/RTE/Device/ARMCM4_FP/startup_ARMCM4.c \
../Drivers/CMSIS/DSP/Examples/ARM/arm_fir_example/RTE/Device/ARMCM4_FP/system_ARMCM4.c 

OBJS += \
./Drivers/CMSIS/DSP/Examples/ARM/arm_fir_example/RTE/Device/ARMCM4_FP/startup_ARMCM4.o \
./Drivers/CMSIS/DSP/Examples/ARM/arm_fir_example/RTE/Device/ARMCM4_FP/system_ARMCM4.o 

C_DEPS += \
./Drivers/CMSIS/DSP/Examples/ARM/arm_fir_example/RTE/Device/ARMCM4_FP/startup_ARMCM4.d \
./Drivers/CMSIS/DSP/Examples/ARM/arm_fir_example/RTE/Device/ARMCM4_FP/system_ARMCM4.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/CMSIS/DSP/Examples/ARM/arm_fir_example/RTE/Device/ARMCM4_FP/%.o Drivers/CMSIS/DSP/Examples/ARM/arm_fir_example/RTE/Device/ARMCM4_FP/%.su Drivers/CMSIS/DSP/Examples/ARM/arm_fir_example/RTE/Device/ARMCM4_FP/%.cyclo: ../Drivers/CMSIS/DSP/Examples/ARM/arm_fir_example/RTE/Device/ARMCM4_FP/%.c Drivers/CMSIS/DSP/Examples/ARM/arm_fir_example/RTE/Device/ARMCM4_FP/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H533xx -c -I../Core/Inc -I"/home/bastien/Documents/robot_mcu/Drivers/CMSIS/DSP/Include/dsp" -I../Drivers/CMSIS/DSP/Include/ -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-CMSIS-2f-DSP-2f-Examples-2f-ARM-2f-arm_fir_example-2f-RTE-2f-Device-2f-ARMCM4_FP

clean-Drivers-2f-CMSIS-2f-DSP-2f-Examples-2f-ARM-2f-arm_fir_example-2f-RTE-2f-Device-2f-ARMCM4_FP:
	-$(RM) ./Drivers/CMSIS/DSP/Examples/ARM/arm_fir_example/RTE/Device/ARMCM4_FP/startup_ARMCM4.cyclo ./Drivers/CMSIS/DSP/Examples/ARM/arm_fir_example/RTE/Device/ARMCM4_FP/startup_ARMCM4.d ./Drivers/CMSIS/DSP/Examples/ARM/arm_fir_example/RTE/Device/ARMCM4_FP/startup_ARMCM4.o ./Drivers/CMSIS/DSP/Examples/ARM/arm_fir_example/RTE/Device/ARMCM4_FP/startup_ARMCM4.su ./Drivers/CMSIS/DSP/Examples/ARM/arm_fir_example/RTE/Device/ARMCM4_FP/system_ARMCM4.cyclo ./Drivers/CMSIS/DSP/Examples/ARM/arm_fir_example/RTE/Device/ARMCM4_FP/system_ARMCM4.d ./Drivers/CMSIS/DSP/Examples/ARM/arm_fir_example/RTE/Device/ARMCM4_FP/system_ARMCM4.o ./Drivers/CMSIS/DSP/Examples/ARM/arm_fir_example/RTE/Device/ARMCM4_FP/system_ARMCM4.su

.PHONY: clean-Drivers-2f-CMSIS-2f-DSP-2f-Examples-2f-ARM-2f-arm_fir_example-2f-RTE-2f-Device-2f-ARMCM4_FP

