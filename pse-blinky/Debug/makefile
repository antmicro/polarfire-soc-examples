################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/platform/mpfs_hal/subdir.mk
-include src/platform/hal/subdir.mk
-include src/platform/drivers/mss_uart/subdir.mk
-include src/platform/drivers/mss_gpio/subdir.mk
-include src/application/inc/subdir.mk
-include src/application/hart1/subdir.mk
-include src/application/hart0/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
pse-blinky.hex \

SECONDARY_LIST += \
pse-blinky.lst \

SECONDARY_SIZE += \
pse-blinky.siz \


# All Target
all: pse-blinky.elf secondary-outputs

# Tool invocations
pse-blinky.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU RISC-V Cross C Linker'
	riscv64-unknown-elf-gcc -march=rv64gc -mabi=lp64d -mcmodel=medlow -msmall-data-limit=8 -mstrict-align -mno-save-restore -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -T "../src/platform/config/linker/mpfs-lim.ld" -nostartfiles -Xlinker --gc-sections -Wl,-Map,"pse-blinky.map" --specs=nano.specs --specs=nosys.specs -o "pse-blinky.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

pse-blinky.hex: pse-blinky.elf
	@echo 'Invoking: GNU RISC-V Cross Create Flash Image'
	riscv64-unknown-elf-objcopy -O ihex "pse-blinky.elf"  "pse-blinky.hex"
	@echo 'Finished building: $@'
	@echo ' '

pse-blinky.lst: pse-blinky.elf
	@echo 'Invoking: GNU RISC-V Cross Create Listing'
	riscv64-unknown-elf-objdump --source --all-headers --demangle --line-numbers --wide "pse-blinky.elf" > "pse-blinky.lst"
	@echo 'Finished building: $@'
	@echo ' '

pse-blinky.siz: pse-blinky.elf
	@echo 'Invoking: GNU RISC-V Cross Print Size'
	riscv64-unknown-elf-size --format=berkeley "pse-blinky.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_UPPER_DEPS)$(C_DEPS) pse-blinky.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets
