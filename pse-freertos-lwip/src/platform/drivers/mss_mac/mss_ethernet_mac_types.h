/*******************************************************************************
 * (c) Copyright 2018 Microsemi - PRO. All rights reserved.
 * 
 * This file contains type definitions used throughout the PSE MSS Ethernet MAC
 * and PHY device drivers. User need not include this file in application source
 * code.
 * Inclusion of mss_ethernet_mac.h inherits these types.
 * 
 * SVN $Revision: 10527 $
 * SVN $Date: 2018-11-10 12:47:05 +0000 (Sat, 10 Nov 2018) $
 */
#ifndef MSS_ETHERNET_MAC_TYPES_H_
#define MSS_ETHERNET_MAC_TYPES_H_

#include <stdint.h>

#include "config/software/drivers/mss_mac/mss_ethernet_mac_user_config.h"

#include "mac_registers.h"
#include "mpfs_hal/mss_plic.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************/
/* Public type definitions                                                     */
/*******************************************************************************/

/*******************************************************************************
 * MAC interface speed
 * This type definition provides various interface speeds supported by MAC
 * hardware.
 */
typedef enum
{
    MSS_MAC_10MBPS     = 0x00,
    MSS_MAC_100MBPS    = 0x01,
    MSS_MAC_1000MBPS   = 0x02,
    INVALID_SPEED      = 0x03
} mss_mac_speed_t;

/*******************************************************************************
 * MAC RX interrupt control
 * This type definition indicates the action to take in relation to the RX
 * interrupt when configuring an RX buffer with MSS_MAC_receive_pkt.
 */
typedef enum
{
    MSS_MAC_INT_ARM     = -1, /* Last buffer in chain so arm the RX interrupt */
    MSS_MAC_INT_DISABLE =  0, /* Disable interrupts on exit */
    MSS_MAC_INT_ENABLE  =  1, /* Leave interrupts enabled on exit */
} mss_mac_rx_int_ctrl_t;

