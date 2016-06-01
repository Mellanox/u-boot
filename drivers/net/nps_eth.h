/*
 * Register definitions for the NPS LAN driver
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <net.h>

#define NPS_ETH_NAME "nps_eth"
#define NPS_ETH_ZLEN			60
#define NPS_ETH_MAX_RX_FRAME_LENGTH	1518
#define NPS_ETH_MAX_TX_FRAME_LENGTH	0x7FF
#define NPS_ETH_WATCHDOG_CNTR		100000
#define NPS_ETH_WORDS_NUM(x)		((x + 3) >> 2)

/* dbg_lan register values */
#define NPS_ETH_ENABLE				1
#define NPS_ETH_GE_MAC_CFG_0_TX_FC_RETR		0x7
#define NPS_ETH_GE_MAC_CFG_0_RX_IFG		0x5
#define NPS_ETH_GE_MAC_CFG_0_TX_IFG		0xC
#define NPS_ETH_GE_MAC_CFG_0_TX_PR_LEN		0x7
#define NPS_ETH_GE_MAC_CFG_2_STAT_EN		0x3
#define NPS_ETH_GE_MAC_CFG_3_RX_IFG_TH		0x14

/* dbg_lan registers */
#ifdef CONFIG_NPS_DBG_LAN_WEST
#define NPS_ETH_DBG_LAN_BASE		0xF7070000
#else
#define NPS_ETH_DBG_LAN_BASE		0xF7470000
#endif
#define NPS_ETH_DBG_LAN_TX_CTL		(u32 *)(NPS_ETH_DBG_LAN_BASE + 0x800)
#define NPS_ETH_DBG_LAN_TX_BUF		(u32 *)(NPS_ETH_DBG_LAN_BASE + 0x808)
#define NPS_ETH_DBG_LAN_RX_CTL		(u32 *)(NPS_ETH_DBG_LAN_BASE + 0x810)
#define NPS_ETH_DBG_LAN_RX_BUF		(u32 *)(NPS_ETH_DBG_LAN_BASE + 0x818)
#define NPS_ETH_DBG_LAN_GE_MAC_CFG_0	(u32 *)(NPS_ETH_DBG_LAN_BASE + 0x1000)
#define NPS_ETH_DBG_LAN_GE_MAC_CFG_1	(u32 *)(NPS_ETH_DBG_LAN_BASE + 0x1004)
#define NPS_ETH_DBG_LAN_GE_MAC_CFG_2	(u32 *)(NPS_ETH_DBG_LAN_BASE + 0x1008)
#define NPS_ETH_DBG_LAN_GE_MAC_CFG_3	(u32 *)(NPS_ETH_DBG_LAN_BASE + 0x100C)
#define NPS_ETH_DBG_LAN_GE_RST		(u32 *)(NPS_ETH_DBG_LAN_BASE + 0x1400)
#define NPS_ETH_DBG_LAN_PHASE_FIFO_CTL	(u32 *)(NPS_ETH_DBG_LAN_BASE + 0x1404)

/*
 * Tx control register
 */
struct nps_eth_tx_ctl {
	union {
		/*
		 * ct: SW sets to indicate frame ready in Tx buffer for
		 *     transmission. HW resets to when transmission done
		 * et: Transmit error
		 * nt: Length in bytes of Tx frame loaded to Tx buffer
		 */
		struct {
			u32
			__reserved1:16,
			ct:1,
			et:1,
			__reserved2:3,
			nt:11;
		};

		u32 value;
	};
};

/*
 * Rx control register
 */
struct nps_eth_rx_ctl {
	union {
		/*
		 * cr:  HW sets to indicate frame ready in Rx buffer.
		 *      SW resets to indicate host read received frame
		 *      and new frames can be written to Rx buffer
		 * er:  Rx error indication
		 * crc: Rx CRC error indication
		 * nr:  Length in bytes of Rx frame loaded by MAC to Rx buffer
		 */
		struct {
			u32
			__reserved1:16,
			cr:1,
			er:1,
			crc:1,
			__reserved2:2,
			nr:11;
		};

		u32 value;
	};
};

/*
 * Gbps Eth MAC Configuration 0 register
 */
