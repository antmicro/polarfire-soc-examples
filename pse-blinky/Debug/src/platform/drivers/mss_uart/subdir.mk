################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/platform/drivers/mss_uart/mss_uart.c 

OBJS += \
./src/platform/drivers/mss_uart/mss_uart.o 

C_DEPS += \
./src/platform/drivers/mss_uart/mss_uart.d 


# Each subdirectory must supply rules for building sources it contributes
src/platform/drivers/mss_uart/%.o: ../src/platform/drivers/mss_uart/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv64-unknown-elf-gcc -march=rv64gc -mabi=lp64d -mcmodel=medlow -msmall-data-limit=8 -mstrict-align -mno-save-restore -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DMPFS_HAL_LAST_HART=1 -I"../src/application" -I"../src/modules" -I"../src/platform" -std=gnu11 -Wa,-adhlns="$@.lst" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


