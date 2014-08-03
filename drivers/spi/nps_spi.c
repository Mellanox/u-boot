/*
 * EZchip NPS SPI driver
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "nps_spi.h"
#include <malloc.h>
#include <asm/io.h>

static inline struct nps_spi_slave *to_nps_spi(struct spi_slave *slave)
{
	return container_of(slave, struct nps_spi_slave, slave);
}

static void nps_spi_send_recv(int byteLen, u8 *txd, u8 *rxd,
				struct nps_spi_regs *regs)
{
	int k = byteLen;
	int n;
	u32 tmp;
	u32 *fifo = &regs->dr[0];
	u32 *status = &regs->sr;

	if (txd != NULL) {
		for (n = 0 ; n < k ; n++) {
			tmp = txd[n];
			out_be32(fifo, tmp);
		}
	} else {
		tmp = 0;
		for (n = 0 ; n < k ; n++) {
			/* only to create clocks */
			out_be32(fifo, tmp);
		}
	}

	if (rxd != NULL) {
		for (n = 0 ; n < k ; n++ , rxd++) {
			while ((in_be32(status) & NPS_SPI_SR_RFNE) == 0)
				;
			tmp = in_be32(fifo);
			*rxd = (u8)tmp;
		}
	} else {
		for (n = 0 ; n < k ; n++, rxd++) {
			while ((in_be32(status) & NPS_SPI_SR_RFNE) == 0)
				;
			/* only to clean FIFO */
			tmp = in_be32(fifo);
		}
	}
}

static int nps_spi_calc_baudrate(u32 max_hz)
{
	int baudrate;

	baudrate = DIV_ROUND_UP(CONFIG_SYS_CLK_FREQ, max_hz);

	/* Baudrate must be an even value */
	if (baudrate % 2) {
//		if (((long long)baudrate -
//			((long long)CONFIG_SYS_CLK_FREQ / (long long)max_hz)) > 0)
//			baudrate--;
//		else
			baudrate++;
	}

	/* Check min and max value */
	if (baudrate < NPS_SPI_MIN_BAUDRATE)
		baudrate = NPS_SPI_MIN_BAUDRATE;
	else if (baudrate > NPS_SPI_MAX_BAUDRATE)
		baudrate = NPS_SPI_MAX_BAUDRATE;

	return baudrate;
}

void spi_init(void)
{

}

struct spi_slave *spi_setup_slave(u32 bus, u32 cs, u32 max_hz, u32 mode)
{
	struct nps_spi_slave *ds;
	struct nps_spi_regs *regs;

	debug("%s: bus %u, CS %u, MAX_HZ %u, mode 0x%X\n",
		__func__, bus, cs, max_hz, mode);

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	ds = (struct nps_spi_slave *)calloc(1, sizeof(struct nps_spi_slave));
	if (!ds)
		return NULL;

	ds->slave.cs = cs;
	ds->slave.bus = bus;
	ds->slave.wordlen = SPI_DEFAULT_WORDLEN;

	ds->baudr = nps_spi_calc_baudrate(max_hz);
	ds->base = CONFIG_NPS_SPI_BASE;
	ds->mode = mode; /* Motorola mode */
	ds->txthrs = 0;
	ds->rxthrs = 0;

	/* Disable chip select */
	regs = (struct nps_spi_regs *)CONFIG_NPS_SPI_BASE;
#ifdef NPS_SPI_RSVD_2_ADDRESS
	out_be32(NPS_SPI_RSVD_2_ADDRESS, NPS_SPI_CS_FLASH_DONE);
#else
	out_be32(&regs->rsvd_2, NPS_SPI_CS_FLASH_DONE);
#endif

	return &ds->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct nps_spi_slave *ds = to_nps_spi(slave);

	free(ds);
}