/*******************************************************************************
  SmartFusion2 Ethernet MAC Configuration Structure.
  The mss_mac_cfg_t type provides the prototype for the configuration values of the
  SmartFusion2 Ethernet MAC.You need to create a record of this type to hold the 
  configuration of the MAC. The MSS_MAC_cfg_struct_def_init() API should be used
  to initialize the configuration record to default values. Later, the 
  configuration elements in the record can be changed to desired values.
  
  speed_duplex_select
    The speed_duplex_select configuration parameter specifies the allowed link
    speeds. It is a bit-mask of the various link speed and duplex modes. The
    speed_duplex_select configuration can be set to a bitmask of the following
    defines to specify the allowed link speed and duplex mode:
        � MSS_MAC_ANEG_10M_FD
        � MSS_MAC_ANEG_10M_HD
        � MSS_MAC_ANEG_100M_FD
        � MSS_MAC_ANEG_100M_HD
        � MSS_MAC_ANEG_1000M_FD
        � MSS_MAC_ANEG_1000M_HD
    The MSS_MAC_cfg_struct_def_init() function sets this configuration parameter
    to MSS_MAC_ANEG_ALL_SPEEDS indicating that a link will be setup for best
    available speed and duplex combination.

  mac_addr
    The mac_addr configuration parameter is a 6-byte array containing the local
    MAC address of the Ethernet MAC.

  phy_address
    The phy_address parameter specifies the address of the PHY device, set in
    hardware by the address pins of the PHY device.

  pcs_phy_address
    The phy_address parameter specifies the address of the control device for
    hidden SGMII type interfaces such as that in the HW emulation system,
    set in hardware by the address ports of the PHY device.

  tx_edc_enable 
    The tx_edc_enable parameter specifies enable or disable error detection and
    correction for tx FIFOs. The allowed values for the tx_edc_enable
    configuration parameter are:
        � MSS_MAC_ERR_DET_CORR_ENABLE 
        � MSS_MAC_ERR_DET_CORR_DISABLE 
    The MSS_MAC_cfg_struct_def_init() function sets this configuration parameter
    to MSS_MAC_ERR_DET_CORR_ENABLE.

  rx_edc_enable
    The rx_edc_enable parameter specifies enable or disable error detection and
    correction for rx FIFOs. The allowed values for the rx_edc_enable
    configuration parameter are:
        � MSS_MAC_ERR_DET_CORR_ENABLE 
        � MSS_MAC_ERR_DET_CORR_DISABLE 
    The MSS_MAC_cfg_struct_def_init() function sets this configuration parameter
    to MSS_MAC_ERR_DET_CORR_ENABLE.

  preamble_length
    The preamble_length parameter specifies the length of the preamble field of
    the packet in bytes. The allowed values for the preamble_length
    configuration parameter are: 
        � MSS_MAC_PREAMLEN_DEFVAL
        � MSS_MAC_PREAMLEN_MAXVAL
    The MSS_MAC_cfg_struct_def_init() function sets this configuration parameter
    to MSS_MAC_PREAMLEN_DEFVAL.

  jumbo_frame_enable
    The jumbo_frame_enable parameter specifies enable or disable jumbo frame support.
    When enabled, it allows frames longer than the maximum frame length to be
    transmitted and received. When disabled, the MAC limits the length of frames
    at the maximum frame length. The allowed values for the jumbo frame_enable
    configuration parameter are: 
        � MSS_MAC_HUGE_FRAME_ENABLE
        � MSS_MAC_HUGE_FRAME_DISABLE
    The MSS_MAC_cfg_struct_def_init() function sets this configuration parameter
    to MSS_MAC_HUGE_FRAME_DISABLE.

  length_field_check
    The length_field_check parameter specifies enable or disable length filed
    check. When enabled, the MAC checks the frame�s length field to ensure it
    matches the actual data field length. The allowed values for the
    length_field_check configuration parameter are: 
        � MSS_MAC_LENGTH_FILED_CHECK_ENABLE 
        � MSS_MAC_LENGTH_FILED_CHECK_DISABLE
    The MSS_MAC_cfg_struct_def_init() function sets this configuration parameter
    to MSS_MAC_LENGTH_FILED_CHECK_ENABLE

  append_CRC
    The append_CRC parameter specifies enable or disable appending CRC. When
    enabled, the MAC appends a CRC to all the frames. When disabled, frames
    presented to the MAC, have a valid length and contain a valid CRC. The
    allowed values for the append_CRC configuration parameter are: 
        � MSS_MAC_CRC_ENABLE 
        � MSS_MAC_CRC_DISABLE
    The MSS_MAC_cfg_struct_def_init() function sets this configuration parameter
    to MSS_MAC_CRC_ENABLE.

  fullduplex 
    The fullduplex parameter specifies enable or disable full duplex. When
    enabled, the MAC operates in full duplex mode. When disabled, the MAC
    operates in half duplex mode. The allowed values for the fullduplex
    configuration parameter are: 
        � MSS_MAC_FULLDUPLEX_ENABLE
        � MSS_MAC_FULLDUPLEX_DISABLE
    The MSS_MAC_cfg_struct_def_init() function sets this configuration parameter
    to MSS_MAC_FULLDUPLEX_ENABLE.

  loopback
    The loopback parameter specifies enable or disable loop back mode. When
    enabled, the MAC transmit outputs to be looped back to its receiving inputs.
    The allowed values for the loopback configuration parameter are: 
        � MSS_MAC_LOOPBACK_ENABLE 
        � MSS_MAC_LOOPBACK_DISABLE
    The MSS_MAC_cfg_struct_def_init() function sets this configuration parameter
    to MSS_MAC_LOOPBACK_DISABLE.

  rx_flow_ctrl
    The rx_flow_ctrl parameter specifies enable or disable receiver flow control.
    When enabled, the MAC detects and acts on PAUSE flow control frames. When
    disabled, it ignores PAUSE flow control frames. The allowed values for the
    rx_flow_ctrl configuration parameter are: 
        � MSS_MAC_RX_FLOW_CTRL_ENABLE 
        � MSS_MAC_RX_FLOW_CTRL_DISABLE
    The MSS_MAC_cfg_struct_def_init() function sets this configuration parameter
    to MSS_MAC_RX_FLOW_CTRL_ENABLE.

  tx_flow_ctrl
    The tx_flow_ctrl parameter specifies enable or disable transmitter flow
    control. When enabled, the transmitter sends PAUSE flow control frames when
    requested by the system. When disabled, prevents the transmitter from
    sending flow control frames. The allowed values for the tx_flow_ctrl
    configuration parameter are: 
        � MSS_MAC_TX_FLOW_CTRL_ENABLE 
        � MSS_MAC_TX_FLOW_CTRL_DISABLE
    The MSS_MAC_cfg_struct_def_init() function sets this configuration parameter
    to MSS_MAC_TX_FLOW_CTRL_ENABLE.

  ipg_multiplier
  ipg_divisor

    The ipg_multiplier and ipg_divisor parameters specify the minimum size of
    gap (IPG/IFG) to enforce between frames (expressed in bit times). They are
    both 8 bit values and are used to calculate an IPG value based on the last
    packet sent by multiplying the length by ipg_multiplier and dividing the
    result by ipg_divisor. The resulting number of bits is used if it is greater
    than the default 96 bits.
    The default value to select standard 96 bit IPG is:
        MSS_MAC_IPG_DEFVAL

    The MSS_MAC_cfg_struct_def_init() function sets this configuration parameter
    to MSS_MAC_IPG_DEFVAL.

  phyclk
    The phyclk parameter specifies MII management clock divider value. PCLK is
    the source clock. The allowed values for the phyclk configuration parameter
    are: 
        � MSS_MAC_DEF_PHY_CLK
        � MSS_MAC_BY8_PHY_CLK
        � MSS_MAC_BY16_PHY_CLK
        � MSS_MAC_BY32_PHY_CLK
        � MSS_MAC_BY48_PHY_CLK
        � MSS_MAC_BY64_PHY_CLK
        � MSS_MAC_BY96_PHY_CLK
        � MSS_MAC_BY128_PHY_CLK
        � MSS_MAC_BY224_PHY_CLK
    The MSS_MAC_cfg_struct_def_init() function sets this configuration parameter
    to MSS_MAC_DEF_PHY_CLK.

  max_frame_length
    The max_frame_length parameter specifies the maximum frame size in both the
    transmit and receive directions. The allowed values for the max_frame_length
    configuration parameter are: 
        � MSS_MAC_MAXFRAMELEN_DEFVAL 
        � MSS_MAC_MAXFRAMELEN_MAXVAL
    The MSS_MAC_cfg_struct_def_init() function sets this configuration parameter
    to MSS_MAC_MAXFRAMELEN_DEFVAL.

  non_btb_IFG
    The non_btb_IFG parameter specifies non back to back inter-frame gap value.
    The allowed values for the non_btb_IFG configuration parameter are: 
        � MSS_MAC_NONBTBIFG_DEFVAL 
        � MSS_MAC_NONBTBIFG_MAXVAL
    The MSS_MAC_cfg_struct_def_init() function sets this configuration parameter
    to MSS_MAC_NONBTBIFG_DEFVAL.

  slottime
    The slottime parameter specifies the slot time or collision window during
    which collisions might occur in a properly configured network. The allowed 
    values for the slottime configuration parameter are: 
        � MSS_MAC_SLOTTIME_DEFVAL 
        � MSS_MAC_SLOTTIME_MAXVAL
    The MSS_MAC_cfg_struct_def_init() function sets this configuration parameter
    to MSS_MAC_SLOTTIME_DEFVAL.

  framedrop_mask
    The framedrop_mask parameter specifies to drop frames based on receive
    statistics. The allowed values for the framedrop_mask configuration parameter
    are: 
        � MSS_MAC_FRAME_DROP_MASK_DEFVAL
    The MSS_MAC_cfg_struct_def_init() function sets this configuration parameter
    to MSS_MAC_FRAME_DROP_MASK_DEFVAL.
 */

