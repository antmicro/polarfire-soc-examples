/***********************************************************************************
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions. All rights reserved.
 *
 * code running on e51
 *
 * SVN $Revision: 9661 $
 * SVN $Date: 2018-01-15 10:43:33 +0000 (Mon, 15 Jan 2018) $
 */

#include "config/hardware/hw_platform.h"
#include "config/software/drivers/mss_mac/mss_ethernet_mac_user_config.h"
#include "config/lwip-2.0.0-wip/network_interface_settings.h"

#include "drivers/mss_uart/mss_uart.h"

#include "drivers/mss_mac/mac_registers.h"
#include "drivers/mss_mac/pse_mac_regs.h"
#include "drivers/mss_mac/mss_ethernet_mac.h"

#include "mpfs_hal/system_startup.h"
#include "mpfs_hal/mss_util.h"

#include "FreeRTOS.h"
#include "../portable/GCC/RISCV/port.h"

#include "lwip/tcpip.h"
#include "web-server/httpserver-netconn.h"

#include "printf.h"

#include "utils/common_functions.h"
#include "utils/configure_network.h"

#include "startup_mss_setup.h"

TaskHandle_t thandle_web;

void mss_config_task( void *pvParameters )
{
    int count;
    volatile int i;
    int8_t info_string[100];
    uint64_t mcycle_start = 0;
    uint64_t mcycle_end = 0;
    uint64_t delta_mcycle = 0;
    uint32_t num_loops = 1000000;
    uint32_t hartid = read_csr(mhartid);
    static volatile uint64_t dummy_h0 = 0;
    uint8_t rx_buff[1];
    uint32_t rx_idx  = 0;
    uint8_t rx_size = 0;
    volatile uint32_t gpio_inputs;

    printf_("Configuring MSS\r\n");
    mss_config();

    PLIC_init();
    __disable_local_irq((int8_t)MMUART0_E51_INT);

    vPortSetupTimer();

    __enable_irq();

    printf_("IRQ enabled\r\n");

#if defined(MSS_MAC_USE_DDR) && (MSS_MAC_USE_DDR == MSS_MAC_MEM_CRYPTO)
    *ATHENA_CR = SYSREG_ATHENACR_RESET | SYSREG_ATHENACR_RINGOSCON;
    *ATHENA_CR = SYSREG_ATHENACR_RINGOSCON;
    CALIni();
    MSS_UART_polled_tx_string(&g_mss_uart0_lo, "CALIni() done..\r\n");
#endif

    PLIC_SetPriority(MAC0_INT_PLIC,  7);
    PLIC_SetPriority(MAC1_INT_PLIC,  7);
    PLIC_SetPriority(MAC0_EMAC_PLIC, 7);
    PLIC_SetPriority(MAC1_EMAC_PLIC, 7);
    PLIC_SetPriority(MAC0_MMSL_PLIC, 7);
    PLIC_SetPriority(MAC1_MMSL_PLIC, 7);

#if defined MSS_MAC_QUEUES
    PLIC_SetPriority(MAC0_QUEUE1_PLIC, 7);
    PLIC_SetPriority(MAC0_QUEUE2_PLIC, 7);
    PLIC_SetPriority(MAC0_QUEUE3_PLIC, 7);
    PLIC_SetPriority(MAC1_QUEUE1_PLIC, 7);
    PLIC_SetPriority(MAC1_QUEUE2_PLIC, 7);
    PLIC_SetPriority(MAC1_QUEUE3_PLIC, 7);
#endif

    printf_("Init TCP/IP\r\n");

    tcpip_init(prvEthernetConfigureInterface, NULL);

    /* hack - must use a semaphore or other synchronization?
     * Hold off starting these tasks until network is active
     * */
    while(0 == g_mac0.g_mac_available)
    {
        vTaskDelay(1);
    }

    vTaskResume(thandle_web);
    printf_("Configuration done.\r\n");

    vTaskSuspend(NULL);
}


void e51(void)
{

    BaseType_t rtos_result;

    write_csr(mscratch, 0);
    write_csr(mcause, 0);
    write_csr(mepc, 0);
    init_memory();

    PLIC_init();


    MSS_UART_init( &g_mss_uart0_lo,
                MSS_UART_115200_BAUD,
                MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

    printf_("PolarFire SoC Icicle FreeRTOS WebServer Demo\r\n");

    rtos_result = xTaskCreate( mss_config_task, "mss_config_task", 4000, NULL,
            uartPRIMARY_PRIORITY, NULL );
    if(1 != rtos_result)
    {
        printf_("Creating of e51_task task failed\r\n");
        infinite_loop();
    }

    rtos_result = xTaskCreate(http_server_netconn_thread, (char *) "http_server",
            4000, NULL, uartPRIMARY_PRIORITY + 3, &thandle_web );
    if(1 != rtos_result)
    {
        printf_("Creating of http_server_netconn_thread task failed\r\n");
        infinite_loop();
    }
    vTaskSuspend(thandle_web);

    printf_("To bridge Linux's host network with the emulation make sure \r\n");
    printf_("that the debug launcher invokes the bridge macro.\r\n");
    printf_("Modify the debug launcher \"pse-freertos-lwip Renode hart0 Debug\"\r\n");
    printf_("Append to \"Startup -> Initialization Commands\" the following:\r\n");
    printf_("monitor runMacro $BridgeNetworkMac0\r\n\r\n");

    printf_("To see wireshark logger on each start append the following:\r\n");
    printf_("monitor runMacro $WiresharkRun\r\n\r\n");

    printf_("At the moment these features are not supported on Windows\r\n");
    if (g_network_addr_mode == NTM_DHCP_FIXED)
    {
        int32_t maskBits = currentMaskToBits();
        if (-1 == maskBits)
        {
            printf_("IP Mask might not be configured correctly!\r\n");
            printf_("Will continue, but networking might not work\r\n");
        }

        printf_("When networks are bridged, issue as super user on your Linux \r\n");
        printf_("terminal the following command: \r\n");
        printf_("ifconfig renode-tap0 %s/%d up\r\n\r\n", STATIC_IPV4_GATEWAY, maskBits);

        printf_("Open your web-browser and point it to location:\r\n");
        printf_("http://%s\r\n\r\n", STATIC_IPV4_ADDRESS);
    }
    else
    {
        printf_("Check your DHCP IP address and open browser with its IP.\r\n");
        printf_("Make sure the renode-tap0 is up and configured appropriately.\r\n");
    }

    /* Start the kernel.  From here on, only tasks and interrupts will run. */
    printf_("Starting kernel\r\n");
    vTaskStartScheduler();
}
