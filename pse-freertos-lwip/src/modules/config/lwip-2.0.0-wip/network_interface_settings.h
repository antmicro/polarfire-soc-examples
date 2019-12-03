
#ifndef NETWORK_INTERFACE_SETTINGS_H_
#define NETWORK_INTERFACE_SETTINGS_H_

#include <stdint.h>

#include "lwip/ip4_addr.h"
#include "utils/configure_network.h"

#define NETWORK_DHCP_MODE NTM_DHCP_FIXED /* Use static address */

#define STATIC_IPV4_ADDRESS "172.16.0.3"
#define STATIC_IPV4_MASK    "255.255.255.0"
#define STATIC_IPV4_GATEWAY "172.16.0.1"

#define CONFIG_MAC0_BYTE_0 0x00
#define CONFIG_MAC0_BYTE_1 0xFC
#define CONFIG_MAC0_BYTE_2 0x00
#define CONFIG_MAC0_BYTE_3 0x12
#define CONFIG_MAC0_BYTE_4 0x34
#define CONFIG_MAC0_BYTE_5 0x56

void prvEthernetConfigureInterface(void * param);
void str_to_ipv4_address(ip4_addr_t *address, char *string);
int32_t ethernetMaskToBits(ip4_addr_t mask);
int32_t currentMaskToBits();

#endif /* NETWORK_INTERFACE_SETTINGS_H_ */