/*******************************************************************************
 * Pointer to phy init function
 *
 * void MSS_MAC_phy_init(mss_mac_instance_t *this_mac, uint8_t phy_addr);
 */
typedef void (*mss_mac_phy_init_t)(/* mss_mac_instance_t*/ void *this_mac, uint8_t phy_addr);


/*******************************************************************************
 * Pointer to phy set link speed function
 *
 * void MSS_MAC_phy_set_link_speed(mss_mac_instance_t *this_mac, uint32_t speed_duplex_select);
 */
typedef void (*mss_mac_phy_set_speed_t)(/* mss_mac_instance_t*/ void *this_mac, uint32_t speed_duplex_select);


/*******************************************************************************
 * Pointer to phy auto-negotiate function
 *
 * void MSS_MAC_phy_autonegotiate(mss_mac_instance_t *this_mac);
 */
typedef void (*mss_mac_phy_autonegotiate_t)(/* mss_mac_instance_t*/ void *this_mac);


/*******************************************************************************
 * Pointer to phy get link status function
 *
 * uint8_t MSS_MAC_phy_get_link_status
 * (
 *     mss_mac_instance_t *this_mac,
 *     mss_mac_speed_t * speed,
 *     uint8_t * full duplex
 * );
 */
typedef uint8_t (*mss_mac_phy_get_link_status_t)
(
    /* mss_mac_instance_t*/ void *this_mac,
    mss_mac_speed_t * speed,
    uint8_t * fullduplex
);


#if MSS_MAC_USE_PHY_DP83867
/*******************************************************************************
 * Pointer to phy extended read function
 *
 * uint16_t ti_read_extended_regs(mss_mac_instance_t * this_mac, uint16_t reg);
 */
typedef uint16_t (*mss_mac_phy_extended_read_t)(/* mss_mac_instance_t*/ void *this_mac, uint16_t reg);


/*******************************************************************************
 * Pointer to phy extended write function
 *
 * void ti_write_extended_regs(mss_mac_instance_t * this_mac, uint16_t reg);
 */
typedef void (*mss_mac_phy_extended_write_t)(/* mss_mac_instance_t*/ void *this_mac, uint16_t reg, uint16_t data);
#endif


/*
 * Note: Even though most of these values are small, we use uint32_t for most values
 * here as they will be used in calculations that are based on uint32_t values and
 * this avoids having to put casts everywhere...
 */
