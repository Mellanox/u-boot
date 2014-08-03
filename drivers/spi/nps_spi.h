/*
 * Register definitions for the NPS SPI Controller
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _NPS_SPI_H_
#define _NPS_SPI_H_

#include <common.h>
#include <spi.h>

/* SPI defines */
#define NPS_SPI_MIN_BAUDRATE		2
#define NPS_SPI_MAX_BAUDRATE		65534
#define NPS_SPI_CS_FLASH_DONE		1
#define NPS_SPI_FIFO_SIZE		16
#define	NPS_SPI_DATA_FRAME_SIZE		8
#define	NPS_SPI_WAIT_FOR_BUSY_RETRY	1000

/* SR register fields */
#define NPS_SPI_SR_RFNE	(1 << 3)
#define NPS_SPI_SR_BUSY	(1 << 0)

struct nps_spi_slave {
	struct spi_slave slave;
	u32 base;
	u32 baudr;
	u32 mode;
	u32 txthrs;
	u32 rxthrs;
};

struct nps_spi_ctrlr0 {
	union {
		struct {
			u32
				__reserved:16,
				cfs:4,
				srl:1,
				slv_oe:1,
				tmod:2,
				scpol:1,
				scph:1,
				frf:2,
				dfs:4;
		};

		u32 value;
	};
};

/* SPI controller registers */
struct nps_spi_regs {
	struct nps_spi_ctrlr0 ctrlr0;
	u32	ctrlr1;
	u32	ssienr;
	u32	mwcr;
	u32	ser;
	u32	baudr;
	u32	txftlr;
	u32	rxftlr;
	u32	txflr;
	u32	rxflr;
	u32	sr;
	u32	imr;
	u32	isr;
	u32	risr;
	u32	txoicr;
	u32	rxoicr;
	u32	rxuicr;
	u32	msticr;
	u32	icr;
	u32	dmacr;
	u32	dmatdlr;
	u32	dmardlr;
	u32	idr;
	u32	ssi_comp_version;
	/* Each address location is 16-bit only */
	u32	dr[36];
	/* This register is 8-bit length */
	u32	rx_sample_dly;
	u32	rsvd_0;
	u32	rsvd_1;
	u32	rsvd_2;
};

#endif /* _NPS_SPI_H_ */

