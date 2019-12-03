/*******************************************************************************
 * (c) Copyright 2018 Microsemi - PRO. All rights reserved.
 *
 * PSE 10/100/1000 Mbps Ethernet MAC bare metal software driver implementation.
 *
 * SVN $Revision: 7657 $
 * SVN $Date: 2015-08-13 13:19:00 +0100 (Thu, 13 Aug 2015) $
 */

#include <stdint.h>

#include "config/hardware/hw_platform.h"
#include "config/software/drivers/mss_mac/mss_ethernet_mac_user_config.h"

#include "mpfs_hal/mss_plic.h"
#include "mpfs_hal/mss_sysreg.h"

#include "hal/hal.h"
#include "hal/hal_assert.h"

#include "mac_registers.h"

#include "pse_mac_regs.h"
#include "mss_ethernet_mac.h"
#include "phy.h"

#if defined(USING_FREERTOS)
#include "FreeRTOS.h"
#endif

#if defined (TI_PHY)
#include "mss_gpio/mss_gpio.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************/
/* Preprocessor Macros                                                    */
/**************************************************************************/

#define NULL_POINTER                    0

#if !defined(NDEBUG)
#define IS_STATE(x)                     ( ((x) == MSS_MAC_ENABLE) || ((x) == MSS_MAC_DISABLE) )

#endif  /* NDEBUG */

#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
#define IS_WORD_ALIGNED(x)              ((uint64_t)0 == ((uint64_t)(x) & (uint64_t)3))
#else
#define IS_WORD_ALIGNED(x)              ((uint32_t)0 == ((uint32_t)(x) & (uint32_t)3))
#endif

#define BMSR_AUTO_NEGOTIATION_COMPLETE  0x0020u

#define INVALID_INDEX                   (0xFFFFFFFF)

#define DMA_TX_DISABLED                 0u
#define DMA_TX_ENABLED                  1u

#define PHY_ADDRESS_MIN                 0U
#define PHY_ADDRESS_MAX                 31U

/*
 * Defines for determining DMA descriptor sizes 
 */
//#if !defined(MSS_MAC_TIME_STAMPED_MODE)
//#define MSS_MAC_TIME_STAMPED_MODE      0 /* Default to non time stamped descriptors */
//#endif

#if defined(TARGET_ALOE)
#define MSS_MAC0_BASE  0x10090000;
#elif defined(TARGET_G5_SOC)
#define MSS_MAC0_BASE  0x20110000
#define MSS_EMAC0_BASE 0x20111000
#define MSS_MAC1_BASE  0x20112000
#define MSS_EMAC1_BASE 0x20113000
#else
#warning "No target platform defined for MSS Ethernet MAC"
#endif

/*******************************************************************************
 * MAC interrupt definitions
 */
#define MSS_MAC_TXPKTSENT_IRQ    0u
#define MSS_MAC_TXPKTUNDER_IRQ   1u
#define MSS_MAC_TXPKTBERR_IRQ    3u
#define MSS_MAC_RXPKTRCVD_IRQ    4u
#define MSS_MAC_RXPKTOVER_IRQ    6u
#define MSS_MAC_RXPKTBERR_IRQ    7u

#define MSS_MAC_TXPKTSENT_IRQ_MASK      0x01u
#define MSS_MAC_TXPKTUNDER_IRQ_MASK     0x02u
#define MSS_MAC_TXPKTBERR_IRQ_MASK      0x08u
#define MSS_MAC_RXPKTRCVD_IRQ_MASK      0x10u
#define MSS_MAC_RXPKTOVER_IRQ_MASK      0x40u
#define MSS_MAC_RXPKTBERR_IRQ_MASK      0x80u

/**************************************************************************/
/* Private variables                                                      */
/**************************************************************************/


#if defined(TARGET_ALOE)
static volatile uint32_t *GEMGXL_tx_clk_sel = (volatile uint32_t *)0x100A0000ul;
static volatile uint32_t *GEMGXL_speed_mode = (volatile uint32_t *)0x100A0020ul;
#endif

static int instances_init_done = 0;

/**************************************************************************
 * Global variables                                                       *
 *                                                                        *
 * Note: there are two instances of the GMAC for G5 SOC and each has a    *
 * primary MAC and a secondary eMAC for time sensitive traffic. The FU540 *
 * device on the Aloe board has a single primary MAC.                     *
 **************************************************************************/

#if defined(TARGET_G5_SOC)
mss_mac_instance_t g_mac0;
mss_mac_instance_t g_mac1;
mss_mac_instance_t g_emac0;
mss_mac_instance_t g_emac1;
#endif

#if defined(MSS_MAC_USE_DDR)
#if MSS_MAC_USE_DDR == MSS_MAC_MEM_DDR
uint8_t *g_mss_mac_ddr_ptr = (uint8_t *)0xC0000000LLU;
#elif MSS_MAC_USE_DDR == MSS_MAC_MEM_FIC0
uint8_t *g_mss_mac_ddr_ptr = (uint8_t *)0x60000000LLU;
#elif MSS_MAC_USE_DDR == MSS_MAC_MEM_FIC1
uint8_t *g_mss_mac_ddr_ptr = (uint8_t *)0xE0000000LLU;
#elif MSS_MAC_USE_DDR == MSS_MAC_MEM_CRYPTO
uint8_t *g_mss_mac_ddr_ptr = (uint8_t *)0x22002000LLU;
#else
#error "bad memory region defined"
#endif
#endif


#if defined(TARGET_ALOE)
mss_mac_instance_t g_mac0;
#endif


/**************************************************************************/
/* Private Functions                                                      */
/**************************************************************************/
static void mac_reset(void);
static void config_mac_hw(mss_mac_instance_t *this_mac, const mss_mac_cfg_t * cfg);
static void tx_desc_ring_init(mss_mac_instance_t *this_mac);
static void rx_desc_ring_init(mss_mac_instance_t *this_mac);
static void assign_station_addr(mss_mac_instance_t *this_mac, const uint8_t mac_addr[6]);
static void generic_mac_irq_handler(mss_mac_instance_t *this_mac, uint32_t queue_no);
static void rxpkt_handler(mss_mac_instance_t *this_mac, uint32_t queue_no);
static void txpkt_handler(mss_mac_instance_t *this_mac, uint32_t queue_no);
static void update_mac_cfg(mss_mac_instance_t *this_mac);
static uint8_t probe_phy(mss_mac_instance_t *this_mac);
static void instances_init(void);

static void msgmii_init(mss_mac_instance_t *this_mac);
static void msgmii_autonegotiate(mss_mac_instance_t *this_mac);

/**************************************************************************/
/* Public Functions                                                       */
/**************************************************************************/
/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */
void 
MSS_MAC_init
( 
    mss_mac_instance_t *this_mac,
    mss_mac_cfg_t      *cfg 
)
{
    int32_t queue_no;
    HAL_ASSERT(cfg != NULL_POINTER);
#if defined(TARGET_ALOE)
    HAL_ASSERT(this_mac == &g_mac0);

    if(!instances_init_done) /* One time init of structures */
    {
        instances_init();
    }
    
    if((cfg != NULL_POINTER) && (this_mac == &g_mac0))
#endif
#if defined(TARGET_G5_SOC)
    HAL_ASSERT((this_mac == &g_mac0) || (this_mac == &g_mac1) || (this_mac == &g_emac0) || (this_mac == &g_emac1));
    if(!instances_init_done) /* One time init of structures */
    {
        instances_init();
    }

    /*
     * Always reset GEM if the pMAC is selected for init but not if eMAC
     * The pMAC should always be initialised first followed by the eMAC so this
     * is OK...
     */
    if(this_mac == &g_mac0)
    {
        SYSREG->SUBBLK_CLOCK_CR |= 2u;
        /* Reset MAC */
        SYSREG->SOFT_RESET_CR |= 2u;
        {
            volatile int index;

            index = 0;
            while(1000 != index) /* Don't know if necessary, but delay a bit before de-asserting reset... */
            {
                index++;
            }
        }
        /* Take MAC out of reset. */
        SYSREG->SOFT_RESET_CR &= ~2u;
    }

    if(this_mac == &g_mac1)
    {
        SYSREG->SUBBLK_CLOCK_CR |= 4u;
        /* Reset MAC */
        SYSREG->SOFT_RESET_CR |= 4u;
        {
            volatile int index;

            index = 0;
            while(1000 != index) /* Don't know if necessary, but delay a bit before de-asserting reset... */
            {
                index++;
            }
        }
        /* Take MAC out of reset. */
        SYSREG->SOFT_RESET_CR &= ~4u;
    }


/* PMCS: for the Emulation platform we need to select the non default TSU clock
 * for the moment or TX won't work */

    if(!this_mac->is_emac)
    {
        this_mac->mac_base->USER_IO = 1u;
    }

    if((cfg != NULL_POINTER) && ((this_mac == &g_mac0) || (this_mac == &g_mac1) || (this_mac == &g_emac0) || (this_mac == &g_emac1)))
#endif /* defined(TARGET_G5_SOC) */
    {
        this_mac->phy_addr            = cfg->phy_addr;
        this_mac->pcs_phy_addr        = cfg->pcs_phy_addr;
        this_mac->interface_type      = cfg->interface_type;
        this_mac->jumbo_frame_enable  = cfg->jumbo_frame_enable;
        this_mac->phy_type            = cfg->phy_type;
        this_mac->phy_autonegotiate   = cfg->phy_autonegotiate;
        this_mac->phy_get_link_status = cfg->phy_get_link_status;
        this_mac->phy_init            = cfg->phy_init;
        this_mac->phy_set_link_speed  = cfg->phy_set_link_speed;
        this_mac->append_CRC          = cfg->append_CRC;
#if MSS_MAC_USE_PHY_DP83867
        this_mac->phy_extended_read   = cfg->phy_extended_read;
        this_mac->phy_extended_write  = cfg->phy_extended_write;
#endif

        /*
         * In the following if an interrupt is set to NoInterrupt_IRQn then
         * the PLIC will ignore it as interrupt 0 is a dummy one.
          */
        PLIC_DisableIRQ(this_mac->mac_q_int[0]);
        PLIC_DisableIRQ(this_mac->mac_q_int[1]);
        PLIC_DisableIRQ(this_mac->mac_q_int[2]);
        PLIC_DisableIRQ(this_mac->mac_q_int[3]);
        PLIC_DisableIRQ(this_mac->mmsl_int);

        mac_reset();
        
        config_mac_hw(this_mac, cfg);
        
        /* Assign MAC station address */
        assign_station_addr(this_mac, cfg->mac_addr);
        
        /* Initialize Tx & Rx descriptor rings */
        tx_desc_ring_init(this_mac);
        rx_desc_ring_init(this_mac);
        
        this_mac->rx_discard = 0; /* Ensure normal RX operation */

        for(queue_no = 0; queue_no != MSS_MAC_QUEUE_COUNT; queue_no++)
        {
            /* Initialize Tx descriptors related variables. */
            this_mac->queue[queue_no].nb_available_tx_desc    = MSS_MAC_TX_RING_SIZE;

            /* Initialize Rx descriptors related variables. */
            this_mac->queue[queue_no].nb_available_rx_desc    = MSS_MAC_RX_RING_SIZE;
            this_mac->queue[queue_no].next_free_rx_desc_index = 0;
            this_mac->queue[queue_no].first_rx_desc_index     = 0;

            /* initialize default interrupt handlers */
            this_mac->queue[queue_no].tx_complete_handler     = NULL_POINTER;
            this_mac->queue[queue_no].pckt_rx_callback        = NULL_POINTER;

            /* Added these to MAC structure to make them MAC specific... */

            this_mac->queue[queue_no].ingress     = 0;
            this_mac->queue[queue_no].egress      = 0;
            this_mac->queue[queue_no].rx_overflow = 0;
            this_mac->queue[queue_no].hresp_error = 0;
            this_mac->queue[queue_no].rx_restart  = 0;
            this_mac->queue[queue_no].rx_fail     = 0;
            this_mac->queue[queue_no].tx_fail     = 0;
        }
#if 0        
        /* Initialize PHY interface */
        if(MSS_MAC_AUTO_DETECT_PHY_ADDRESS == cfg->phy_addr)
        {
            cfg->phy_addr = probe_phy();
        }
#endif
if(!this_mac->is_emac) /* Only do the PHY stuff for primary MAC */
    {
        if(TBI == this_mac->interface_type)
        {
            msgmii_init(this_mac);
        }
#if defined(TARGET_ALOE)
            this_mac->phy_addr = 0;
#endif
//        for(;;)
            //dump_vsc_regs(this_mac);

        this_mac->phy_init(this_mac, 0);
        this_mac->phy_set_link_speed(this_mac, cfg->speed_duplex_select);
        this_mac->phy_autonegotiate(this_mac);

//#if (MSS_MAC_PHY_INTERFACE == TBI) || (MSS_MAC_PHY_INTERFACE == GMII_SGMII)
        if(TBI == this_mac->interface_type)
        {
        msgmii_autonegotiate(this_mac);
        }
    }
        update_mac_cfg(this_mac);

        /*
         * Enable TX Packet and TX Packet Bus Error interrupts.
         * We don't enable the tx underrun interrupt as we only send on demand
         * and don't need to explicitly note an underrun overrun as that is the "normal"
         * state for the interface to be in.
         */
        /*
         * Enable RX Packet, RX Packet Overflow and RX Packet Bus Error
         * interrupts.
         */
        if(this_mac->is_emac)
        {
            this_mac->emac_base->INT_ENABLE = GEM_RECEIVE_OVERRUN_INT | GEM_TRANSMIT_COMPLETE |
                                              GEM_RX_USED_BIT_READ | GEM_RECEIVE_COMPLETE | GEM_RESP_NOT_OK_INT;
        }
        else
        {
            for(queue_no = 0; queue_no != MSS_MAC_QUEUE_COUNT; queue_no++)
            {
                /* Enable pause related interrupts if pause control is selected */
                if((MSS_MAC_ENABLE == cfg->rx_flow_ctrl) || (MSS_MAC_ENABLE == cfg->tx_flow_ctrl))
                {
                    *this_mac->queue[queue_no].int_enable = GEM_RECEIVE_OVERRUN_INT | GEM_TRANSMIT_COMPLETE |
                            GEM_RX_USED_BIT_READ | GEM_RECEIVE_COMPLETE | GEM_RESP_NOT_OK_INT |
                            GEM_PAUSE_FRAME_TRANSMITTED | GEM_PAUSE_TIME_ELAPSED |
                            GEM_PAUSE_FRAME_WITH_NON_0_PAUSE_QUANTUM_RX;
                }
                else
                {
                    *this_mac->queue[queue_no].int_enable = GEM_RECEIVE_OVERRUN_INT | GEM_TRANSMIT_COMPLETE |
                            GEM_RX_USED_BIT_READ | GEM_RECEIVE_COMPLETE | GEM_RESP_NOT_OK_INT;
                }

#if 1 /* PMCS - set this to 0 if you want to check for un-handled interrupt conditions */
                *this_mac->queue[queue_no].int_enable |= GEM_PAUSE_FRAME_TRANSMITTED | GEM_PAUSE_TIME_ELAPSED |
                        GEM_PAUSE_FRAME_WITH_NON_0_PAUSE_QUANTUM_RX | GEM_LINK_CHANGE |
                        GEM_TX_LOCKUP_DETECTED | GEM_RX_LOCKUP_DETECTED |
                        GEM_AMBA_ERROR  | GEM_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION |
                        GEM_TRANSMIT_UNDER_RUN;
#endif
            }
        }

        /*
         * At this stage, the MSS MAC interrupts are disabled and won't be enabled
         * until at least one of the FIFOs is configured with a buffer(s) for
         * data transfer.
         */
    }

    this_mac->g_mac_available   = 1;
}

/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */
void
MSS_MAC_update_hw_address
(
    mss_mac_instance_t *this_mac,
    mss_mac_cfg_t * cfg
)
{
    HAL_ASSERT(cfg != NULL_POINTER);

    if(cfg != NULL_POINTER)
    {
        /* Assign MAC station address */
        assign_station_addr(this_mac, cfg->mac_addr);
    }
}

/**************************************************************************//**
 * 
 */
static void update_mac_cfg(mss_mac_instance_t *this_mac)
{
    mss_mac_speed_t speed;
    uint8_t fullduplex;
    uint8_t link_up;
    uint32_t temp_cr;
    
    link_up = this_mac->phy_get_link_status(this_mac, &speed, &fullduplex);

    if(link_up != MSS_MAC_LINK_DOWN)
    {
        if(this_mac->is_emac)
        {
            temp_cr = this_mac->emac_base->NETWORK_CONFIG;
        }
        else
        {
            temp_cr = this_mac->mac_base->NETWORK_CONFIG;
        }
        
        temp_cr &= (uint32_t)(~(GEM_GIGABIT_MODE_ENABLE | GEM_SPEED | GEM_FULL_DUPLEX));

        if(MSS_MAC_1000MBPS == speed)
        {
#if defined(TARGET_ALOE)
            *GEMGXL_tx_clk_sel = 0;
            *GEMGXL_speed_mode = 2;
#endif
            temp_cr |= GEM_GIGABIT_MODE_ENABLE;
        }
        else if(MSS_MAC_100MBPS == speed)
        {
#if defined(TARGET_ALOE)
            *GEMGXL_tx_clk_sel = 1;
            *GEMGXL_speed_mode = 1;
#endif
            temp_cr |= GEM_SPEED;
        }
        else
        {
#if defined(TARGET_ALOE)
            *GEMGXL_tx_clk_sel = 1;
            *GEMGXL_speed_mode = 0;
#endif
        }
        
//        SYSREG->MAC_CR = (SYSREG->MAC_CR & ~MAC_CONFIG_SPEED_MASK) | (uint32_t)speed;
        
        /* Configure duplex mode */
        if(MSS_MAC_FULL_DUPLEX == fullduplex)
        {
            temp_cr |= GEM_FULL_DUPLEX;
        }
        
        if(this_mac->is_emac)
        {
            this_mac->emac_base->NETWORK_CONFIG = temp_cr;
        }
        else
        {
            this_mac->mac_base->NETWORK_CONFIG = temp_cr;
        }
    }
}

/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */
uint8_t MSS_MAC_get_link_status
(
    mss_mac_instance_t *this_mac,
    mss_mac_speed_t * speed,
    uint8_t *     fullduplex
)
{
    /* Todo: These statics will only work for the single MAC Aloe case... */
    static mss_mac_speed_t previous_speed = INVALID_SPEED;
    static uint8_t previous_duplex = 0xAAu;
    mss_mac_speed_t link_speed;
    uint8_t link_fullduplex;
    uint8_t link_up;
    
    link_up = this_mac->phy_get_link_status(this_mac, &link_speed, &link_fullduplex);

    if(link_up != MSS_MAC_LINK_DOWN)
    {
        /*----------------------------------------------------------------------
         * Update MAC configuration if link characteristics changed.
         */
        if((link_speed != previous_speed) || (link_fullduplex != previous_duplex))
        {
            uint32_t temp_cr;

            if(this_mac->is_emac)
            {
                temp_cr = this_mac->emac_base->NETWORK_CONFIG;
            }
            else
            {
                temp_cr = this_mac->mac_base->NETWORK_CONFIG;
            }
            
            temp_cr &= (uint32_t)(~(GEM_GIGABIT_MODE_ENABLE | GEM_SPEED | GEM_FULL_DUPLEX));

            if(MSS_MAC_1000MBPS == link_speed)
            {
#if defined(TARGET_ALOE)
                *GEMGXL_tx_clk_sel = 0;
                *GEMGXL_speed_mode = 2;
#endif
                temp_cr |= GEM_GIGABIT_MODE_ENABLE;
            }
            else if(MSS_MAC_100MBPS == link_speed)
            {
#if defined(TARGET_ALOE)
                *GEMGXL_tx_clk_sel = 1;
                *GEMGXL_speed_mode = 1;
#endif
                temp_cr |= GEM_SPEED;
            }
            else
            {
#if defined(TARGET_ALOE)
                *GEMGXL_tx_clk_sel = 1;
                *GEMGXL_speed_mode = 0;
#endif
            }
            
            /* Configure duplex mode */
            if(MSS_MAC_FULL_DUPLEX == link_fullduplex)
            {
                temp_cr |= GEM_FULL_DUPLEX;
            }
            
            if(this_mac->is_emac)
            {
                this_mac->emac_base->NETWORK_CONFIG = temp_cr;
            }
            else
            {
                this_mac->mac_base->NETWORK_CONFIG = temp_cr;
            }
        }

        previous_speed = link_speed;
        previous_duplex = link_fullduplex;
        
        /*----------------------------------------------------------------------
         * Return current link speed and duplex mode.
         */
        if(speed != NULL_POINTER)
        {
            *speed = link_speed;
        }
        
        if(fullduplex != NULL_POINTER)
        {
            *fullduplex = link_fullduplex;
        }
        if(GMII_SGMII == this_mac->interface_type) /* Emulation platform with embedded SGMII link for PHY connection and GMII for MAC connection */
        {
            uint16_t phy_reg;
            uint16_t sgmii_link_up;

            /*
             * Find out if link is up on SGMII link between HW emulation GMII core and
             * external PHY.
             *
             * The link status bit latches 0 state until read to record fact
             * link has failed since last read so you need to read it twice to
             * get the current status...
             */
            phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMSR);
            phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMSR);
            sgmii_link_up = phy_reg & BMSR_LSTATUS;

            if(0u == sgmii_link_up)
            {
                /* Initiate auto-negotiation on the SGMII link. */
                phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMCR);
                phy_reg |= BMCR_ANENABLE;
                MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMCR, phy_reg);
                phy_reg |= BMCR_ANRESTART;
                MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMCR, phy_reg);
            }
         }

#if 0 /* TBD old SF2 stuff keep here for now for guidance... */
#if ((MSS_MAC_PHY_INTERFACE == SGMII) || (MSS_MAC_PHY_INTERFACE == TBI))
#if (MSS_MAC_PHY_INTERFACE == TBI)
/*----------------------------------------------------------------------
 * Make sure SGMII interface link is up. if interface is TBI
 */
#define MDIO_PHY_ADDR   SF2_MSGMII_PHY_ADDR
#endif /* #if (MSS_MAC_PHY_INTERFACE == TBI) */

#if (MSS_MAC_PHY_INTERFACE == SGMII)
/*----------------------------------------------------------------------
 * Make sure SGMII/1000baseX interface link is up. if interface is 
 * SGMII/1000baseX
 */
#define MDIO_PHY_ADDR   MSS_MAC_INTERFACE_MDIO_ADDR
#endif /* #if ((MSS_MAC_PHY_INTERFACE == SGMII) || (MSS_MAC_PHY_INTERFACE == BASEX1000)) */

        {
            uint16_t phy_reg;
            uint16_t sgmii_link_up;
            
            /* Find out if link is up on SGMII link between MAC and external PHY. */
            phy_reg = MSS_MAC_read_phy_reg(MDIO_PHY_ADDR, MII_BMSR);
            sgmii_link_up = phy_reg & BMSR_LSTATUS;
            
            if(0u == sgmii_link_up)
            {
                /* Initiate auto-negotiation on the SGMII link. */
                phy_reg = MSS_MAC_read_phy_reg(MDIO_PHY_ADDR, MII_BMCR);
                phy_reg |= BMCR_ANENABLE;
                MSS_MAC_write_phy_reg(MDIO_PHY_ADDR, MII_BMCR, phy_reg);
                phy_reg |= BMCR_ANRESTART;
                MSS_MAC_write_phy_reg(MDIO_PHY_ADDR, MII_BMCR, phy_reg);
            }
         }
#endif 
#endif
    }   
    return link_up;
}

/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */
void 
MSS_MAC_cfg_struct_def_init
(
    mss_mac_cfg_t * cfg
)
{
    HAL_ASSERT(NULL_POINTER != cfg);
    if(NULL_POINTER != cfg)
    {
        memset(cfg, 0, sizeof(mss_mac_cfg_t)); /* Start with clean slate */

        cfg->speed_duplex_select = MSS_MAC_ANEG_ALL_SPEEDS;
#if defined(TARGET_ALOE)
        cfg->phy_addr           = 0;
#endif
#if defined(TARGET_G5_SOC)
        cfg->phy_addr            = PHY_NULL_MDIO_ADDR; //MSS_MAC_AUTO_DETECT_PHY_ADDRESS;
#endif
        cfg->tx_edc_enable       = MSS_MAC_ERR_DET_CORR_ENABLE;
        cfg->rx_edc_enable       = MSS_MAC_ERR_DET_CORR_ENABLE;
        cfg->jumbo_frame_enable  = MSS_MAC_JUMBO_FRAME_DISABLE;
        cfg->jumbo_frame_default = MSS_MAC_MAX_PACKET_SIZE;
        cfg->length_field_check  = MSS_MAC_LENGTH_FILED_CHECK_ENABLE;
// TBD: Remove        cfg->pad_n_CRC           = MSS_MAC_PAD_N_CRC_ENABLE;
        cfg->append_CRC          = MSS_MAC_CRC_ENABLE;
        cfg->loopback            = MSS_MAC_LOOPBACK_DISABLE;
        cfg->rx_flow_ctrl        = MSS_MAC_RX_FLOW_CTRL_ENABLE;
        cfg->tx_flow_ctrl        = MSS_MAC_TX_FLOW_CTRL_ENABLE;
        cfg->ipg_multiplier      = MSS_MAC_IPG_DEFVAL;
        cfg->ipg_divisor         = MSS_MAC_IPG_DEFVAL;
        cfg->phyclk              = MSS_MAC_DEF_PHY_CLK;
        cfg->max_frame_length    = MSS_MAC_MAXFRAMELEN_DEFVAL;
        cfg->framedrop_mask      = MSS_MAC_FRAME_DROP_MASK_DEFVAL;
        cfg->mac_addr[0]         = 0x00;
        cfg->mac_addr[1]         = 0x00;
        cfg->mac_addr[2]         = 0x00;
        cfg->mac_addr[3]         = 0x00;
        cfg->mac_addr[4]         = 0x00;
        cfg->mac_addr[5]         = 0x00;
        cfg->queue_enable[0]     = MSS_MAC_QUEUE_DISABLE;
        cfg->queue_enable[1]     = MSS_MAC_QUEUE_DISABLE;
        cfg->queue_enable[2]     = MSS_MAC_QUEUE_DISABLE;
        cfg->phy_type            = MSS_MAC_DEV_PHY_NULL;
        cfg->interface_type      = NULL_PHY;
        cfg->phy_autonegotiate   = MSS_MAC_NULL_phy_autonegotiate;
        cfg->phy_get_link_status = MSS_MAC_NULL_phy_get_link_status;
        cfg->phy_init            = MSS_MAC_NULL_phy_init;
        cfg->phy_set_link_speed  = MSS_MAC_NULL_phy_set_link_speed;
#if MSS_MAC_USE_PHY_DP83867
        cfg->phy_extended_read   = NULL_ti_read_extended_regs;
        cfg->phy_extended_write  = NULL_ti_write_extended_regs;
#endif
    }
}

/**************************************************************************//**
 * 
 */
static void 
mac_reset
(
    void
)
{
}

#if defined(TARGET_ALOE)

/**************************************************************************//**
 * PLL and Reset registers after reset in "wait for debug mode"
 * 0x10000000 : 0x10000000 <Hex Integer>
 * Address   0 - 3     4 - 7     8 - B     C - F
 * 10000000  C0000000  030187C1  00000000  030187C1
 * 10000010  00000000  00000000  00000000  030187C1
 * 10000020  00000000  00000001  00000000  00000004
 *
 * PLL and Reset registers after Linux boot.
 * 
 * 0x10000000 : 0x10000000 <Hex Integer>
 * Address   0 - 3     4 - 7     8 - B     C - F
 * 10000000  C0000000  82110EC0  00000000  82110DC0
 * 10000010  80000000  00000000  00000000  82128EC0
 * 10000020  80000000  00000000  0000002F  00000004
 *
 */

/**************************************************************************//**
 *
 */
#define __I  const volatile
#define __IO volatile
#define __O volatile

typedef struct
{
    __IO uint32_t  HFXOSCCFG;      /* 0x0000 */
    __IO uint32_t  COREPLLCFG0;    /* 0x0004 */
    __IO uint32_t  reserved0;      /* 0x0008 */
    __IO uint32_t  DDRPLLCFG0;     /* 0x000C */
    __IO uint32_t  DDRPLLCFG1;     /* 0x0010 */
    __IO uint32_t  reserved1;      /* 0x0014 */
    __IO uint32_t  reserved2;      /* 0x0018 */
    __IO uint32_t  GEMGXLPLLCFG0;  /* 0x001C */
    __IO uint32_t  GEMGXLPLLCFG1;  /* 0x0020 */
    __IO uint32_t  CORECLKSEL;     /* 0x0024 */
    __IO uint32_t  DEVICERESETREG; /* 0x0028 */
} AloePRCI_TypeDef;

AloePRCI_TypeDef *g_aloe_prci = (AloePRCI_TypeDef *)0x10000000ul;

typedef struct
{
    __IO uint32_t  PWMCFG;         /* 0x0000 */
    __IO uint32_t  reserved0;      /* 0x0004 */
    __IO uint32_t  PWMCOUNT;       /* 0x0008 */
    __IO uint32_t  reserved1;      /* 0x000C */
    __IO uint32_t  PWMS;           /* 0x0010 */
    __IO uint32_t  reserved2;      /* 0x0014 */
    __IO uint32_t  reserved3;      /* 0x0018 */
    __IO uint32_t  reserved4;      /* 0x001C */
    __IO uint32_t  PWMCMP0;        /* 0x0020 */
    __IO uint32_t  PWMCMP1;        /* 0x0024 */
    __IO uint32_t  PWMCMP2;        /* 0x0028 */
    __IO uint32_t  PWMCMP3;        /* 0x002C */
} AloePWM_TypeDef;

AloePWM_TypeDef *g_aloe_pwm0 = (AloePWM_TypeDef *)0x10020000ul;

static void config_mac_pll_and_reset(void);
static void config_mac_pll_and_reset(void)
{
    volatile int ix;
    volatile int counter;
    volatile int loops = 0;

    /*
     * COREPLLCFG0 reset value = 0x030187C1
     *  divr = 1
     *  divf = 1F
     *  divq = 3
     *  range = 0
     *  bypass = 1
     *  fse = 1
     *  lock = 0
     *
     *  Desired value = 82110EC0
     *  divr = 0
     *  divf = 1D
     *  divq = 2
     *  range = 4
     *  bypass = 0
     *  fse = 1
     *  lock = 1
     */
#if 0 /* Test code for proving Core clock speed switching works */
    g_aloe_pwm0->PWMCFG = 0x0000020Eu;
    g_aloe_pwm0->PWMCMP0 = 0x0000FFFFu;
    g_aloe_pwm0->PWMCMP1 = 0x0000FFFFu;
    g_aloe_pwm0->PWMCMP2 = 0x0000FFFFu;
    g_aloe_pwm0->PWMCMP3 = 0x0000FFFFu;

    while(loops < 16)
    {
        if(ix & 1)
        {
            g_aloe_pwm0->PWMCMP0 = 0x00000000u;
        }
        else
        {
            g_aloe_pwm0->PWMCMP0 = 0x0000FFFFu;
        }

        if(ix & 2)
        {
            g_aloe_pwm0->PWMCMP1 = 0x00000000u;
        }
        else
        {
            g_aloe_pwm0->PWMCMP1 = 0x0000FFFFu;
        }

        if(ix & 4)
        {
            g_aloe_pwm0->PWMCMP2 = 0x00000000u;
        }
        else
        {
            g_aloe_pwm0->PWMCMP2 = 0x0000FFFFu;
        }

        if(ix & 8)
        {
            g_aloe_pwm0->PWMCMP3 = 0x00000000u;
        }
        else
        {
            g_aloe_pwm0->PWMCMP3 = 0x0000FFFFu;
        }

        ix++;
        for(counter = 0; counter != 100000; counter++)
            ;

        loops++;
    }
#endif

    g_aloe_prci->COREPLLCFG0 = 0x03110EC0u; /* Configure Core Clock PLL */
    while(g_aloe_prci->COREPLLCFG0 & 0x80000000u) /* Wait for lock with PLL bypassed */
        ix++;

    g_aloe_prci->COREPLLCFG0 = 0x02110EC0u; /* Take PLL out of bypass */
    g_aloe_prci->CORECLKSEL  = 0x00000000u; /* Switch to PLL as clock source */

#if 0 /* Test code for proving Core clock speed switching works */
    loops = 0;
    while(loops < 20)
    {
        if(ix & 1)
        {
            g_aloe_pwm0->PWMCMP0 = 0x0000FFFFu;
        }
        else
        {
            g_aloe_pwm0->PWMCMP0 = 0x00000000u;
        }

        ix++;
        for(counter = 0; counter != 3000000; counter++)
            ;

        loops++;
    }
#endif
    /*
     * GEMGXLPLLCFG0 reset value = 0x030187C1
     *  divr = 1
     *  divf = 1F
     *  divq = 3
     *  range = 0
     *  bypass = 1
     *  fse = 1
     *  lock = 0
     *
     *  Desired value = 82128EC0
     *  divr = 0
     *  divf = 3B
     *  divq = 5
     *  range = 4
     *  bypass = 0
     *  fse = 1
     *  lock = 1
     */

    g_aloe_prci->GEMGXLPLLCFG0 = 0x03128EC0u; /* Configure GEM Clock PLL */
    while(g_aloe_prci->GEMGXLPLLCFG0 & 0x80000000u) /* Wait for lock with PLL bypassed */
        ix++;

    g_aloe_prci->GEMGXLPLLCFG0 = 0x02128EC0u; /* Take PLL out of bypass */
    g_aloe_prci->GEMGXLPLLCFG1  = 0x80000000u; /* Switch to PLL as clock source */

    g_aloe_prci->DEVICERESETREG |= 0x00000020u; /* Release MAC from reset */

}
#endif

/**************************************************************************//**
 *
 */
static void config_mac_hw(mss_mac_instance_t *this_mac, const mss_mac_cfg_t * cfg)
{
    uint32_t temp_net_config = 0;
    uint32_t temp_net_control = 0;
    uint32_t temp_length;
    
    /* Check for validity of configuration parameters */
    HAL_ASSERT( IS_STATE(cfg->tx_edc_enable) );
    HAL_ASSERT( IS_STATE(cfg->rx_edc_enable) );
    HAL_ASSERT( MSS_MAC_PREAMLEN_MAXVAL >= cfg->preamble_length );
    HAL_ASSERT( IS_STATE(cfg->jumbo_frame_enable) );
    HAL_ASSERT( IS_STATE(cfg->length_field_check) );
    HAL_ASSERT( IS_STATE(cfg->append_CRC) );
    HAL_ASSERT( IS_STATE(cfg->loopback) );
    HAL_ASSERT( IS_STATE(cfg->rx_flow_ctrl) );
    HAL_ASSERT( IS_STATE(cfg->tx_flow_ctrl) );
    HAL_ASSERT( MSS_MAC_SLOTTIME_MAXVAL >= cfg->slottime );
    
#if defined(TARGET_ALOE)
    config_mac_pll_and_reset();
#endif
    /*--------------------------------------------------------------------------
     * Configure MAC Network Control register
     */
    temp_net_control = GEM_MAN_PORT_EN | GEM_CLEAR_ALL_STATS_REGS | GEM_PFC_ENABLE;
//    temp_net_control |= GEM_LOOPBACK_LOCAL; /* PMCS uncomment this to force local loop back */
    if(MSS_MAC_ENABLE == cfg->loopback)
    {
        temp_net_control |= GEM_LOOPBACK_LOCAL;
    }

#if defined(MSS_MAC_TIME_STAMPED_MODE)
    temp_net_control |= GEM_PTP_UNICAST_ENA;
#endif
    
    /*
     *  eMAC has to be configured as external TSU although it is actually using
     *  the pMAC TSU. There is only really 1 TSU per GEM instance and all
     *  adjustments etc should really be done via the pMAC.
     */
    if(this_mac->is_emac)
    {
        temp_net_control |= GEM_EXT_TSU_PORT_ENABLE;
    }
    /*--------------------------------------------------------------------------
     * Configure MAC Network Config and Network Control registers
     */
     
#if defined(TARGET_G5_SOC)
    if(TBI == this_mac->interface_type)
    {
        temp_net_config = (uint32_t)((1ul << GEM_DATA_BUS_WIDTH_SHIFT) | ((cfg->phyclk & GEM_MDC_CLOCK_DIVISOR_MASK) << GEM_MDC_CLOCK_DIVISOR_SHIFT) | GEM_PCS_SELECT | GEM_SGMII_MODE_ENABLE);
    }
    else if (GMII_SGMII == this_mac->interface_type)
    {
        /* Actually for the HW emulation platform the interface is GMII... */
        temp_net_config = (uint32_t)((1ul << GEM_DATA_BUS_WIDTH_SHIFT) | ((cfg->phyclk & GEM_MDC_CLOCK_DIVISOR_MASK) << GEM_MDC_CLOCK_DIVISOR_SHIFT));
    }
#endif
#if defined(TARGET_ALOE)
    /* No pause frames received in memory, divide PCLK by 224 for MDC */
    temp_net_config = (cfg->phyclk & GEM_MDC_CLOCK_DIVISOR_MASK) << GEM_MDC_CLOCK_DIVISOR_SHIFT);