int spi_claim_bus(struct spi_slave *slave)
{
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{

}

/*
 * SPI transfer
 *
 * This writes "bitlen" bits out the SPI MOSI port and simultaneously clocks
 * "bitlen" bits in the SPI MISO port.  That's just the way SPI works.
 *
 * The source of the outgoing bits is the "dout" parameter and the
 * destination of the input bits is the "din" parameter.  Note that "dout"
 * and "din" can point to the same memory location, in which case the
 * input data overwrites the output data (since both are buffered by
 * temporary variables, this is OK).
 */
int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
	const void *dout, void *din, unsigned long flags)
{
	struct nps_spi_slave *ss;
	struct nps_spi_regs *regs;
	u8 *txd = (u8 *)dout;
	u8 *rxd = (u8 *)din;
	u32 i = 0;
	int byteLen = bitlen / NPS_SPI_DATA_FRAME_SIZE;

	debug("%s: slave %u:%u, dout 0x%08X, din 0x%08X, bitlen %u, flags 0x%X\n",
		__func__,
		slave->bus,
		slave->cs,
		(txd == NULL) ? -1 : *(uint *)txd,
		(int)rxd,
		bitlen,
		(int)flags);

	ss = to_nps_spi(slave);
	regs = (struct nps_spi_regs *)ss->base;

	debug("%s: checking if the bus is busy\n", __func__);

	while (in_be32(&regs->sr) & (NPS_SPI_SR_BUSY)) {
		if (i > NPS_SPI_WAIT_FOR_BUSY_RETRY) {
			printf("%s: Bus busy timeout\n", __func__);
			return -1;
		}
		i++;
		udelay(5);
	}

	debug("%s: bus is not busy\n", __func__);

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	debug("%s: starting transfer\n", __func__);

	while (byteLen > 0) {
		i = NPS_SPI_FIFO_SIZE;

		if (i > byteLen)
			i = byteLen;

		nps_spi_send_recv(i, txd, rxd, regs);

		if (txd != NULL)
			txd += i;

		if (rxd != NULL)
			rxd += i;

		byteLen -= i;
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return 0;
}

int spi_cs_is_valid(u32 bus, u32 cs)
{
	return ((bus == 0) && (cs < 4));
}

void spi_cs_activate(struct spi_slave *slave)
{
	struct nps_spi_slave *ss = to_nps_spi(slave);
	struct nps_spi_regs *regs = (struct nps_spi_regs *)ss->base;
	struct nps_spi_ctrlr0 ctrlr0;
	u32 mask = (1 << slave->cs) & 0xFFFF;

	/* Disable SSI to setup transfer parameters */
	out_be32(&regs->ssienr, 0);

	/* Enable slave */
	out_be32(&regs->ser, mask);

	ctrlr0.value = in_be32(&regs->ctrlr0);

	/* Set serial clock polarity */
	if (ss->mode & SPI_CPOL)
		ctrlr0.scpol = 1;
	else
		ctrlr0.scpol = 0;

	/* Set serial clock phase */
	if (ss->mode & SPI_CPHA)
		ctrlr0.scph = 1;
	else
		ctrlr0.scph = 0;

	/* Set Data Frame Size */
	ctrlr0.dfs = NPS_SPI_DATA_FRAME_SIZE - 1;

	/* Set transmit mode = Transmit & Receive */
	ctrlr0.tmod = 0;

	/* Set frame format = Motorola SPI 01 */
	ctrlr0.frf = 0;

	out_be32(&regs->ctrlr0, ctrlr0.value);

	/* Set baudrate */
	out_be32(&regs->baudr, ss->baudr);
	out_be32(&regs->ctrlr1, 0xF);

	/* Set FIFO thresholds */
	out_be32(&regs->txftlr, ss->txthrs);
	out_be32(&regs->rxftlr, ss->rxthrs);
	out_be32(&regs->ssienr, 1);

	/* Set chip select */
#ifdef NPS_SPI_RSVD_2_ADDRESS
	out_be32(NPS_SPI_RSVD_2_ADDRESS, 0);
#else
	out_be32(&regs->rsvd_2, 0);
#endif
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	struct nps_spi_slave *ss = to_nps_spi(slave);
	struct nps_spi_regs *regs = (struct nps_spi_regs *)ss->base;

	/* Disable SSI to setup transfer parameters */
	out_be32(&regs->ssienr, 0);

	/* Disable slave */
	out_be32(&regs->ser, 0);

	/* Disable chip select */
#ifdef NPS_SPI_RSVD_2_ADDRESS
	out_be32(NPS_SPI_RSVD_2_ADDRESS, NPS_SPI_CS_FLASH_DONE);
#else
	out_be32(&regs->rsvd_2, NPS_SPI_CS_FLASH_DONE);
#endif
}

