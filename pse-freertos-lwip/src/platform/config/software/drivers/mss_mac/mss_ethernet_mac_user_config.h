/*******************************************************************************
 * (c) Copyright 2018 Microsemi - PRO. All rights reserved.
 *
 * This file contains system specific definitions for the PSE MSS Ethernet MAC
 * device driver.
 * 
 * SVN $Revision: 10527 $
 * SVN $Date: 2018-11-10 12:47:05 +0000 (Sat, 10 Nov 2018) $
 *
 */

#ifndef ACTEL__FIRMWARE__PSE_MSS_ETHERNET_MAC_DRIVER__1_0_100_CONFIGURATION_HEADER
#define ACTEL__FIRMWARE__PSE_MSS_ETHERNET_MAC_DRIVER__1_0_100_CONFIGURATION_HEADER


#define CORE_VENDOR "Actel"
#define CORE_LIBRARY "Firmware"
#define CORE_NAME "PSE_MSS_Ethernet_MAC_Driver"
#define CORE_VERSION "1.0.100"

/*
 * Defines for OS and network stack specific support
 * Un-comment as necessary or define in project properties etc.
 */

/* #define USING_FREERTOS */
/* #define USING_LWIP */

/* Allowed PHY interface types: */

#define NULL_PHY                        0x0001 /* No PHY in connection, for example GEM0 and GEM1 connected via fabric */
#define GMII                            0x0002 /* Currently only on Aloe board */
#define TBI                             0x0004 /* HW emulation designs with TBI */
#define GMII_SGMII						0x0008 /* HW emulation designs with SGMII to GMII conversion */
#define BASEX1000                       0x0010 /* Not currently available */
#define RGMII                           0x0020 /* Not currently available */
#define RMII                            0x0040 /* Not currently available */
#define SGMII                           0x0080 /* Not currently available */

/* Allowed PHY models: */

#define MSS_MAC_DEV_PHY_NULL			0x0001
#define MSS_MAC_DEV_PHY_VSC8575			0x0002
#define MSS_MAC_DEV_PHY_VSC8541			0x0004
#define MSS_MAC_DEV_PHY_DP83867			0x0008


#if defined(TARGET_ALOE)
#define MSS_MAC_PHY_INTERFACE GMII /* Only one option allowed here... */
#define MSS_MAC_RX_RING_SIZE 4
#define MSS_MAC_TX_RING_SIZE 2
#define MSS_MAC_PHYS (MSS_MAC_DEV_PHY_NULL | MSS_MAC_DEV_PHY_VSC8541)
#endif

#if defined(TARGET_G5_SOC)
#define MSS_MAC_PHYS (MSS_MAC_DEV_PHY_NULL | MSS_MAC_DEV_PHY_VSC8575 | MSS_MAC_DEV_PHY_DP83867)


/*
 * Defines for the different hardware configurations on the HW emulation etc.
 *
 * Used to allow software configure GPIO etc, to support the appropriate
 * hardware configuration. Not strictly part of the driver but we manage them
 * here to keep things tidy.
 */