#endif

    if((MSS_MAC_ENABLE == cfg->rx_flow_ctrl) || (MSS_MAC_ENABLE == cfg->tx_flow_ctrl))
    {
        temp_net_config |= GEM_FCS_REMOVE | GEM_DISABLE_COPY_OF_PAUSE_FRAMES | GEM_RECEIVE_1536_BYTE_FRAMES | GEM_PAUSE_ENABLE | GEM_FULL_DUPLEX | GEM_GIGABIT_MODE_ENABLE;
    }
    else
    {
        temp_net_config |= GEM_FCS_REMOVE | GEM_DISABLE_COPY_OF_PAUSE_FRAMES | GEM_RECEIVE_1536_BYTE_FRAMES | GEM_FULL_DUPLEX | GEM_GIGABIT_MODE_ENABLE;
    }
    
    if(MSS_MAC_ENABLE == cfg->length_field_check)
    {
        temp_net_config |= GEM_LENGTH_FIELD_ERROR_FRAME_DISCARD;
    }
    
    if(MSS_MAC_IPG_DEFVAL != cfg->ipg_multiplier) /* If we have a non zero value here then enable IPG stretching */
    {
        uint32_t stretch;
        temp_net_config |= GEM_IPG_STRETCH_ENABLE;

        stretch  = cfg->ipg_multiplier & GEM_IPG_STRETCH_MUL_MASK;
        stretch |= (uint32_t)((cfg->ipg_divisor & GEM_IPG_STRETCH_DIV_MASK) << GEM_IPG_STRETCH_DIV_SHIFT);

        if(this_mac->is_emac)
        {
            this_mac->emac_base->STRETCH_RATIO = stretch;
        }
        else
        {
            this_mac->mac_base->STRETCH_RATIO = stretch;
        }
    }

    if(this_mac->is_emac)
    {
        this_mac->emac_base->NETWORK_CONTROL = temp_net_control;
        this_mac->emac_base->NETWORK_CONFIG  = temp_net_config;
        this_mac->mac_base->NETWORK_CONFIG   = temp_net_config;
    }
    else
    {
        this_mac->mac_base->NETWORK_CONTROL  = temp_net_control;
        this_mac->mac_base->NETWORK_CONFIG   = temp_net_config;
#if defined(TARGET_G5_SOC)
        this_mac->emac_base->NETWORK_CONFIG  = temp_net_config;
#endif
    }

    /*--------------------------------------------------------------------------
     * Reset PCS
     */
    if(!this_mac->is_emac)
    {
        this_mac->mac_base->PCS_CONTROL |= 0x8000;
    }

    /*--------------------------------------------------------------------------
     * Configure MAC Network DMA Config register
     */
    if(this_mac->is_emac)
    {
#if defined(MSS_MAC_TIME_STAMPED_MODE)
        this_mac->emac_base->DMA_CONFIG = GEM_DMA_ADDR_BUS_WIDTH_1 | (MSS_MAC_RX_BUF_VALUE << GEM_RX_BUF_SIZE_SHIFT) |
                                         GEM_TX_PBUF_SIZE | (0x3ul << GEM_RX_PBUF_SIZE_SHIFT/* |  16ul*/) |
                                         GEM_TX_BD_EXTENDED_MODE_EN | GEM_RX_BD_EXTENDED_MODE_EN;

        /* Record TS for all packets by default */
        this_mac->emac_base->TX_BD_CONTROL = GEM_BD_TS_MODE;
        this_mac->emac_base->RX_BD_CONTROL = GEM_BD_TS_MODE;
#else
//        this_mac->emac_base->DMA_CONFIG = GEM_DMA_ADDR_BUS_WIDTH_1 | (MSS_MAC_RX_BUF_VALUE << GEM_RX_BUF_SIZE_SHIFT) |
//                                         GEM_TX_PBUF_SIZE | (0x3ul << GEM_RX_PBUF_SIZE_SHIFT | 16ul);
        this_mac->emac_base->DMA_CONFIG = (MSS_MAC_RX_BUF_VALUE << GEM_RX_BUF_SIZE_SHIFT) |
                                         GEM_TX_PBUF_SIZE | (0x3ul << GEM_RX_PBUF_SIZE_SHIFT | 16ul);

#endif
    }
    else
    {
        int queue_index;

#if defined(MSS_MAC_TIME_STAMPED_MODE)
        this_mac->mac_base->DMA_CONFIG = GEM_DMA_ADDR_BUS_WIDTH_1 |  (MSS_MAC_RX_BUF_VALUE << GEM_RX_BUF_SIZE_SHIFT) |
                                         GEM_TX_PBUF_SIZE | (0x3ul << GEM_RX_PBUF_SIZE_SHIFT | 16ul) |
                                         GEM_TX_BD_EXTENDED_MODE_EN | GEM_RX_BD_EXTENDED_MODE_EN;

        /* Record TS for all packets by default */
        this_mac->mac_base->TX_BD_CONTROL = GEM_BD_TS_MODE;
        this_mac->mac_base->RX_BD_CONTROL = GEM_BD_TS_MODE;
#else
        this_mac->mac_base->DMA_CONFIG  = GEM_DMA_ADDR_BUS_WIDTH_1 |  (MSS_MAC_RX_BUF_VALUE << GEM_RX_BUF_SIZE_SHIFT) |
                                         GEM_TX_PBUF_SIZE | (0x3ul << GEM_RX_PBUF_SIZE_SHIFT | 16ul);
#endif
#if (MSS_MAC_QUEUE_COUNT > 1)
        for(queue_index = 1; queue_index != MSS_MAC_QUEUE_COUNT; queue_index++)
        {
            *this_mac->queue[queue_index].dma_rxbuf_size = 0x18;
        }
#endif
    }

    /*
     * Disable the other queues as the GEM reset leaves them enabled with an
     * address pointer of 0 for some unfathomable reason... This screws things
     * up when we enable transmission or reception.
     *
     * Setting b0 of the queue pointer disables a queue.
     */
    if(!this_mac->is_emac)
    {     /* Added these to MAC structure to make them MAC specific... */
        this_mac->mac_base->TRANSMIT_Q1_PTR = ((uint32_t)(uint64_t)this_mac->queue[0].tx_desc_tab) | 1; /* Use address of valid descriptor but set b0 to disable */
        this_mac->mac_base->TRANSMIT_Q2_PTR = ((uint32_t)(uint64_t)this_mac->queue[0].tx_desc_tab) | 1;
        this_mac->mac_base->TRANSMIT_Q3_PTR = ((uint32_t)(uint64_t)this_mac->queue[0].tx_desc_tab) | 1;
        this_mac->mac_base->RECEIVE_Q1_PTR  = ((uint32_t)(uint64_t)this_mac->queue[0].rx_desc_tab) | 1;
        this_mac->mac_base->RECEIVE_Q2_PTR  = ((uint32_t)(uint64_t)this_mac->queue[0].rx_desc_tab) | 1;
        this_mac->mac_base->RECEIVE_Q3_PTR  = ((uint32_t)(uint64_t)this_mac->queue[0].rx_desc_tab) | 1;
    }

    /* Set up maximum initial jumbo frame size - but bounds check first  */
    if(cfg->jumbo_frame_default > MSS_MAC_JUMBO_MAX)
    {
        temp_length = MSS_MAC_JUMBO_MAX;
    }
    else
    {
        temp_length = cfg->jumbo_frame_default;
    }

    if(this_mac->is_emac)
    {
        this_mac->emac_base->JUMBO_MAX_LENGTH = temp_length;
    }
    else
    {
        this_mac->mac_base->JUMBO_MAX_LENGTH = temp_length;
    }

    /*--------------------------------------------------------------------------
     * Disable all ints for now
     */
    if(this_mac->is_emac)
    {
        this_mac->emac_base->INT_DISABLE = 0xFFFFFFFFul; /* Only one queue here... */
    }
    else
    {
        int queue_no;
        for(queue_no = 0; queue_no != MSS_MAC_QUEUE_COUNT; queue_no++)
        {
            *this_mac->queue[queue_no].int_disable  = 0xFFFFFFFFul;
        }
    }

}

/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */
void
MSS_MAC_write_phy_reg
(
    mss_mac_instance_t *this_mac, 
    uint8_t phyaddr,
    uint8_t regaddr,
    uint16_t regval
)
{
    volatile uint32_t phy_op;
    psr_t lev;

    HAL_ASSERT(MSS_MAC_PHYADDR_MAXVAL >= phyaddr);
    HAL_ASSERT(MSS_MAC_PHYREGADDR_MAXVAL >= regaddr);
    /* 
     * Write PHY address in MII Mgmt address register.
     * Makes previous register address 0 & invalid.
     */ 
    if((MSS_MAC_PHYADDR_MAXVAL >= phyaddr) && 
       (MSS_MAC_PHYREGADDR_MAXVAL >= regaddr))
    {
        phy_op = (uint32_t)(GEM_WRITE1 | (GEM_PHY_OP_CL22_WRITE << GEM_OPERATION_SHIFT) | (2ul << GEM_WRITE10_SHIFT) | (uint32_t)regval);
        phy_op |= (uint32_t)(((uint32_t)phyaddr << GEM_PHY_ADDRESS_SHIFT) & GEM_PHY_ADDRESS);
        phy_op |= (uint32_t)(((uint32_t)regaddr << GEM_REGISTER_ADDRESS_SHIFT) & GEM_REGISTER_ADDRESS);

        lev = HAL_disable_interrupts();
        /*
         * Always use the pMAC for this as the eMAC MDIO interface is not
         * connected to the outside world...
         */
        /* Wait for MII Mgmt interface to complete previous operation. */
        do
        {
            volatile int ix;
            ix++;
        } while(0 == (this_mac->mac_base->NETWORK_STATUS & GEM_MAN_DONE));

        this_mac->mac_base->PHY_MANAGEMENT = phy_op;
        HAL_restore_interrupts(lev);
    }
}

/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */
uint16_t
MSS_MAC_read_phy_reg
(
    mss_mac_instance_t *this_mac, 
    uint8_t phyaddr,
    uint8_t regaddr
)
{
    volatile uint32_t phy_op;
    psr_t lev;

    HAL_ASSERT(MSS_MAC_PHYADDR_MAXVAL >= phyaddr);
    HAL_ASSERT(MSS_MAC_PHYREGADDR_MAXVAL >= regaddr);
    /* 
     * Write PHY address in MII Mgmt address register.
     * Makes previous register address 0 & invalid.
     */ 
    if((MSS_MAC_PHYADDR_MAXVAL >= phyaddr) && 
       (MSS_MAC_PHYREGADDR_MAXVAL >= regaddr))
    {
        phy_op = GEM_WRITE1 | (GEM_PHY_OP_CL22_READ << GEM_OPERATION_SHIFT) | (2ul << GEM_WRITE10_SHIFT);
        phy_op |= (uint32_t)(((uint32_t)phyaddr << GEM_PHY_ADDRESS_SHIFT) & GEM_PHY_ADDRESS);
        phy_op |= (uint32_t)(((uint32_t)regaddr << GEM_REGISTER_ADDRESS_SHIFT) & GEM_REGISTER_ADDRESS);
        
        /*
         * Always use the pMAC for this as the eMAC MDIO interface is not
         * connected to the outside world...
         */
        lev = HAL_disable_interrupts();
        /* Wait for MII Mgmt interface to complete previous operation. */
        do
        {
            volatile int ix;
            ix++;
        } while(0 == (this_mac->mac_base->NETWORK_STATUS & GEM_MAN_DONE));

        this_mac->mac_base->PHY_MANAGEMENT = phy_op;

        do
        {
            volatile int ix;
            ix++;
        } while(0 == (this_mac->mac_base->NETWORK_STATUS & GEM_MAN_DONE));

        phy_op = this_mac->mac_base->PHY_MANAGEMENT;
        HAL_restore_interrupts(lev);
    }
    else
    {
        phy_op = 0;
    }

    return((uint16_t)phy_op);
}

/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

#define GEM_REG_OFFSET(x) (volatile uint32_t *)(&(((MAC_TypeDef *)0)->x))

uint32_t 
MSS_MAC_read_stat
(
    mss_mac_instance_t *this_mac, 
    mss_mac_stat_t stat
)
{
    uint32_t stat_val = 0u;

    static volatile uint32_t * const stat_regs_lut[] =
    {
        GEM_REG_OFFSET(OCTETS_TXED_BOTTOM),
        GEM_REG_OFFSET(OCTETS_TXED_TOP),
        GEM_REG_OFFSET(FRAMES_TXED_OK),
        GEM_REG_OFFSET(BROADCAST_TXED),
        GEM_REG_OFFSET(MULTICAST_TXED),
        GEM_REG_OFFSET(PAUSE_FRAMES_TXED),
        GEM_REG_OFFSET(FRAMES_TXED_64),
        GEM_REG_OFFSET(FRAMES_TXED_65),
        GEM_REG_OFFSET(FRAMES_TXED_128),
        GEM_REG_OFFSET(FRAMES_TXED_256),
        GEM_REG_OFFSET(FRAMES_TXED_512),
        GEM_REG_OFFSET(FRAMES_TXED_1024),
        GEM_REG_OFFSET(FRAMES_TXED_1519),
        GEM_REG_OFFSET(TX_UNDERRUNS),
        GEM_REG_OFFSET(SINGLE_COLLISIONS),
        GEM_REG_OFFSET(MULTIPLE_COLLISIONS),
        GEM_REG_OFFSET(EXCESSIVE_COLLISIONS),
        GEM_REG_OFFSET(LATE_COLLISIONS),
        GEM_REG_OFFSET(DEFERRED_FRAMES),
        GEM_REG_OFFSET(CRS_ERRORS),
        GEM_REG_OFFSET(OCTETS_RXED_BOTTOM),
        GEM_REG_OFFSET(OCTETS_RXED_TOP),
        GEM_REG_OFFSET(FRAMES_RXED_OK),
        GEM_REG_OFFSET(BROADCAST_RXED),
        GEM_REG_OFFSET(MULTICAST_RXED),
        GEM_REG_OFFSET(PAUSE_FRAMES_RXED),
        GEM_REG_OFFSET(FRAMES_RXED_64),
        GEM_REG_OFFSET(FRAMES_RXED_65),
        GEM_REG_OFFSET(FRAMES_RXED_128),
        GEM_REG_OFFSET(FRAMES_RXED_256),
        GEM_REG_OFFSET(FRAMES_RXED_512),
        GEM_REG_OFFSET(FRAMES_RXED_1024),
        GEM_REG_OFFSET(FRAMES_RXED_1519),
        GEM_REG_OFFSET(UNDERSIZE_FRAMES),
        GEM_REG_OFFSET(EXCESSIVE_RX_LENGTH),
        GEM_REG_OFFSET(RX_JABBERS),
        GEM_REG_OFFSET(FCS_ERRORS),
        GEM_REG_OFFSET(RX_LENGTH_ERRORS),
        GEM_REG_OFFSET(RX_SYMBOL_ERRORS),
        GEM_REG_OFFSET(ALIGNMENT_ERRORS),
        GEM_REG_OFFSET(RX_RESOURCE_ERRORS),
        GEM_REG_OFFSET(RX_OVERRUNS),
        GEM_REG_OFFSET(RX_IP_CK_ERRORS),
        GEM_REG_OFFSET(RX_TCP_CK_ERRORS),
        GEM_REG_OFFSET(RX_UDP_CK_ERRORS),
        GEM_REG_OFFSET(AUTO_FLUSHED_PKTS)
    };
    
    ASSERT(MSS_MAC_LAST_STAT > stat);
    
    if(MSS_MAC_LAST_STAT > stat)
    {
        if(this_mac->is_emac)
        {
            /* Divide by 4 as offset is in bytes but pointer is 4 bytes */
            stat_val = *(((uint32_t *)this_mac->emac_base) + ((uint64_t)stat_regs_lut[stat] / 4));
        }
        else
        {
            /* Divide by 4 as offset is in bytes but pointer is 4 bytes */
            stat_val = *(((uint32_t *)this_mac->mac_base) + ((uint64_t)stat_regs_lut[stat] / 4));
        }
    }

    return stat_val;
}


/***************************************************************************//**
 See mss_ethernet_mac.h for details of how to use this function
*/
void MSS_MAC_clear_statistics
(
    mss_mac_instance_t *this_mac
)
{
    if(this_mac->is_emac)
    {
        this_mac->emac_base->NETWORK_CONTROL |= GEM_CLEAR_ALL_STATS_REGS;
    }
    else
    {
        this_mac->mac_base->NETWORK_CONTROL |= GEM_CLEAR_ALL_STATS_REGS;
    }
}

/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */
uint8_t
MSS_MAC_receive_pkt
(
    mss_mac_instance_t *this_mac,
    uint32_t queue_no,
    uint8_t * rx_pkt_buffer,
    void * p_user_data,
    mss_mac_rx_int_ctrl_t enable
)
{
    uint8_t status = MSS_MAC_FAILED;
    /* Make this function atomic w.r.to EMAC interrupt */

    /* PLIC_DisableIRQ() Et al should not be called from the associated interrupt... */
    if(0 == this_mac->queue[queue_no].in_isr)
    {
        PLIC_DisableIRQ(this_mac->mac_q_int[queue_no]); /* Single interrupt from GEM? */
    }

    HAL_ASSERT(NULL_POINTER != rx_pkt_buffer);
    HAL_ASSERT(IS_WORD_ALIGNED(rx_pkt_buffer));
    
    if(this_mac->queue[queue_no].nb_available_rx_desc > 0)
    {
        uint32_t next_rx_desc_index;
        
        if(MSS_MAC_INT_DISABLE == enable)
        {
            /*
             * When setting up the chain of buffers, we don't want the DMA
             * engine active so shut down reception.
             */
            if(this_mac->is_emac)
            {
                this_mac->emac_base->NETWORK_CONTROL &= (uint32_t)(~GEM_ENABLE_RECEIVE);
            }
            else
            {
                this_mac->mac_base->NETWORK_CONTROL &= (uint32_t)(~GEM_ENABLE_RECEIVE);
            }
        }

        --this_mac->queue[queue_no].nb_available_rx_desc;
        next_rx_desc_index = this_mac->queue[queue_no].next_free_rx_desc_index;
        
        if((MSS_MAC_RX_RING_SIZE - 1) == next_rx_desc_index)
        {
            this_mac->queue[queue_no].rx_desc_tab[next_rx_desc_index].addr_low = (uint32_t)((uint64_t)rx_pkt_buffer | 2ul);  /* Set wrap bit */
        }
        else
        {
            this_mac->queue[queue_no].rx_desc_tab[next_rx_desc_index].addr_low = (uint32_t)((uint64_t)rx_pkt_buffer);
        }
           
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
        this_mac->queue[queue_no].rx_desc_tab[next_rx_desc_index].addr_high = (uint32_t)((uint64_t)rx_pkt_buffer >> 32);
#endif
        this_mac->queue[queue_no].rx_caller_info[next_rx_desc_index] = p_user_data;

        /* 
           If the RX is found disabled, then it might be because this is the
           first time a packet is scheduled for reception or the RX ENABLE is 
           made zero by RX overflow or RX bus error. In either case, this
           function tries to schedule the current packet for reception.

           Don't bother if we are not enabling interrupt at the end of all this...
        */
        if(MSS_MAC_INT_ARM == enable)
        {
            /*
             * Starting receive operations off with a chain of buffers set up.
             */
            
            if(this_mac->is_emac)
            {
                this_mac->emac_base->NETWORK_CONTROL &= (uint32_t)(~GEM_ENABLE_RECEIVE);
                this_mac->emac_base->RECEIVE_Q_PTR = (uint32_t)((uint64_t)this_mac->queue[queue_no].rx_desc_tab);
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
                this_mac->emac_base->UPPER_RX_Q_BASE_ADDR = (uint32_t)((uint64_t)this_mac->queue[queue_no].rx_desc_tab >> 32);
#endif
                this_mac->emac_base->NETWORK_CONTROL |= GEM_ENABLE_RECEIVE;
            }
            else
            {
                this_mac->mac_base->NETWORK_CONTROL &= (uint32_t)(~GEM_ENABLE_RECEIVE);
                *this_mac->queue[queue_no].receive_q_ptr = (uint32_t)((uint64_t)this_mac->queue[queue_no].rx_desc_tab);
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
                this_mac->mac_base->UPPER_RX_Q_BASE_ADDR = (uint32_t)((uint64_t)this_mac->queue[queue_no].rx_desc_tab >> 32);
#endif
                this_mac->mac_base->NETWORK_CONTROL |= GEM_ENABLE_RECEIVE;
            }
        }
        else if(MSS_MAC_INT_DISABLE != enable)
        {
            uint32_t temp_cr;
            
            if(this_mac->is_emac)
            {
                temp_cr = this_mac->emac_base->NETWORK_CONFIG;
            }
            else
            {
                temp_cr = this_mac->mac_base->NETWORK_CONFIG;
            }
            if(0 == (temp_cr & GEM_ENABLE_RECEIVE))
            {
                /* RX disabled so restart it... */
                if(this_mac->is_emac)
                {
                    this_mac->emac_base->NETWORK_CONTROL |= GEM_ENABLE_RECEIVE;
                }
                else
                {
                    this_mac->mac_base->NETWORK_CONTROL |= GEM_ENABLE_RECEIVE;
                }
            }
        }

        /* Point the next_rx_desc to next free descriptor in the ring */
        /* Wrap around in case next descriptor is pointing to last in the ring */
        ++this_mac->queue[queue_no].next_free_rx_desc_index;
        this_mac->queue[queue_no].next_free_rx_desc_index %= MSS_MAC_RX_RING_SIZE;
        
    }

    /*
     * Only call Ethernet Interrupt Enable function if the user says so.
     * See note above for disable...
     */
    if((MSS_MAC_INT_ARM == enable) && (0 == this_mac->queue[queue_no].in_isr))
    {
        PLIC_EnableIRQ(this_mac->mac_q_int[queue_no]); /* Single interrupt from GEM? */
    }

    return status;
}

