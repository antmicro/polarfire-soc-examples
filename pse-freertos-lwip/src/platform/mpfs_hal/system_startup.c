/***************************************************************************
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions. All rights reserved.
 *
 * first C code called on startup. Will call user code created outside the HAL.
 *
 * SVN $Revision: 9661 $
 * SVN $Date: 2018-01-15 10:43:33 +0000 (Mon, 15 Jan 2018) $
 */
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mss_mpu.h"
#include "mss_hal.h"
#include "mss_seg.h"

#if defined(MSS_MAC_USE_DDR)
#include "config/mss_mac/mss_ethernet_mac_user_config.h"
#endif

#include "system_startup.h"

/*------------------------------------------------------------------------------
 * Symbols from the linker script used to locate the text, data and bss sections.
 */

extern unsigned int __text_load;    // @suppress("Unused variable declaration in file scope")
extern unsigned int __text_start;   // @suppress("Unused variable declaration in file scope")
extern unsigned int _etext;         // @suppress("Unused variable declaration in file scope")
extern unsigned int _uPROM_start;   // @suppress("Unused variable declaration in file scope")
extern unsigned int _uPROM_end;     // @suppress("Unused variable declaration in file scope")

extern unsigned int __data_load;    // @suppress("Unused variable declaration in file scope")
extern unsigned int __data_start;   // @suppress("Unused variable declaration in file scope")
extern unsigned int _edata;         // @suppress("Unused variable declaration in file scope")

extern unsigned int __sbss_start;   // @suppress("Unused variable declaration in file scope")
extern unsigned int __sbss_end;     // @suppress("Unused variable declaration in file scope")

extern unsigned int __bss_start;    // @suppress("Unused variable declaration in file scope")
extern unsigned int __bss_end;      // @suppress("Unused variable declaration in file scope")

extern unsigned int __sc_load;      // @suppress("Unused variable declaration in file scope")
extern unsigned int __sc_start;     // @suppress("Unused variable declaration in file scope")
extern unsigned int __sc_end;       // @suppress("Unused variable declaration in file scope")

extern unsigned int __vector_table_load;     // @suppress("Unused variable declaration in file scope")
extern unsigned int _vector_table_end_load;  // @suppress("Unused variable declaration in file scope")


/*
 * Function Declarations
 */
void e51(void);
void u54_1(void);
void u54_2(void);
void u54_3(void);
void u54_4(void);


/*==============================================================================
 * E51 startup.
 * If you need to modify this function, create your own one if a user directory space
 * e.g. /hart0/e51.c
 */
