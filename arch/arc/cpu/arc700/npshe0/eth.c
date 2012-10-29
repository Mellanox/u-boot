

#include <asm/u-boot.h>		/* Board data structure	*/
#include <config.h>		/* ARC700 clock freq.	*/
#include <asm/errno.h>
#include <malloc.h>
#include <common.h>
#include <net.h>
#include <linux/types.h>
#include "iomap.h"

/*=============================*/

#define TX_LAN_BUSY     0x00008000
#define TX_LAN_ERROR    0x00004000
#define TX_LAN_RESET    0x00000800

#define RX_LAN_BUSY     0x00008000
#define RX_LAN_ERROR    0x00004000
#define RX_LAN_CRC      0x00002000
#define RX_LAN_RESET    0x00000800

#define FRAME_SIZE_MASK 0x000007FF
#define MAC_INIT_VALUE  0x70C8501F

#define LAN_BUFFER_SIZE 0x600
#define MAX_BUFFERS     8

#define ETH_ZLEN 60
#define ETH_MTU	1500

struct npsEth_priv {
	unsigned char rx_buffers[MAX_BUFFERS][LAN_BUFFER_SIZE];
	unsigned char tx_buff[ETH_ZLEN];
	int curr_rx;
	int tx_error;
	int lan_rx_error;
	int lan_rx_crc;
	int lan_rx_small;
};

#define NPSETHER_NAME "npshe_eth"
/*=============================*/


static void inline lan_hw_disable(void)
{
    REG_GEMAC_TX_CTL |= TX_LAN_RESET;
    REG_GEMAC_RX_CTL |= RX_LAN_RESET;
}

static void inline lan_hw_enable(void)
{
    REG_GEMAC_TX_CTL &= ~(TX_LAN_RESET);
    REG_GEMAC_RX_CTL &= ~(RX_LAN_RESET);
}

static int arc_eth_reset(struct eth_device *net_current ,bd_t *bd)
{
    lan_hw_disable();
    lan_hw_enable();
    return 0;
}

static void arc_eth_halt(struct eth_device * net_current)
{
	lan_hw_disable();
}

static int arc_eth_send (struct eth_device *net_current, volatile void *packet, int length)
{
    int k;
    int i;
    struct npsEth_priv *npsEth_private = net_current->priv;
    int *src = (int *)packet;

	if (!packet || !net_current) {
		printf(NPSETHER_NAME ": %s: Invalid argument\n", __func__);
		return -EINVAL;
	}

	/* packet must be a 4 byte boundary */
	if ((int)packet & (4 - 1)) {
		printf(NPSETHER_NAME ": %s: packet not 4 byte alligned\n", __func__);
		return -EFAULT;
	}

	if (length > ETH_MTU) {
	     printf(NPSETHER_NAME ": %s: packet length greater than MTU\n", __func__);
	     return -EMSGSIZE;
	}

	if ( length < ETH_ZLEN ) {
		memcpy(npsEth_private->tx_buff,src,length);
		src = (int *)npsEth_private->tx_buff;
		while (length < ETH_ZLEN)
		   npsEth_private->tx_buff[length++] = '\0';
	}

    k = (length+3)>>2;

    /* Wait for LAN to finish sending */
    while ( (REG_GEMAC_TX_CTL & TX_LAN_BUSY) != 0 );

    /* Clean Tx error */
    if ( (REG_GEMAC_TX_CTL & TX_LAN_ERROR) != 0 ) {
        REG_GEMAC_TX_CTL &= ~(TX_LAN_ERROR);
        npsEth_private->tx_error++;
    }

    /* Copy frame to Tx FIFO buffer */
    for (i = 0; i < k; i++)
    	REG_GEMAC_TXBUF_DATA = src[i];

    REG_GEMAC_TX_CTL = (REG_GEMAC_TX_CTL & ~(FRAME_SIZE_MASK)) | length;

    /* Send Frame */
    REG_GEMAC_TX_CTL |= TX_LAN_BUSY;
    return 0;
}

static void *getEthBuf(struct npsEth_priv *npsEth_private)
{
    void *retValue = (void *)npsEth_private->rx_buffers[npsEth_private->curr_rx];
    npsEth_private->curr_rx++;
    if ( npsEth_private->curr_rx >= MAX_BUFFERS ) {
    	npsEth_private->curr_rx = 0;
    }
    return retValue;
}

static void clean_rx_fifo(int length)
{
	int i;
	int buff;
	for (i = 0; i < length; i++)
		buff = REG_GEMAC_RXBUF_DATA;
}

static void read_rx_fifo(int *buff, int length)
{
	int i;
	for (i = 0; i < length; i++)
		buff[i] = REG_GEMAC_RXBUF_DATA;
}

static int arc_eth_rx (struct eth_device * net_current)
{
    int *recv_data;
    int length;
    int k;
    struct npsEth_priv *npsEth_private;

	if (!net_current) {
		printf(NPSETHER_NAME ": %s: Invalid argument\n", __func__);
		return -EINVAL;
	}
	npsEth_private = net_current->priv;

    int curRxCtl = REG_GEMAC_RX_CTL;

    /* as long as there is a Frame in Rx */
    while ( ( curRxCtl & RX_LAN_BUSY ) != 0 ) {
        length = curRxCtl & FRAME_SIZE_MASK;
        k = (length + 3) >> 2;

        /* Check Rx length */
        if (length < ETH_ZLEN) {
            k++;
            npsEth_private->lan_rx_small++;
            goto arc_eth_rx_check;
        }

        /* Check Rx error */
        if ( ( curRxCtl & RX_LAN_ERROR ) != 0 ) {
            npsEth_private->lan_rx_error++;
            goto arc_eth_rx_check;
        }

        /* Check Rx crc error */
        if ( ( curRxCtl & RX_LAN_CRC ) != 0 ) {
        	npsEth_private->lan_rx_crc++;
            goto arc_eth_rx_check;
        }
        /* get Buffer */
        recv_data = (int *)getEthBuf(npsEth_private);

        read_rx_fifo(recv_data,k);

        REG_GEMAC_RX_CTL = 0;

        NetReceive((uchar *)recv_data,length);
        curRxCtl = REG_GEMAC_RX_CTL;
        continue;
arc_eth_rx_check:
		clean_rx_fifo(k);
        REG_GEMAC_RX_CTL = 0;
        curRxCtl = REG_GEMAC_RX_CTL;
    }
    return 0;
}

int board_eth_init(bd_t *bd)
{
    struct eth_device *dev;
    struct npsEth_priv *priv;

    dev = (struct eth_device *) malloc(sizeof(*dev));
    memset(dev, 0, sizeof(*dev));
    priv = (struct npsEth_priv*) malloc(sizeof(*priv));
    memset(priv, 0, sizeof(*priv));
    sprintf(dev->name,NPSETHER_NAME);

    dev->priv = priv;
    dev->init = arc_eth_reset;
    dev->halt = arc_eth_halt;
    dev->send = arc_eth_send;
    dev->recv = arc_eth_rx;

    /* initialize GEMAC register */
    REG_GEMAC_MAC_CFG = MAC_INIT_VALUE;
    REG_GEMAC_RX_CTL = 0;
    lan_hw_disable();

    eth_register(dev);

    return 0;
}


