################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include utilities/subdir.mk
-include startup/subdir.mk
-include source/subdir.mk
-include drivers/subdir.mk
-include board/subdir.mk
-include CMSIS_driver/subdir.mk
-include CMSIS/subdir.mk
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := Thesis_cmsis_driver_examples_i2c_interrupt_transfer_Test
BUILD_ARTIFACT_EXTENSION := axf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all:
	+@$(MAKE) --no-print-directory main-build && $(MAKE) --no-print-directory post-build

# Main-build Target
main-build: Thesis_cmsis_driver_examples_i2c_interrupt_transfer_Test.axf

# Tool invocations
Thesis_cmsis_driver_examples_i2c_interrupt_transfer_Test.axf: $(OBJS) $(USER_OBJS) makefile $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -Xlinker -Map="Thesis_cmsis_driver_examples_i2c_interrupt_transfer_Test.map" -Xlinker --gc-sections -Xlinker -print-memory-usage -Xlinker --sort-section=alignment -Xlinker --cref -mcpu=cortex-m0plus -mthumb -T Thesis_cmsis_driver_examples_i2c_interrupt_transfer_Test_Debug.ld -o "Thesis_cmsis_driver_examples_i2c_interrupt_transfer_Test.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) Thesis_cmsis_driver_examples_i2c_interrupt_transfer_Test.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "Thesis_cmsis_driver_examples_i2c_interrupt_transfer_Test.axf"; # arm-none-eabi-objcopy -v -O binary "Thesis_cmsis_driver_examples_i2c_interrupt_transfer_Test.axf" "Thesis_cmsis_driver_examples_i2c_interrupt_transfer_Test.bin" ; # checksum -p MKL26Z128xxx4 -d "Thesis_cmsis_driver_examples_i2c_interrupt_transfer_Test.bin";
	-@echo ' '

.PHONY: all clean dependents main-build post-build

-include ../makefile.targets