typedef struct
{
    uint32_t interface_type;            /* Type of network interface associated with this GEM */
    uint32_t phy_type;                  /* PHY device type associated with this GEM */
    /* PHY interface functions */
    mss_mac_phy_init_t            phy_init;
    mss_mac_phy_set_speed_t       phy_set_link_speed;
    mss_mac_phy_autonegotiate_t   phy_autonegotiate;
    mss_mac_phy_get_link_status_t phy_get_link_status;
#if MSS_MAC_USE_PHY_DP83867
    mss_mac_phy_extended_read_t   phy_extended_read;
    mss_mac_phy_extended_write_t  phy_extended_write;
#endif
    uint32_t queue_enable[MSS_MAC_QUEUE_COUNT]; /* Enables for additional queues */
    uint32_t speed_duplex_select;       /* Link speed and duplex mode allowed to setup a link. */
    uint8_t  mac_addr[6];               /* Station's MAC address */
    uint32_t phy_addr;                  /* Address of Ethernet PHY on MII management interface. */
    uint32_t pcs_phy_addr;              /* Address of SGMII interface controller on MII management interface. */
    uint32_t tx_edc_enable;             /* Enable / disable error detection and correction for tx FIFOs */
    uint32_t rx_edc_enable;             /* Enable / disable error detection and correction for rx FIFOs */
    uint32_t jumbo_frame_enable;        /* Enable / disable jumbo frame support: default is disable 0 */
    uint32_t jumbo_frame_default;       /* Default maximum size for jumbo frames */
    uint32_t length_field_check;        /* Enable / disable length field checking */
    uint32_t pad_n_CRC;                 /* Enable / disable padding and appending CRC */
    uint32_t append_CRC;                /* Enable / disable appending CRC */
    uint32_t fullduplex;                /* Enable / disable full duplex: default is disable 0 */
    uint32_t loopback;                  /* Enable / disable loopback mode: default is disable 0 */
    uint32_t rx_flow_ctrl;              /* Enable / disable receiver flow control: default is disable 0 */
    uint32_t tx_flow_ctrl;              /* Enable / disable transmitter flow control: default is disable 0 */
    uint32_t ipg_multiplier;            /* 8-bit IPG multiplication factor, if 0 we disable IPG stretching */
    uint32_t ipg_divisor;               /* 8-bit back to back inter-frame gap value */
    uint32_t phyclk;                    /* 3-bit MGMT clock divider value */
    uint32_t max_frame_length;          /* Maximum frame length: default value is 0x0600(1536d) */
    uint32_t slottime;                  /* 10-bit collision window value : default is 0x37 */
    uint32_t framedrop_mask;            /* 18-bit mask to drop frames based on receive statistics */
} mss_mac_cfg_t;

/*******************************************************************************
 * Transmit and Receive statistics.
 * Statistic of below type, which is desired to be read is to be passed to the
 * function MSS_MAC_read_stat(). The width of returned statistic value is 
 * mentioned in the comment against the statistic.
 */
typedef enum
{
    MSS_MAC_TX_OCTETS_LOW,                /* 32-bit */
    MSS_MAC_TX_OCTETS_HIGH,               /* 16-bit */
    MSS_MAC_TX_FRAMES_OK,                 /* 32-bit */
    MSS_MAC_TX_BCAST_FRAMES_OK,           /* 32-bit */
    MSS_MAC_TX_MCAST_FRAMES_OK,           /* 32-bit */
    MSS_MAC_TX_PAUSE_FRAMES_OK,           /* 32-bit */
    MSS_MAC_TX_64_BYTE_FRAMES_OK,         /* 32-bit */
    MSS_MAC_TX_65_BYTE_FRAMES_OK,         /* 32-bit */
    MSS_MAC_TX_128_BYTE_FRAMES_OK,        /* 32-bit */
    MSS_MAC_TX_256_BYTE_FRAMES_OK,        /* 32-bit */
    MSS_MAC_TX_512_BYTE_FRAMES_OK,        /* 32-bit */
    MSS_MAC_TX_1024_BYTE_FRAMES_OK,       /* 32-bit */
    MSS_MAC_TX_1519_BYTE_FRAMES_OK,       /* 32-bit */
    MSS_MAC_TX_UNDERRUNS,                 /* 10-bit */
    MSS_MAC_TX_SINGLE_COLLISIONS,         /* 18-bit */
    MSS_MAC_TX_MULTIPLE_COLLISIONS,       /* 18-bit */
    MSS_MAC_TX_EXCESSIVE_COLLISIONS,      /* 10-bit */
    MSS_MAC_TX_LATE_COLLISIONS,           /* 10-bit */
    MSS_MAC_TX_DEFERRED_FRAMES,           /* 18-bit */
    MSS_MAC_TX_CRS_ERRORS,                /* 10-bit */

    MSS_MAC_RX_OCTETS_LOW,                /* 32-bit */
    MSS_MAC_RX_OCTETS_HIGH,               /* 16-bit */
    MSS_MAC_RX_FRAMES_OK,                 /* 32-bit */
    MSS_MAC_RX_BCAST_FRAMES_OK,           /* 32-bit */
    MSS_MAC_RX_MCAST_FRAMES_OK,           /* 32-bit */
    MSS_MAC_RX_PAUSE_FRAMES_OK,           /* 32-bit */
    MSS_MAC_RX_64_BYTE_FRAMES_OK,         /* 32-bit */
    MSS_MAC_RX_65_BYTE_FRAMES_OK,         /* 32-bit */
    MSS_MAC_RX_128_BYTE_FRAMES_OK,        /* 32-bit */
    MSS_MAC_RX_256_BYTE_FRAMES_OK,        /* 32-bit */
    MSS_MAC_RX_512_BYTE_FRAMES_OK,        /* 32-bit */
    MSS_MAC_RX_1024_BYTE_FRAMES_OK,       /* 32-bit */
    MSS_MAC_RX_1519_BYTE_FRAMES_OK,       /* 32-bit */
    MSS_MAC_RX_UNDERSIZE_FRAMES_OK,       /* 10-bit */
    MSS_MAC_RX_OVERSIZE_FRAMES_OK,        /* 10-bit */
    MSS_MAC_RX_JABBERS,                   /* 10-bit */
    MSS_MAC_RX_FCS_ERRORS,                /* 10-bit */
    MSS_MAC_RX_LENGTH_ERRORS,             /* 10-bit */
    MSS_MAC_RX_SYMBOL_ERRORS,             /* 10-bit */
    MSS_MAC_RX_ALIGNMENT_ERRORS,          /* 10-bit */
    MSS_MAC_RX_RESOURCE_ERRORS,           /* 18-bit */
    MSS_MAC_RX_OVERRUNS,                  /* 10-bit */
    MSS_MAC_RX_IP_CHECKSUM_ERRORS,        /*  8-bit */
    MSS_MAC_RX_TCP_CHECKSUM_ERRORS,       /*  8-bit */
    MSS_MAC_RX_UDP_CHECKSUM_ERRORS,       /*  8-bit */
    MSS_MAC_RX_AUTO_FLUSHED_PACKETS,      /* 16-bit */

    MSS_MAC_LAST_STAT
} mss_mac_stat_t;

