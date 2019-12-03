/*
 * configure_network.h
 *
 */

#ifndef UTILS_CONFIGURE_NETWORK_H_
#define UTILS_CONFIGURE_NETWORK_H_

#include "drivers/mss_mac/mss_ethernet_mac.h"

#include "lwip/ip_addr.h"
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
//#include "queue.h"

extern mss_mac_cfg_t g_mac_config;
extern TaskHandle_t thandle_web;

#define IP4_IP_ADDR 0
#define IP4_IP_MASK 1
#define IP4_IP_GW   2

typedef enum __ntm_dhcp_mode_t
{
    NTM_DHCP_FIXED    = 0,
    NTM_DHCP_PURE     = 1,
    NTM_DHCP_FALLBACK = 2
} ntm_dhcp_mode_t;

extern ntm_dhcp_mode_t g_network_addr_mode; /* Current network addressing mode */
extern ip4_addr_t      g_ip_address;        /* Configured fixed/default address */
extern ip4_addr_t      g_ip_mask;           /* Configured fixed/default mask */
extern ip4_addr_t      g_ip_gateway;        /* Configured fixed/default gateway address */

uint32_t get_ipv4_address(int32_t select);
void     set_ipv4_address(int32_t select, ip4_addr_t *address, uint32_t commit);

void     set_ipv4_address_mode(int32_t mode);

void     get_mac_address(uint8_t * mac_addr);
void     set_mac_address(uint8_t * mac_addr);

void prvEthernetConfigureInterface(void * param);

#endif /* UTILS_CONFIGURE_NETWORK_H_ */
