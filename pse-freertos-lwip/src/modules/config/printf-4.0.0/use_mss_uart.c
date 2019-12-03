/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 */


#include "drivers/mss_uart/mss_uart.h"

/*
 * This allows the printf to use MSS_UART0, but can't be invoked before
 * initialising the MSS_UART0 peripheral, make sure to configure the MSS
 * properly before invoking printf
 */
void _putchar(char character)
{
    MSS_UART_polled_tx(&g_mss_uart0_lo, &character, 1);
}