/*******************************************************************************
 * Which direction unicast packet is being referenced.
 */
typedef enum
{
    MSS_MAC_TSU_UNICAST_RX,
    MSS_MAC_TSU_UNICAST_TX
} mss_mac_tsu_addr_t;

/*******************************************************************************
 * DMA descriptor time stamp insertion modes.
 */
typedef enum
{
    MSS_MAC_TSU_MODE_DISABLED,  /* Time stamp insertion disabled */
    MSS_MAC_TSU_MODE_PTP_EVENT, /* Time stamp insertion for PTP Event frames only */
    MSS_MAC_TSU_MODE_PTP_ALL,   /* Time stamp insertion all PTP frames */
    MSS_MAC_TSU_MODE_ALL,       /* Time stamp insertion for all frames */
    MSS_MAC_TSU_MODE_END
} mss_mac_tsu_mode_t;

/*******************************************************************************
 * Hash Matching modes.
 */
typedef enum
{
    MSS_MAC_HASH_NONE      = 0x00,     /* Hash matching disabled */
    MSS_MAC_HASH_MULTICAST = 0x40,     /* Multicast matching enabled */
    MSS_MAC_HASH_UNIICAST  = 0x80,     /* Unicast matching enabled */
    MSS_MAC_HASH_ALL       = 0xC0      /* Multicast and Unicast matching enabled */
} mss_mac_hash_mode_t;

/*******************************************************************************
 * Sync time stamp adjust modes.
 */
typedef enum
{
    MSS_MAC_OSS_MODE_DISABLED,  /* Sync time stamp adjust disabled */
    MSS_MAC_OSS_MODE_REPLACE,   /* Sync time stamp replace mode */
    MSS_MAC_OSS_MODE_ADJUST,    /* Sync time stamp adjust correction field mode */
    MSS_MAC_OSS_MODE_INVALID,   /* Only for reporting mis-configured setup, not setting things... */
    MSS_MAC_OSS_MODE_END
} mss_mac_oss_mode_t;


/*******************************************************************************
 * Preemption fragment size.
 */
typedef enum
{
    MSS_MAC_FRAG_SIZE_64,
    MSS_MAC_FRAG_SIZE_128,
    MSS_MAC_FRAG_SIZE_192,
    MSS_MAC_FRAG_SIZE_256,
    MSS_MAC_FRAG_SIZE_END
} mss_mac_frag_size_t;


/*******************************************************************************
 * DMA Descriptor bit field defines.
 *
 * Driver creates and manages two descriptor rings for transmission and
 * reception.
 */
 
#define GEM_RX_DMA_TS_PRESENT       BIT_02
#define GEM_RX_DMA_WRAP             BIT_01
#define GEM_RX_DMA_USED             BIT_00

