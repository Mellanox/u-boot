/*
 * EZchip NPS LAN driver
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/errno.h>
#include <asm/io.h>
#include <malloc.h>
#include "nps_eth.h"
#include <asm/arcregs.h>

static int nps_eth_init(struct eth_device *dev, bd_t *bd)
{
	struct nps_eth_priv *priv = dev->priv;
	struct nps_eth_ge_rst ge_rst = {.value = 0};
	struct nps_eth_phase_fifo_ctl phase_fifo_ctl = {.value = 0};
	struct nps_eth_ge_mac_cfg_2 *ge_mac_cfg_2 = &priv->ge_mac_cfg_2;
	struct nps_eth_ge_mac_cfg_3 ge_mac_cfg_3 = {.value = 0};
	struct nps_eth_ge_mac_cfg_0 ge_mac_cfg_0 = {.value = 0};

	/* pcs reset sequence*/
	ge_rst.gmac_0 = NPS_ETH_ENABLE;
	out_be32(NPS_ETH_DBG_LAN_GE_RST, ge_rst.value);
	udelay(10);
	ge_rst.value = 0;
	out_be32(NPS_ETH_DBG_LAN_GE_RST, ge_rst.value);

	/* tx fifo reset sequence */
	phase_fifo_ctl.rst = NPS_ETH_ENABLE;
	phase_fifo_ctl.init = NPS_ETH_ENABLE;
	out_be32(NPS_ETH_DBG_LAN_PHASE_FIFO_CTL, phase_fifo_ctl.value);
	udelay(10);
	phase_fifo_ctl.value = 0;
	out_be32(NPS_ETH_DBG_LAN_PHASE_FIFO_CTL, phase_fifo_ctl.value);

	/* enable statistics */
	ge_mac_cfg_2->stat_en = NPS_ETH_GE_MAC_CFG_2_STAT_EN;

	/* enable flow control frames */
	ge_mac_cfg_0.tx_fc_en = NPS_ETH_ENABLE;
	ge_mac_cfg_0.rx_fc_en = NPS_ETH_ENABLE;
	ge_mac_cfg_0.tx_fc_retr = NPS_ETH_GE_MAC_CFG_0_TX_FC_RETR;
	ge_mac_cfg_3.cf_drop = NPS_ETH_ENABLE;

	/* IFG configuration */
	ge_mac_cfg_0.rx_ifg = NPS_ETH_GE_MAC_CFG_0_RX_IFG;
	ge_mac_cfg_0.tx_ifg = NPS_ETH_GE_MAC_CFG_0_TX_IFG;
	ge_mac_cfg_3.rx_ifg_th = NPS_ETH_GE_MAC_CFG_3_RX_IFG_TH;

	/* Rx and Tx HW features */
	ge_mac_cfg_0.tx_pad_en = NPS_ETH_ENABLE;
	ge_mac_cfg_0.tx_crc_en = NPS_ETH_ENABLE;
	ge_mac_cfg_0.rx_crc_strip = NPS_ETH_ENABLE;
	ge_mac_cfg_2->disc_mc = NPS_ETH_ENABLE;
	ge_mac_cfg_3.max_len = NPS_ETH_MAX_RX_FRAME_LENGTH;

	/* preamble configuration */
	ge_mac_cfg_0.rx_pr_check_en = NPS_ETH_ENABLE;
	ge_mac_cfg_0.tx_pr_len = NPS_ETH_GE_MAC_CFG_0_TX_PR_LEN;

	/* enable Rx and Tx */
	ge_mac_cfg_0.rx_en = NPS_ETH_ENABLE;
	ge_mac_cfg_0.tx_en = NPS_ETH_ENABLE;

	out_be32(NPS_ETH_DBG_LAN_GE_MAC_CFG_2, ge_mac_cfg_2->value);
	out_be32(NPS_ETH_DBG_LAN_GE_MAC_CFG_3, ge_mac_cfg_3.value);
	out_be32(NPS_ETH_DBG_LAN_GE_MAC_CFG_0, ge_mac_cfg_0.value);

	return 0;
}

static void nps_eth_halt(struct eth_device *dev)
{
	/* disable Rx and Tx */
	out_be32(NPS_ETH_DBG_LAN_GE_MAC_CFG_0, 0);
}

static int nps_eth_send(struct eth_device *dev,
			void *packet, int length)
{
	int i, k, counter = 0;
	struct nps_eth_tx_ctl tx_ctl = { .value = 0 };

	/* check frame length */
	if (length > NPS_ETH_MAX_TX_FRAME_LENGTH) {
		debug("%s: %s: packet length greater than max Tx length\n",
			dev->name,
			__func__);
		return -EMSGSIZE;
	}

	/* copy frame to Tx FIFO buffer */
	k = NPS_ETH_WORDS_NUM(length);
	for (i = 0; i < k; i++)
		out_be32(NPS_ETH_DBG_LAN_TX_BUF, ((u32 *)packet)[i]);

	/* send Frame */
	tx_ctl.nt = length;
	tx_ctl.ct = NPS_ETH_ENABLE;
	out_be32(NPS_ETH_DBG_LAN_TX_CTL, tx_ctl.value);

	/* wait for LAN to finish sending */
	do {
		tx_ctl.value = in_be32(NPS_ETH_DBG_LAN_TX_CTL);
		if (counter++ > NPS_ETH_WATCHDOG_CNTR) {
			out_be32(NPS_ETH_DBG_LAN_TX_CTL, 0);
			debug("%s: %s: Tx send timeout\n",
				dev->name,
				__func__);
			return -ETIME;
		}
	} while (tx_ctl.ct);

	/* check Tx error */
	if (tx_ctl.et) {
		debug("%s: %s: Tx error\n", dev->name,  __func__);
		return -ENETDOWN;
	}

	return 0;
}