/***************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */
uint8_t 
MSS_MAC_send_pkt
(
    mss_mac_instance_t *this_mac,
    uint32_t queue_no,
    uint8_t const * tx_buffer,
    uint32_t tx_length,
    void * p_user_data
)
{
    /*
     * Simplified transmit operation which depends on the following assumptions:
     * 
     * 1. The TX DMA buffer size is big enough to contain a full packet.
     * 2. We will only transmit one packet at a time.
     *
     * We do transmission by using two buffer descriptors. The first contains the
     * packet to transmit and the second is a dummy one with the USED bit set. This
     * halts transmission once the first packet is transmitted. We always reset the
     * TX DMA to point to the first packet when we send a packet so we don't have
     * to juggle buffer positions or worry about wrap.
     */
     
    uint8_t status = MSS_MAC_FAILED;
    /* Hack for system testing. If b31 of the tx_length is 1 then we want to
     * send without a CRC appended. This is used for loopback testing where we
     * can get the GEM to receive packets with CRC appended and send them
     * straight back.
     * */
    int no_crc = 0;

    /* Is config option for disabling CRC set? */
    if(MSS_MAC_CRC_DISABLE == this_mac->append_CRC)
    {
        no_crc = 1;
    }

    /* Or has the user requested just this packet? */
    if(tx_length & 0x80000000UL)
    {
        no_crc = 1;
    }

    tx_length &= 0x7FFFFFFFUL; /* Make sure high bit is now clear */

    /* Make this function atomic w.r.to EMAC interrupt */
    /* PLIC_DisableIRQ() et al should not be called from the associated interrupt... */
    if(0 == this_mac->queue[queue_no].in_isr)
    {
        PLIC_DisableIRQ(this_mac->mac_q_int[queue_no]); /* Single interrupt from GEM? */
    }

    HAL_ASSERT(NULL_POINTER != tx_buffer);
    HAL_ASSERT(0 != tx_length);
    HAL_ASSERT(IS_WORD_ALIGNED(tx_buffer));
    
#if defined(MSS_MAC_SIMPLE_TX_QUEUE)
    if(this_mac->queue[queue_no].nb_available_tx_desc == MSS_MAC_TX_RING_SIZE)
    {
        this_mac->queue[queue_no].nb_available_tx_desc = 0;
        this_mac->queue[queue_no].tx_desc_tab[0].addr_low = (uint32_t)((uint64_t)tx_buffer);
        this_mac->queue[queue_no].tx_desc_tab[0].status = (uint32_t)((tx_length & GEM_TX_DMA_BUFF_LEN) | GEM_TX_DMA_LAST); /* Mark as last buffer for frame */
        if(no_crc)
        {
            this_mac->queue[queue_no].tx_desc_tab[0].status |= GEM_TX_DMA_NO_CRC;
        }
//      this_mac->tx_desc_tab[0].status = (tx_length & GEM_TX_DMA_BUFF_LEN) | GEM_TX_DMA_LAST | GEM_TX_DMA_USED; /* PMCS deliberate error ! */
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
        this_mac->queue[queue_no].tx_desc_tab[0].addr_high = (uint32_t)((uint64_t)tx_buffer >> 32);
        this_mac->queue[queue_no].tx_desc_tab[0].unused    = 0;
#endif
        this_mac->queue[queue_no].tx_caller_info[0] = p_user_data;

        if(this_mac->is_emac)
        {
            this_mac->emac_base->NETWORK_CONTROL |= GEM_ENABLE_TRANSMIT | GEM_TRANSMIT_HALT;
            this_mac->emac_base->TRANSMIT_Q_PTR   = (uint32_t)((uint64_t)this_mac->queue[queue_no].tx_desc_tab);
            this_mac->emac_base->NETWORK_CONTROL |= GEM_TRANSMIT_START;
        }
        else
        {
            this_mac->mac_base->NETWORK_CONTROL      |= GEM_ENABLE_TRANSMIT | GEM_TRANSMIT_HALT;
            *this_mac->queue[queue_no].transmit_q_ptr  = (uint32_t)((uint64_t)this_mac->queue[queue_no].tx_desc_tab);
            this_mac->mac_base->NETWORK_CONTROL      |= GEM_TRANSMIT_START;
        }

        this_mac->queue[queue_no].egress += tx_length;
        status = MSS_MAC_SUCCESS;
    }
#else
    /* TBD PMCS need to implement multi packet queuing... */
#warning "Nothing implemented for multi packet tx yet"
#endif
    /* Ethernet Interrupt Enable function. */
    /* PLIC_DisableIRQ() et al should not be called from the associated interrupt... */
    if(0 == this_mac->queue[queue_no].in_isr)
    {
        PLIC_EnableIRQ(this_mac->mac_q_int[queue_no]); /* Single interrupt from GEM? */
    }
    return status;
}

/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */
#if defined(USING_FREERTOS)
extern UBaseType_t uxCriticalNesting;
#endif
#if defined(TARGET_ALOE)
uint8_t  MAC0_plic_53_IRQHandler(void);
uint8_t  MAC0_plic_53_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac0, 0);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac0, 0);
#endif
    return(EXT_IRQ_KEEP_ENABLED);
}
#else
uint8_t mac0_int_plic_IRQHandler(void);
uint8_t mac0_queue1_plic_IRQHandler(void);
uint8_t mac0_queue2_plic_IRQHandler(void);
uint8_t mac0_queue3_plic_IRQHandler(void);
uint8_t mac0_emac_plic_IRQHandler(void);
uint8_t mac0_mmsl_plic_IRQHandler(void);
uint8_t mac1_int_plic_IRQHandler(void);
uint8_t mac1_queue1_plic_IRQHandler(void);
uint8_t mac1_queue2_plic_IRQHandler(void);
uint8_t mac1_queue3_plic_IRQHandler(void);
uint8_t mac1_emac_plic_IRQHandler(void);
uint8_t mac1_mmsl_plic_IRQHandler(void);

uint8_t mac0_int_plic_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac0, 0);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac0, 0);
#endif
    return(EXT_IRQ_KEEP_ENABLED);
}

uint8_t mac0_queue1_plic_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac0, 1);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac0, 1);
#endif
    return(EXT_IRQ_KEEP_ENABLED);
}

uint8_t mac0_queue2_plic_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac0, 2);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac0, 2);
#endif
    return(EXT_IRQ_KEEP_ENABLED);
}

uint8_t mac0_queue3_plic_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac0, 3);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac0, 3);
#endif
    return(EXT_IRQ_KEEP_ENABLED);
}

uint8_t mac0_emac_plic_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_emac0, 0);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_emac0, 0);
#endif
    return(EXT_IRQ_KEEP_ENABLED);
}

uint8_t mac0_mmsl_plic_IRQHandler(void)
{
    return(EXT_IRQ_KEEP_ENABLED);
}

uint8_t mac1_int_plic_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac1, 0);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac1, 0);
#endif
    return(EXT_IRQ_KEEP_ENABLED);
}

uint8_t mac1_queue1_plic_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac1, 1);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac1, 1);
#endif
    return(EXT_IRQ_KEEP_ENABLED);
}

uint8_t mac1_queue2_plic_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac1, 2);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac1, 2);
#endif
    return(EXT_IRQ_KEEP_ENABLED);
}

uint8_t mac1_queue3_plic_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac1, 3);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac1, 3);
#endif
    return(EXT_IRQ_KEEP_ENABLED);
}

uint8_t mac1_emac_plic_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_emac1, 0);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_emac1, 0);
#endif
    return(EXT_IRQ_KEEP_ENABLED);
}

uint8_t mac1_mmsl_plic_IRQHandler(void)
{
    return(EXT_IRQ_KEEP_ENABLED);
}

#endif
/* Define the following if your GEM is configured to clear on read for int flags
 * In this case you should not write to the int status reg ... */
/* #define GEM_FLAGS_CLR_ON_RD */

static void generic_mac_irq_handler(mss_mac_instance_t *this_mac, uint32_t queue_no)
{
    volatile uint32_t int_pending;  /* We read and hold a working copy as many
                                     * of the bits can be clear on read if
                                     * the GEM is configured that way... */
    volatile uint32_t *rx_status;   /* Address of receive status register */
    volatile uint32_t *tx_status;   /* Address of transmit status register */
    volatile uint32_t *int_status;  /* Address of interrupt status register */
    
    this_mac->queue[queue_no].in_isr = 1;
    int_status  = this_mac->queue[queue_no].int_status;
    int_pending =  *int_status & ~(*this_mac->queue[queue_no].int_mask);

    if(this_mac->is_emac)
    {
        rx_status   = &this_mac->emac_base->RECEIVE_STATUS;
        tx_status   = &this_mac->emac_base->TRANSMIT_STATUS;
    }
    else
    {
        rx_status   = &this_mac->mac_base->RECEIVE_STATUS;
        tx_status   = &this_mac->mac_base->TRANSMIT_STATUS;
    }
    
    /*
     * Note, in the following code we generally clear any flags first and then
     * handle the condition as this allows any new events which occur in the
     * course of the ISR to be picked up later.
     */

    /* Packet received interrupt - first in line as most time critical */
    if((int_pending & GEM_RECEIVE_COMPLETE) != 0u)
    {
       *rx_status = GEM_FRAME_RECEIVED;
#if !defined(GEM_FLAGS_CLR_ON_RD)
#if defined(TARGET_ALOE)
        *int_status = 3; /* PMCS: Should be 2 but that does not work on Aloe... */
#else
        *int_status = 2;
#endif
        rxpkt_handler(this_mac, queue_no);
        this_mac->queue[queue_no].overflow_counter = 0; /* Reset counter as we have received something */
#endif
    }

    if((int_pending & GEM_RECEIVE_OVERRUN_INT) != 0u)
    {
        *rx_status = GEM_RECEIVE_OVERRUN;
#if !defined(GEM_FLAGS_CLR_ON_RD)
        *int_status = GEM_RECEIVE_OVERRUN_INT;
#endif
        rxpkt_handler(this_mac, queue_no);
        this_mac->queue[queue_no].overflow_counter++;
        this_mac->queue[queue_no].rx_overflow++;
    }

    if((int_pending & GEM_RX_USED_BIT_READ) != 0u)
    {
        *rx_status = GEM_BUFFER_NOT_AVAILABLE;
#if !defined(GEM_FLAGS_CLR_ON_RD)
        *int_status = GEM_RX_USED_BIT_READ;
#endif
        rxpkt_handler(this_mac, queue_no);
        this_mac->queue[queue_no].rx_overflow++;
        this_mac->queue[queue_no].overflow_counter++;
    }

    if((int_pending & GEM_RESP_NOT_OK_INT) != 0u) /* Hope this is transient and restart rx... */
    {
        *rx_status = GEM_RX_RESP_NOT_OK;
#if !defined(GEM_FLAGS_CLR_ON_RD)
        *int_status = GEM_RESP_NOT_OK_INT;
#endif
        rxpkt_handler(this_mac, queue_no);
        if(this_mac->is_emac)
        {
            this_mac->emac_base->NETWORK_CONTROL |= GEM_ENABLE_RECEIVE;
        }
        else
        {
            this_mac->mac_base->NETWORK_CONTROL |= GEM_ENABLE_RECEIVE;
        }

        this_mac->queue[queue_no].hresp_error++;
    }

    /* Transmit packet sent interrupt */
    if((int_pending & GEM_TRANSMIT_COMPLETE) != 0u)
    {
        if((*tx_status & GEM_STAT_TRANSMIT_COMPLETE) != 0) /* If loopback test or other hasn't taken care of this... */
        {
            *tx_status  = GEM_STAT_TRANSMIT_COMPLETE;
#if !defined(GEM_FLAGS_CLR_ON_RD)
            *int_status = GEM_TRANSMIT_COMPLETE;
#endif
            txpkt_handler(this_mac, queue_no);
        }
    }

    if(this_mac->queue[queue_no].overflow_counter > 4) /* looks like we are stuck in a rut here... */
    {
        /* Restart receive operation from scratch */
        this_mac->queue[queue_no].overflow_counter = 0;
        this_mac->queue[queue_no].rx_restart++;

        if(this_mac->is_emac)
        {
            this_mac->emac_base->NETWORK_CONTROL &= (uint32_t)(~GEM_ENABLE_RECEIVE);
        }
        else
        {
            this_mac->mac_base->NETWORK_CONTROL &= (uint32_t)(~GEM_ENABLE_RECEIVE);
        }

        this_mac->queue[queue_no].nb_available_rx_desc;
        this_mac->queue[queue_no].next_free_rx_desc_index;
        this_mac->queue[queue_no].first_rx_desc_index;

        if(this_mac->is_emac)
        {
            this_mac->emac_base->RECEIVE_Q_PTR = (uint32_t)((uint64_t)this_mac->queue[queue_no].rx_desc_tab);
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
            this_mac->emac_base->UPPER_RX_Q_BASE_ADDR = (uint32_t)((uint64_t)this_mac->queue[queue_no].rx_desc_tab >> 32);
#endif
            this_mac->emac_base->NETWORK_CONTROL |= GEM_ENABLE_RECEIVE;
        }
        else
        {
            this_mac->mac_base->RECEIVE_Q_PTR = (uint32_t)((uint64_t)this_mac->queue[queue_no].rx_desc_tab);
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
            this_mac->mac_base->UPPER_RX_Q_BASE_ADDR = (uint32_t)((uint64_t)this_mac->queue[queue_no].rx_desc_tab >> 32);
#endif
            this_mac->mac_base->NETWORK_CONTROL |= GEM_ENABLE_RECEIVE;
        }
    }

    if((int_pending & GEM_PAUSE_FRAME_TRANSMITTED) != 0u)
    {
        *int_status = GEM_PAUSE_FRAME_TRANSMITTED;
        this_mac->tx_pause++;
    }
    if((int_pending & GEM_PAUSE_TIME_ELAPSED) != 0u)
    {
        *int_status = GEM_PAUSE_TIME_ELAPSED;
        this_mac->pause_elapsed++;
    }
    if((int_pending & GEM_PAUSE_FRAME_WITH_NON_0_PAUSE_QUANTUM_RX) != 0u)
    {
        *int_status = GEM_PAUSE_FRAME_WITH_NON_0_PAUSE_QUANTUM_RX;
        this_mac->rx_pause++;
    }

    /* Mask off checked ints and see if any left pending */
    int_pending &= (uint32_t)(~(GEM_RECEIVE_OVERRUN_INT | GEM_RX_USED_BIT_READ | GEM_RECEIVE_COMPLETE |
                     GEM_TRANSMIT_COMPLETE | GEM_RESP_NOT_OK_INT | GEM_PAUSE_FRAME_TRANSMITTED |
                     GEM_PAUSE_TIME_ELAPSED | GEM_PAUSE_FRAME_WITH_NON_0_PAUSE_QUANTUM_RX));
    if(int_pending)
    {
        if((int_pending & GEM_AMBA_ERROR) != 0u)
        {
            *int_status = 0x60; /* Should be 0x40 but that doesn't clear it... */
        }
        else
        {
            volatile int index;
            while(1)
            {
                index++;
            }

            HAL_ASSERT(0); /* Need to think about how to deal with this... */
        }
    }

    this_mac->queue[queue_no].in_isr = 0;
}

/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */
void MSS_MAC_set_tx_callback
(
    mss_mac_instance_t *this_mac,
    uint32_t queue_no,
    mss_mac_transmit_callback_t tx_complete_handler
)
{
    this_mac->queue[queue_no].tx_complete_handler = tx_complete_handler;
}