#define GEM_RX_DMA_BCAST            BIT_31
#define GEM_RX_DMA_MULTICAST_HASH   BIT_30
#define GEM_RX_DMA_UNICAST_HASH     BIT_29
#define GEM_RX_DMA_EXT_ADDR_MATCH   BIT_28
#define GEM_RX_DMA_SPECIFIC_ADDR    BIT_27
#define GEM_RX_DMA_ADDR_REGISTER    (BIT_25 | BIT_26)
#define GEM_RX_DMA_TYPE_ID_MATCH    BIT_24
#define GEM_RX_DMA_TYPE_ID          (BIT_22 | BIT_23)
#define GEM_RX_DMA_VLAN_TAG         BIT_21
#define GEM_RX_DMA_PRIORITY_TAG     BIT_20
#define GEM_RX_DMA_VLAN_PRIORITY    (BITS_03 << 17)
#define GEM_RX_DMA_FCS_ERROR        BIT_16
#define GEM_RX_DMA_START_OF_FRAME   BIT_15
#define GEM_RX_DMA_END_OF_FRAME     BIT_14
#define GEM_RX_DMA_JUMBO_BIT_13     BIT_13
#define GEM_RX_DMA_BUFF_LEN         BITS_13

#define GEM_TX_DMA_USED             BIT_31
#define GEM_TX_DMA_WRAP             BIT_30
#define GEM_TX_DMA_RETRY_ERROR      BIT_29
#define GEM_TX_DMA_UNDERRUN         BIT_28
#define GEM_TX_DMA_BUS_ERROR        BIT_27
#define GEM_TX_DMA_LATE_COL_ERROR   BIT_26
#define GEM_TX_DMA_TS_PRESENT       BIT_23
#define GEM_TX_DMA_OFFLOAD_ERRORS   (BITS_03 << 20)
#define GEM_TX_DMA_NO_CRC           BIT_16
#define GEM_TX_DMA_LAST             BIT_15
#define GEM_TX_DMA_BUFF_LEN         BITS_14
 
typedef struct mss_mac_tx_desc mss_mac_tx_desc_t;
typedef struct mss_mac_rx_desc mss_mac_rx_desc_t;

/*******************************************************************************
 * Note: in the following definitions we use void * for the this_mac parameter
 * as they are used in the definition of the mss_mac_instance_t structure and
 * this confuses the compiler if we try to use the proper structure pointer in
 * the definition of these function pointers...
 */


/*******************************************************************************
 * Transmit callback function.
 *
 *   this_mac    - pointer to global structure for the MAC in question.
 *   queue_no    - 0 to 3 for pMAC and always 0 for eMAC.
 *   cdesc       - pointer to the DMA descriptor associated with this packet.
 *   p_user_data - original user data pointer associated with the packet buffer.
 */
typedef void (*mss_mac_transmit_callback_t)(/* mss_mac_instance_t*/ void *this_mac,
                                       uint32_t queue_no,
                                       mss_mac_tx_desc_t *cdesc,
                                       void * p_user_data);

/*******************************************************************************
 * Receive callback function.
 *
 *   this_mac    - pointer to global structure for the MAC in question.
 *   queue_no    - 0 to 3 for pMAC and always 0 for eMAC.
 *   p_rx_packet - pointer to the buffer for the packet to be processed.
 *   pckt_length - length of packet to be processed.
 *   cdesc       - pointer to the DMA descriptor associated with this packet.
 *   p_user_data - original user data pointer associated with the packet buffer.
 */
typedef void (*mss_mac_receive_callback_t)(/* mss_mac_instance_t*/ void *this_mac,
                                       uint32_t queue_no,
                                       uint8_t *p_rx_packet,
                                       uint32_t pckt_length,
                                       mss_mac_rx_desc_t *cdesc,
                                       void *p_user_data);

/*******************************************************************************
 * Transmit DMA descriptor.
 */
struct mss_mac_tx_desc
{
    uint32_t          addr_low;     /* Buffer address low portion  */
    volatile uint32_t status;       /* Status and options for transmit operation */
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
    uint32_t          addr_high;    /* High portion of address in 64bit addressing mode */
    uint32_t          unused;       /* Unused word in 64bit mode */
#endif 
#if defined(MSS_MAC_TIME_STAMPED_MODE)
    volatile uint32_t nano_seconds; /* Nanoseconds and LSBs of seconds for timestamp */
    volatile uint32_t seconds;      /* MSBs of timestamp seconds */
#endif
};

/*******************************************************************************
 * Receive DMA descriptor.
 */
struct mss_mac_rx_desc
{
    uint32_t          addr_low;     /* Buffer address low portion  */
    volatile uint32_t status;       /* Status and options for transmit operation */
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
    uint32_t          addr_high;    /* High portion of address in 64bit addressing mode */
    uint32_t          unused;       /* Unused word in 64bit mode */
#endif 
#if defined(MSS_MAC_TIME_STAMPED_MODE)
    volatile uint32_t nano_seconds; /* Nanoseconds and LSBs of seconds for timestamp */
    volatile uint32_t seconds;      /* MSBs of timestamp seconds */
#endif
};

/*******************************************************************************
 * TSU timer time value.
 */
typedef struct mss_mac_tsu_time mss_mac_tsu_time_t;
struct mss_mac_tsu_time
{
    uint32_t secs_msb;        /* Most significant bits of seconds count  */
    uint32_t secs_lsb;        /* Least significant bits of seconds count  */
    uint32_t nanoseconds;     /* Nanoseconds count  */
};


