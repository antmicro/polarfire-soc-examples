
/*******************************************************************************
 * (c) Copyright 2018 Microsemi - PRO. All rights reserved.
 *
 * Microsemi VSC8541 PHY interface driver implementation to support the FU540
 * Aloe board.
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

#if MSS_MAC_USE_PHY_VSC8541

/**************************************************************************/
/* Preprocessor Macros                                                    */
/**************************************************************************/

#define BMSR_AUTO_NEGOTIATION_COMPLETE  0x0020u


/**************************************************************************//**
 * 
 */
#define ANEG_REQUESTED          0x80000000u
#define FORCED_CFG_REQUESTED    0x40000000u


/**************************************************************************//**
 * 
 */
 
#define __I  const volatile
#define __IO volatile
#define __O volatile

typedef struct
{
    __IO uint32_t  INPUT_VAL;   /* 0x0000 */
    __IO uint32_t  INPUT_EN;    /* 0x0004 */
    __IO uint32_t  OUTPUT_VAL;  /* 0x0008 */
    __IO uint32_t  OUTPUT_EN;   /* 0x000C */
    __IO uint32_t  PUE;         /* 0x0010 */
    __IO uint32_t  DS;          /* 0x0014 */
    __IO uint32_t  RISE_IE;     /* 0x0018 */
    __IO uint32_t  RISE_IP;     /* 0x001C */
    __IO uint32_t  FALL_IE;     /* 0x0020 */
    __IO uint32_t  FALL_IP;     /* 0x0024 */
    __IO uint32_t  HIGH_IE;     /* 0x0028 */
    __IO uint32_t  HIGH_IP;     /* 0x002C */
    __IO uint32_t  LOW_IE;      /* 0x0030 */
    __IO uint32_t  LOW_IP;      /* 0x0034 */
    __IO uint32_t  reserved0;   /* 0x0038 */
    __IO uint32_t  reserved1;   /* 0x003C */
    __IO uint32_t  OUT_XOR;     /* 0x0040 */
} AloeGPIO_TypeDef;

AloeGPIO_TypeDef *g_aloe_gpio = (AloeGPIO_TypeDef *)0x10060000ul;


void MSS_MAC_VSC8541_phy_init(/* mss_mac_instance_t*/ void *v_this_mac, uint8_t phy_addr)
{
#if defined(TARGET_ALOE)
	mss_mac_instance_t *this_mac = (mss_mac_instance_t *)v_this_mac;
    volatile uint32_t loop;
    
    (void)phy_addr;
/*
 * Init includes toggling the reset line which is connected to GPIO 0 pin 12. 
 * This is the only pin I can see on the 16 GPIO which is currently set as an.
 * output. We will hard code the setup here to avoid having to have a GPIO
 * driver as well...
 *
 * The Aloe board is strapped for unmanaged mode and needs two pulses of the
 * reset line to configure the device properly.
 *
 * The RX_CLK, TX_CLK and RXD7 pins are strapped high and the remainder low.
 * This selects GMII mode with auto 10/100/1000 and 125MHz clkout.
 */
    g_aloe_gpio->OUTPUT_EN  |= 0x00001000ul;  /* Configure pin 12 as an output */
    g_aloe_gpio->OUTPUT_VAL &= 0x0000EFFFul;  /* Clear pin 12 to reset PHY */
    for(loop = 0; loop != 1000; loop++)     /* Short delay, I'm not sure how much is needed... */
    {
        ;
    }
    g_aloe_gpio->OUTPUT_VAL  |= 0x00001000ul; /* Take PHY^ out of reset */
    for(loop = 0; loop != 1000; loop++)     /* Short delay, I'm not sure how much is needed... */
    {
        ;
    }
    g_aloe_gpio->OUTPUT_VAL &= 0x0000EFFFul;  /* Second reset pulse */
    for(loop = 0; loop != 1000; loop++)     /* Short delay, I'm not sure how much is needed... */
    {
        ;
    }
    g_aloe_gpio->OUTPUT_VAL  |= 0x00001000ul; /* Out of reset once more */

    /* Need at least 15mS delay before accessing PHY after reset... */
    for(loop = 0; loop != 10000000; loop++)     /* Long delay, I'm not sure how much is needed... */
    {
        ;
    }
#endif
}

/**************************************************************************//**
 * 
 */
void MSS_MAC_VSC8541_phy_set_link_speed(/* mss_mac_instance_t*/ void *v_this_mac, uint32_t speed_duplex_select)
{
	mss_mac_instance_t *this_mac = (mss_mac_instance_t *)v_this_mac;
    uint16_t phy_reg;
    uint32_t inc;
    uint32_t speed_select;
    const uint16_t mii_advertise_bits[4] = {ADVERTISE_10FULL, ADVERTISE_10HALF,
                                            ADVERTISE_100FULL, ADVERTISE_100HALF};
    
    /* Set auto-negotiation advertisement. */
    
    /* Set 10Mbps and 100Mbps advertisement. */
    phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_ADVERTISE);
    phy_reg &= (uint16_t)(~(ADVERTISE_10HALF | ADVERTISE_10FULL |
                 ADVERTISE_100HALF | ADVERTISE_100FULL));
                 
    speed_select = speed_duplex_select;
    for(inc = 0u; inc < 4u; ++inc)
    {
        uint32_t advertise;
        advertise = speed_select & 0x00000001u;
        if(advertise != 0u)
        {
            phy_reg |= mii_advertise_bits[inc];
        }
        speed_select = speed_select >> 1u;
    }
    
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_ADVERTISE, phy_reg);

    /* Set 1000Mbps advertisement. */
    phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_CTRL1000);
    phy_reg &= (uint16_t)(~(ADVERTISE_1000FULL | ADVERTISE_1000HALF));
    
    if((speed_duplex_select & MSS_MAC_ANEG_1000M_FD) != 0u)
    {
        phy_reg |= ADVERTISE_1000FULL;
    }
    
    if((speed_duplex_select & MSS_MAC_ANEG_1000M_HD) != 0u)
    {
        phy_reg |= ADVERTISE_1000HALF;
    }
    
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_CTRL1000, phy_reg);
}

