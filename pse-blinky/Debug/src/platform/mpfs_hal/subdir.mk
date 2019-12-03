################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/platform/mpfs_hal/mss_h2f.c \
../src/platform/mpfs_hal/mss_mpu.c \
../src/platform/mpfs_hal/mss_stubs.c \
../src/platform/mpfs_hal/mss_util.c \
../src/platform/mpfs_hal/mtrap.c \
../src/platform/mpfs_hal/newlib_stubs.c \
../src/platform/mpfs_hal/system_startup.c 

S_UPPER_SRCS += \
../src/platform/mpfs_hal/entry.S \
../src/platform/mpfs_hal/mss_mutex.S 

OBJS += \
./src/platform/mpfs_hal/entry.o \
./src/platform/mpfs_hal/mss_h2f.o \
./src/platform/mpfs_hal/mss_mpu.o \
./src/platform/mpfs_hal/mss_mutex.o \
./src/platform/mpfs_hal/mss_stubs.o \
./src/platform/mpfs_hal/mss_util.o \
./src/platform/mpfs_hal/mtrap.o \
./src/platform/mpfs_hal/newlib_stubs.o \
./src/platform/mpfs_hal/system_startup.o 

S_UPPER_DEPS += \
./src/platform/mpfs_hal/entry.d \
./src/platform/mpfs_hal/mss_mutex.d 

C_DEPS += \
./src/platform/mpfs_hal/mss_h2f.d \
./src/platform/mpfs_hal/mss_mpu.d \
./src/platform/mpfs_hal/mss_stubs.d \
./src/platform/mpfs_hal/mss_util.d \
./src/platform/mpfs_hal/mtrap.d \
./src/platform/mpfs_hal/newlib_stubs.d \
./src/platform/mpfs_hal/system_startup.d 


# Each subdirectory must supply rules for building sources it contributes
src/platform/mpfs_hal/%.o: ../src/platform/mpfs_hal/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross Assembler'
	riscv64-unknown-elf-gcc -march=rv64gc -mabi=lp64d -mcmodel=medlow -msmall-data-limit=8 -mstrict-align -mno-save-restore -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -x assembler-with-cpp -I"../src/application" -I"../src/modules" -I"../src/platform" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/platform/mpfs_hal/%.o: ../src/platform/mpfs_hal/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv64-unknown-elf-gcc -march=rv64gc -mabi=lp64d -mcmodel=medlow -msmall-data-limit=8 -mstrict-align -mno-save-restore -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DMPFS_HAL_LAST_HART=1 -I"../src/application" -I"../src/modules" -I"../src/platform" -std=gnu11 -Wa,-adhlns="$@.lst" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


