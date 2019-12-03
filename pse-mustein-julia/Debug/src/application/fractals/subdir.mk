################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/application/fractals/fractal_display.c \
../src/application/fractals/fractal_engine.c 

OBJS += \
./src/application/fractals/fractal_display.o \
./src/application/fractals/fractal_engine.o 

C_DEPS += \
./src/application/fractals/fractal_display.d \
./src/application/fractals/fractal_engine.d 


# Each subdirectory must supply rules for building sources it contributes
src/application/fractals/%.o: ../src/application/fractals/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv64-unknown-elf-gcc -march=rv64gc -mabi=lp64d -mcmodel=medlow -msmall-data-limit=8 -mstrict-align -mno-save-restore -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DMPFS_HAL_LAST_HART=1 -I"../src/application" -I"../src/modules" -I"../src/platform" -std=gnu11 -Wa,-adhlns="$@.lst" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


