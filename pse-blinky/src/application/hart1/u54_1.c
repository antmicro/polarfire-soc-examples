/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software example
 *
 */

#include <stdio.h>

#include "mpfs_hal/mss_hal.h"

#include "drivers/mss_uart/mss_uart.h"

#include "inc/common.h"

volatile uint64_t count_sw_ints_h1 = 0;
volatile uint64_t dummy_h1 = 0;


void Software_h1_IRQHandler(void)
{
	uint32_t hart_id = read_csr(mhartid);
	if (hart_id == 1)
	{
		count_sw_ints_h1++;
	}
}

/* Synchronizing the hart's applications because they have interdependencies */
void u54_1_init_hal(void)
{
    /* Clear pending software interrupt in case there was any.
     * Enable only the software interrupt so that the E51 core can bring this
     * core out of WFI by raising a software interrupt. */
    clear_soft_interrupt();
    set_csr(mie, MIP_MSIP);

    /* Put this hart into sleep with WFI. Keep putting hart into sleep until
     * a correct IRQ is received */
    do
    {
        __asm("wfi");
    } while (0 == (read_csr(mip) & MIP_MSIP));

    /* The hart is out of WFI, clear the SW interrupt. Hear onwards Application
     * can enable and use any interrupts as required*/
    clear_soft_interrupt();

    __enable_irq();
}


void u54_1_setup(void)
{
    /* Put your configuration, driver setup here, or do it in the e51 */
}


void u54_1_application(void)
{
    time_benchmark_t mcycle         = { .start = 0, .end = 0, .delta = 0};
    volatile uint64_t loop_count_h1 = 0;
    const uint64_t num_loops        = 100000;
    uint64_t hartid                 = read_csr(mhartid);
    char uart_buf[100];

    while (1)
    {
        mcycle.start = readmcycle();

        for (uint64_t i = 0; i < num_loops; i++) {
            dummy_h1 = i;
        }

        sprintf(uart_buf, "Hart %ld, mcycle_delta=%ld SW_IRQs=%ld mcycle=%ld\r\n",
                hartid, mcycle.delta, count_sw_ints_h1, readmcycle());

        safe_MSS_UART0_polled_tx_string(uart_buf);

        hartid       = read_csr(mhartid);
        mcycle.end   = readmcycle();
        mcycle.delta = mcycle.end - mcycle.start;

        loop_count_h1++;
    }
}

/* Main function for the HART1(U54_1 processor).
 * Application code running on HART1 is placed here
 *
 * The HART1 goes into WFI. HART0 brings it out of WFI when it raises the first
 * Software interrupt to this HART
 */
void u54_1(void)
{
    u54_1_init_hal();
    u54_1_setup();
    u54_1_application();

    /* Shouldn't never reach this point */
    while (1)
    {
       volatile static uint64_t counter = 0U;

       /* Added some code as gdb hangs when stepping through an empty infinite loop */
       counter = counter + 1;
    }
}