__attribute__((weak)) int main_first_hart(void)
{
  volatile int i;

	/* mscratch must be init to zero- On the unleashed issue occurs in trap handling code if not init. */
	write_csr(mscratch, 0);
	write_csr(mcause, 0);
	write_csr(mepc, 0);

	uint32_t hartid = read_csr(mhartid);
	if(hartid == 0)
	{
		init_memory();
#if PSE
		SYSREG->SUBBLK_CLOCK_CR = 0xffffffff;       /* all clocks on */
#if defined(HW_EMUL_USE_GEM0)
		MPU[4].CFG[0].pmp = MPU_CFG(0x00000000, 12u);  // Address 0
		MPU[4].CFG[0].mode = MPU_MODE_READ | MPU_MODE_NAPOT;
		MPU[4].CFG[1].pmp = MPU_CFG(0x08000000, 21u);  // LIM 2MB
		MPU[4].CFG[1].mode = MPU_MODE_READ | MPU_MODE_WRITE |MPU_MODE_NAPOT;

#if defined(MSS_MAC_USE_DDR)
#if MSS_MAC_USE_DDR == MSS_MAC_MEM_DDR
		MPU[4].CFG[2].pmp = MPU_CFG(0xC0000000, 21u);
#elif MSS_MAC_USE_DDR == MSS_MAC_MEM_FIC0
		MPU[4].CFG[2].pmp = MPU_CFG(0x60000000, 21u);
#elif MSS_MAC_USE_DDR == MSS_MAC_MEM_FIC1
		MPU[4].CFG[2].pmp = MPU_CFG(0xE0000000, 21u);
#elif MSS_MAC_USE_DDR == MSS_MAC_MEM_CRYPTO
		MPU[4].CFG[2].pmp = MPU_CFG(0x22002000, 21u);
#else
#error "bad memory region defined"
#endif
		MPU[4].CFG[2].mode = MPU_MODE_READ | MPU_MODE_WRITE |MPU_MODE_NAPOT;
#endif /* defined(MSS_MAC_USE_DDR) */
#endif /* defined(HW_EMUL_USE_GEM0) */

#if defined(HW_EMUL_USE_GEM1)
		MPU[5].CFG[0].pmp = MPU_CFG(0x00000000, 12u);  // Address 0
		MPU[5].CFG[0].mode = MPU_MODE_READ | MPU_MODE_NAPOT;
		MPU[5].CFG[1].pmp = MPU_CFG(0x08000000, 21u);  // LIM 2MB
		MPU[5].CFG[1].mode = MPU_MODE_READ | MPU_MODE_WRITE |MPU_MODE_NAPOT;

#if defined(MSS_MAC_USE_DDR)
#if MSS_MAC_USE_DDR == MSS_MAC_MEM_DDR
		MPU[5].CFG[2].pmp = MPU_CFG(0xC0000000, 21u);
#elif MSS_MAC_USE_DDR == MSS_MAC_MEM_FIC0
		MPU[5].CFG[2].pmp = MPU_CFG(0x60000000, 21u);
#elif MSS_MAC_USE_DDR == MSS_MAC_MEM_FIC1
		MPU[5].CFG[2].pmp = MPU_CFG(0xE0000000, 21u);
#elif MSS_MAC_USE_DDR == MSS_MAC_MEM_CRYPTO
		MPU[5].CFG[2].pmp = MPU_CFG(0x22002000, 21u);
#else
#error "bad memory region defined"
#endif
		MPU[5].CFG[2].mode = MPU_MODE_READ | MPU_MODE_WRITE |MPU_MODE_NAPOT;
#endif /* defined(MSS_MAC_USE_DDR) */
#endif /* defined(HW_EMUL_USE_GEM1) */

		SEG[0].CFG[0].offset = -(0x0080000000ll >> 24u);
		SEG[0].CFG[1].offset = -(0x1000000000ll >> 24u);
		SEG[1].CFG[2].offset = -(0x00C0000000ll >> 24u);
		SEG[1].CFG[3].offset = -(0x1400000000ll >> 24u);
		SEG[1].CFG[4].offset = -(0x00D0000000ll >> 24u);
		SEG[1].CFG[5].offset = -(0x1800000000ll >> 24u);
#endif
        e51();
	}

   /* should never get here */
	while(1)
	{
		/* add code here */
		i++;				/* added some code as debugger hangs if in loop doing nothing */
		if(i == 0x1000)
			i = 0;
	}

  return 0;
}

/*==============================================================================
 * U54s startup.
 * This is called from entry.S
 * If you need to modify this function, create your own one if a user directory space
 */
__attribute__((weak)) int main_other_hart(void)
{

  uint32_t hartid = read_csr(mhartid);

  /* mscratch must be init to zero- On the unleashed issue occurs in trap handling code if not init. */
    write_csr(mscratch, 0);
    write_csr(mcause, 0);
    write_csr(mepc, 0);

  switch(hartid)
  {
    case 1:
      u54_1();
      break;

    case 2:
      u54_2();
      break;

    case 3:
      u54_3();
      break;

    case 4:
      u54_4();
      break;

    default:

      break;
  }

  	 /* should never get here */
	volatile uint32_t i;
	while(1)
	{
		/* add code here */
		i++;				/* added some code as debugger hangs if in loop doing nothing */
		if(i == 0x1000)
			i = 0;
	}

  return 0;

}

