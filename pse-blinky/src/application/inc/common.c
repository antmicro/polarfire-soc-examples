/*
 * common.c
 *
 *  Created on: 26 Jul 2019
 */


#include "drivers/mss_uart/mss_uart.h"
#include "mpfs_hal/mss_util.h"
#include "common.h"

extern uint64_t uart_lock;

void safe_MSS_UART0_polled_tx_string(const char *string)
{
    mss_take_mutex((uint64_t)&uart_lock);
    MSS_UART_polled_tx_string(&g_mss_uart0_lo, string);
    mss_release_mutex((uint64_t)&uart_lock);
}