struct nps_eth_ge_mac_cfg_0 {
	union {
		/*
		 * tx_pr_len:          Transmit preamble length in bytes
		 * tx_ifg_nib:         Tx idle pattern
		 * nib_mode:           Nibble (4-bit) Mode
		 * rx_pr_check_en:     Receive preamble Check Enable
		 * tx_ifg:             Transmit inter-Frame Gap
		 * rx_ifg:             Receive inter-Frame Gap
		 * tx_fc_retr:         Transmit Flow Control Retransmit Mode
		 * rx_length_check_en: Receive Length Check Enable
		 * rx_crc_ignore:      Results of the CRC check are ignored
		 * rx_crc_strip:       MAC strips the CRC from received frames
		 * rx_fc_en:           Receive Flow Control Enable
		 * tx_crc_en:          Transmit CRC Enabled
		 * tx_pad_en:          Transmit Padding Enable
		 * tx_cf_en:           Transmit Flow Control Enable
		 * tx_en:              Transmit Enable
		 * rx_en:              Receive Enable
		 */
		struct {
			u32
			tx_pr_len:4,
			tx_ifg_nib:4,
			nib_mode:1,
			rx_pr_check_en:1,
			tx_ifg:6,
			rx_ifg:4,
			tx_fc_retr:3,
			rx_length_check_en:1,
			rx_crc_ignore:1,
			rx_crc_strip:1,
			rx_fc_en:1,
			tx_crc_en:1,
			tx_pad_en:1,
			tx_fc_en:1,
			tx_en:1,
			rx_en:1;
		};

		u32 value;
	};
};

/*
 * Gbps Eth MAC Configuration 1 register
 */
struct nps_eth_ge_mac_cfg_1 {
	union {
		/*
		 * octet_3: MAC address octet 3
		 * octet_2: MAC address octet 2
		 * octet_1: MAC address octet 1
		 * octet_0: MAC address octet 0
		 */
		struct {
			u32
			octet_3:8,
			octet_2:8,
			octet_1:8,
			octet_0:8;
		};

		u32 value;
	};
};

/*
 * Gbps Eth MAC Configuration 2 register
 */
struct nps_eth_ge_mac_cfg_2 {
	union {
		/*
		 * transmit_flush_en: MAC flush enable
		 * stat_en:           RMON statistics interface enable
		 * disc_da:           Discard frames with DA different
		 *                    from MAC address
		 * disc_bc:           Discard broadcast frames
		 * disc_mc:           Discard multicast frames
		 * octet_5:           MAC address octet 5
		 * octet_4:           MAC address octet 4
		 */
		struct {
			u32
			transmit_flush_en:1,
			__reserved_1:5,
			stat_en:2,
			__reserved_2:1,
			disc_da:1,
			disc_bc:1,
			disc_mc:1,
			__reserved_3:4,
			octet_5:8,
			octet_4:8;
		};

		u32 value;
	};
};

/*
 * Gbps Eth MAC Configuration 3 register
 */
struct nps_eth_ge_mac_cfg_3 {
	union {
		/*
		 * ext_oob_cbfc_sel:  Selects one of the 4 profiles for
		 *                    extended OOB in-flow-control indication
		 * max_len:           Maximum receive frame length in bytes
		 * tx_cbfc_en:        Enable transmission of class-based
		 *                    flow control packets
		 * rx_ifg_th:         Threshold for IFG status reporting via OOB
		 * cf_timeout:        Configurable time to decrement FC counters
		 * cf_drop:           Drop control frames
		 * redirect_cbfc_sel: Selects one of CBFC redirect profiles
		 * rx_cbfc_redir_en:  Enable Rx class-based flow
		 *                    control redirect
		 * rx_cbfc_en:        Enable Rx class-based flow control
		 * tm_hd_mode:        TM header mode
		 */
		struct {
			u32
			ext_oob_cbfc_sel:2,
			max_len:14,
			tx_cbfc_en:1,
			rx_ifg_th:5,
			cf_timeout:4,
			cf_drop:1,
			redirect_cbfc_sel:2,
			rx_cbfc_redir_en:1,
			rx_cbfc_en:1,
			tm_hd_mode:1;
		};

		u32 value;
	};
};

/*
 * GE MAC, PCS reset control register
 */
struct nps_eth_ge_rst {
	union {
		/*
		 * gmac_0: GE MAC reset
		 * spcs_0: SGMII PCS reset
		 */
		struct {
			u32
			__reserved1:23,
			gmac_0:1,
			__reserved2:7,
			spcs_0:1;
		};

		u32 value;
	};
};

/*
 * Tx phase sync FIFO control register
 */
struct nps_eth_phase_fifo_ctl {
	union {
		/*
		 * init: initialize serdes TX phase sync FIFO pointers
		 * rst:  reset serdes TX phase sync FIFO
		 */
		struct {
			u32
			__reserved:30,
			init:1,
			rst:1;
		};

		u32 value;
	};
};

struct nps_eth_priv {
	struct nps_eth_ge_mac_cfg_2 ge_mac_cfg_2;
};

