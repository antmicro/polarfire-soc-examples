
/*******************************************************************************
 *  (c) Copyright 2018 Microsemi - PRO. All rights reserved.
 *
 * NULL PHY implementation.
 *
 * This PHY interface is used when there is a direct connection between two GEM
 * instances which does not involve the use of a PHY device.
 *
 * Also used when setting up the default config so that the pointers for the
 * PHY functions in the config always have some valid values.
 *
 * SVN $Revision: 10527 $
 * SVN $Date: 2018-11-10 12:47:05 +0000 (Sat, 10 Nov 2018) $
 */
#include "config/hardware/hw_platform.h"
#include "config/software/drivers/mss_mac/mss_ethernet_mac_user_config.h"

#include "hal/hal.h"
#include "hal/hal_assert.h"

#include "mpfs_hal/mss_plic.h"

#include "mac_registers.h"
#include "pse_mac_regs.h"
#include "mss_ethernet_mac.h"
#include "phy.h"


#ifdef __cplusplus
extern "C" {
#endif

#if MSS_MAC_USE_PHY_NULL

/**************************************************************************//**
 * 
 */
 

void MSS_MAC_NULL_phy_init(/* mss_mac_instance_t*/ void *v_this_mac, uint8_t phy_addr)
{
	/* Nothing to see here... */
    (void)v_this_mac;
    (void)phy_addr;
}

/**************************************************************************//**
 * 
 */
void MSS_MAC_NULL_phy_set_link_speed(/* mss_mac_instance_t*/ void *v_this_mac, uint32_t speed_duplex_select)
{
	/* Nothing to see here... */
    (void)v_this_mac;
    (void)speed_duplex_select;
}

/**************************************************************************//**
 * 
 */
void MSS_MAC_NULL_phy_autonegotiate(/* mss_mac_instance_t*/ void *v_this_mac)
{
	/* Nothing to see here... */
    (void)v_this_mac;
}

/**************************************************************************//**
 * 
 */
uint8_t MSS_MAC_NULL_phy_get_link_status
(
		/* mss_mac_instance_t*/ void *v_this_mac,
    mss_mac_speed_t * speed,
    uint8_t *     fullduplex
)
{
    uint8_t link_status;

    (void)v_this_mac;
    /* Assume link is up. */
    link_status = MSS_MAC_LINK_UP;

    /* Pick fastest for now... */
    
    *fullduplex = MSS_MAC_FULL_DUPLEX;
    *speed = MSS_MAC_1000MBPS;

    return link_status;
}

#if MSS_MAC_USE_PHY_DP83867
/**************************************************************************//**
 *
 */
uint16_t NULL_ti_read_extended_regs(/* mss_mac_instance_t*/ void *v_this_mac, uint16_t reg)
{
    (void)v_this_mac;
    (void)reg;

    return(0);
}

/**************************************************************************//**
 *
 */
void NULL_ti_write_extended_regs(/* mss_mac_instance_t*/ void *v_this_mac, uint16_t reg, uint16_t data)
{
    (void)v_this_mac;
    (void)reg;
    (void)data;

	/* Nothing to see here... */
}
#endif

#endif /* MSS_MAC_USE_PHY_NULL */
#ifdef __cplusplus
}
#endif

/******************************** END OF FILE ******************************/






