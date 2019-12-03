################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/platform/mpfs_hal/subdir.mk
-include src/platform/hal/subdir.mk
-include src/platform/drivers/mustein_gpu/subdir.mk
-include src/application/hart1/subdir.mk
-include src/application/hart0/subdir.mk
-include src/application/fractals/subdir.mk
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
pse-mustein-julia.hex \

SECONDARY_LIST += \
pse-mustein-julia.lst \

SECONDARY_SIZE += \
pse-mustein-julia.siz \


# All Target
all: pse-mustein-julia.elf secondary-outputs

# Tool invocations
pse-mustein-julia.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU RISC-V Cross C Linker'
	riscv64-unknown-elf-gcc -march=rv64gc -mabi=lp64d -mcmodel=medlow -msmall-data-limit=8 -mstrict-align -mno-save-restore -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -T "../src/platform/config/linker/mpfs-lim.ld" -nostartfiles -Xlinker --gc-sections -Wl,-Map,"pse-mustein-julia.map" --specs=nano.specs --specs=nosys.specs -o "pse-mustein-julia.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

pse-mustein-julia.hex: pse-mustein-julia.elf
	@echo 'Invoking: GNU RISC-V Cross Create Flash Image'
	riscv64-unknown-elf-objcopy -O ihex "pse-mustein-julia.elf"  "pse-mustein-julia.hex"
	@echo 'Finished building: $@'
	@echo ' '

pse-mustein-julia.lst: pse-mustein-julia.elf
	@echo 'Invoking: GNU RISC-V Cross Create Listing'
	riscv64-unknown-elf-objdump --source --all-headers --demangle --line-numbers --wide "pse-mustein-julia.elf" > "pse-mustein-julia.lst"
	@echo 'Finished building: $@'
	@echo ' '

pse-mustein-julia.siz: pse-mustein-julia.elf
	@echo 'Invoking: GNU RISC-V Cross Print Size'
	riscv64-unknown-elf-size --format=berkeley "pse-mustein-julia.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_UPPER_DEPS)$(C_DEPS) pse-mustein-julia.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets
