################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/platform/drivers/mustein_gpu/mustein_gpu32.c \
../src/platform/drivers/mustein_gpu/mustein_gpu64.c 

OBJS += \
./src/platform/drivers/mustein_gpu/mustein_gpu32.o \
./src/platform/drivers/mustein_gpu/mustein_gpu64.o 

C_DEPS += \
./src/platform/drivers/mustein_gpu/mustein_gpu32.d \
./src/platform/drivers/mustein_gpu/mustein_gpu64.d 


# Each subdirectory must supply rules for building sources it contributes
src/platform/drivers/mustein_gpu/%.o: ../src/platform/drivers/mustein_gpu/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv64-unknown-elf-gcc -march=rv64gc -mabi=lp64d -mcmodel=medlow -msmall-data-limit=8 -mstrict-align -mno-save-restore -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DMPFS_HAL_LAST_HART=1 -I"../src/application" -I"../src/modules" -I"../src/platform" -std=gnu11 -Wa,-adhlns="$@.lst" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


