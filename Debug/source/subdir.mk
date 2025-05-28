################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/cmsis_i2c_interrupt_transfer.c \
../source/mtb.c \
../source/semihost_hardfault.c 

C_DEPS += \
./source/cmsis_i2c_interrupt_transfer.d \
./source/mtb.d \
./source/semihost_hardfault.d 

OBJS += \
./source/cmsis_i2c_interrupt_transfer.o \
./source/mtb.o \
./source/semihost_hardfault.o 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c source/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_MKL26Z128VLH4 -DCPU_MKL26Z128VLH4_cm0plus -DDEBUG -DFRDM_KL26Z -DFREEDOM -DSDK_DEBUGCONSOLE=0 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -I"D:\College\Theses\Workspace NXP\Thesis_cmsis_driver_examples_i2c_interrupt_transfer_Test\source" -I"D:\College\Theses\Workspace NXP\Thesis_cmsis_driver_examples_i2c_interrupt_transfer_Test" -I"D:\College\Theses\Workspace NXP\Thesis_cmsis_driver_examples_i2c_interrupt_transfer_Test\CMSIS" -I"D:\College\Theses\Workspace NXP\Thesis_cmsis_driver_examples_i2c_interrupt_transfer_Test\CMSIS_driver" -I"D:\College\Theses\Workspace NXP\Thesis_cmsis_driver_examples_i2c_interrupt_transfer_Test\drivers" -I"D:\College\Theses\Workspace NXP\Thesis_cmsis_driver_examples_i2c_interrupt_transfer_Test\utilities" -I"D:\College\Theses\Workspace NXP\Thesis_cmsis_driver_examples_i2c_interrupt_transfer_Test\startup" -I"D:\College\Theses\Workspace NXP\Thesis_cmsis_driver_examples_i2c_interrupt_transfer_Test\board" -O0 -fno-common -g -gdwarf-4 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-source

clean-source:
	-$(RM) ./source/cmsis_i2c_interrupt_transfer.d ./source/cmsis_i2c_interrupt_transfer.o ./source/mtb.d ./source/mtb.o ./source/semihost_hardfault.d ./source/semihost_hardfault.o

.PHONY: clean-source

