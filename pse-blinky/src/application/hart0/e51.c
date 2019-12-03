/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software example
 *
 */

#include "mpfs_hal/mss_hal.h"

#include "drivers/mss_gpio/mss_gpio.h"
#include "drivers/mss_uart/mss_uart.h"

#include "inc/common.h"

uint64_t uart_lock;


uint8_t gpio0_bit0_or_gpio2_bit13_plic_0_IRQHandler(void)
{
	MSS_UART_polled_tx_string(&g_mss_uart0_lo,
			"\r\nSetting output 0 to high\r\n");

	MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_0, 1);
	MSS_GPIO_clear_irq(GPIO0_LO, MSS_GPIO_0);
	return EXT_IRQ_KEEP_ENABLED;
}


uint8_t gpio0_bit1_or_gpio2_bit13_plic_1_IRQHandler(void)
{
	MSS_UART_polled_tx_string(&g_mss_uart0_lo,
			"\r\nSetting output 1 to high\r\n");

	MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_1, 1);
	MSS_GPIO_clear_irq(GPIO0_LO, MSS_GPIO_1);
	return EXT_IRQ_KEEP_ENABLED;
}


uint8_t gpio0_bit2_or_gpio2_bit13_plic_2_IRQHandler(void)
{
	MSS_UART_polled_tx_string(&g_mss_uart0_lo,
			"\r\nSetting output 2 to high\r\n");

	MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_2, 1);
	MSS_GPIO_clear_irq(GPIO0_LO, MSS_GPIO_2);
	return EXT_IRQ_KEEP_ENABLED;
}


uint8_t gpio0_non_direct_plic_IRQHandler(void)
{
	return EXT_IRQ_KEEP_ENABLED;
}


uint8_t gpio1_non_direct_plic_IRQHandler(void)
{
	return EXT_IRQ_KEEP_ENABLED;
}


uint8_t gpio2_non_direct_plic_IRQHandler(void)
{
	return EXT_IRQ_KEEP_ENABLED;
}


void e51_setup(void)
{
    /*This mutex is used to serialize accesses to UART0 when all harts want to
     * TX/RX on UART0. This mutex is shared across all harts.*/
    mss_init_mutex((uint64_t)&uart_lock);

    /* Bring the UART0, GPIO0, GPIO1 and GPIO2 out of Reset */
    SYSREG->SOFT_RESET_CR &= ~((1u << 0u) | (1u << 4u) | (1u << 5u)
            | (1u << 19u) | (1u << 20u) | (1u << 21u) | (1u << 22u)
            | (1u << 23u) | (1u << 28u));


    /* Making sure that the default GPIO0 and GPIO1 are used on interrupts.
     * No Non-direct interrupts enabled of GPIO0 and GPIO1.
     * Please see the mss_gpio.h for more description on how GPIO interrupts
     * are routed to the PLIC */
    SYSREG->GPIO_INTERRUPT_FAB_CR = 0x00000000UL;

    /* All clocks on */
    SYSREG->SUBBLK_CLOCK_CR = 0xffffffff;

    /*************************************************************************/
    PLIC_init();
    PLIC_SetPriority_Threshold(0);
    PLIC_SetPriority(GPIO0_BIT0_or_GPIO2_BIT0_PLIC_0, 2);
    PLIC_SetPriority(GPIO0_BIT1_or_GPIO2_BIT1_PLIC_1, 2);
    PLIC_SetPriority(GPIO0_BIT2_or_GPIO2_BIT2_PLIC_2, 2);

    PLIC_SetPriority(GPIO0_NON_DIRECT_PLIC, 2);
    PLIC_SetPriority(GPIO1_NON_DIRECT_PLIC, 2);
    PLIC_SetPriority(GPIO2_NON_DIRECT_PLIC, 2);

    __disable_local_irq((int8_t) MMUART0_E51_INT);
    __enable_irq();

    /* GPIO0 */
    MSS_GPIO_init(GPIO0_LO);

    MSS_GPIO_config(GPIO0_LO, MSS_GPIO_0,
    MSS_GPIO_INPUT_MODE | MSS_GPIO_IRQ_EDGE_POSITIVE);

    MSS_GPIO_config(GPIO0_LO, MSS_GPIO_1,
    MSS_GPIO_INPUT_MODE | MSS_GPIO_IRQ_EDGE_POSITIVE);

    MSS_GPIO_config(GPIO0_LO, MSS_GPIO_2,
    MSS_GPIO_INPUT_MODE | MSS_GPIO_IRQ_EDGE_POSITIVE);

    MSS_GPIO_enable_irq(GPIO0_LO, MSS_GPIO_0);
    MSS_GPIO_enable_irq(GPIO0_LO, MSS_GPIO_1);
    MSS_GPIO_enable_irq(GPIO0_LO, MSS_GPIO_2);

    /* GPIO1 */
    MSS_GPIO_init(GPIO1_LO);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_0, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_1, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_2, MSS_GPIO_OUTPUT_MODE);

    /* Set all outputs of GPIO1 to 0 */
    MSS_GPIO_set_outputs(GPIO1_LO, 0x0);

    MSS_UART_init(&g_mss_uart0_lo, MSS_UART_115200_BAUD,
            MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY);

    /* Synchronizing the hart's applications because they have interdependencies
     *
     * Waking up hart 1 from sleep. It needs to be sent after the hart1 entered
     * sleep. Atomic orchestration needs to be implemented, or a delay
     * introduced when hart0 init takes less time to execute than hart init
     * which is trying to synch with. This can happen when hart0 is not
     * using/initializing many peripherals. Configuring GPIO/UART/IRQ will take
     * enough time for the hart1 to be in sleep when this point of code is
     * reached. */
    raise_soft_interrupt(1u);
}


void e51_application(void)
{
    safe_MSS_UART0_polled_tx_string("Hello World from e51 (hart 0).\r\n");

    while (1)
    {
        // Stay in the infinite loop, never return from main

        const uint64_t delay_loop_max = 10000;
        volatile uint64_t delay_loop_sum = 0;

        for (uint64_t i = 0; i< delay_loop_max; i++) {
            delay_loop_sum = delay_loop_sum + i;
        }

        safe_MSS_UART0_polled_tx_string("Setting outputs 0, 1 and 2 to high\r\n");

        MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_0, 1);
        MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_1, 1);
        MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_2, 1);

        for (uint64_t i = 0; i< delay_loop_max; i++) {
            delay_loop_sum = delay_loop_sum + i;
        }

        safe_MSS_UART0_polled_tx_string("Setting outputs 0, 1 and 2 to low\r\n");
        MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_0, 0);
        MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_1, 0);
        MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_2, 0);
    }
}


/* Main function for the HART0(E51 processor).
 * Application code running on HART0 is placed here.
 * UART0 PLIC interrupt is enabled on hart0.*/
void e51(void)
{
    e51_setup();
    e51_application();

    /* Shouldn't never reach this point */
    while (1)
    {
       volatile static uint64_t counter = 0U;

       /* Added some code as gdb hangs when stepping through an empty infinite loop */
       counter = counter + 1;
    }
}