/**************************************************************************//**
 * 
 */
void MSS_MAC_VSC8541_phy_autonegotiate(/* mss_mac_instance_t*/ void *v_this_mac)
{
	mss_mac_instance_t *this_mac = (mss_mac_instance_t *)v_this_mac;
    volatile uint16_t phy_reg;
    uint16_t autoneg_complete;
    volatile uint32_t copper_aneg_timeout = 1000000u;
    
    phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 2);
    phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 3);

    /* Enable auto-negotiation. */
    phy_reg = 0x1340;
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_BMCR, phy_reg);
    
    /* Wait for copper auto-negotiation to complete. */
    do {
        phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_BMSR);
        autoneg_complete = phy_reg & BMSR_AUTO_NEGOTIATION_COMPLETE;
        --copper_aneg_timeout;
    } while((!autoneg_complete && (copper_aneg_timeout != 0u)) || (0xFFFF == phy_reg));
}

/**************************************************************************//**
 * 
 */
uint8_t MSS_MAC_VSC8541_phy_get_link_status
(
	/* mss_mac_instance_t*/ void *v_this_mac,
    mss_mac_speed_t * speed,
    uint8_t *     fullduplex
)
{
	mss_mac_instance_t *this_mac = (mss_mac_instance_t *)v_this_mac;
    uint16_t phy_reg;
    uint16_t link_up;
    uint8_t link_status;

    phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_BMSR);
    link_up = phy_reg & BMSR_LSTATUS;
    
    if(link_up != MSS_MAC_LINK_DOWN)
    {
        uint16_t duplex;
        uint16_t speed_field;
        
        /* Link is up. */
        link_status = MSS_MAC_LINK_UP;
        
        phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1C); /* Device Auxillary Control and Status */
        duplex = phy_reg & 0x0020u;
        speed_field = phy_reg & 0x0018u;
        
        if(MSS_MAC_HALF_DUPLEX == duplex)
        {
            *fullduplex = MSS_MAC_HALF_DUPLEX;
        }
        else
        {
            *fullduplex = MSS_MAC_FULL_DUPLEX;
        }
        
        switch(speed_field >> 3)
        {
            case 0:
                *speed = MSS_MAC_10MBPS;
            break;
            
            case 1:
                *speed = MSS_MAC_100MBPS;
            break;
            
            case 2:
                *speed = MSS_MAC_1000MBPS;
            break;
            
            default:
                link_status = MSS_MAC_LINK_DOWN;
            break;
        }
    }
    else
    {
        /* Link is down. */
        link_status = MSS_MAC_LINK_DOWN;
    }

    return link_status;
}


/**************************************************************************//**
 *
 */
uint16_t VSC8541_reg_0[32];
uint16_t VSC8541_reg_1[16];
uint16_t VSC8541_reg_2[16];
uint16_t VSC8541_reg_16[32];

void dump_vsc8541_regs(mss_mac_instance_t * this_mac);
void dump_vsc8541_regs(mss_mac_instance_t * this_mac)
{
    int32_t count;
    uint16_t page;
    uint16_t old_page;
    uint16_t *pdata;
    volatile psr_t lev;

    for(page = 0; page <= 0x10; page++)
    {
        if(0 == page)
            pdata = VSC8541_reg_0;
        else if(1 == page)
            pdata = VSC8541_reg_1;
        else if(2 == page)
            pdata = VSC8541_reg_2;
        else if(16 == page)
            pdata = VSC8541_reg_16;
        else
            pdata = VSC8541_reg_0;

        if((0 == page) || (0x10 == page))
        {
            for(count = 0; count <= 0x1F; count++)
            {
                lev = HAL_disable_interrupts();
                old_page = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1f);

                MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1F, page);

                pdata[count] = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, (uint8_t)count);

                MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1F, old_page);
                HAL_restore_interrupts(lev);
            }
        }
        else
        {
            for(count = 0x10; count <= 0x1F; count++)
            {
                lev = HAL_disable_interrupts();
                old_page = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1f);

                MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1F, page);

                pdata[count - 0X10] = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, (uint8_t)count);

                MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1F, old_page);
                HAL_restore_interrupts(lev);
            }
        }

        if(2 == page)
            page = 0x0f;
    }
}

#endif /* #if defined(TARGET_ALOE) */
#ifdef __cplusplus
}
#endif

/******************************** END OF FILE ******************************/