/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */
void MSS_MAC_set_rx_callback
(
    mss_mac_instance_t *this_mac,
    uint32_t queue_no,
    mss_mac_receive_callback_t rx_callback
)
{
    this_mac->queue[queue_no].pckt_rx_callback = rx_callback;
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_init_TSU(mss_mac_instance_t *this_mac, mss_mac_tsu_config_t *tsu_cfg)
{
    uint32_t temp;

    temp  = (tsu_cfg->sub_ns_inc & 0xFF) << 24;
    temp |= (tsu_cfg->sub_ns_inc >> 8) & 0xFFFF;

    if(this_mac->is_emac)
    {
#if 0 /* Shouldn't really allow setting of tsu through eMAC as it is slaved to pMAC TSU */
        this_mac->emac_base->TSU_TIMER_INCR_SUB_NSEC = temp;
        this_mac->emac_base->TSU_TIMER_INCR          = tsu_cfg->ns_inc;

        /* PMCS: I'm not 100% sure about the sequencing here... */

        this_mac->emac_base->TSU_TIMER_MSB_SEC       = tsu_cfg->secs_msb;
        this_mac->emac_base->TSU_TIMER_SEC           = tsu_cfg->secs_lsb;
        this_mac->emac_base->TSU_TIMER_NSEC          = tsu_cfg->nanoseconds;
#endif
    }
    else
    {
        this_mac->mac_base->TSU_TIMER_INCR_SUB_NSEC = temp;
        this_mac->mac_base->TSU_TIMER_INCR          = tsu_cfg->ns_inc;
        this_mac->mac_base->TSU_TIMER_MSB_SEC       = tsu_cfg->secs_msb;
        this_mac->mac_base->TSU_TIMER_SEC           = tsu_cfg->secs_lsb;
        this_mac->mac_base->TSU_TIMER_NSEC          = tsu_cfg->nanoseconds;
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_read_TSU(mss_mac_instance_t *this_mac, mss_mac_tsu_time_t *tsu_time)
{
    int got_time = 0;

    do
    {
        if(this_mac->is_emac)
        {
            tsu_time->secs_lsb    = this_mac->emac_base->TSU_TIMER_SEC;
            tsu_time->secs_msb    = this_mac->emac_base->TSU_TIMER_MSB_SEC;
            tsu_time->nanoseconds = this_mac->emac_base->TSU_TIMER_NSEC;

            /* Check for nanoseconds roll over and exit loop if none otherwise do again */
            if(tsu_time->secs_lsb == this_mac->emac_base->TSU_TIMER_SEC)
            {
                got_time = 1;
            }
        }
        else
        {
            tsu_time->secs_lsb    = this_mac->mac_base->TSU_TIMER_SEC;
            tsu_time->secs_msb    = this_mac->mac_base->TSU_TIMER_MSB_SEC;
            tsu_time->nanoseconds = this_mac->mac_base->TSU_TIMER_NSEC;

            /* Check for nanoseconds roll over and exit loop if none otherwise do again */
            if(tsu_time->secs_lsb == this_mac->mac_base->TSU_TIMER_SEC)
            {
                got_time = 1;
            }
        }
    } while(!got_time);
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_TSU_rx_mode(mss_mac_instance_t *this_mac, mss_mac_tsu_mode_t tsu_mode)
{
    if(this_mac->is_emac)
    {
        this_mac->emac_base->RX_BD_CONTROL = ((uint32_t)tsu_mode << GEM_BD_TS_MODE_SHIFT) & GEM_BD_TS_MODE;
    }
    else
    {
        this_mac->mac_base->RX_BD_CONTROL = ((uint32_t)tsu_mode << GEM_BD_TS_MODE_SHIFT) & GEM_BD_TS_MODE;
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_TSU_tx_mode(mss_mac_instance_t *this_mac, mss_mac_tsu_mode_t tsu_mode)
{
    if(this_mac->is_emac)
    {
        this_mac->emac_base->TX_BD_CONTROL = ((uint32_t)tsu_mode << GEM_BD_TS_MODE_SHIFT) & GEM_BD_TS_MODE;
    }
    else
    {
        this_mac->mac_base->TX_BD_CONTROL = ((uint32_t)tsu_mode << GEM_BD_TS_MODE_SHIFT) & GEM_BD_TS_MODE;
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

mss_mac_tsu_mode_t MSS_MAC_get_TSU_rx_mode(mss_mac_instance_t *this_mac)
{
    mss_mac_tsu_mode_t ret_val;

    if(this_mac->is_emac)
    {
        ret_val = (mss_mac_tsu_mode_t)((this_mac->emac_base->RX_BD_CONTROL & GEM_BD_TS_MODE) >> GEM_BD_TS_MODE_SHIFT);
    }
    else
    {
        ret_val = (mss_mac_tsu_mode_t)((this_mac->mac_base->RX_BD_CONTROL & GEM_BD_TS_MODE) >> GEM_BD_TS_MODE_SHIFT);
    }

    return(ret_val);
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

mss_mac_tsu_mode_t MSS_MAC_get_TSU_tx_mode(mss_mac_instance_t *this_mac)
{
    mss_mac_tsu_mode_t ret_val;

    if(this_mac->is_emac)
    {
        ret_val = (mss_mac_tsu_mode_t)((this_mac->emac_base->TX_BD_CONTROL & GEM_BD_TS_MODE) >> GEM_BD_TS_MODE_SHIFT);
    }
    else
    {
        ret_val = (mss_mac_tsu_mode_t)((this_mac->mac_base->TX_BD_CONTROL & GEM_BD_TS_MODE) >> GEM_BD_TS_MODE_SHIFT);
    }

    return(ret_val);
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_TSU_oss_mode(mss_mac_instance_t *this_mac, mss_mac_oss_mode_t oss_mode)
{
    volatile uint32_t temp_control;

    if(this_mac->is_emac)
    {
        temp_control = this_mac->emac_base->NETWORK_CONTROL;
    }
    else
    {
        temp_control = this_mac->mac_base->NETWORK_CONTROL;
    }

    /*
     * Note. The docs don't say these are mutually exclusive but I don't think
     * it makes sense to allow both modes at once...
     */
    if(MSS_MAC_OSS_MODE_DISABLED == oss_mode)
    {
        temp_control &= (uint32_t)(~(GEM_OSS_CORRECTION_FIELD | GEM_ONE_STEP_SYNC_MODE));
    }
    else if(MSS_MAC_OSS_MODE_REPLACE == oss_mode)
    {
        temp_control &= (uint32_t)(~GEM_OSS_CORRECTION_FIELD);
        temp_control |= GEM_ONE_STEP_SYNC_MODE;

    }
    else if(MSS_MAC_OSS_MODE_ADJUST == oss_mode)
    {
        temp_control |= GEM_OSS_CORRECTION_FIELD;
        temp_control &= (uint32_t)(~GEM_ONE_STEP_SYNC_MODE);
    }

    if(this_mac->is_emac)
    {
        this_mac->emac_base->NETWORK_CONTROL = temp_control;
    }
    else
    {
        this_mac->mac_base->NETWORK_CONTROL = temp_control;
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

mss_mac_oss_mode_t MSS_MAC_get_TSU_oss_mode(mss_mac_instance_t *this_mac)
{
    mss_mac_oss_mode_t ret_val;
    volatile uint32_t temp_control;

    if(this_mac->is_emac)
    {
        temp_control = this_mac->emac_base->NETWORK_CONTROL;
    }
    else
    {
        temp_control = this_mac->mac_base->NETWORK_CONTROL;
    }

    /*
     * Note. The docs don't say these are mutually exclusive but I don't think
     * it makes sense to allow both modes at once so report this as invalid...
     */
    if((GEM_OSS_CORRECTION_FIELD | GEM_ONE_STEP_SYNC_MODE) == (temp_control & (GEM_OSS_CORRECTION_FIELD | GEM_ONE_STEP_SYNC_MODE)))
    {
        ret_val = MSS_MAC_OSS_MODE_INVALID;
    }
    else if(GEM_OSS_CORRECTION_FIELD == (temp_control & GEM_OSS_CORRECTION_FIELD))
    {
        ret_val = MSS_MAC_OSS_MODE_ADJUST;
    }
    else if(GEM_ONE_STEP_SYNC_MODE == (temp_control & GEM_ONE_STEP_SYNC_MODE))
    {
        ret_val = MSS_MAC_OSS_MODE_REPLACE;
    }
    else
    {
        ret_val = MSS_MAC_OSS_MODE_DISABLED;
    }

    return(ret_val);
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_TSU_unicast_addr(mss_mac_instance_t *this_mac, mss_mac_tsu_addr_t select, uint32_t ip_address)
{
    if(this_mac->is_emac)
    {
        if(MSS_MAC_TSU_UNICAST_RX == select)
        {
            this_mac->emac_base->RX_PTP_UNICAST = ip_address;
        }
        else
        {
            this_mac->emac_base->TX_PTP_UNICAST = ip_address;
        }
    }
    else
    {
        if(MSS_MAC_TSU_UNICAST_RX == select)
        {
            this_mac->mac_base->RX_PTP_UNICAST = ip_address;
        }
        else
        {
            this_mac->mac_base->TX_PTP_UNICAST = ip_address;
        }
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

uint32_t MSS_MAC_get_TSU_unicast_addr(mss_mac_instance_t *this_mac, mss_mac_tsu_addr_t select)
{
    uint32_t ret_val;

    if(this_mac->is_emac)
    {
        if(MSS_MAC_TSU_UNICAST_RX == select)
        {
            ret_val = this_mac->emac_base->RX_PTP_UNICAST;
        }
        else
        {
            ret_val = this_mac->emac_base->TX_PTP_UNICAST;
        }
    }
    else
    {
        if(MSS_MAC_TSU_UNICAST_RX == select)
        {
            ret_val = this_mac->mac_base->RX_PTP_UNICAST;
        }
        else
        {
            ret_val = this_mac->mac_base->TX_PTP_UNICAST;
        }
    }

    return(ret_val);
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_VLAN_only_mode(mss_mac_instance_t *this_mac, bool enable)
{
    if(this_mac->is_emac)
    {
        if(false == enable)
        {
            this_mac->emac_base->NETWORK_CONFIG &= (uint32_t)(~GEM_DISCARD_NON_VLAN_FRAMES);
        }
        else
        {
            this_mac->emac_base->NETWORK_CONFIG |= GEM_DISCARD_NON_VLAN_FRAMES;
        }
    }
    else
    {
        if(false == enable)
        {
            this_mac->mac_base->NETWORK_CONFIG &= (uint32_t)(~GEM_DISCARD_NON_VLAN_FRAMES);
        }
        else
        {
            this_mac->mac_base->NETWORK_CONFIG |= GEM_DISCARD_NON_VLAN_FRAMES;
        }
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

bool MSS_MAC_get_VLAN_only_mode(mss_mac_instance_t *this_mac)
{
    bool ret_val = false;

    if(this_mac->is_emac)
    {
        ret_val = 0 != (this_mac->emac_base->NETWORK_CONFIG & GEM_DISCARD_NON_VLAN_FRAMES);
    }
    else
    {
        ret_val = 0 != (this_mac->mac_base->NETWORK_CONFIG & GEM_DISCARD_NON_VLAN_FRAMES);
    }

    return(ret_val);
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_stacked_VLAN(mss_mac_instance_t *this_mac, uint16_t tag)
{
    if(this_mac->is_emac)
    {
        if(GEM_VLAN_ETHERTYPE_MIN > tag)
        {
            this_mac->emac_base->STACKED_VLAN = 0;
        }
        else
        {
            this_mac->emac_base->STACKED_VLAN = (uint32_t)((uint32_t)tag | GEM_ENABLE_PROCESSING);
        }
    }
    else
    {
        if(GEM_VLAN_ETHERTYPE_MIN > tag)
        {
            this_mac->mac_base->STACKED_VLAN = 0;
        }
        else
        {
            this_mac->mac_base->STACKED_VLAN = (uint32_t)((uint32_t)tag | GEM_ENABLE_PROCESSING);
        }
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

uint16_t MSS_MAC_get_stacked_VLAN(mss_mac_instance_t *this_mac)
{
    uint16_t ret_val = GEM_VLAN_NO_STACK; /* Return 0 if stacked VLANs not enabled */

    if(this_mac->is_emac)
    {
        if(this_mac->emac_base->STACKED_VLAN & GEM_ENABLE_PROCESSING)
        {
            ret_val = (uint16_t)this_mac->emac_base->STACKED_VLAN;
        }
    }
    else
    {
        if(this_mac->mac_base->STACKED_VLAN & GEM_ENABLE_PROCESSING)
        {
            ret_val = (uint16_t)this_mac->mac_base->STACKED_VLAN;
        }
    }

    return(ret_val);
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_hash(mss_mac_instance_t *this_mac, uint64_t hash)
{
    if(this_mac->is_emac)
    {
        if(0ll == hash) /* Short cut for disabling */
        {
            this_mac->emac_base->NETWORK_CONFIG &= (uint32_t)(~(GEM_UNICAST_HASH_ENABLE | GEM_MULTICAST_HASH_ENABLE));
        }

        this_mac->emac_base->HASH_BOTTOM = (uint32_t)hash;
        hash >>= 32;
        this_mac->emac_base->HASH_TOP = (uint32_t)hash;
    }
    else
    {
        if(0ll == hash) /* Short cut for disabling */
        {
            this_mac->mac_base->NETWORK_CONFIG &= (uint32_t)(~(GEM_UNICAST_HASH_ENABLE | GEM_MULTICAST_HASH_ENABLE));
        }

        this_mac->mac_base->HASH_BOTTOM = (uint32_t)hash;
        hash >>= 32;
        this_mac->mac_base->HASH_TOP = (uint32_t)hash;
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

uint64_t MSS_MAC_get_hash(mss_mac_instance_t *this_mac)
{
    uint64_t ret_val = 0;

    if(this_mac->is_emac)
    {
        ret_val   = (uint64_t)this_mac->emac_base->HASH_TOP;
        ret_val <<= 32;
        ret_val  |= (uint64_t)this_mac->emac_base->HASH_BOTTOM;
    }
    else
    {
        ret_val   = (uint64_t)this_mac->mac_base->HASH_TOP;
        ret_val <<= 32;
        ret_val  |= (uint64_t)this_mac->mac_base->HASH_BOTTOM;
    }
    return(ret_val);
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_hash_mode(mss_mac_instance_t *this_mac, mss_mac_hash_mode_t mode)
{
    uint32_t temp;

    /*
     * Enum values are matched to register bits but just to be safe we mask them
     * to ensure only the two hash control bits are modified...
     */
    if(this_mac->is_emac)
    {
        temp = this_mac->emac_base->NETWORK_CONFIG & (uint32_t)(~(GEM_UNICAST_HASH_ENABLE | GEM_MULTICAST_HASH_ENABLE));
        temp |= (uint32_t)mode & (uint32_t)(GEM_UNICAST_HASH_ENABLE | GEM_MULTICAST_HASH_ENABLE);
        this_mac->emac_base->NETWORK_CONFIG = temp;
    }
    else
    {
        temp = this_mac->mac_base->NETWORK_CONFIG & (uint32_t)(~(GEM_UNICAST_HASH_ENABLE | GEM_MULTICAST_HASH_ENABLE));
        temp |= (uint32_t)mode & (uint32_t)(GEM_UNICAST_HASH_ENABLE | GEM_MULTICAST_HASH_ENABLE);
        this_mac->mac_base->NETWORK_CONFIG = temp;
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

mss_mac_hash_mode_t MSS_MAC_get_hash_mode(mss_mac_instance_t *this_mac)
{
    mss_mac_hash_mode_t ret_val = 0;

    if(this_mac->is_emac)
    {
        ret_val = (mss_mac_hash_mode_t)(this_mac->emac_base->NETWORK_CONFIG & (GEM_UNICAST_HASH_ENABLE | GEM_MULTICAST_HASH_ENABLE));
    }
    else
    {
        ret_val = (mss_mac_hash_mode_t)(this_mac->mac_base->NETWORK_CONFIG & (GEM_UNICAST_HASH_ENABLE | GEM_MULTICAST_HASH_ENABLE));
    }

    return(ret_val);
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_type_filter(mss_mac_instance_t *this_mac, uint32_t filter, uint16_t value)
{
    uint32_t volatile *p_reg;

    if(this_mac->is_emac)
    {
        p_reg = &this_mac->emac_base->SPEC_TYPE1;
    }
    else
    {
        p_reg = &this_mac->mac_base->SPEC_TYPE1;
    }
    if((filter <= 4) && (filter > 0)) /* Filter is in range 1 to 4 to match register naming */
    {
        p_reg += filter - 1;
        if(0 == value) /* Disable filter if match value is 0 as this should not be a valid type */
        {
            *p_reg = 0;
        }
        else
        {
            *p_reg = (uint32_t)(0x80000000UL | (uint32_t)value);
        }
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

uint16_t MSS_MAC_get_type_filter(mss_mac_instance_t *this_mac, uint32_t filter)
{
    uint32_t volatile *p_reg;
    uint16_t ret_val = 0;

    if(this_mac->is_emac)
    {
        p_reg = &this_mac->emac_base->SPEC_TYPE1;
    }
    else
    {
        p_reg = &this_mac->mac_base->SPEC_TYPE1;
    }

    if((filter <= 4) && (filter > 0)) /* Filter is in range 1 to 4 to match register naming */
    {
        p_reg += filter - 1;
        if(0 != (*p_reg & 0x80000000UL)) /* Not disabled filter... */
        {
            ret_val = (uint16_t)*p_reg;
        }
    }

    return(ret_val);
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_sa_filter(mss_mac_instance_t *this_mac, uint32_t filter, uint16_t control, uint8_t *mac_addr)
{
    uint32_t volatile *p_reg;
    uint32_t address32_l;
    uint32_t address32_h;

    HAL_ASSERT(NULL_POINTER!= mac_addr);

    if(NULL_POINTER != mac_addr)
    {
        if((filter >= 2) && (filter <= 4))
        {
            if(this_mac->is_emac)
            {
                p_reg = &this_mac->emac_base->SPEC_ADD2_BOTTOM;
            }
            else
            {
                p_reg = &this_mac->mac_base->SPEC_ADD2_BOTTOM;
            }

            p_reg = p_reg + ((filter - 2) * 2);

            if(MSS_MAC_SA_FILTER_DISABLE == control)
            {
                /*
                 * Clear filter and disable - must be done in this order...
                 *  Writing to [0] disables and [1] enables So this sequence
                 *  clears the registers and disables the filter as opposed to
                 *  setting a destination address filter for 00:00:00:00:00:00.
                 */
                p_reg[1] = 0;
                p_reg[0] = 0;
            }
            else
            {
                /* Assemble correct register values */
                address32_l  = ((uint32_t)mac_addr[3]) << 24u;
                address32_l |= ((uint32_t)mac_addr[2]) << 16u;
                address32_l |= ((uint32_t)mac_addr[1]) << 8u;
                address32_l |= ((uint32_t)mac_addr[0]);
                address32_h =  ((uint32_t)mac_addr[5]) << 8u;
                address32_h |= ((uint32_t)mac_addr[4]);

                address32_h |= (uint32_t)control << 16;

                /* Update hardware registers - must be done in this order... */
                p_reg[0] = address32_l;
                p_reg[1] = address32_h;
            }
        }
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

uint16_t MSS_MAC_get_sa_filter(mss_mac_instance_t *this_mac, uint32_t filter, uint8_t *mac_addr)
{
    uint32_t volatile *p_reg;
    uint32_t temp_reg;
    uint16_t ret_val = 0;

    if(0 != mac_addr)
    {
        memset(mac_addr, 0, 6); /* Consistent result if bad parameters passed... */
    }

    if((filter >= 2) && (filter <= 4))
    {
        if(this_mac->is_emac)
        {
            p_reg = &this_mac->emac_base->SPEC_ADD2_BOTTOM;
        }
        else
        {
            p_reg = &this_mac->mac_base->SPEC_ADD2_BOTTOM;
        }

        p_reg = p_reg + ((filter - 2) * 2);

        if(0 != mac_addr) /* Want MAC address and filter control info? */
        {
            temp_reg = p_reg[0];
            *mac_addr++ = (uint8_t)temp_reg & BITS_08;
            temp_reg >>= 8;
            *mac_addr++ = (uint8_t)temp_reg & BITS_08;
            temp_reg >>= 8;
            *mac_addr++ = (uint8_t)temp_reg & BITS_08;
            temp_reg >>= 8;
            *mac_addr++ = (uint8_t)temp_reg & BITS_08;

            temp_reg = p_reg[1];
            *mac_addr++ = (uint8_t)temp_reg & BITS_08;
            temp_reg >>= 8;
            *mac_addr++ = (uint8_t)temp_reg & BITS_08;
            temp_reg >>= 8;
            ret_val = (uint16_t)temp_reg;
        }
        else
        {
            ret_val = (uint16_t)(p_reg[1] >> 16);
        }
    }

    return(ret_val);
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_type_1_filter(mss_mac_instance_t *this_mac, uint32_t filter_no, mss_mac_type_1_filter_t *filter)
{
    if(filter_no < MSS_MAC_TYPE_1_SCREENERS)
    {
        uint32_t volatile *p_reg;
        uint32_t temp_reg;

        if(this_mac->is_emac)
        {
            p_reg = &this_mac->emac_base->SCREENING_TYPE_1_REGISTER_0;
        }
        else
        {
            p_reg = &this_mac->mac_base->SCREENING_TYPE_1_REGISTER_0;
        }

        temp_reg  = (uint32_t)filter->queue_no & GEM_QUEUE_NUMBER;
        temp_reg |= (uint32_t)(((uint32_t)filter->dstc << GEM_DSTC_MATCH_SHIFT) & GEM_DSTC_MATCH);
        temp_reg |= (uint32_t)(((uint32_t)filter->udp_port << GEM_UDP_PORT_MATCH_SHIFT) & GEM_UDP_PORT_MATCH);
        if(filter->drop_on_match)
        {
            temp_reg |= GEM_DROP_ON_MATCH;
        }

        if(filter->dstc_enable)
        {
            temp_reg |= GEM_DSTC_ENABLE;
        }

        if(filter->udp_port_enable)
        {
            temp_reg |= GEM_UDP_PORT_MATCH_ENABLE;
        }

        p_reg[filter_no] = temp_reg;
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_get_type_1_filter(mss_mac_instance_t *this_mac, uint32_t filter_no, mss_mac_type_1_filter_t *filter)
{
    memset(filter, 0, sizeof(mss_mac_type_1_filter_t)); /* Blank canvass to start */

    if(filter_no < MSS_MAC_TYPE_1_SCREENERS)
    {
        uint32_t volatile *p_reg;
        uint32_t temp_reg;

        if(this_mac->is_emac)
        {
            p_reg = &this_mac->emac_base->SCREENING_TYPE_1_REGISTER_0;
        }
        else
        {
            p_reg = &this_mac->mac_base->SCREENING_TYPE_1_REGISTER_0;
        }

        temp_reg = p_reg[filter_no];

        filter->queue_no = temp_reg & GEM_QUEUE_NUMBER;
        temp_reg >>= 4;

        filter->dstc = (uint8_t)(temp_reg & GEM_DSTC_MATCH);
        temp_reg >>= GEM_DSTC_MATCH_SHIFT;

        filter->udp_port = (uint16_t)(temp_reg & GEM_UDP_PORT_MATCH);
        temp_reg >>= GEM_UDP_PORT_MATCH_SHIFT;

        filter->dstc_enable = temp_reg & 1;
        temp_reg >>= 1;

        filter->udp_port_enable = temp_reg & 1;
        temp_reg >>= 1;

        filter->drop_on_match = temp_reg & 1;
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_type_2_ethertype(mss_mac_instance_t *this_mac, uint32_t ethertype_no, uint16_t ethertype)
{
    uint32_t volatile *p_reg;

    if(!this_mac->is_emac) /* Ethertype filter not supported on eMAC */
    {
        p_reg = &this_mac->mac_base->SCREENING_TYPE_2_ETHERTYPE_REG_0;
        if(ethertype_no < MSS_MAC_TYPE_2_ETHERTYPES)
        {
            p_reg[ethertype_no] = (uint32_t)ethertype;
        }
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

uint16_t MSS_MAC_get_type_2_ethertype(mss_mac_instance_t *this_mac, uint32_t ethertype_no)
{
    uint32_t volatile *p_reg;
    uint16_t temp_reg = 0;

    if(!this_mac->is_emac) /* Ethertype filter not supported on eMAC */
    {
        p_reg = &this_mac->mac_base->SCREENING_TYPE_2_ETHERTYPE_REG_0;
        if(ethertype_no < MSS_MAC_TYPE_2_ETHERTYPES)
        {
            temp_reg = (uint16_t)p_reg[ethertype_no];
        }
    }

    return(temp_reg);
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_type_2_compare(mss_mac_instance_t *this_mac, uint32_t comparer_no, mss_mac_type_2_compare_t *comparer)
{
    uint32_t volatile *p_reg;
    uint32_t limit;
    uint32_t temp_reg;

    if(this_mac->is_emac) /* eMAC limits are different to pMAC ones */
    {
        p_reg = &this_mac->emac_base->TYPE2_COMPARE_0_WORD_0;
        limit = MSS_MAC_EMAC_TYPE_2_COMPARERS;
    }
    else
    {
        p_reg = &this_mac->mac_base->TYPE2_COMPARE_0_WORD_0;
        limit = MSS_MAC_TYPE_2_COMPARERS;
    }

    if(comparer_no < limit)
    {
        comparer_no *= 2; /* Working with consecutive pairs of registers for this one */
        if(comparer->disable_mask)
        {
            p_reg[comparer_no] = comparer->data; /* Mask disabled so just 4 byte compare value */
            temp_reg = GEM_DISABLE_MASK;         /* and no mask */
        }
        else
        {
            temp_reg  = comparer->data << 16;     /* 16 bit compare value and 16 bit mask */
            temp_reg |= (uint32_t)comparer->mask;
            p_reg[comparer_no] = temp_reg;
            temp_reg = 0;
        }

        if(comparer->compare_vlan_c_id)
        {
            temp_reg |= GEM_COMPARE_VLAN_ID;
        }
        else if(comparer->compare_vlan_s_id)
        {
            temp_reg |= GEM_COMPARE_VLAN_ID | GEM_COMPARE_S_TAG;
        }
        else
        {
            temp_reg |= (uint32_t)(comparer->offset_value & BITS_07);
            temp_reg |= ((uint32_t)(comparer->compare_offset & BITS_02)) << GEM_COMPARE_OFFSET_SHIFT;
        }

        p_reg[comparer_no + 1] = temp_reg; /* Set second word of comparer */
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_get_type_2_compare(mss_mac_instance_t *this_mac, uint32_t comparer_no, mss_mac_type_2_compare_t *comparer)
{
    uint32_t volatile *p_reg;
    uint32_t limit;

    if(this_mac->is_emac) /* eMAC limits are different to pMAC ones */
    {
        p_reg = &this_mac->emac_base->TYPE2_COMPARE_0_WORD_0;
        limit = MSS_MAC_EMAC_TYPE_2_COMPARERS;
    }
    else
    {
        p_reg = &this_mac->mac_base->TYPE2_COMPARE_0_WORD_0;
        limit = MSS_MAC_TYPE_2_COMPARERS;
    }

    memset(comparer, 0, sizeof(mss_mac_type_2_compare_t));
    if(comparer_no < limit)
    {
        comparer_no *= 2; /* Working with consecutive pairs of registers for this one */
        if(p_reg[comparer_no + 1] & GEM_DISABLE_MASK)
        {
            comparer->data = p_reg[comparer_no]; /* Mask disabled so just 4 byte compare value */
            comparer->disable_mask = 1;          /* and no mask */
        }
        else
        {
            comparer->data = p_reg[comparer_no] >> 16;     /* 16 bit compare value and 16 bit mask */
            comparer->mask = (uint16_t)p_reg[comparer_no];
        }

        if(p_reg[comparer_no + 1] & GEM_COMPARE_VLAN_ID)
        {
            if(p_reg[comparer_no + 1] & GEM_COMPARE_S_TAG)
            {
                comparer->compare_vlan_s_id = 1;
            }
            else
            {
                comparer->compare_vlan_c_id = 1;
            }
        }
        else
        {
            comparer->compare_offset = (uint8_t)((p_reg[comparer_no + 1] >> GEM_COMPARE_OFFSET_SHIFT) & BITS_02);
        }

        comparer->offset_value = (uint8_t)(p_reg[comparer_no + 1] & GEM_OFFSET_VALUE);
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_type_2_filter(mss_mac_instance_t *this_mac, uint32_t filter_no, mss_mac_type_2_filter_t *filter)
{
    uint32_t volatile *p_reg;
    uint32_t limit;
    uint32_t temp_reg = 0;

    if(this_mac->is_emac) /* eMAC limits are different to pMAC ones */
    {
        p_reg = &this_mac->emac_base->SCREENING_TYPE_2_REGISTER_0;
        limit = MSS_MAC_EMAC_TYPE_2_SCREENERS;
    }
    else
    {
        p_reg = &this_mac->mac_base->SCREENING_TYPE_2_REGISTER_0;
        limit = MSS_MAC_TYPE_2_SCREENERS;
    }

    if(filter_no < limit) /* Lets go build a filter... */
    {
        if(filter->drop_on_match)
        {
            temp_reg = GEM_T2_DROP_ON_MATCH;
        }
        else
        {
            temp_reg = 0;
        }

        if(filter->compare_a_enable)
        {
            temp_reg |= GEM_COMPARE_A_ENABLE;
            temp_reg |= (uint32_t)(filter->compare_a_index & BITS_05) << GEM_COMPARE_A_SHIFT;
        }

        if(filter->compare_b_enable)
        {
            temp_reg |= GEM_COMPARE_B_ENABLE;
            temp_reg |= (uint32_t)(filter->compare_b_index & BITS_05) << GEM_COMPARE_B_SHIFT;
        }

        if(filter->compare_c_enable)
        {
            temp_reg |= GEM_COMPARE_C_ENABLE;
            temp_reg |= (uint32_t)(filter->compare_c_index & BITS_05) << GEM_COMPARE_C_SHIFT;
        }

        if(filter->ethertype_enable)
        {
            temp_reg |= GEM_ETHERTYPE_ENABLE;
            temp_reg |= (uint32_t)(filter->ethertype_index & BITS_03) << GEM_ETHERTYPE_REG_INDEX_SHIFT;
        }

        if(filter->vlan_priority_enable)
        {
            temp_reg |= GEM_VLAN_ENABLE;
            temp_reg |= (uint32_t)(filter->vlan_priority & BITS_03) << GEM_VLAN_PRIORITY_SHIFT;
        }

        temp_reg |= (uint32_t)(filter->queue_no & GEM_QUEUE_NUMBER);

        p_reg[filter_no] = temp_reg; /* Set filter up at last */
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_get_type_2_filter(mss_mac_instance_t *this_mac, uint32_t filter_no, mss_mac_type_2_filter_t *filter)
{
    uint32_t volatile *p_reg;
    uint32_t limit;

    memset(filter, 0, sizeof(mss_mac_type_2_filter_t));
    if(this_mac->is_emac) /* eMAC limits are different to pMAC ones */
    {
        p_reg = &this_mac->emac_base->SCREENING_TYPE_2_REGISTER_0;
        limit = MSS_MAC_EMAC_TYPE_2_SCREENERS;
    }
    else
    {
        p_reg = &this_mac->mac_base->SCREENING_TYPE_2_REGISTER_0;
        limit = MSS_MAC_TYPE_2_SCREENERS;
    }

    if(filter_no < limit) /* Lets go fetch a filter... */
    {
        if(p_reg[filter_no] & GEM_T2_DROP_ON_MATCH)
        {
            filter->drop_on_match = 1;
        }

        if(p_reg[filter_no] & GEM_COMPARE_A_ENABLE)
        {
            filter->compare_a_enable = 1;
        }

        if(p_reg[filter_no] & GEM_COMPARE_B_ENABLE)
        {
            filter->compare_b_enable = 1;
        }

        if(p_reg[filter_no] & GEM_COMPARE_C_ENABLE)
        {
            filter->compare_c_enable = 1;
        }

        if(p_reg[filter_no] & GEM_ETHERTYPE_ENABLE)
        {
            filter->ethertype_enable = 1;
        }

        if(p_reg[filter_no] & GEM_VLAN_ENABLE)
        {
            filter->vlan_priority_enable = 1;
        }

        filter->compare_a_index = (uint8_t)((p_reg[filter_no] & GEM_COMPARE_A) >> GEM_COMPARE_A_SHIFT);
        filter->compare_b_index = (uint8_t)((p_reg[filter_no] & GEM_COMPARE_B) >> GEM_COMPARE_B_SHIFT);
        filter->compare_c_index = (uint8_t)((p_reg[filter_no] & GEM_COMPARE_C) >> GEM_COMPARE_C_SHIFT);
        filter->ethertype_index = (uint8_t)((p_reg[filter_no] & GEM_ETHERTYPE_REG_INDEX) >> GEM_ETHERTYPE_REG_INDEX_SHIFT);
        filter->vlan_priority   = (uint8_t)((p_reg[filter_no] & GEM_VLAN_PRIORITY) >> GEM_VLAN_PRIORITY_SHIFT);
        filter->queue_no        = (uint8_t)(p_reg[filter_no] & GEM_QUEUE_NUMBER);
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_mmsl_mode(mss_mac_instance_t *this_mac, mss_mac_mmsl_config_t *mmsl_cfg)
{
    if(!this_mac->is_emac) /* The MMSL registers are attached to the pMAC definition */
    {
        if(mmsl_cfg->preemption) /* Preemption is enabled so deal with it */
        {
            /*
             * Need to shut preemption down in case it is already enabled
             * otherwise the new settings may not be recognised properly.
             */
            this_mac->mac_base->MMSL_CONTROL = 0L;

            if(mmsl_cfg->verify_disable)
            {
                this_mac->mac_base->MMSL_CONTROL |= (uint32_t)mmsl_cfg->frag_size | GEM_VERIFY_DISABLE;
                this_mac->mac_base->MMSL_CONTROL |= GEM_PRE_ENABLE;
            }
            else
            {
                this_mac->mac_base->MMSL_CONTROL |= (uint32_t)mmsl_cfg->frag_size;
                this_mac->mac_base->MMSL_CONTROL |= GEM_PRE_ENABLE;
            }
        }
        else /* Preemption is not enabled so see which MAC we want to use */
        {
            if(mmsl_cfg->use_pmac)
            {
                this_mac->mac_base->MMSL_CONTROL = GEM_ROUTE_RX_TO_PMAC;
            }
            else
            {
                this_mac->mac_base->MMSL_CONTROL = 0L;
            }
        }
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_get_mmsl_mode(mss_mac_instance_t *this_mac, mss_mac_mmsl_config_t *mmsl_cfg)
{
    memset(mmsl_cfg, 0, sizeof(mss_mac_mmsl_config_t));
    if(!this_mac->is_emac) /* The MMSL registers are attached to the pMAC definition */
    {
        if(this_mac->mac_base->MMSL_CONTROL & GEM_ROUTE_RX_TO_PMAC)
        {
            mmsl_cfg->use_pmac = 1;
        }

        if(this_mac->mac_base->MMSL_CONTROL & GEM_PRE_ENABLE)
        {
            mmsl_cfg->preemption = 1;
        }

        if(0 == (this_mac->mac_base->MMSL_CONTROL & GEM_VERIFY_DISABLE))
        {
            mmsl_cfg->verify_disable = 1;
        }

        mmsl_cfg->frag_size = (mss_mac_frag_size_t)(this_mac->mac_base->MMSL_CONTROL & BITS_02);
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_start_preemption_verify(mss_mac_instance_t *this_mac)
{
    if(!this_mac->is_emac) /* The MMSL registers are attached to the pMAC definition */
    {
        if(this_mac->mac_base->MMSL_CONTROL | GEM_PRE_ENABLE) /* Preemption is enabled */
        {
            this_mac->mac_base->MMSL_CONTROL |= GEM_RESTART_VER;
        }
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

uint32_t MSS_MAC_get_mmsl_status(mss_mac_instance_t *this_mac)
{
    uint32_t ret_val = 0;

    if(!this_mac->is_emac) /* The MMSL registers are attached to the pMAC definition */
    {
        ret_val = this_mac->mac_base->MMSL_STATUS; /* Just return raw value, user can decode using defined bits */
    }

    return(ret_val);
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_get_mmsl_stats(mss_mac_instance_t *this_mac, mss_mac_mmsl_stats_t *stats)
{
    /*
     * We return these differently to the general statistics as they are pMAC
     * specific and don't have a corresponding eMAC equivalent.
     */
    memset(stats, 0, sizeof(mss_mac_mmsl_stats_t));
    if(!this_mac->is_emac) /* The MMSL registers are attached to the pMAC definition */
    {
        stats->smd_err_count = (this_mac->mac_base->MMSL_ERR_STATS & GEM_SMD_ERR_COUNT) >> GEM_SMD_ERR_COUNT_SHIFT;
        stats->ass_err_count = this_mac->mac_base->MMSL_ERR_STATS & GEM_ASS_ERR_COUNT;
        stats->ass_ok_count  = this_mac->mac_base->MMSL_ASS_OK_COUNT;
        stats->frag_count_rx = this_mac->mac_base->MMSL_FRAG_COUNT_RX;
        stats->frag_count_tx = this_mac->mac_base->MMSL_FRAG_COUNT_TX;
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_tx_cutthru(mss_mac_instance_t *this_mac, uint32_t level)
{
    uint32_t volatile *p_reg;
    uint32_t mask;

    p_reg = this_mac->is_emac ? &this_mac->emac_base->PBUF_TXCUTTHRU : &this_mac->mac_base->PBUF_TXCUTTHRU;
    mask  = this_mac->is_emac ? GEM_DMA_EMAC_CUTTHRU_THRESHOLD : GEM_DMA_TX_CUTTHRU_THRESHOLD;

    if(0 == level) /* Disabling cutthru? */
    {
        *p_reg = 0;
    }
    else
    {
        *p_reg = GEM_DMA_CUTTHRU | (level & mask);
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_rx_cutthru(mss_mac_instance_t *this_mac, uint32_t level)
{
    uint32_t volatile *p_reg;
    uint32_t mask;

    p_reg = this_mac->is_emac ? &this_mac->emac_base->PBUF_RXCUTTHRU : &this_mac->mac_base->PBUF_RXCUTTHRU;
    mask  = this_mac->is_emac ? GEM_DMA_EMAC_CUTTHRU_THRESHOLD : GEM_DMA_RX_CUTTHRU_THRESHOLD;

    if(0 == level) /* Disabling cutthru? */
    {
        *p_reg = 0;
    }
    else
    {
        *p_reg = GEM_DMA_CUTTHRU | (level & mask);
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

uint32_t MSS_MAC_get_tx_cutthru(mss_mac_instance_t *this_mac)
{
    uint32_t temp_reg;
    uint32_t volatile *p_reg;
    uint32_t mask;

    p_reg = this_mac->is_emac ? &this_mac->emac_base->PBUF_TXCUTTHRU : &this_mac->mac_base->PBUF_TXCUTTHRU;
    mask  = this_mac->is_emac ? GEM_DMA_EMAC_CUTTHRU_THRESHOLD : GEM_DMA_TX_CUTTHRU_THRESHOLD;

    temp_reg = *p_reg;
    if(0 == (temp_reg & GEM_DMA_CUTTHRU))
    {
        temp_reg = 0;
    }
    else
    {
        temp_reg &= mask;
    }

    return(temp_reg);
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

uint32_t MSS_MAC_get_rx_cutthru(mss_mac_instance_t *this_mac)
{
    uint32_t temp_reg;
    uint32_t volatile *p_reg;
    uint32_t mask;

    p_reg = this_mac->is_emac ? &this_mac->emac_base->PBUF_RXCUTTHRU : &this_mac->mac_base->PBUF_RXCUTTHRU;
    mask  = this_mac->is_emac ? GEM_DMA_EMAC_CUTTHRU_THRESHOLD : GEM_DMA_RX_CUTTHRU_THRESHOLD;

    temp_reg = *p_reg;
    if(0 == (temp_reg & GEM_DMA_CUTTHRU))
    {
        temp_reg = 0;
    }
    else
    {
        temp_reg &= mask;
    }

    return(temp_reg);
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_tx_enable(mss_mac_instance_t *this_mac)
{
    /* Don't do this if already done in case it has side effects... */
    if(this_mac->is_emac)
    {
        if(0 == (this_mac->emac_base->NETWORK_CONTROL & GEM_ENABLE_TRANSMIT))
        {
            this_mac->emac_base->NETWORK_CONTROL |= GEM_ENABLE_TRANSMIT;
        }
    }
    else
    {
        if(0 == (this_mac->mac_base->NETWORK_CONTROL & GEM_ENABLE_TRANSMIT))
        {
            this_mac->mac_base->NETWORK_CONTROL |= GEM_ENABLE_TRANSMIT;
        }
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_jumbo_frames_mode(mss_mac_instance_t *this_mac, bool state)
{
    volatile uint32_t *p_reg = &this_mac->mac_base->NETWORK_CONFIG;

    if(this_mac->is_emac)
    {
        p_reg = &this_mac->emac_base->NETWORK_CONFIG;
    }

    if(this_mac->jumbo_frame_enable) /* Only look at this if the feature is enabled in config */
    {
        if(state)
        {
            *p_reg |= GEM_JUMBO_FRAMES;
        }
        else
        {
            *p_reg &= (uint32_t)(~GEM_JUMBO_FRAMES);
        }
    }
    else /* Ensure it is disabled if not allowed... */
    {
        *p_reg &= (uint32_t)(~GEM_JUMBO_FRAMES);
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

bool MSS_MAC_get_jumbo_frames_mode(mss_mac_instance_t *this_mac)
{
    bool ret_val = false;
    volatile uint32_t *p_reg = &this_mac->mac_base->NETWORK_CONFIG;

    if(this_mac->is_emac)
    {
        p_reg = &this_mac->emac_base->NETWORK_CONFIG;
    }

    ret_val = (*p_reg & ~GEM_JUMBO_FRAMES) != 0;

    return(ret_val);
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_jumbo_frame_length(mss_mac_instance_t *this_mac, uint32_t length)
{
    volatile uint32_t *p_reg = &this_mac->mac_base->JUMBO_MAX_LENGTH;

    if(this_mac->is_emac)
    {
        p_reg = &this_mac->emac_base->JUMBO_MAX_LENGTH;
    }

    /* Set up maximum jumbo frame size - but bounds check first  */
    if(length > MSS_MAC_JUMBO_MAX)
    {
        length = MSS_MAC_JUMBO_MAX;
    }

    if(this_mac->jumbo_frame_enable) /* Only look at this if the feature is enabled in config */
    {
        *p_reg = length;
    }
    else /* Ensure it is set to reset value if not allowed... */
    {
        *p_reg = MSS_MAC_JUMBO_MAX;
    }
}


/**************************************************************************//**
 * See mss_ethernet_mac.h for details of how to use this function.
 */

uint32_t MSS_MAC_get_jumbo_frame_length(mss_mac_instance_t *this_mac)
{
    uint32_t ret_val = 0;
    volatile uint32_t *p_reg = &this_mac->mac_base->JUMBO_MAX_LENGTH;

    if(this_mac->is_emac)
    {
        p_reg = &this_mac->emac_base->JUMBO_MAX_LENGTH;
    }

    ret_val = *p_reg;

    return(ret_val);
}


/**************************************************************************/
/* Private Function definitions                                           */
/**************************************************************************/  

/**************************************************************************//**
 * This is default "Receive packet interrupt handler. This function finds the
 * descriptor that received the packet and caused the interrupt.
 * This informs the received packet size to the application and
 * relinquishes the packet buffer from the associated DMA descriptor.
 */
static void 
rxpkt_handler
(
    mss_mac_instance_t *this_mac, uint32_t queue_no
)
{
    mss_mac_rx_desc_t *cdesc = &this_mac->queue[queue_no].rx_desc_tab[
                                 this_mac->queue[queue_no].first_rx_desc_index
                               ];

    /* Loop while there are packets available, but check in the beginning
     * for a case where the packet was received in middle of previous IRQ.
     * As the previous IRQ already handled the packet and therefore
     * there is no new packet to be handled in this IRQ event. */
    while(0 != (cdesc->addr_low & GEM_RX_DMA_USED))
    {
        uint8_t * p_rx_packet;
        uint32_t pckt_length;
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
        uint64_t addr_temp;
#else
        uint32_t addr_temp;
#endif

        ++this_mac->queue[queue_no].nb_available_rx_desc;
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
        addr_temp  = (uint64_t)(cdesc->addr_low & ~(GEM_RX_DMA_WRAP | GEM_RX_DMA_USED | GEM_RX_DMA_TS_PRESENT));
        addr_temp |= (uint64_t)cdesc->addr_high << 32;
#else
        addr_temp = (cdesc->addr_low & ~(GEM_RX_DMA_WRAP | GEM_RX_DMA_USED | GEM_RX_DMA_TS_PRESENT));
#endif
        p_rx_packet = (uint8_t *)addr_temp;
        /* Pass received packet up to application layer - if enabled... */
        if((NULL_POINTER != this_mac->queue[queue_no].pckt_rx_callback) && (0 == this_mac->rx_discard))
        {
            pckt_length = cdesc->status & (GEM_RX_DMA_BUFF_LEN | GEM_RX_DMA_JUMBO_BIT_13);
            this_mac->queue[queue_no].ingress += pckt_length;

            this_mac->queue[queue_no].pckt_rx_callback(this_mac, queue_no, p_rx_packet, pckt_length, cdesc, this_mac->queue[queue_no].rx_caller_info[this_mac->queue[queue_no].first_rx_desc_index]);
        }
        if(0 != this_mac->rx_discard)
        {
            /*
             * Need to return receive packet buffer to the queue as rx handler
             * hasn't been called to do it for us...
             */
            MSS_MAC_receive_pkt(this_mac, queue_no, p_rx_packet, 0, 1);
        }

        /* Mark buffer as unused again */
        cdesc->addr_low &= (uint32_t)(~GEM_RX_DMA_USED);

        /* Point the curr_rx_desc to next descriptor in the ring */
        /* Wrap around in case next descriptor is pointing to last in the ring */
        ++this_mac->queue[queue_no].first_rx_desc_index;
        this_mac->queue[queue_no].first_rx_desc_index %= MSS_MAC_RX_RING_SIZE;

       cdesc = &this_mac->queue[queue_no]
               .rx_desc_tab[this_mac->queue[queue_no].first_rx_desc_index];
    };
}

/**************************************************************************//**
 * This is default "Transmit packet interrupt handler. This function finds the
 * descriptor that transmitted the packet and caused the interrupt.
 * This relinquishes the packet buffer from the associated DMA descriptor.
 */
static void 
txpkt_handler
(
        mss_mac_instance_t *this_mac, uint32_t queue_no
)
{
    HAL_ASSERT(g_mac.first_tx_index != INVALID_INDEX);
    HAL_ASSERT(g_mac.last_tx_index != INVALID_INDEX);
#if defined(MSS_MAC_SIMPLE_TX_QUEUE)
    /*
     * Simple single packet TX queue where only the one packet buffer is needed
     * but two descriptors are required to stop DMA engine running over itself...
     */
    if(NULL_POINTER != this_mac->queue[queue_no].tx_complete_handler)
    {
        this_mac->queue[queue_no].tx_complete_handler(this_mac, queue_no, this_mac->queue[queue_no].tx_desc_tab, this_mac->queue[queue_no].tx_caller_info[0]);
    }

    this_mac->queue[queue_no].nb_available_tx_desc = MSS_MAC_TX_RING_SIZE; /* Release transmit queue... */

#else    
    uint32_t empty_flag;
    uint32_t index;
    uint32_t completed = 0u;
    /* TBD PMCS multi packet tx queue not implemented yet */
    index = g_mac.first_tx_index;
    do
    {
        ++g_mac.nb_available_tx_desc;
        /* Call packet Tx completion handler if it exists. */
        if(NULL_POINTER != g_mac.tx_complete_handler)
        {
            g_mac.tx_complete_handler(g_mac.tx_desc_tab[index].caller_info);
        }
        
        if(index == g_mac.last_tx_index)
        {
            /* all pending tx packets sent. */
            g_mac.first_tx_index = INVALID_INDEX;
            g_mac.last_tx_index = INVALID_INDEX;
            completed = 1u;
        }
        else
        {
            /* Move on to next transmit descriptor. */
            ++index;
            index %= MSS_MAC_TX_RING_SIZE;
            g_mac.first_tx_index = index;
            /* Check if we reached a descriptor still pending tx. */
            empty_flag = g_mac.tx_desc_tab[index].pkt_size & DMA_DESC_EMPTY_FLAG_MASK;
            if(0u == empty_flag)
            {
                completed = 1u;
            }
        }
        
        
        /* Clear the tx packet sent interrupt. Please note that this must be
         * done for every packet sent as it decrements the TXPKTCOUNT. */
        set_bit_reg32(&MAC->DMA_TX_STATUS, DMA_TXPKTSENT);
    } while(0u == completed);
#endif
}

/**************************************************************************//**
 * 
 */
static void tx_desc_ring_init(mss_mac_instance_t *this_mac)
{
    int32_t inc;
    int32_t queue_no;
    
    for(queue_no = 0; queue_no != MSS_MAC_QUEUE_COUNT; queue_no++)
    {
#if defined(MSS_MAC_USE_DDR)
        this_mac->queue[queue_no].tx_desc_tab = g_mss_mac_ddr_ptr;
        g_mss_mac_ddr_ptr += MSS_MAC_TX_RING_SIZE * sizeof(mss_mac_tx_desc_t);
#endif
        for(inc = 0; inc < MSS_MAC_TX_RING_SIZE; ++inc)
        {
            this_mac->queue[queue_no].tx_desc_tab[inc].addr_low = 0u;
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
            this_mac->queue[queue_no].tx_desc_tab[inc].addr_high = 0u;
#endif
            this_mac->queue[queue_no].tx_desc_tab[inc].status = GEM_TX_DMA_USED;
        }

        inc--; /* Step back to last buffer descriptor and mark as end of list */
        this_mac->queue[queue_no].tx_desc_tab[inc].status |= GEM_TX_DMA_WRAP;
    }
}

/**************************************************************************//**
 * 
 */
static void rx_desc_ring_init(mss_mac_instance_t *this_mac)
{
    int32_t inc;
    int32_t queue_no;
    
    for(queue_no = 0; queue_no != MSS_MAC_QUEUE_COUNT; queue_no++)
    {
#if defined(MSS_MAC_USE_DDR)
        this_mac->queue[queue_no].rx_desc_tab = g_mss_mac_ddr_ptr;
        g_mss_mac_ddr_ptr += MSS_MAC_RX_RING_SIZE * sizeof(mss_mac_rx_desc_t);
#endif

        for(inc = 0; inc < MSS_MAC_RX_RING_SIZE; ++inc)
        {
            this_mac->queue[queue_no].rx_desc_tab[inc].addr_low = 0u; /* Mark buffers as used for now in case DMA gets enabled before we attach a buffer */
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
            this_mac->queue[queue_no].rx_desc_tab[inc].addr_high = 0u;
#endif
            this_mac->queue[queue_no].rx_desc_tab[inc].status = 0;
        }

        inc--; /* Step back to last buffer descriptor and mark as end of list */
        this_mac->queue[queue_no].rx_desc_tab[inc].addr_low |= GEM_RX_DMA_WRAP;
    
#if defined(MSS_MAC_SIMPLE_TX_QUEUE)
        /* Do this once here to avoid doing it for every send... */
        this_mac->queue[queue_no].tx_desc_tab[1].status |= GEM_TX_DMA_USED; /* Mark second buffer used so DMA stops cleanly */
#endif
    }
}

/**************************************************************************//**
 * 
 */
static void assign_station_addr
(
    mss_mac_instance_t *this_mac,
    const uint8_t mac_addr[6]
)
{
    uint32_t address32_l;
    uint32_t address32_h;

    HAL_ASSERT(NULL_POINTER!= mac_addr);

    if(NULL_POINTER != mac_addr)
    {
        /* Update current instance data */
        memcpy(this_mac->mac_addr, mac_addr, sizeof(this_mac->mac_addr));

        /* Assemble correct register values */
        address32_l  = ((uint32_t)mac_addr[3]) << 24u;
        address32_l |= ((uint32_t)mac_addr[2]) << 16u;
        address32_l |= ((uint32_t)mac_addr[1]) << 8u;
        address32_l |= ((uint32_t)mac_addr[0]);
        address32_h =  ((uint32_t)mac_addr[5]) << 8u;
        address32_h |= ((uint32_t)mac_addr[4]);

        /* Update hardware registers */
        if(this_mac->is_emac)
        {
            this_mac->emac_base->SPEC_ADD1_BOTTOM = address32_l;
            this_mac->emac_base->SPEC_ADD1_TOP    = address32_h;
            this_mac->emac_base->SPEC_ADD2_BOTTOM = 0ul;
            this_mac->emac_base->SPEC_ADD2_TOP    = 0ul;
            this_mac->emac_base->SPEC_ADD3_BOTTOM = 0ul;
            this_mac->emac_base->SPEC_ADD3_TOP    = 0ul;
            this_mac->emac_base->SPEC_ADD4_BOTTOM = 0ul;
            this_mac->emac_base->SPEC_ADD4_TOP    = 0ul;
        }
        else
        {
            this_mac->mac_base->SPEC_ADD1_BOTTOM  = address32_l;
            this_mac->mac_base->SPEC_ADD1_TOP     = address32_h;
            this_mac->mac_base->SPEC_ADD2_BOTTOM  = 0ul;
            this_mac->mac_base->SPEC_ADD2_TOP     = 0ul;
            this_mac->mac_base->SPEC_ADD3_BOTTOM  = 0ul;
            this_mac->mac_base->SPEC_ADD3_TOP     = 0ul;
            this_mac->mac_base->SPEC_ADD4_BOTTOM  = 0ul;
            this_mac->mac_base->SPEC_ADD4_TOP     = 0ul;
        }
    }
}

/***************************************************************************//**
 * Auto-detect the PHY's address by attempting to read the PHY identification
 * register containing the PHY manufacturer's identifier.
 * Attempting to read a PHY register using an incorrect PHY address will result
 * in a value with all bits set to one on the MDIO bus. Reading any other value
 * means that a PHY responded to the read request, therefore we have found the
 * PHY's address.
 * This function returns the detected PHY's address or 32 (PHY_ADDRESS_MAX + 1)
 * if no PHY is responding.
 */
static uint8_t probe_phy(mss_mac_instance_t *this_mac) // @suppress("Unused static function")
{
    uint8_t phy_address = PHY_ADDRESS_MIN;
    const uint16_t ALL_BITS_HIGH = 0xffffU;
    const uint8_t PHYREG_PHYID1R = 0x02U;   /* PHY Identifier 1 register address. */
    uint32_t found;
    
    do
    {
        uint16_t reg;
        
        reg = MSS_MAC_read_phy_reg(this_mac, phy_address, PHYREG_PHYID1R);
        if (reg != ALL_BITS_HIGH)
        {
            found = 1U;
        }
        else
        {
            found = 0U;
            ++phy_address;
        }
    }
    while ((phy_address <= PHY_ADDRESS_MAX) && (0U == found));    
    
    return phy_address;
}

/***************************************************************************//**
 * MSS MAC TBI interface
 */
static void msgmii_init(mss_mac_instance_t *this_mac)
{
    if(GMII_SGMII == this_mac->interface_type)
    {
        if(!this_mac->is_emac)
        {
            this_mac->mac_base->PCS_CONTROL = 0x9000ul; /* Reset and enable autonegotiation */
        }
    }

    if(TBI == this_mac->interface_type)
    {
        if(!this_mac->is_emac)
        {
            this_mac->mac_base->PCS_CONTROL = 0x9000ul; /* Reset and enable autonegotiation */
        }
    }
}

/***************************************************************************//**
 *
 */
 static void msgmii_autonegotiate(mss_mac_instance_t *this_mac)
 {
    uint16_t phy_reg;
    uint16_t autoneg_complete;
    uint8_t link_fullduplex;
    mss_mac_speed_t link_speed;
    uint8_t copper_link_up;
    
    volatile uint32_t sgmii_aneg_timeout = 100000u;

    copper_link_up = this_mac->phy_get_link_status(this_mac, &link_speed, &link_fullduplex);
    
    if(MSS_MAC_LINK_UP == copper_link_up)
    {
        /* Initiate auto-negotiation on the TBI SGMII link. */
        if(TBI == this_mac->interface_type)
        {
            phy_reg = (uint16_t)this_mac->mac_base->PCS_CONTROL;
            phy_reg |= 0x1000;
            this_mac->mac_base->PCS_CONTROL = phy_reg;
            phy_reg |= 0x0200;
            this_mac->mac_base->PCS_CONTROL = phy_reg;

            /* Wait for SGMII auto-negotiation to complete. */
            do
            {
                phy_reg = (uint16_t)this_mac->mac_base->PCS_STATUS;
                autoneg_complete = phy_reg & BMSR_AUTO_NEGOTIATION_COMPLETE;
                --sgmii_aneg_timeout;
            } while((!autoneg_complete && sgmii_aneg_timeout != 0u)|| (0xFFFF == phy_reg));
        }
    }
}

#if 0
/***************************************************************************//**
 * SGMII or 1000BaseX interface with CoreSGMII
 */
#if (MSS_MAC_PHY_INTERFACE == SGMII)

#define CORE_SGMII_PHY_ADDR    MSS_MAC_INTERFACE_MDIO_ADDR
/***************************************************************************//**
 *
 */
static void coresgmii_init(void)
{
    uint16_t phy_reg;
    
    /* Reset C-SGMII. */
    MSS_MAC_write_phy_reg(CORE_SGMII_PHY_ADDR, 0x00, 0x9000u);
    /* Register 0x04 of C-SGMII must be always be set to 0x0001. */
    MSS_MAC_write_phy_reg(CORE_SGMII_PHY_ADDR, 0x04, 0x0001);

    /* Enable auto-negotiation inside CoreSGMII block. */
    phy_reg = MSS_MAC_read_phy_reg(CORE_SGMII_PHY_ADDR, 0x00);
    phy_reg |= 0x1000;
    MSS_MAC_write_phy_reg(CORE_SGMII_PHY_ADDR, 0x00, phy_reg);
}

/***************************************************************************//**
 *
 */ 
static void coresgmii_autonegotiate(void)
{
    uint16_t phy_reg;
    uint16_t autoneg_complete;
    volatile uint32_t sgmii_aneg_timeout = 1000000u;
    
    uint8_t link_fullduplex;
    mss_mac_speed_t link_speed;
    uint8_t copper_link_up;
     
    copper_link_up = MSS_MAC_phy_get_link_status(&link_speed, &link_fullduplex);
    
    if(MSS_MAC_LINK_UP == copper_link_up)
    {
        SYSREG->MAC_CR = (SYSREG->MAC_CR & ~MAC_CONFIG_SPEED_MASK) | link_speed;
        
        /* Configure duplex mode */
        if(MSS_MAC_HALF_DUPLEX == link_fullduplex)
        {
            /* half duplex */
            MAC->CFG2 &= ~CFG2_FDX_MASK;
        }
        else
        {
            /* full duplex */
            MAC->CFG2 |= CFG2_FDX_MASK;
        }  
        /* Initiate auto-negotiation on the SGMII link. */
        phy_reg = MSS_MAC_read_phy_reg(CORE_SGMII_PHY_ADDR, 0x00);
        phy_reg |= 0x1000;
        MSS_MAC_write_phy_reg(CORE_SGMII_PHY_ADDR, 0x00, phy_reg);
        phy_reg |= 0x0200;
        MSS_MAC_write_phy_reg(CORE_SGMII_PHY_ADDR, 0x00, phy_reg);
    
        /* Wait for SGMII auto-negotiation to complete. */
        do {
            phy_reg = MSS_MAC_read_phy_reg(CORE_SGMII_PHY_ADDR, MII_BMSR);
            autoneg_complete = phy_reg & BMSR_AUTO_NEGOTIATION_COMPLETE;
            --sgmii_aneg_timeout;
        } while((!autoneg_complete && sgmii_aneg_timeout != 0u) || (0xFFFF == phy_reg));
    }
}

/***************************************************************************//**
 * Generate clock 2.5/25/125MHz for 10/100/1000Mbps using Clock Condition Circuit(CCC)
 */
static void coresgmii_set_link_speed(uint32_t speed)
{
    uint16_t phy_reg;

    phy_reg = MSS_MAC_read_phy_reg(CORE_SGMII_PHY_ADDR, 0x11u);
    phy_reg |= (speed << 2u);
    MSS_MAC_write_phy_reg(CORE_SGMII_PHY_ADDR, 0x11u, phy_reg);
}

#endif /* #if (MSS_MAC_PHY_INTERFACE == SGMII) */
#endif

/***************************************************************************//**
 * Setup hardware addresses etc for instance structure(s).
 */
static void instances_init(void)
{
#if defined(TARGET_ALOE)
    memset(&g_mac0, 0, sizeof(g_mac0));
    g_mac0.is_emac     = 0;
    g_mac0.mac_base    = (MAC_TypeDef  *)MSS_MAC0_BASE;
    g_mac0.emac_base   = (eMAC_TypeDef *)0;
    g_mac0.mac_q_int   = ethernet_PLIC_53;
#endif
#if defined(TARGET_G5_SOC)
    /**************************************************************************
     * GEM 0 pMAC
     */
    memset(&g_mac0, 0, sizeof(g_mac0));
    g_mac0.is_emac      = 0;
    g_mac0.mac_base     = (MAC_TypeDef  *)MSS_MAC0_BASE;
    g_emac0.emac_base   = (eMAC_TypeDef *)MSS_EMAC0_BASE;
    g_mac0.mac_q_int[0] = MAC0_INT_PLIC;
    g_mac0.mac_q_int[1] = MAC0_QUEUE1_PLIC;
    g_mac0.mac_q_int[2] = MAC0_QUEUE2_PLIC;
    g_mac0.mac_q_int[3] = MAC0_QUEUE3_PLIC;
    g_mac0.mmsl_int     = NoInterrupt_IRQn;
    
    g_mac0.queue[0].int_status = &g_mac0.mac_base->INT_STATUS;
    g_mac0.queue[1].int_status = &g_mac0.mac_base->INT_Q1_STATUS;
    g_mac0.queue[2].int_status = &g_mac0.mac_base->INT_Q2_STATUS;
    g_mac0.queue[3].int_status = &g_mac0.mac_base->INT_Q3_STATUS;

    g_mac0.queue[0].int_mask = &g_mac0.mac_base->INT_MASK;
    g_mac0.queue[1].int_mask = &g_mac0.mac_base->INT_Q1_MASK;
    g_mac0.queue[2].int_mask = &g_mac0.mac_base->INT_Q2_MASK;
    g_mac0.queue[3].int_mask = &g_mac0.mac_base->INT_Q3_MASK;

    g_mac0.queue[0].int_enable = &g_mac0.mac_base->INT_ENABLE;
    g_mac0.queue[1].int_enable = &g_mac0.mac_base->INT_Q1_ENABLE;
    g_mac0.queue[2].int_enable = &g_mac0.mac_base->INT_Q2_ENABLE;
    g_mac0.queue[3].int_enable = &g_mac0.mac_base->INT_Q3_ENABLE;

    g_mac0.queue[0].int_disable = &g_mac0.mac_base->INT_DISABLE;
    g_mac0.queue[1].int_disable = &g_mac0.mac_base->INT_Q1_DISABLE;
    g_mac0.queue[2].int_disable = &g_mac0.mac_base->INT_Q2_DISABLE;
    g_mac0.queue[3].int_disable = &g_mac0.mac_base->INT_Q3_DISABLE;

    g_mac0.queue[0].receive_q_ptr = &g_mac0.mac_base->RECEIVE_Q_PTR;
    g_mac0.queue[1].receive_q_ptr = &g_mac0.mac_base->RECEIVE_Q1_PTR;
    g_mac0.queue[2].receive_q_ptr = &g_mac0.mac_base->RECEIVE_Q2_PTR;
    g_mac0.queue[3].receive_q_ptr = &g_mac0.mac_base->RECEIVE_Q3_PTR;

    g_mac0.queue[0].transmit_q_ptr = &g_mac0.mac_base->TRANSMIT_Q_PTR;
    g_mac0.queue[1].transmit_q_ptr = &g_mac0.mac_base->TRANSMIT_Q1_PTR;
    g_mac0.queue[2].transmit_q_ptr = &g_mac0.mac_base->TRANSMIT_Q2_PTR;
    g_mac0.queue[3].transmit_q_ptr = &g_mac0.mac_base->TRANSMIT_Q3_PTR;

    g_mac0.queue[0].dma_rxbuf_size = &g_mac0.mac_base->DMA_RXBUF_SIZE_Q1; /* Not really true as this is done differently */
    g_mac0.queue[1].dma_rxbuf_size = &g_mac0.mac_base->DMA_RXBUF_SIZE_Q1;
    g_mac0.queue[2].dma_rxbuf_size = &g_mac0.mac_base->DMA_RXBUF_SIZE_Q2;
    g_mac0.queue[3].dma_rxbuf_size = &g_mac0.mac_base->DMA_RXBUF_SIZE_Q3;

    /**************************************************************************
     * GEM 0 eMAC
     */
    memset(&g_emac0, 0, sizeof(g_emac0));
    g_emac0.is_emac      = 1;
    g_emac0.mac_base     = (MAC_TypeDef  *)MSS_MAC0_BASE;
    g_emac0.emac_base    = (eMAC_TypeDef *)MSS_EMAC0_BASE;
    g_emac0.mac_q_int[0] = MAC0_EMAC_PLIC;
    g_emac0.mac_q_int[1] = NoInterrupt_IRQn;
    g_emac0.mac_q_int[2] = NoInterrupt_IRQn;
    g_emac0.mac_q_int[3] = NoInterrupt_IRQn;
    g_emac0.mmsl_int     = MAC0_MMSL_PLIC;

    g_emac0.queue[0].int_status = &g_emac0.emac_base->INT_STATUS;
    g_emac0.queue[1].int_status = &g_emac0.emac_base->INT_STATUS; /* Not Really correct but don't want 0 here... */
    g_emac0.queue[2].int_status = &g_emac0.emac_base->INT_STATUS;
    g_emac0.queue[3].int_status = &g_emac0.emac_base->INT_STATUS;

    g_emac0.queue[0].int_mask = &g_emac0.emac_base->INT_MASK;
    g_emac0.queue[1].int_mask = &g_emac0.emac_base->INT_MASK;
    g_emac0.queue[2].int_mask = &g_emac0.emac_base->INT_MASK;
    g_emac0.queue[3].int_mask = &g_emac0.emac_base->INT_MASK;

    g_emac0.queue[0].int_enable = &g_emac0.emac_base->INT_ENABLE;
    g_emac0.queue[1].int_enable = &g_emac0.emac_base->INT_ENABLE;
    g_emac0.queue[2].int_enable = &g_emac0.emac_base->INT_ENABLE;
    g_emac0.queue[3].int_enable = &g_emac0.emac_base->INT_ENABLE;

    g_emac0.queue[0].int_disable = &g_emac0.emac_base->INT_DISABLE;
    g_emac0.queue[1].int_disable = &g_emac0.emac_base->INT_DISABLE;
    g_emac0.queue[2].int_disable = &g_emac0.emac_base->INT_DISABLE;
    g_emac0.queue[3].int_disable = &g_emac0.emac_base->INT_DISABLE;

    g_emac0.queue[0].receive_q_ptr = &g_emac0.emac_base->RECEIVE_Q_PTR;
    g_emac0.queue[1].receive_q_ptr = &g_emac0.emac_base->RECEIVE_Q_PTR;
    g_emac0.queue[2].receive_q_ptr = &g_emac0.emac_base->RECEIVE_Q_PTR;
    g_emac0.queue[3].receive_q_ptr = &g_emac0.emac_base->RECEIVE_Q_PTR;

    g_emac0.queue[0].transmit_q_ptr = &g_emac0.emac_base->TRANSMIT_Q_PTR;
    g_emac0.queue[1].transmit_q_ptr = &g_emac0.emac_base->TRANSMIT_Q_PTR;
    g_emac0.queue[2].transmit_q_ptr = &g_emac0.emac_base->TRANSMIT_Q_PTR;
    g_emac0.queue[3].transmit_q_ptr = &g_emac0.emac_base->TRANSMIT_Q_PTR;

    g_emac0.queue[0].dma_rxbuf_size = &g_emac0.mac_base->DMA_RXBUF_SIZE_Q1;
    g_emac0.queue[1].dma_rxbuf_size = &g_emac0.mac_base->DMA_RXBUF_SIZE_Q1; /* Not Really correct but don't want 0 here... */
    g_emac0.queue[2].dma_rxbuf_size = &g_emac0.mac_base->DMA_RXBUF_SIZE_Q2;
    g_emac0.queue[3].dma_rxbuf_size = &g_emac0.mac_base->DMA_RXBUF_SIZE_Q3;

    /**************************************************************************
     * GEM 1 pMAC
     */
    memset(&g_mac1, 0, sizeof(g_mac1));
    g_mac1.is_emac      = 0;
    g_mac1.mac_base     = (MAC_TypeDef  *)MSS_MAC1_BASE;
    g_mac1.emac_base    = (eMAC_TypeDef *)MSS_EMAC1_BASE;
    g_mac1.mac_q_int[0] = MAC1_INT_PLIC;
    g_mac1.mac_q_int[1] = MAC1_QUEUE1_PLIC;
    g_mac1.mac_q_int[2] = MAC1_QUEUE2_PLIC;
    g_mac1.mac_q_int[3] = MAC1_QUEUE3_PLIC;
    g_mac1.mmsl_int     = NoInterrupt_IRQn;

    g_mac1.queue[0].int_status = &g_mac1.mac_base->INT_STATUS;
    g_mac1.queue[1].int_status = &g_mac1.mac_base->INT_Q1_STATUS;
    g_mac1.queue[2].int_status = &g_mac1.mac_base->INT_Q2_STATUS;
    g_mac1.queue[3].int_status = &g_mac1.mac_base->INT_Q3_STATUS;

    g_mac1.queue[0].int_mask = &g_mac1.mac_base->INT_MASK;
    g_mac1.queue[1].int_mask = &g_mac1.mac_base->INT_Q1_MASK;
    g_mac1.queue[2].int_mask = &g_mac1.mac_base->INT_Q2_MASK;
    g_mac1.queue[3].int_mask = &g_mac1.mac_base->INT_Q3_MASK;

    g_mac1.queue[0].int_enable = &g_mac1.mac_base->INT_ENABLE;
    g_mac1.queue[1].int_enable = &g_mac1.mac_base->INT_Q1_ENABLE;
    g_mac1.queue[2].int_enable = &g_mac1.mac_base->INT_Q2_ENABLE;
    g_mac1.queue[3].int_enable = &g_mac1.mac_base->INT_Q3_ENABLE;

    g_mac1.queue[0].int_disable = &g_mac1.mac_base->INT_DISABLE;
    g_mac1.queue[1].int_disable = &g_mac1.mac_base->INT_Q1_DISABLE;
    g_mac1.queue[2].int_disable = &g_mac1.mac_base->INT_Q2_DISABLE;
    g_mac1.queue[3].int_disable = &g_mac1.mac_base->INT_Q3_DISABLE;

    g_mac1.queue[0].receive_q_ptr = &g_mac1.mac_base->RECEIVE_Q_PTR;
    g_mac1.queue[1].receive_q_ptr = &g_mac1.mac_base->RECEIVE_Q1_PTR;
    g_mac1.queue[2].receive_q_ptr = &g_mac1.mac_base->RECEIVE_Q2_PTR;
    g_mac1.queue[3].receive_q_ptr = &g_mac1.mac_base->RECEIVE_Q3_PTR;

    g_mac1.queue[0].transmit_q_ptr = &g_mac1.mac_base->TRANSMIT_Q_PTR;
    g_mac1.queue[1].transmit_q_ptr = &g_mac1.mac_base->TRANSMIT_Q1_PTR;
    g_mac1.queue[2].transmit_q_ptr = &g_mac1.mac_base->TRANSMIT_Q2_PTR;
    g_mac1.queue[3].transmit_q_ptr = &g_mac1.mac_base->TRANSMIT_Q3_PTR;

    g_mac1.queue[0].dma_rxbuf_size = &g_mac1.mac_base->DMA_RXBUF_SIZE_Q1; /* Not really true as this is done differently */
    g_mac1.queue[1].dma_rxbuf_size = &g_mac1.mac_base->DMA_RXBUF_SIZE_Q1;
    g_mac1.queue[2].dma_rxbuf_size = &g_mac1.mac_base->DMA_RXBUF_SIZE_Q2;
    g_mac1.queue[3].dma_rxbuf_size = &g_mac1.mac_base->DMA_RXBUF_SIZE_Q3;

    /**************************************************************************
     * GEM 1 eMAC
     */
    memset(&g_emac1, 0, sizeof(g_emac1));
    g_emac1.is_emac      = 1;
    g_emac1.mac_base     = (MAC_TypeDef  *)MSS_MAC1_BASE;
    g_emac1.emac_base    = (eMAC_TypeDef *)MSS_EMAC1_BASE;
    g_emac1.mac_q_int[0] = MAC1_EMAC_PLIC;
    g_emac1.mac_q_int[1] = NoInterrupt_IRQn;
    g_emac1.mac_q_int[2] = NoInterrupt_IRQn;
    g_emac1.mac_q_int[3] = NoInterrupt_IRQn;
    g_emac1.mmsl_int     = MAC1_MMSL_PLIC;

    g_emac1.queue[0].int_status = &g_emac1.emac_base->INT_STATUS;
    g_emac1.queue[1].int_status = &g_emac1.emac_base->INT_STATUS; /* Not Really correct but don't want 0 here... */
    g_emac1.queue[2].int_status = &g_emac1.emac_base->INT_STATUS;
    g_emac1.queue[3].int_status = &g_emac1.emac_base->INT_STATUS;

    g_emac1.queue[0].int_mask = &g_emac1.emac_base->INT_MASK;
    g_emac1.queue[1].int_mask = &g_emac1.emac_base->INT_MASK;
    g_emac1.queue[2].int_mask = &g_emac1.emac_base->INT_MASK;
    g_emac1.queue[3].int_mask = &g_emac1.emac_base->INT_MASK;

    g_emac1.queue[0].int_enable = &g_emac1.emac_base->INT_ENABLE;
    g_emac1.queue[1].int_enable = &g_emac1.emac_base->INT_ENABLE;
    g_emac1.queue[2].int_enable = &g_emac1.emac_base->INT_ENABLE;
    g_emac1.queue[3].int_enable = &g_emac1.emac_base->INT_ENABLE;

    g_emac1.queue[0].int_disable = &g_emac1.emac_base->INT_DISABLE;
    g_emac1.queue[1].int_disable = &g_emac1.emac_base->INT_DISABLE;
    g_emac1.queue[2].int_disable = &g_emac1.emac_base->INT_DISABLE;
    g_emac1.queue[3].int_disable = &g_emac1.emac_base->INT_DISABLE;

    g_emac1.queue[0].receive_q_ptr = &g_emac1.emac_base->RECEIVE_Q_PTR;
    g_emac1.queue[1].receive_q_ptr = &g_emac1.emac_base->RECEIVE_Q_PTR;
    g_emac1.queue[2].receive_q_ptr = &g_emac1.emac_base->RECEIVE_Q_PTR;
    g_emac1.queue[3].receive_q_ptr = &g_emac1.emac_base->RECEIVE_Q_PTR;

    g_emac1.queue[0].transmit_q_ptr = &g_emac1.emac_base->TRANSMIT_Q_PTR;
    g_emac1.queue[1].transmit_q_ptr = &g_emac1.emac_base->TRANSMIT_Q_PTR;
    g_emac1.queue[2].transmit_q_ptr = &g_emac1.emac_base->TRANSMIT_Q_PTR;
    g_emac1.queue[3].transmit_q_ptr = &g_emac1.emac_base->TRANSMIT_Q_PTR;

    g_emac1.queue[0].dma_rxbuf_size = &g_emac1.mac_base->DMA_RXBUF_SIZE_Q1;
    g_emac1.queue[1].dma_rxbuf_size = &g_emac1.mac_base->DMA_RXBUF_SIZE_Q1; /* Not Really correct but don't want 0 here... */
    g_emac1.queue[2].dma_rxbuf_size = &g_emac1.mac_base->DMA_RXBUF_SIZE_Q2;
    g_emac1.queue[3].dma_rxbuf_size = &g_emac1.mac_base->DMA_RXBUF_SIZE_Q3;

#endif
    instances_init_done = 1;
}

#ifdef __cplusplus
}
#endif

/******************************** END OF FILE ******************************/
