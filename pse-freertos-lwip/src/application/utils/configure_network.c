#include <ctype.h>

#include "utils/configure_network.h"

#include "config/lwip-2.0.0-wip/network_interface_settings.h"

#include "utils/socket_config.h" /* Order of this include is sensitive */

#include "drivers/mss_mac/phy.h"

#include "lwip/inet.h"
#include "lwip/def.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"



/*==============================================================================
 * Network configuration globals.
 */

ntm_dhcp_mode_t g_network_addr_mode = NETWORK_DHCP_MODE;

ip4_addr_t      g_ip_address; /* Configured fixed/default address */
ip4_addr_t      g_ip_mask;    /* Configured fixed/default mask */
ip4_addr_t      g_ip_gateway; /* Configured fixed/default gateway address */

/*==============================================================================
 *
 */
/* lwIP MAC configuration. */
struct netif g_EMAC_if;

/*==============================================================================
 *
 */
uint32_t get_ipv4_address(int32_t select)
{
    uint32_t ret_val;

    switch(select)
    {
        case IP4_IP_MASK:
#if (LWIP_IPV4 && LWIP_IPV6)
            ret_val = ((uint32_t)(g_EMAC_if.netmask.u_addr.ip4.addr));
#else
            ret_val = (uint32_t)(g_EMAC_if.netmask.addr);
#endif
            break;

        case IP4_IP_GW:
#if (LWIP_IPV4 && LWIP_IPV6)
            ret_val = ((uint32_t)(g_EMAC_if.gw.u_addr.ip4.addr));
#else
            ret_val = (uint32_t)(g_EMAC_if.gw.addr);
#endif
            break;

        case IP4_IP_ADDR:
        default:
#if (LWIP_IPV4 && LWIP_IPV6)
            ret_val = ((uint32_t)(g_EMAC_if.ip_addr.u_addr.ip4.addr));
#else
            ret_val = (uint32_t)(g_EMAC_if.ip_addr.addr);
#endif
            break;
    }

    return(ret_val);
}


/*==============================================================================
 *
 */
void get_mac_address(uint8_t * mac_addr)
{
    uint32_t inc;

    for(inc = 0; inc < 6; ++inc)
    {
        mac_addr[inc] = g_EMAC_if.hwaddr[inc];
    }
}


/*==============================================================================
 *
 */
void str_to_ipv4_address(ip4_addr_t *address, char *string)
{
    int count;
    int index;
    uint8_t raw_addr[4];

    index = 0;
    for(count = 0; count != 4; count++)
    {
        raw_addr[count] = strtol(&string[index], 0, 10); /* Convert 1/4 of address to binary */

        while(isdigit(string[index])) /* Step over current number */
            index++;

        index++; /* Then step over '.' */
    }

    IP4_ADDR( address, raw_addr[0], raw_addr[1], raw_addr[2], raw_addr[3] );
}

/*==============================================================================
 *
 */

void set_mac_address(uint8_t * mac_addr)
{

    /* Update stored copies of mac address */
    memcpy(g_EMAC_if.hwaddr,      mac_addr, 6);
    memcpy(g_mac_config.mac_addr, mac_addr, 6);

}


/*==============================================================================
 *
 */
void prvEthernetConfigureInterface(void * param)
{
    extern err_t ethernetif_init( struct netif *netif );


    /* Parameters are not used - suppress compiler error. */
    ( void ) param;

    /*
     * Set mac address
     */
    g_mac_config.phy_addr = PHY_VSC8575_MDIO_ADDR;
    g_mac_config.mac_addr[0] = CONFIG_MAC0_BYTE_0;
    g_mac_config.mac_addr[1] = CONFIG_MAC0_BYTE_1;
    g_mac_config.mac_addr[2] = CONFIG_MAC0_BYTE_2;
    g_mac_config.mac_addr[3] = CONFIG_MAC0_BYTE_3;
    g_mac_config.mac_addr[4] = CONFIG_MAC0_BYTE_4;
    g_mac_config.mac_addr[5] = CONFIG_MAC0_BYTE_5;

    memcpy(g_EMAC_if.hwaddr, g_mac_config.mac_addr, 6);


    /*
     * Start with a default fixed address so that we can bring the network
     * interface up quickly. Will most likely be the normal mode of operation
     * anyway...
     */

    str_to_ipv4_address(&g_ip_address, STATIC_IPV4_ADDRESS);
    str_to_ipv4_address(&g_ip_gateway, STATIC_IPV4_GATEWAY);
    str_to_ipv4_address(&g_ip_mask,    STATIC_IPV4_MASK);


    /* Start out with the default address */
    netif_add( &g_EMAC_if, &g_ip_address, &g_ip_mask, &g_ip_gateway, NULL,
            ethernetif_init, tcpip_input );

    /* Bring interface up */
    netif_set_up(&g_EMAC_if);

    if(NTM_DHCP_FIXED != g_network_addr_mode)
    {
        dhcp_start(&g_EMAC_if);
    }

    /* Make it the default interface */
    netif_set_default(&g_EMAC_if);
}


/*
 * Calculate how many bits are in a specific mask
 */
int32_t ethernetMaskToBits(ip4_addr_t mask)
{
    uint32_t i;
    uint32_t j;
    uint32_t reorderedMask = (mask.addr & 0xff000000u) >> 24 |
                             (mask.addr & 0x00ff0000u) >> 8  |
                             (mask.addr & 0x0000ff00u) << 8  |
                             (mask.addr & 0x000000ffu) << 24;

    /* Break on first 0 */
    for (i = 0; i < 32; ++i) {
        if (!(reorderedMask & 1u << (31 - i))) break;
    }

    /* No need to continue when whole mask is full of 1 */
    if (i == 32) return i;

    /* Continue from the first 0 and test if the rest contains any 1 */
    for (j = i; j < 32; ++j) {
        if (reorderedMask & 1u << (31 - i)) break;
    }

    /* After the 0 there was unexpected 1 */
    if (j != 32) {
        return -1;
    }

    /* Everything was correct, the mask is valid, return the result */
    return i;
}


int32_t currentMaskToBits()
{
    ip4_addr_t mask;
    str_to_ipv4_address(&mask, STATIC_IPV4_MASK);

    return ethernetMaskToBits(mask);
}