static int nps_eth_rx(struct eth_device *dev)
{
	int i, k;
	int curr_rx_buf = 0, length;
	u32 *rx_buf;
	struct nps_eth_rx_ctl rx_ctl;

	rx_ctl.value = in_be32(NPS_ETH_DBG_LAN_RX_CTL);

	/* as long as there is a packet in Rx */
	while (rx_ctl.cr) {
		length = rx_ctl.nr;
		k = NPS_ETH_WORDS_NUM(length);

		/* check Rx error */
		if (rx_ctl.er) {
			debug("%s: %s: rx error\n", dev->name, __func__);
			goto nps_eth_rx_clean;
		}

		/* check Rx length */
		if (length < NPS_ETH_ZLEN) {
			debug("%s: %s: rx length error\n",
				dev->name,
				__func__);
			goto nps_eth_rx_clean;
		}

		/* check Rx CRC error */
		if (rx_ctl.crc) {
			debug("%s: %s: rx CRC error\n", dev->name, __func__);
			goto nps_eth_rx_clean;
		}

		/* read Rx Buffer */
		rx_buf = (u32 *)NetRxPackets[curr_rx_buf];
		for (i = 0; i < k; i++)
			rx_buf[i] = in_be32(NPS_ETH_DBG_LAN_RX_BUF);

		/* read finished */
		out_be32(NPS_ETH_DBG_LAN_RX_CTL, 0);

		NetReceive(NetRxPackets[curr_rx_buf], length);
		if (++curr_rx_buf >= PKTBUFSRX)
			curr_rx_buf = 0;

		rx_ctl.value = in_be32(NPS_ETH_DBG_LAN_RX_CTL);

		continue;
nps_eth_rx_clean:
		for (i = 0; i < k; i++)
			in_be32(NPS_ETH_DBG_LAN_RX_BUF);

		out_be32(NPS_ETH_DBG_LAN_RX_CTL, 0);
		rx_ctl.value = in_be32(NPS_ETH_DBG_LAN_RX_CTL);
	}

	return 0;
}

static int nps_eth_write_hwaddr(struct eth_device *dev)
{
	struct nps_eth_priv *priv = dev->priv;
	struct nps_eth_ge_mac_cfg_1 ge_mac_cfg_1;
	struct nps_eth_ge_mac_cfg_2 *ge_mac_cfg_2 = &priv->ge_mac_cfg_2;

	/* set MAC address */
	ge_mac_cfg_1.octet_0 = dev->enetaddr[0];
	ge_mac_cfg_1.octet_1 = dev->enetaddr[1];
	ge_mac_cfg_1.octet_2 = dev->enetaddr[2];
	ge_mac_cfg_1.octet_3 = dev->enetaddr[3];
	ge_mac_cfg_2->octet_4 = dev->enetaddr[4];
	ge_mac_cfg_2->octet_5 = dev->enetaddr[5];

	/* discard packets with different MAC address */
	ge_mac_cfg_2->disc_da = NPS_ETH_ENABLE;

	out_be32(NPS_ETH_DBG_LAN_GE_MAC_CFG_1, ge_mac_cfg_1.value);
	out_be32(NPS_ETH_DBG_LAN_GE_MAC_CFG_2, ge_mac_cfg_2->value);

	return 0;
}

int nps_eth_initialize(void)
{
	struct eth_device *dev;
	struct nps_eth_priv *priv;

	dbg_lan_base = is_east_dgb_lan() ? 
		NPS_ETH_EAST_DBG_LAN_BLOCK_ADDR:NPS_ETH_WEST_DBG_LAN_BLOCK_ADDR;
	dev = (struct eth_device *)malloc(sizeof(*dev));
	if (!dev)
		return 0;
	priv = (struct nps_eth_priv *)malloc(sizeof(*priv));
	if (!priv) {
		free(dev);
		return 0;
	}

	memset(dev, 0, sizeof(*dev));
	memset(priv, 0, sizeof(*priv));

	sprintf(dev->name, NPS_ETH_NAME);

	dev->priv = priv;
	dev->init = nps_eth_init;
	dev->halt = nps_eth_halt;
	dev->send = nps_eth_send;
	dev->recv = nps_eth_rx;
	dev->write_hwaddr = nps_eth_write_hwaddr;

	eth_register(dev);

	return 1;
}