#define MSS_MAC_DESIGN_ALOE                0  /* ALOE board from Sifive                                           */
#define MSS_MAC_DESIGN_AKRAM_GMII          1  /* Akrams VSC8575 designs with HW emulation GMII to SGMII bridge on GEM0  */
#define MSS_MAC_DESIGN_AKRAM_TBI           2  /* Akrams VSC8575 designs with HW emulation TBI to SGMII bridge on GEM0   */
#define MSS_MAC_DESIGN_AKRAM_DUAL_INTERNAL 3  /* Akrams Dual GEM design with loopback in fabric                   */
#define MSS_MAC_DESIGN_KEN_GMII            4  /* Kens DP83867 design with HW emulation GMII to SGMII bridge             */
#define MSS_MAC_DESIGN_AKRAM_DUAL_EX_TI    5  /* Akrams Dual GEM design with external TI PHY on GEM1 (GMII)       */
#define MSS_MAC_DESIGN_AKRAM_DUAL_EX_VTS   6  /* Akrams Dual GEM design with external Vitess PHY on GEM0 (GMII)   */
#define MSS_MAC_DESIGN_AKRAM_GMII_GEM1     7  /* Akrams VSC8575 designs with HW emulation GMII to SGMII bridge on GEM 1 */
#define MSS_MAC_DESIGN_AKRAM_DUAL_EXTERNAL 8  /* Akrams Dual GEM design with GEM0 -> VSC, GEM1 -> TI (both GMII)  */
#define MSS_MAC_DESIGN_AKRAM_TBI_GEM1      9  /* Akrams VSC8575 designs with HW emulation TBI to SGMII bridge GEM1      */
#define MSS_MAC_DESIGN_AKRAM_TBI_GEM1_TI   10 /* Akrams DP83867 designs with HW emulation TBI to SGMII bridge GEM0      */

//#define MSS_MAC_HW_PLATFORM MSS_MAC_DESIGN_AKRAM_GMII_GEM1
//#define MSS_MAC_HW_PLATFORM MSS_MAC_DESIGN_AKRAM_DUAL_EX_TI
#define MSS_MAC_HW_PLATFORM MSS_MAC_DESIGN_AKRAM_DUAL_EX_VTS

//#define MSS_MAC_RX_RING_SIZE 16
#define MSS_MAC_RX_RING_SIZE 9
#define MSS_MAC_TX_RING_SIZE 4 /* PMCS should be 2 but want to be able to force duplicate
                                * tx to stuff multiple packets into FIFO for testing */
#endif

#define MSS_MAC_USE_PHY_VSC8575 (0 != (MSS_MAC_PHYS & MSS_MAC_DEV_PHY_VSC8575))
#define MSS_MAC_USE_PHY_VSC8541 (0 != (MSS_MAC_PHYS & MSS_MAC_DEV_PHY_VSC8541))
#define MSS_MAC_USE_PHY_DP83867 (0 != (MSS_MAC_PHYS & MSS_MAC_DEV_PHY_DP83867))
#define MSS_MAC_USE_PHY_NULL    (0 != (MSS_MAC_PHYS & MSS_MAC_DEV_PHY_NULL))

#define MSS_MAC_TIME_STAMPED_MODE      0 /* Enable time stamp support */ */

/*
 * Defines for different memory areas. Set the macro MSS_MAC_USE_DDR to one of
 * these values to select the area of memory and buffer sizes to use when
 * testing for non LIM based areas of memory.
 */

#define MSS_MAC_MEM_DDR    0
#define MSS_MAC_MEM_FIC0   1
#define MSS_MAC_MEM_FIC1   2
#define MSS_MAC_MEM_CRYPTO 3

/*
 * Number of additional queues for PMAC.
 *
 * Note. We explicitly set the number of queues in the MAC structure as we have
 * to indicate the Interrupt Number so this is slightly artificial...
 */
#if defined(TARGET_ALOE)
#define MSS_MAC_QUEUE_COUNT 1
#else
#define MSS_MAC_QUEUE_COUNT 4
#endif


/*
 * Number of Type 1 and 2 screeners
 */

#define MSS_MAC_TYPE_1_SCREENERS  4
#define MSS_MAC_TYPE_2_SCREENERS  4
#define MSS_MAC_TYPE_2_ETHERTYPES 4
#define MSS_MAC_TYPE_2_COMPARERS  4

/* These are hard coded and not user selectable */
#define MSS_MAC_EMAC_TYPE_2_SCREENERS  2
#define MSS_MAC_EMAC_TYPE_2_COMPARERS  6

#endif /* ACTEL__FIRMWARE__PSE_MSS_ETHERNET_MAC_DRIVER__1_0_100_CONFIGURATION_HEADER */