/*==============================================================================
 * E51 code executing after system startup.
 * If you need to modify this function, create your own one if a user directory space
 */
 __attribute__((weak)) void e51(void)
 {
	uint32_t hartid = read_csr(mhartid);
	volatile uint32_t i;

	while(1)
	{
		/* add code here */
		i++;				/* added some code as debugger hangs if in loop doing nothing */
		if(i == 0x1000)
			i = 0;
	}
}

 /*==============================================================================
  * First U54.
  * If you need to modify this function, create your own one if a user directory space
  */
 __attribute__((weak)) void u54_1(void)
 {
 	uint32_t hartid = read_csr(mhartid);
 	volatile uint32_t i;

 	asm("wfi");

 	while(1)
 	{
 		/* add code here */
 		i++;				/* added some code as debugger hangs if in loop doing nothing */
 		if(i == 0x1000)
 			i = 0;
 	}
 }


/*==============================================================================
 * Second U54.
 * If you need to modify this function, create your own one if a user directory space
 */
__attribute__((weak)) void u54_2(void)
{
	uint32_t hartid = read_csr(mhartid);
	volatile uint32_t i;

    asm("wfi");

    while(1)
	{
		/* add code here */
		i++;				/* added some code as debugger hangs if in loop doing nothing */
		if(i == 0x1000)
			i = 0;
	}
}


/*==============================================================================
 * Third U54.
 * If you need to modify this function, create your own one if a user directory space
 */
 __attribute__((weak)) void u54_3(void)
 {
	uint32_t hartid = read_csr(mhartid);
	volatile uint32_t i;

    asm("wfi");

    while(1)
	{
		/* add code here */
		i++;				/* added some code as debugger hangs if in loop doing nothing */
		if(i == 0x1000)
			i = 0;
	}
}


/*==============================================================================
 * Fourth U54.
 * If you need to modify this function, create your own one if a user directory space
 */
 __attribute__((weak)) void u54_4(void)
 {
	uint32_t hartid = read_csr(mhartid);
	volatile uint32_t i;

    asm("wfi");

    while(1)
	{
		/* add code here */
		i++;				/* added some code as debugger hangs if in loop doing nothing */
		if(i == 0x1000)
			i = 0;
	}
}


 /*------------------------------------------------------------------------------
  * _start() function called invoked
  * This function is called from  startup_cortexm1.S on power up and warm reset.
  */
 __attribute__((weak)) void init_memory( void)
{
#if 0 /* todo: show example, not currently required */
    /*
     * Copy text section if required (copy executable from LMA to VMA).
     */
    {
        unsigned int * text_lma = &__text_load;
        unsigned int * end_text_vma = &_etext;
        unsigned int * text_vma = &__text_start;

        if ( text_vma != text_lma)
        {
            while ( text_vma <= end_text_vma)
            {
                *text_vma++ = *text_lma++;
            }
        }
    }
#endif
#if 0 /* todo: show example, not currently required */
    /*
     * Copy data section if required (initialized variables).
     */
    {
        unsigned int * data_lma = &__data_load;
        unsigned int * end_data_vma = &_edata;
        unsigned int * data_vma = &__data_start;

        if ( data_vma != data_lma )
        {
            while ( data_vma <= end_data_vma )
            {
                *data_vma++ = *data_lma++;
            }
        }
    }
#endif
    /*
     * Zero out the bss section (set non-initialized variables to 0).
     */
    {
        unsigned int * bss     = &__sbss_start;
        unsigned int * bss_end = &__sbss_end;
        if ( bss_end > bss)
        {
            while ( bss <= bss_end )
            {
                *bss++ = 0;
            }
        }

        bss = &__bss_start;
        bss_end = &__bss_end;

        if ( bss_end > bss)
        {
             while ( bss <= bss_end )
            {
                *bss++ = 0;
            }
        }
    }
}




