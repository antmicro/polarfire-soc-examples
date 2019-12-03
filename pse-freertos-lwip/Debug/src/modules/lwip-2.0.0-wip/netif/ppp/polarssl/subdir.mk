################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/modules/lwip-2.0.0-wip/netif/ppp/polarssl/arc4.c \
../src/modules/lwip-2.0.0-wip/netif/ppp/polarssl/des.c \
../src/modules/lwip-2.0.0-wip/netif/ppp/polarssl/md4.c \
../src/modules/lwip-2.0.0-wip/netif/ppp/polarssl/md5.c \
../src/modules/lwip-2.0.0-wip/netif/ppp/polarssl/sha1.c 

OBJS += \
./src/modules/lwip-2.0.0-wip/netif/ppp/polarssl/arc4.o \
./src/modules/lwip-2.0.0-wip/netif/ppp/polarssl/des.o \
./src/modules/lwip-2.0.0-wip/netif/ppp/polarssl/md4.o \
./src/modules/lwip-2.0.0-wip/netif/ppp/polarssl/md5.o \
./src/modules/lwip-2.0.0-wip/netif/ppp/polarssl/sha1.o 

C_DEPS += \
./src/modules/lwip-2.0.0-wip/netif/ppp/polarssl/arc4.d \
./src/modules/lwip-2.0.0-wip/netif/ppp/polarssl/des.d \
./src/modules/lwip-2.0.0-wip/netif/ppp/polarssl/md4.d \
./src/modules/lwip-2.0.0-wip/netif/ppp/polarssl/md5.d \
./src/modules/lwip-2.0.0-wip/netif/ppp/polarssl/sha1.d 


# Each subdirectory must supply rules for building sources it contributes
src/modules/lwip-2.0.0-wip/netif/ppp/polarssl/%.o: ../src/modules/lwip-2.0.0-wip/netif/ppp/polarssl/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv64-unknown-elf-gcc -march=rv64gc -mabi=lp64d -mcmodel=medlow -msmall-data-limit=0 -mstrict-align -mno-save-restore -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DPSE=1 -DMPFS_HAL_LAST_HART=0 -DMSS_MAC_64_BIT_ADDRESS_MODE -DUSING_FREERTOS -DUSING_LWIP -DLWIP_DEBUG -DHW_EMUL_USE_GEM0 -DxMSS_MAC_LWIP_USE_EMAC -DxMSS_MAC_USE_DDR -DxTI_PHY -DVTSS_CHIP_CU_PHY -DVTSS_FEATURE_SYNCE -DVTSS_FEATURE_PHY_TS_ONE_STEP_TXFIFO_OPTION -DVTSS_FEATURE_SERDES_MACRO_SETTINGS -DVTSS_OPT_PORT_COUNT=4 -DVTSS_OPT_VCORE_III=0 -DVTSS_PRODUCT_CHIP="PHY" -DVTSS_PHY_API_ONLY -DVTSS_OPT_TRACE=0 -DVTSS_OS_FREERTOS_LWIP -DCMSIS_PROT -DLWIP_COMPAT_MUTEX -DLWIP_SKIP_PACKING_CHECK -DLWIP_PROVIDE_ERRNO -DxTARGET_ALOE -DTARGET_G5_SOC -DMSS_MAC_SIMPLE_TX_QUEUE -DCALCONFIGH=\"config_user.h\" -DSI_FIVE_UNLEASED=0 -DTEST_H2F_CONTROLLER=0 -D_ZL303XX_MIV -DUSING_LWIP -I"../src/application/" -I"../src/platform" -I"../src/modules/config/free-rtos-8.2.3-pfsoc" -I"../src/modules/config/lwip-2.0.0-wip" -I"../src/modules" -I"../src/modules/free-rtos-8.2.3-pfsoc/include" -I"../src/modules/free-rtos-8.2.3-pfsoc/portable/GCC/RISCV" -I"../src/modules/lwip-2.0.0-wip/include" -I"../src/modules/lwip-2.0.0-wip-port-freertos-pfsoc" -I"../src/modules/printf-4.0.0" -std=gnu11 -Wa,-adhlns="$@.lst" -v -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