/*******************************************************************************
 * TSU configuration structure.
 */
typedef struct mss_mac_tsu_config mss_mac_tsu_config_t;
struct mss_mac_tsu_config
{
    /* initial values */
    uint32_t secs_msb;        /* Most significant bits of seconds count  */
    uint32_t secs_lsb;        /* Least significant bits of seconds count  */
    uint32_t nanoseconds;     /* Nanoseconds count  */

    /* Per TSU tick values */
    uint32_t ns_inc;          /* Nanoseconds TSU increment value */
    uint32_t sub_ns_inc;      /* Sub-nanoseconds TSU increment value */
};


/*******************************************************************************
 * Type 1 filter structure.
 */
typedef struct mss_mac_type_1_filter mss_mac_type_1_filter_t;
struct mss_mac_type_1_filter
{
    uint16_t udp_port;        /* UDP port number to match */
    uint8_t  dstc;            /* Designated Services/Traffic Class value to match */
    uint8_t  queue_no;        /* Queue to send to on match */
    uint8_t  drop_on_match;   /* Drop packet instead of routing to queue */
    uint8_t  dstc_enable;     /* Enable DS/TC matching */
    uint8_t  udp_port_enable; /* Enable UDP port matching */
};

/*******************************************************************************
 * Type 2 filter structures and defines.
 */
#define MSS_MAC_OFFSET_BEGINNING  0 /* Offset from frame beginning */
#define MSS_MAC_OFFSET_ETHERTYPE  1 /* Offset from byte after Ethertype */
#define MSS_MAC_OFFSET_IP_HEADER  2 /* Offset from byte after IP header */
#define MSS_MAC_OFFSET_TCP_HEADER 3 /* Offset from byte after TCP header */
#define MSS_MAC_OFFSET_UDP_HEADER 3 /* Offset from byte after UDP header */

typedef struct mss_mac_type_2_filter mss_mac_type_2_filter_t;
struct mss_mac_type_2_filter
{
    uint8_t  ethertype_enable;     /* Enable Ethertype matching */
    uint8_t  ethertype_index;      /* Which Ethertype compare block to use */
    uint8_t  vlan_priority_enable; /* Enable VLAN priority matching */
    uint8_t  vlan_priority;        /* VLAN priority level to use. */
    uint8_t  drop_on_match;        /* Drop packet instead of routing to queue */
    uint8_t  compare_a_enable;     /* Enable data comparer A */
    uint8_t  compare_a_index;      /* Index to data comparator to use for A */
    uint8_t  compare_b_enable;     /* Enable data comparer B */
    uint8_t  compare_b_index;      /* Index to data comparator to use for B */
    uint8_t  compare_c_enable;     /* Enable data comparer C */
    uint8_t  compare_c_index;      /* Index to data comparator to use for C */
    uint8_t  queue_no;             /* The queue to rout packet to on match */
};

typedef struct mss_mac_type_2_compare mss_mac_type_2_compare_t;
struct mss_mac_type_2_compare
{
    uint32_t data;              /* 32 bits of data or 16 bits of data */
    uint16_t mask;              /* 16 bit mask if data is 16 bit */
    uint8_t  disable_mask;      /* True to select raw 32 bit data match */
    uint8_t  compare_vlan_c_id; /* Compare VLAN C tag - higher precedence than S tag */
    uint8_t  compare_vlan_s_id; /* Compare VLAN S tag */
    uint8_t  compare_offset;    /* Offset type - see MSS_MAC_OFFSET_* definitions */
    uint8_t  offset_value;      /* Offset value */
};

/*******************************************************************************
 * Media Merge Sublayer configuration structure.
 */
typedef struct mss_mac_mmsl_config mss_mac_mmsl_config_t;
struct mss_mac_mmsl_config
{
    mss_mac_frag_size_t frag_size; /* See  mss_mac_frag_size_t for details */
    uint8_t preemption;            /* Enable preemption */
    uint8_t verify_disable;        /* Set true to force preemption without testing link */
    uint8_t use_pmac;              /* Receive all to pMAC if not preempting */
};

/*******************************************************************************
 * Media Merge Sublayer statistics structure.
 */
typedef struct mss_mac_mmsl_stats mss_mac_mmsl_stats_t;

/*
 * These values are actually a mix of 8 and 17 bits but we return them all as
 * 32 bits to simplify collecting the stats.
 */

struct mss_mac_mmsl_stats
{
    uint32_t smd_err_count; /* Count of unknown SMD values received */
    uint32_t ass_err_count; /* Count of frames with reassembly errors */
    uint32_t ass_ok_count;  /* Count of frames reassembled ok */
    uint32_t frag_count_rx; /* Count of mPackets received */
    uint32_t frag_count_tx; /* Count of mPackets sent */
};


