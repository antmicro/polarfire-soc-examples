/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */

/***************************************************************************
 *
 * @file mtrap.h
 * @author Microsemi-PRO Embedded Systems Solutions
 * @brief trap functions
 *
 * SVN $Revision: 11865 $
 * SVN $Date: 2019-07-29 19:58:05 +0530 (Mon, 29 Jul 2019) $
 */
#include "mss_ints.h"
#include "mss_clint.h"
#include "mss_plic.h"
#include "mss_util.h"
#include "config/hardware/hw_platform.h"
#include "config/software/mpfs_hal/mss_sw_config.h"

#ifdef __cplusplus
extern "C" {
#endif

static uint64_t g_systick_increment[5] = {0,0,0,0,0};

void handle_local_interrupt(uint8_t interrupt_no);
void handle_m_soft_interrupt(void);
void handle_m_timer_interrupt(void);
void illegal_insn_trap(uintptr_t * regs, uintptr_t mcause, uintptr_t mepc);
void misaligned_store_trap(uintptr_t * regs, uintptr_t mcause, uintptr_t mepc);
void misaligned_load_trap(uintptr_t * regs, uintptr_t mcause, uintptr_t mepc);
void illegal_insn_trap(uintptr_t * regs, uintptr_t mcause, uintptr_t mepc);
void pmp_trap(uintptr_t * regs, uintptr_t mcause, uintptr_t mepc);
void trap_from_machine_mode(uintptr_t * regs, uintptr_t dummy, uintptr_t mepc);
void bad_trap(uintptr_t* regs, uintptr_t dummy, uintptr_t mepc);
void reset_mtime(void);
uint32_t SysTick_Config(void);

void bad_trap(uintptr_t* regs, uintptr_t dummy, uintptr_t mepc)
{
    while(1)
    {
       volatile static uint64_t counter = 0U;

       /* Added some code as debugger hangs if in loop doing nothing */
       counter += counter + 1;
    }
}

void misaligned_store_trap(uintptr_t * regs, uintptr_t mcause, uintptr_t mepc)
{
    while(1)
    {
       volatile static uint64_t counter = 0U;

       /* Added some code as debugger hangs if in loop doing nothing */
       counter += counter + 1;
    }
}

void misaligned_load_trap(uintptr_t * regs, uintptr_t mcause, uintptr_t mepc)
{
    while(1)
    {
       volatile static uint64_t counter = 0U;

       /* Added some code as debugger hangs if in loop doing nothing */
       counter += counter + 1;
    }
}

void illegal_insn_trap(uintptr_t * regs, uintptr_t mcause, uintptr_t mepc)
{
    while(1)
    {
       volatile static uint64_t counter = 0U;

       /* Added some code as debugger hangs if in loop doing nothing */
       counter += counter + 1;
    }
}

void pmp_trap(uintptr_t * regs, uintptr_t mcause, uintptr_t mepc)
{
    while(1)
    {
       volatile static uint64_t counter = 0U;

       /* Added some code as debugger hangs if in loop doing nothing */
       counter += counter + 1;
    }
}

/*------------------------------------------------------------------------------
 * RISC-V interrupt handler for external interrupts.
 */
#ifndef SIFIVE_HIFIVE_UNLEASHED
uint8_t (*ext_irq_handler_table[PLIC_NUM_SOURCES])(void) =
{
  Invalid_IRQHandler,
  l2_metadata_corr_IRQHandler,
  l2_metadata_uncorr_IRQHandler,
  l2_data_corr_IRQHandler,
  l2_data_uncorr_IRQHandler,
  dma_ch0_DONE_IRQHandler,
  dma_ch0_ERR_IRQHandler,
  dma_ch1_DONE_IRQHandler,
  dma_ch1_ERR_IRQHandler,
  dma_ch2_DONE_IRQHandler,
  dma_ch2_ERR_IRQHandler,      //10
  dma_ch3_DONE_IRQHandler,
  dma_ch3_ERR_IRQHandler,
  gpio0_bit0_or_gpio2_bit13_plic_0_IRQHandler,
  gpio0_bit1_or_gpio2_bit13_plic_1_IRQHandler,
  gpio0_bit2_or_gpio2_bit13_plic_2_IRQHandler,
  gpio0_bit3_or_gpio2_bit13_plic_3_IRQHandler,
  gpio0_bit4_or_gpio2_bit13_plic_4_IRQHandler,
  gpio0_bit5_or_gpio2_bit13_plic_5_IRQHandler,
  gpio0_bit6_or_gpio2_bit13_plic_6_IRQHandler,
  gpio0_bit7_or_gpio2_bit13_plic_7_IRQHandler,    //20
  gpio0_bit8_or_gpio2_bit13_plic_8_IRQHandler,
  gpio0_bit9_or_gpio2_bit13_plic_9_IRQHandler,
  gpio0_bit10_or_gpio2_bit13_plic_10_IRQHandler,
  gpio0_bit11_or_gpio2_bit13_plic_11_IRQHandler,
  gpio0_bit12_or_gpio2_bit13_plic_12_IRQHandler,

  gpio0_bit13_or_gpio2_bit13_plic_13_IRQHandler,
  gpio1_bit0_or_gpio2_bit14_plic_14_IRQHandler,
  gpio1_bit1_or_gpio2_bit15_plic_15_IRQHandler,
  gpio1_bit2_or_gpio2_bit16_plic_16_IRQHandler,
  gpio1_bit3_or_gpio2_bit17_plic_17_IRQHandler,
  gpio1_bit4_or_gpio2_bit18_plic_18_IRQHandler,//30
  gpio1_bit5_or_gpio2_bit19_plic_19_IRQHandler,
  gpio1_bit6_or_gpio2_bit20_plic_20_IRQHandler,
  gpio1_bit7_or_gpio2_bit21_plic_21_IRQHandler,
  gpio1_bit8_or_gpio2_bit22_plic_22_IRQHandler,
  gpio1_bit9_or_gpio2_bit23_plic_23_IRQHandler,
  gpio1_bit10_or_gpio2_bit24_plic_24_IRQHandler,
  gpio1_bit11_or_gpio2_bit25_plic_25_IRQHandler,
  gpio1_bit12_or_gpio2_bit26_plic_26_IRQHandler,
  gpio1_bit13_or_gpio2_bit27_plic_27_IRQHandler,

  gpio1_bit14_or_gpio2_bit28_plic_28_IRQHandler,//40
  gpio1_bit15_or_gpio2_bit29_plic_29_IRQHandler,
  gpio1_bit16_or_gpio2_bit30_plic_30_IRQHandler,
  gpio1_bit17_or_gpio2_bit31_plic_31_IRQHandler,

  gpio1_bit18_plic_32_IRQHandler,
  gpio1_bit19_plic_33_IRQHandler,
  gpio1_bit20_plic_34_IRQHandler,
  gpio1_bit21_plic_35_IRQHandler,
  gpio1_bit22_plic_36_IRQHandler,
  gpio1_bit23_plic_37_IRQHandler,

  gpio0_non_direct_plic_IRQHandler,//50
  gpio1_non_direct_plic_IRQHandler,
  gpio2_non_direct_plic_IRQHandler,

  spi0_plic_IRQHandler,
  spi1_plic_IRQHandler,
  external_can0_plic_IRQHandler,
  can1_IRQHandler,
  External_i2c0_main_plic_IRQHandler,
  External_i2c0_alert_plic_IRQHandler,
  i2c0_sus_plic_IRQHandler,
  i2c1_main_plic_IRQHandler,//60
  i2c1_alert_plic_IRQHandler,
  i2c1_sus_plic_IRQHandler,
  mac0_int_plic_IRQHandler,
  mac0_queue1_plic_IRQHandler,
  mac0_queue2_plic_IRQHandler,
  mac0_queue3_plic_IRQHandler,
  mac0_emac_plic_IRQHandler,
  mac0_mmsl_plic_IRQHandler,
  mac1_int_plic_IRQHandler,
  mac1_queue1_plic_IRQHandler,//70
  mac1_queue2_plic_IRQHandler,
  mac1_queue3_plic_IRQHandler,
  mac1_emac_plic_IRQHandler,
  mac1_mmsl_plic_IRQHandler,
  ddrc_train_plic_IRQHandler,
  scb_interrupt_plic_IRQHandler,
  ecc_error_plic_IRQHandler,
  ecc_correct_plic_IRQHandler,
  rtc_wakeup_plic_IRQHandler,
  rtc_match_plic_IRQHandler,//10
  timer1_plic_IRQHandler,
  timer2_plic_IRQHandler,
  envm_plic_IRQHandler,
  qspi_plic_IRQHandler,
  usb_dma_plic_IRQHandler,
  usb_mc_plic_IRQHandler,
  mmc_main_plic_IRQHandler,
  mmc_wakeup_plic_IRQHandler,
  mmuart0_plic_77_IRQHandler,
  mmuart1_plic_IRQHandler,//90
  mmuart2_plic_IRQHandler,
  mmuart3_plic_IRQHandler,
  mmuart4_plic_IRQHandler,

  g5c_devrst_plic_IRQHandler,
  g5c_message_plic_IRQHandler,
  usoc_vc_interrupt_plic_IRQHandler,
  usoc_smb_interrupt_plic_IRQHandler,
  e51_0_Maintence_plic_IRQHandler,

  wdog0_mvrp_plic_IRQHandler,
  wdog1_mvrp_plic_IRQHandler, /*100 contains multiple interrupts- */
  wdog2_mvrp_plic_IRQHandler,
  wdog3_mvrp_plic_IRQHandler,
  wdog4_mvrp_plic_IRQHandler,
  wdog0_tout_plic_IRQHandler,
  wdog1_tout_plic_IRQHandler,
  wdog2_tout_plic_IRQHandler,
  wdog3_tout_plic_IRQHandler,
  wdog4_tout_plic_IRQHandler,

  g5c_mss_spi_plic_IRQHandler,
  volt_temp_alarm_plic_IRQHandler,
  athena_complete_plic_IRQHandler,
  athena_alarm_plic_IRQHandler,
  athena_bus_error_plic_IRQHandler,
  usoc_axic_us_plic_IRQHandler,
  usoc_axic_ds_plic_IRQHandler,

  reserved_104_plic_IRQHandler,

  fabric_f2h_0_plic_IRQHandler,
  fabric_f2h_1_plic_IRQHandler,
  fabric_f2h_2_plic_IRQHandler,
  fabric_f2h_3_plic_IRQHandler,
  fabric_f2h_4_plic_IRQHandler,
  fabric_f2h_5_plic_IRQHandler,
  fabric_f2h_6_plic_IRQHandler,
  fabric_f2h_7_plic_IRQHandler,
  fabric_f2h_8_plic_IRQHandler,
  fabric_f2h_9_plic_IRQHandler,

  fabric_f2h_10_plic_IRQHandler,
  fabric_f2h_11_plic_IRQHandler,
  fabric_f2h_12_plic_IRQHandler,
  fabric_f2h_13_plic_IRQHandler,
  fabric_f2h_14_plic_IRQHandler,
  fabric_f2h_15_plic_IRQHandler,
  fabric_f2h_16_plic_IRQHandler,
  fabric_f2h_17_plic_IRQHandler,
  fabric_f2h_18_plic_IRQHandler,
  fabric_f2h_19_plic_IRQHandler,

  fabric_f2h_20_plic_IRQHandler,
  fabric_f2h_21_plic_IRQHandler,
  fabric_f2h_22_plic_IRQHandler,
  fabric_f2h_23_plic_IRQHandler,
  fabric_f2h_24_plic_IRQHandler,
  fabric_f2h_25_plic_IRQHandler,
  fabric_f2h_26_plic_IRQHandler,
  fabric_f2h_27_plic_IRQHandler,
  fabric_f2h_28_plic_IRQHandler,
  fabric_f2h_29_plic_IRQHandler,

  fabric_f2h_30_plic_IRQHandler,
  fabric_f2h_31_plic_IRQHandler,

  fabric_f2h_32_plic_IRQHandler,
  fabric_f2h_33_plic_IRQHandler,
  fabric_f2h_34_plic_IRQHandler,
  fabric_f2h_35_plic_IRQHandler,
  fabric_f2h_36_plic_IRQHandler,
  fabric_f2h_37_plic_IRQHandler,
  fabric_f2h_38_plic_IRQHandler,
  fabric_f2h_39_plic_IRQHandler,
  fabric_f2h_40_plic_IRQHandler,
  fabric_f2h_41_plic_IRQHandler,

  fabric_f2h_42_plic_IRQHandler,
  fabric_f2h_43_plic_IRQHandler,
  fabric_f2h_44_plic_IRQHandler,
  fabric_f2h_45_plic_IRQHandler,
  fabric_f2h_46_plic_IRQHandler,
  fabric_f2h_47_plic_IRQHandler,
  fabric_f2h_48_plic_IRQHandler,
  fabric_f2h_49_plic_IRQHandler,
  fabric_f2h_50_plic_IRQHandler,
  fabric_f2h_51_plic_IRQHandler,

  fabric_f2h_52_plic_IRQHandler,
  fabric_f2h_53_plic_IRQHandler,
  fabric_f2h_54_plic_IRQHandler,
  fabric_f2h_55_plic_IRQHandler,
  fabric_f2h_56_plic_IRQHandler,
  fabric_f2h_57_plic_IRQHandler,
  fabric_f2h_58_plic_IRQHandler,
  fabric_f2h_59_plic_IRQHandler,
  fabric_f2h_60_plic_IRQHandler,
  fabric_f2h_61_plic_IRQHandler,

  fabric_f2h_62_plic_IRQHandler,
  fabric_f2h_63_plic_IRQHandler,

  bus_error_unit_hart_0_plic_IRQHandler,
  bus_error_unit_hart_1_plic_IRQHandler,
  bus_error_unit_hart_2_plic_IRQHandler,
  bus_error_unit_hart_3_plic_IRQHandler,
  bus_error_unit_hart_4_plic_IRQHandler
};

#define E51_LOCAL_NUM_SOURCES 48

void maintenance_e51_local_IRQHandler_0(void);
void usoc_smb_interrupt_e51_local_IRQHandler_1(void);
void usoc_vc_interrupt_e51_local_IRQHandler_2(void);
void g5c_message_e51_local_IRQHandler_3(void);
void g5c_devrst_e51_local_IRQHandler_4(void);
void wdog4_tout_e51_local_IRQHandler_5(void);
void wdog3_tout_e51_local_IRQHandler_6(void);
void wdog2_tout_e51_local_IRQHandler_7(void);
void wdog1_tout_e51_local_IRQHandler_8(void);
void wdog0_tout_e51_local_IRQHandler_9(void);
void wdog0_mvrp_e51_local_IRQHandler_10(void);
void mmuart0_e51_local_IRQHandler_11(void);
void envm_e51_local_IRQHandler_12(void);
void ecc_correct_e51_local_IRQHandler_13(void);
void ecc_error_e51_local_IRQHandler_14(void);
void scb_interrupt_e51_local_IRQHandler_15(void);
void fabric_f2h_32_e51_local_IRQHandler_16(void);
void fabric_f2h_33_e51_local_IRQHandler_17(void);
void fabric_f2h_34_e51_local_IRQHandler_18(void);
void fabric_f2h_35_e51_local_IRQHandler_19(void);
void fabric_f2h_36_e51_local_IRQHandler_20(void);
void fabric_f2h_37_e51_local_IRQHandler_21(void);
void fabric_f2h_38_e51_local_IRQHandler_22(void);
void fabric_f2h_39_e51_local_IRQHandler_23(void);
void fabric_f2h_40_e51_local_IRQHandler_24(void);
void fabric_f2h_41_e51_local_IRQHandler_25(void);
void fabric_f2h_42_e51_local_IRQHandler_26(void);
void fabric_f2h_43_e51_local_IRQHandler_27(void);
void fabric_f2h_44_e51_local_IRQHandler_28(void);
void fabric_f2h_45_e51_local_IRQHandler_29(void);
void fabric_f2h_46_e51_local_IRQHandler_30(void);
void fabric_f2h_47_e51_local_IRQHandler_31(void);
void fabric_f2h_48_e51_local_IRQHandler_32(void);
void fabric_f2h_49_e51_local_IRQHandler_33(void);
void fabric_f2h_50_e51_local_IRQHandler_34(void);
void fabric_f2h_51_e51_local_IRQHandler_35(void);
void fabric_f2h_52_e51_local_IRQHandler_36(void);
void fabric_f2h_53_e51_local_IRQHandler_37(void);
void fabric_f2h_54_e51_local_IRQHandler_38(void);
void fabric_f2h_55_e51_local_IRQHandler_39(void);
void fabric_f2h_56_e51_local_IRQHandler_40(void);
void fabric_f2h_57_e51_local_IRQHandler_41(void);
void fabric_f2h_58_e51_local_IRQHandler_42(void);
void fabric_f2h_59_e51_local_IRQHandler_43(void);
void fabric_f2h_60_e51_local_IRQHandler_44(void);
void fabric_f2h_61_e51_local_IRQHandler_45(void);
void fabric_f2h_62_e51_local_IRQHandler_46(void);
void fabric_f2h_63_e51_local_IRQHandler_47(void);



void (*local_irq_handler_e51_table[E51_LOCAL_NUM_SOURCES])(void) =
{
    maintenance_e51_local_IRQHandler_0,    /* reference multiple interrupts */
    usoc_smb_interrupt_e51_local_IRQHandler_1,
    usoc_vc_interrupt_e51_local_IRQHandler_2,
    g5c_message_e51_local_IRQHandler_3,
    g5c_devrst_e51_local_IRQHandler_4,
    wdog4_tout_e51_local_IRQHandler_5,
    wdog3_tout_e51_local_IRQHandler_6,
    wdog2_tout_e51_local_IRQHandler_7,
    wdog1_tout_e51_local_IRQHandler_8,
    wdog0_tout_e51_local_IRQHandler_9,
    wdog0_mvrp_e51_local_IRQHandler_10,
    mmuart0_e51_local_IRQHandler_11,
    envm_e51_local_IRQHandler_12,
    ecc_correct_e51_local_IRQHandler_13,
    ecc_error_e51_local_IRQHandler_14,
    scb_interrupt_e51_local_IRQHandler_15,
    fabric_f2h_32_e51_local_IRQHandler_16,
    fabric_f2h_33_e51_local_IRQHandler_17,
    fabric_f2h_34_e51_local_IRQHandler_18,
    fabric_f2h_35_e51_local_IRQHandler_19,
    fabric_f2h_36_e51_local_IRQHandler_20,
    fabric_f2h_37_e51_local_IRQHandler_21,
    fabric_f2h_38_e51_local_IRQHandler_22,
    fabric_f2h_39_e51_local_IRQHandler_23,
    fabric_f2h_40_e51_local_IRQHandler_24,
    fabric_f2h_41_e51_local_IRQHandler_25,

    fabric_f2h_42_e51_local_IRQHandler_26,
    fabric_f2h_43_e51_local_IRQHandler_27,
    fabric_f2h_44_e51_local_IRQHandler_28,
    fabric_f2h_45_e51_local_IRQHandler_29,
    fabric_f2h_46_e51_local_IRQHandler_30,
    fabric_f2h_47_e51_local_IRQHandler_31,
    fabric_f2h_48_e51_local_IRQHandler_32,
    fabric_f2h_49_e51_local_IRQHandler_33,
    fabric_f2h_50_e51_local_IRQHandler_34,
    fabric_f2h_51_e51_local_IRQHandler_35,

    fabric_f2h_52_e51_local_IRQHandler_36,
    fabric_f2h_53_e51_local_IRQHandler_37,
    fabric_f2h_54_e51_local_IRQHandler_38,
    fabric_f2h_55_e51_local_IRQHandler_39,
    fabric_f2h_56_e51_local_IRQHandler_40,
    fabric_f2h_57_e51_local_IRQHandler_41,
    fabric_f2h_58_e51_local_IRQHandler_42,
    fabric_f2h_59_e51_local_IRQHandler_43,
    fabric_f2h_60_e51_local_IRQHandler_44,
    fabric_f2h_61_e51_local_IRQHandler_45,

    fabric_f2h_62_e51_local_IRQHandler_46,
    fabric_f2h_63_e51_local_IRQHandler_47
};

/* reference multiple interrupts */
void spare_u54_local_IRQHandler_0(void);
void spare_u54_local_IRQHandler_1(void);
void spare_u54_local_IRQHandler_2(void);

/* parse hart ID to discover which mac is the source */
void mac_mmsl_u54_1_local_IRQHandler_3(void);
void mac_emac_u54_1_local_IRQHandler_4(void);
void mac_queue3_u54_1_local_IRQHandler_5(void);
void mac_queue2_u54_1_local_IRQHandler_6(void);
void mac_queue1_u54_1_local_IRQHandler_7(void);
void mac_int_u54_1_local_IRQHandler_8(void);

/* parse hart ID to discover which mac is the source */
void mac_mmsl_u54_2_local_IRQHandler_3(void);
void mac_emac_u54_2_local_IRQHandler_4(void);
void mac_queue3_u54_2_local_IRQHandler_5(void);
void mac_queue2_u54_2_local_IRQHandler_6(void);
void mac_queue1_u54_2_local_IRQHandler_7(void);
void mac_int_u54_2_local_IRQHandler_8(void);

/* parse hart ID to discover which mac is the source */
void mac_mmsl_u54_3_local_IRQHandler_3(void);
void mac_emac_u54_3_local_IRQHandler_4(void);
void mac_queue3_u54_3_local_IRQHandler_5(void);
void mac_queue2_u54_3_local_IRQHandler_6(void);
void mac_queue1_u54_3_local_IRQHandler_7(void);
void mac_int_u54_3_local_IRQHandler_8(void);

/* parse hart ID to discover which mac is the source */
void mac_mmsl_u54_4_local_IRQHandler_3(void);
void mac_emac_u54_4_local_IRQHandler_4(void);
void mac_queue3_u54_4_local_IRQHandler_5(void);
void mac_queue2_u54_4_local_IRQHandler_6(void);
void mac_queue1_u54_4_local_IRQHandler_7(void);
void mac_int_u54_4_local_IRQHandler_8(void);

void wdog_tout_u54_h1_local_IRQHandler_9(void);
void wdog_tout_u54_h2_local_IRQHandler_9(void);
void wdog_tout_u54_h3_local_IRQHandler_9(void);
void wdog_tout_u54_h4_local_IRQHandler_9(void);
void mvrp_u54_local_IRQHandler_10(void);
void mmuart_u54_h1_local_IRQHandler_11(void);
void mmuart_u54_h2_local_IRQHandler_11(void);
void mmuart_u54_h3_local_IRQHandler_11(void);
void mmuart_u54_h4_local_IRQHandler_11(void);

void spare_u54_local_IRQHandler_12(void);
void spare_u54_local_IRQHandler_13(void);
void spare_u54_local_IRQHandler_14(void);
void spare_u54_local_IRQHandler_15(void);

void fabric_f2h_0_u54_local_IRQHandler_16(void);
void fabric_f2h_1_u54_local_IRQHandler_17(void);
void fabric_f2h_2_u54_local_IRQHandler_18(void);
void fabric_f2h_3_u54_local_IRQHandler_19(void);
void fabric_f2h_4_u54_local_IRQHandler_20(void);
void fabric_f2h_5_u54_local_IRQHandler_21(void);
void fabric_f2h_6_u54_local_IRQHandler_22(void);
void fabric_f2h_7_u54_local_IRQHandler_23(void);
void fabric_f2h_8_u54_local_IRQHandler_24(void);
void fabric_f2h_9_u54_local_IRQHandler_25(void);

void fabric_f2h_10_u54_local_IRQHandler_26(void);
void fabric_f2h_11_u54_local_IRQHandler_27(void);
void fabric_f2h_12_u54_local_IRQHandler_28(void);
void fabric_f2h_13_u54_local_IRQHandler_29(void);
void fabric_f2h_14_u54_local_IRQHandler_30(void);
void fabric_f2h_15_u54_local_IRQHandler_31(void);
void fabric_f2h_16_u54_local_IRQHandler_32(void);
void fabric_f2h_17_u54_local_IRQHandler_33(void);
void fabric_f2h_18_u54_local_IRQHandler_34(void);
void fabric_f2h_19_u54_local_IRQHandler_35(void);

void fabric_f2h_20_u54_local_IRQHandler_36(void);
void fabric_f2h_21_u54_local_IRQHandler_37(void);
void fabric_f2h_22_u54_local_IRQHandler_38(void);
void fabric_f2h_23_u54_local_IRQHandler_39(void);
void fabric_f2h_24_u54_local_IRQHandler_40(void);
void fabric_f2h_25_u54_local_IRQHandler_41(void);
void fabric_f2h_26_u54_local_IRQHandler_42(void);
void fabric_f2h_27_u54_local_IRQHandler_43(void);
void fabric_f2h_28_u54_local_IRQHandler_44(void);
void fabric_f2h_29_u54_local_IRQHandler_45(void);

void fabric_f2h_30_u54_local_IRQHandler_46(void);
void fabric_f2h_31_u54_local_IRQHandler_47(void);

typedef void (*local_int_p_t)(void);

/* U54 1 */
local_int_p_t local_irq_handler_u54_1_table[E51_LOCAL_NUM_SOURCES] =
{
    /*reference multiple interrupts*/
    spare_u54_local_IRQHandler_0,
    spare_u54_local_IRQHandler_1,
    spare_u54_local_IRQHandler_2,

    /*parse hart ID to discover which mac is the source*/
    mac_mmsl_u54_1_local_IRQHandler_3,
    mac_emac_u54_1_local_IRQHandler_4,
    mac_queue3_u54_1_local_IRQHandler_5,
    mac_queue2_u54_1_local_IRQHandler_6,
    mac_queue1_u54_1_local_IRQHandler_7,
    mac_int_u54_1_local_IRQHandler_8,

    /*parse hart ID to discover which wdog is the source*/
    wdog_tout_u54_h1_local_IRQHandler_9,
    mvrp_u54_local_IRQHandler_10,
    mmuart_u54_h1_local_IRQHandler_11,

    spare_u54_local_IRQHandler_12,
    spare_u54_local_IRQHandler_13,
    spare_u54_local_IRQHandler_14,
    spare_u54_local_IRQHandler_15,

    fabric_f2h_0_u54_local_IRQHandler_16,
    fabric_f2h_1_u54_local_IRQHandler_17,
    fabric_f2h_2_u54_local_IRQHandler_18,
    fabric_f2h_3_u54_local_IRQHandler_19,
    fabric_f2h_4_u54_local_IRQHandler_20,
    fabric_f2h_5_u54_local_IRQHandler_21,
    fabric_f2h_6_u54_local_IRQHandler_22,
    fabric_f2h_7_u54_local_IRQHandler_23,
    fabric_f2h_8_u54_local_IRQHandler_24,
    fabric_f2h_9_u54_local_IRQHandler_25,

    fabric_f2h_10_u54_local_IRQHandler_26,
    fabric_f2h_11_u54_local_IRQHandler_27,
    fabric_f2h_12_u54_local_IRQHandler_28,
    fabric_f2h_13_u54_local_IRQHandler_29,
    fabric_f2h_14_u54_local_IRQHandler_30,
    fabric_f2h_15_u54_local_IRQHandler_31,
    fabric_f2h_16_u54_local_IRQHandler_32,
    fabric_f2h_17_u54_local_IRQHandler_33,
    fabric_f2h_18_u54_local_IRQHandler_34,
    fabric_f2h_19_u54_local_IRQHandler_35,

    fabric_f2h_20_u54_local_IRQHandler_36,
    fabric_f2h_21_u54_local_IRQHandler_37,
    fabric_f2h_22_u54_local_IRQHandler_38,
    fabric_f2h_23_u54_local_IRQHandler_39,
    fabric_f2h_24_u54_local_IRQHandler_40,
    fabric_f2h_25_u54_local_IRQHandler_41,
    fabric_f2h_26_u54_local_IRQHandler_42,
    fabric_f2h_27_u54_local_IRQHandler_43,
    fabric_f2h_28_u54_local_IRQHandler_44,
    fabric_f2h_29_u54_local_IRQHandler_45,

    fabric_f2h_30_u54_local_IRQHandler_46,
    fabric_f2h_31_u54_local_IRQHandler_47
};

/* U54 2 */
local_int_p_t local_irq_handler_u54_2_table[E51_LOCAL_NUM_SOURCES] =
{
    /*reference multiple interrupts*/
    spare_u54_local_IRQHandler_0,
    spare_u54_local_IRQHandler_1,
    spare_u54_local_IRQHandler_2,

    /*parse hart ID to discover which mac is the source*/
    mac_mmsl_u54_2_local_IRQHandler_3,
    mac_emac_u54_2_local_IRQHandler_4,
    mac_queue3_u54_2_local_IRQHandler_5,
    mac_queue2_u54_2_local_IRQHandler_6,
    mac_queue1_u54_2_local_IRQHandler_7,
    mac_int_u54_2_local_IRQHandler_8,

    /*parse hart ID to discover which wdog is the source*/
    wdog_tout_u54_h2_local_IRQHandler_9,
    mvrp_u54_local_IRQHandler_10,
    mmuart_u54_h2_local_IRQHandler_11,

    spare_u54_local_IRQHandler_12,
    spare_u54_local_IRQHandler_13,
    spare_u54_local_IRQHandler_14,
    spare_u54_local_IRQHandler_15,

    fabric_f2h_0_u54_local_IRQHandler_16,
    fabric_f2h_1_u54_local_IRQHandler_17,
    fabric_f2h_2_u54_local_IRQHandler_18,
    fabric_f2h_3_u54_local_IRQHandler_19,
    fabric_f2h_4_u54_local_IRQHandler_20,
    fabric_f2h_5_u54_local_IRQHandler_21,
    fabric_f2h_6_u54_local_IRQHandler_22,
    fabric_f2h_7_u54_local_IRQHandler_23,
    fabric_f2h_8_u54_local_IRQHandler_24,
    fabric_f2h_9_u54_local_IRQHandler_25,

    fabric_f2h_10_u54_local_IRQHandler_26,
    fabric_f2h_11_u54_local_IRQHandler_27,
    fabric_f2h_12_u54_local_IRQHandler_28,
    fabric_f2h_13_u54_local_IRQHandler_29,
    fabric_f2h_14_u54_local_IRQHandler_30,
    fabric_f2h_15_u54_local_IRQHandler_31,
    fabric_f2h_16_u54_local_IRQHandler_32,
    fabric_f2h_17_u54_local_IRQHandler_33,
    fabric_f2h_18_u54_local_IRQHandler_34,
    fabric_f2h_19_u54_local_IRQHandler_35,

    fabric_f2h_20_u54_local_IRQHandler_36,
    fabric_f2h_21_u54_local_IRQHandler_37,
    fabric_f2h_22_u54_local_IRQHandler_38,
    fabric_f2h_23_u54_local_IRQHandler_39,
    fabric_f2h_24_u54_local_IRQHandler_40,
    fabric_f2h_25_u54_local_IRQHandler_41,
    fabric_f2h_26_u54_local_IRQHandler_42,
    fabric_f2h_27_u54_local_IRQHandler_43,
    fabric_f2h_28_u54_local_IRQHandler_44,
    fabric_f2h_29_u54_local_IRQHandler_45,

    fabric_f2h_30_u54_local_IRQHandler_46,
    fabric_f2h_31_u54_local_IRQHandler_47
};

/* U54 3 */
local_int_p_t local_irq_handler_u54_3_table[E51_LOCAL_NUM_SOURCES] =
{
    /*reference multiple interrupts*/
    spare_u54_local_IRQHandler_0,
    spare_u54_local_IRQHandler_1,
    spare_u54_local_IRQHandler_2,

    /*parse hart ID to discover which mac is the source*/
    mac_mmsl_u54_3_local_IRQHandler_3,
    mac_emac_u54_3_local_IRQHandler_4,
    mac_queue3_u54_3_local_IRQHandler_5,
    mac_queue2_u54_3_local_IRQHandler_6,
    mac_queue1_u54_3_local_IRQHandler_7,
    mac_int_u54_3_local_IRQHandler_8,

    /*parse hart ID to discover which wdog is the source*/
    wdog_tout_u54_h3_local_IRQHandler_9,
    mvrp_u54_local_IRQHandler_10,
    mmuart_u54_h3_local_IRQHandler_11,

    spare_u54_local_IRQHandler_12,
    spare_u54_local_IRQHandler_13,
    spare_u54_local_IRQHandler_14,
    spare_u54_local_IRQHandler_15,

    fabric_f2h_0_u54_local_IRQHandler_16,
    fabric_f2h_1_u54_local_IRQHandler_17,
    fabric_f2h_2_u54_local_IRQHandler_18,
    fabric_f2h_3_u54_local_IRQHandler_19,
    fabric_f2h_4_u54_local_IRQHandler_20,
    fabric_f2h_5_u54_local_IRQHandler_21,
    fabric_f2h_6_u54_local_IRQHandler_22,
    fabric_f2h_7_u54_local_IRQHandler_23,
    fabric_f2h_8_u54_local_IRQHandler_24,
    fabric_f2h_9_u54_local_IRQHandler_25,

    fabric_f2h_10_u54_local_IRQHandler_26,
    fabric_f2h_11_u54_local_IRQHandler_27,
    fabric_f2h_12_u54_local_IRQHandler_28,
    fabric_f2h_13_u54_local_IRQHandler_29,
    fabric_f2h_14_u54_local_IRQHandler_30,
    fabric_f2h_15_u54_local_IRQHandler_31,
    fabric_f2h_16_u54_local_IRQHandler_32,
    fabric_f2h_17_u54_local_IRQHandler_33,
    fabric_f2h_18_u54_local_IRQHandler_34,
    fabric_f2h_19_u54_local_IRQHandler_35,

    fabric_f2h_20_u54_local_IRQHandler_36,
    fabric_f2h_21_u54_local_IRQHandler_37,
    fabric_f2h_22_u54_local_IRQHandler_38,
    fabric_f2h_23_u54_local_IRQHandler_39,
    fabric_f2h_24_u54_local_IRQHandler_40,
    fabric_f2h_25_u54_local_IRQHandler_41,
    fabric_f2h_26_u54_local_IRQHandler_42,
    fabric_f2h_27_u54_local_IRQHandler_43,
    fabric_f2h_28_u54_local_IRQHandler_44,
    fabric_f2h_29_u54_local_IRQHandler_45,

    fabric_f2h_30_u54_local_IRQHandler_46,
    fabric_f2h_31_u54_local_IRQHandler_47
};

/* U54 4 */
local_int_p_t local_irq_handler_u54_4_table[E51_LOCAL_NUM_SOURCES] =
{
    /*reference multiple interrupts*/
    spare_u54_local_IRQHandler_0,
    spare_u54_local_IRQHandler_1,
    spare_u54_local_IRQHandler_2,

    /*parse hart ID to discover which mac is the source*/
    mac_mmsl_u54_4_local_IRQHandler_3,
    mac_emac_u54_4_local_IRQHandler_4,
    mac_queue3_u54_4_local_IRQHandler_5,
    mac_queue2_u54_4_local_IRQHandler_6,
    mac_queue1_u54_4_local_IRQHandler_7,
    mac_int_u54_4_local_IRQHandler_8,

    /*parse hart ID to discover which wdog is the source*/
    wdog_tout_u54_h4_local_IRQHandler_9,
    mvrp_u54_local_IRQHandler_10,
    mmuart_u54_h4_local_IRQHandler_11,

    spare_u54_local_IRQHandler_12,
    spare_u54_local_IRQHandler_13,
    spare_u54_local_IRQHandler_14,
    spare_u54_local_IRQHandler_15,

    fabric_f2h_0_u54_local_IRQHandler_16,
    fabric_f2h_1_u54_local_IRQHandler_17,
    fabric_f2h_2_u54_local_IRQHandler_18,
    fabric_f2h_3_u54_local_IRQHandler_19,
    fabric_f2h_4_u54_local_IRQHandler_20,
    fabric_f2h_5_u54_local_IRQHandler_21,
    fabric_f2h_6_u54_local_IRQHandler_22,
    fabric_f2h_7_u54_local_IRQHandler_23,
    fabric_f2h_8_u54_local_IRQHandler_24,
    fabric_f2h_9_u54_local_IRQHandler_25,

    fabric_f2h_10_u54_local_IRQHandler_26,
    fabric_f2h_11_u54_local_IRQHandler_27,
    fabric_f2h_12_u54_local_IRQHandler_28,
    fabric_f2h_13_u54_local_IRQHandler_29,
    fabric_f2h_14_u54_local_IRQHandler_30,
    fabric_f2h_15_u54_local_IRQHandler_31,
    fabric_f2h_16_u54_local_IRQHandler_32,
    fabric_f2h_17_u54_local_IRQHandler_33,
    fabric_f2h_18_u54_local_IRQHandler_34,
    fabric_f2h_19_u54_local_IRQHandler_35,

    fabric_f2h_20_u54_local_IRQHandler_36,
    fabric_f2h_21_u54_local_IRQHandler_37,
    fabric_f2h_22_u54_local_IRQHandler_38,
    fabric_f2h_23_u54_local_IRQHandler_39,
    fabric_f2h_24_u54_local_IRQHandler_40,
    fabric_f2h_25_u54_local_IRQHandler_41,
    fabric_f2h_26_u54_local_IRQHandler_42,
    fabric_f2h_27_u54_local_IRQHandler_43,
    fabric_f2h_28_u54_local_IRQHandler_44,
    fabric_f2h_29_u54_local_IRQHandler_45,

    fabric_f2h_30_u54_local_IRQHandler_46,
    fabric_f2h_31_u54_local_IRQHandler_47
};

local_int_p_t *local_int_mux[5] =
{
    local_irq_handler_e51_table,
    local_irq_handler_u54_1_table,
    local_irq_handler_u54_2_table,
    local_irq_handler_u54_3_table,
    local_irq_handler_u54_4_table
};

#else
uint8_t (*ext_irq_handler_table[PLIC_NUM_SOURCES])(void) =
{
    Invalid_IRQHandler,
    External_1_IRQHandler,
    External_2_IRQHandler,
    External_3_IRQHandler,
    USART0_plic_4_IRQHandler,
    External_5_IRQHandler,
    External_6_IRQHandler,
    External_7_IRQHandler,
    External_8_IRQHandler,
    External_9_IRQHandler,
    External_10_IRQHandler,
    External_11_IRQHandler,
    External_12_IRQHandler,
    External_13_IRQHandler,
    External_14_IRQHandler,
    External_15_IRQHandler,
    External_16_IRQHandler,
    External_17_IRQHandler,
    External_18_IRQHandler,
    External_19_IRQHandler,
    External_20_IRQHandler,
    External_21_IRQHandler,
    External_22_IRQHandler,
    dma_ch0_DONE_IRQHandler,
    dma_ch0_ERR_IRQHandler,
    dma_ch1_DONE_IRQHandler,
    dma_ch1_ERR_IRQHandler,
    dma_ch2_DONE_IRQHandler,
    dma_ch2_ERR_IRQHandler,
    dma_ch3_DONE_IRQHandler,
    dma_ch3_ERR_IRQHandler,
    External_31_IRQHandler,
    External_32_IRQHandler,
    External_33_IRQHandler,
    External_34_IRQHandler,
    External_35_IRQHandler,
    External_36_IRQHandler,
    External_37_IRQHandler,
    External_38_IRQHandler,
    External_39_IRQHandler,
    External_40_IRQHandler,
    External_41_IRQHandler,
    External_42_IRQHandler,
    External_43_IRQHandler,
    External_44_IRQHandler,
    External_45_IRQHandler,
    External_46_IRQHandler,
    External_47_IRQHandler,
    External_48_IRQHandler,
    External_49_IRQHandler,
    External_50_IRQHandler,
    External_51_IRQHandler,
    External_52_IRQHandler,
    MAC0_plic_53_IRQHandler

};
#endif
/*------------------------------------------------------------------------------
 *
 */
void handle_m_ext_interrupt(void)
{

    volatile uint32_t int_num  = PLIC_ClaimIRQ();

    if (INVALID_IRQn == int_num)
    {
       return;
    }


    uint8_t disable = EXT_IRQ_KEEP_ENABLED;
#ifndef SIFIVE_HIFIVE_UNLEASHED
    disable = ext_irq_handler_table[int_num /* + OFFSET_TO_MSS_GLOBAL_INTS Think this was required in early bitfile */]();
#else
    disable = ext_irq_handler_table[int_num]();
#endif

    PLIC_CompleteIRQ(int_num);

    if(EXT_IRQ_DISABLE == disable)
    {
        PLIC_DisableIRQ((PLIC_IRQn_Type)int_num);
    }

}


/*------------------------------------------------------------------------------
 *
 */
void handle_local_interrupt(uint8_t interrupt_no)
{
#ifndef SIFIVE_HIFIVE_UNLEASHED    /* no local interrupts on unleashed */
    uint32_t mhart_id = read_csr(mhartid);
    uint8_t local_interrupt_no = interrupt_no - 16;
    local_int_p_t *local_int_table = local_int_mux[mhart_id];

    (*local_int_table[local_interrupt_no])();

#endif
}


/**
 * call once at startup
 * @return
 */
void reset_mtime(void)
{
#if ROLLOVER_TEST
    CLINT->MTIME = 0xFFFFFFFFFFFFF000U;
#else
    CLINT->MTIME = 0U;
#endif
}

/**
 * Configure system tick
 * @return
 */
uint32_t SysTick_Config(void)
{
    const uint32_t tick_rate[5] = {HART0_TICK_RATE_MS,    HART1_TICK_RATE_MS    ,HART2_TICK_RATE_MS    ,HART3_TICK_RATE_MS    ,HART4_TICK_RATE_MS};
    volatile uint32_t ret_val = ERROR;

    uint32_t mhart_id = read_csr(mhartid);

    g_systick_increment[mhart_id] = ((MSS_RTC_TOGGLE_CLK/1000U)  * tick_rate[mhart_id]);

    if (g_systick_increment[mhart_id] > 0)
    {

        CLINT->MTIMECMP[mhart_id] = CLINT->MTIME + g_systick_increment[mhart_id];

        set_csr(mie, MIP_MTIP);   /* mie Register - Machine Timer Interrupt Enable */

        __enable_irq();

        ret_val = SUCCESS;
    }

    return (ret_val);
}





/*------------------------------------------------------------------------------
 * RISC-V interrupt handler for machine timer interrupts.
 */
void handle_m_timer_interrupt(void)
{

    uint32_t hart_id = read_csr(mhartid);
    clear_csr(mie, MIP_MTIP);

    SysTick_Handler(hart_id);

    CLINT->MTIMECMP[read_csr(mhartid)] = CLINT->MTIME + g_systick_increment[hart_id];

    set_csr(mie, MIP_MTIP);

}


/**
 *
 */
void handle_m_soft_interrupt(void)
{
    uint32_t hart_id = read_csr(mhartid);

    switch(hart_id)
    {
        case 0:
            Software_h0_IRQHandler();
            break;
        case 1:
            Software_h1_IRQHandler();
            break;
        case 2:
            Software_h2_IRQHandler();
            break;
        case 3:
            Software_h3_IRQHandler();
            break;
        case 4:
            Software_h4_IRQHandler();
            break;
    }

    /*Clear software interrupt*/
    clear_soft_interrupt();
}

void trap_from_machine_mode(uintptr_t * regs, uintptr_t dummy, uintptr_t mepc)
{
  uintptr_t mcause = read_csr(mcause);

  if ((mcause & MCAUSE_INT) && ((mcause & MCAUSE_CAUSE)  == IRQ_M_EXT))
  {
      handle_m_ext_interrupt();
  }
  else if ((mcause & MCAUSE_INT) && ((mcause & MCAUSE_CAUSE)  > 15U)&& ((mcause & MCAUSE_CAUSE)  < 64U))
  {
      handle_local_interrupt(mcause & MCAUSE_CAUSE);
  }
  else if ((mcause & MCAUSE_INT) && ((mcause & MCAUSE_CAUSE)  == IRQ_M_TIMER))
  {
      handle_m_timer_interrupt();
  }
  else if ((mcause & MCAUSE_INT) && ((mcause & MCAUSE_CAUSE)  == IRQ_M_SOFT))
  {
      handle_m_soft_interrupt();
  }
  else
  {
		uint32_t i;
		while(1)
		{
			/* wait for watchdog */
			i++;        /* added some code as SC debugger hangs if in loop doing nothing */
			if(i == 0x1000U)
				i = 0U;
		}
		switch(mcause)
		{

			case CAUSE_LOAD_PAGE_FAULT:
			case CAUSE_STORE_PAGE_FAULT:
			case CAUSE_FETCH_ACCESS:
			case CAUSE_LOAD_ACCESS:
			case CAUSE_STORE_ACCESS:

				break;
			default:
				bad_trap(regs, dummy, mepc);
		}
  }

}

#ifdef __cplusplus
}
#endif