#if defined(MSS_MAC_SIMPLE_TX_QUEUE)
/* Only need two descriptors... */
#undef MSS_MAC_TX_RING_SIZE
#define MSS_MAC_TX_RING_SIZE 4 /* Ok, I lied we use a couple more for testing
                                * sometimes by stuffing the queue with extra
                                * copies of the packet... */
#endif

/*******************************************************************************
 * Per queue specific info for device management structure.
 */
struct mss_mac_queue
{
#if defined(MSS_MAC_USE_DDR)
    mss_mac_tx_desc_t            *tx_desc_tab;  /* Transmit descriptor table */
#else
    mss_mac_tx_desc_t            tx_desc_tab[MSS_MAC_TX_RING_SIZE];  /* Transmit descriptor table */
#endif

#if defined(MSS_MAC_USE_DDR)
    mss_mac_rx_desc_t            *rx_desc_tab;  /* Receive descriptor table */
#else
    mss_mac_rx_desc_t            rx_desc_tab[MSS_MAC_RX_RING_SIZE];  /* Receive descriptor table */
#endif
    void                        *tx_caller_info[MSS_MAC_TX_RING_SIZE]; /* Pointer to user specific data */
    void                        *rx_caller_info[MSS_MAC_RX_RING_SIZE];
    mss_mac_transmit_callback_t  tx_complete_handler;
    mss_mac_receive_callback_t   pckt_rx_callback;
    volatile uint32_t            nb_available_tx_desc;
    volatile uint32_t            nb_available_rx_desc;
    volatile uint32_t            next_free_rx_desc_index;
    volatile uint32_t            first_rx_desc_index;
    uint32_t                     rx_discard;
    uint32_t                     overflow_counter;
    volatile int32_t             in_isr; /* Set when processing ISR so functions
                                          * don't call PLIC enable/disable for protection */

    /* Queue specific register addresses to simplify the driver code */
    volatile uint32_t           *int_status;        /* interrupt status */
    volatile uint32_t           *int_mask;          /* interrupt mask */
    volatile uint32_t           *int_enable;        /* interrupt enable */
    volatile uint32_t           *int_disable;       /* interrupt disable */
    volatile uint32_t           *receive_q_ptr;     /* RX queue pointer */
    volatile uint32_t           *transmit_q_ptr;    /* TX queue pointer */
    volatile uint32_t           *dma_rxbuf_size;    /* RX queue buffer size */

    /* Statistics counters */

    volatile uint64_t ingress;
    volatile uint64_t egress;
    volatile uint64_t rx_overflow;
    volatile uint64_t hresp_error;
    volatile uint64_t rx_restart;
    volatile uint64_t rx_fail;
    volatile uint64_t tx_fail;
};

typedef struct mss_mac_queue mss_mac_queue_t;


/*******************************************************************************
 * G5SoC Ethernet MAC instance
 * A local record of this type "g_mac" will be created and maintained by the
 * driver.
 */
typedef struct
{
    int32_t           is_emac;    /* 0 for primary MAC and non zero for eMAC */
    MAC_TypeDef       *mac_base;  /* Register start address - NULL if eMAC */
    eMAC_TypeDef      *emac_base; /* Register start address - NULL if primary MAC */
    PLIC_IRQn_Type    mac_q_int[MSS_MAC_QUEUE_COUNT];
#if defined(TARGET_G5_SOC)
    PLIC_IRQn_Type    mmsl_int;
#endif

    mss_mac_queue_t   queue[MSS_MAC_QUEUE_COUNT];

    volatile uint64_t tx_pause;
    volatile uint64_t rx_pause;
    volatile uint64_t pause_elapsed;

    uint32_t          rx_discard;
    volatile int32_t  g_mac_available; /* Flag to show init is done and MAC and PHY can be used */

    /* These are set from the cfg structure */

    uint32_t jumbo_frame_enable;        /* Enable / disable jumbo frame support: */
    uint32_t append_CRC;                /* Enable / disable GEM CRC calculation */
    uint32_t interface_type;            /* Type of network interface associated with this GEM */
    uint32_t phy_type;                  /* PHY device type associated with this GEM */
    uint32_t phy_addr;                  /* Address of Ethernet PHY on MII management interface. */
    uint32_t pcs_phy_addr;              /* Address of SGMII interface controller on MII management interface. */
    uint8_t  mac_addr[6];               /* Station's MAC address */

    /* PHY interface functions */
    mss_mac_phy_init_t            phy_init;
    mss_mac_phy_set_speed_t       phy_set_link_speed;
    mss_mac_phy_autonegotiate_t   phy_autonegotiate;
    mss_mac_phy_get_link_status_t phy_get_link_status;
#if MSS_MAC_USE_PHY_DP83867
    mss_mac_phy_extended_read_t   phy_extended_read;
    mss_mac_phy_extended_write_t  phy_extended_write;
#endif

} mss_mac_instance_t;

#ifdef __cplusplus
}
#endif

#endif /* MSS_ETHERNET_MAC_TYPES_H_ */


