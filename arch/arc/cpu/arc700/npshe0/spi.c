/*
 * Driver for SPI controller on EZchip NPA0 board.
 * EZchip Semiconductors (C) Copyright 2010
 * Arkady Gilinsky, arkadyg@ezchip.com
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include "spi.h"
#include "iomap.h"

/*****************************************************************************/
#undef  EZSPI_DEBUG

#ifdef EZSPI_DEBUG
  #define DPRINT(fmt, args...)  printf("ezspi: " fmt, ##args)
#else
  #define DPRINT(fmt, args...)
#endif

#define	EZSPI_DATA_FRAME_SIZE		8
#define	EZSPI_WAIT_FOR_BUSY_RETRY	0x70000000
#define	EZSPI_IN_FREQ_HZ			CONFIG_SYS_HZ

#define  EZBIT(_x_)					(1<<(_x_))
#define  EZBIT_MASK(h,l)			(EZBIT(h) | (EZBIT(h) - EZBIT(l)))

/*****************************************************************************/

#define ezspi_read32(_a_)			readl(_a_)
#define ezspi_write32(_a_,_v_)	writel(_v_,_a_)
#define ezspi_clrsetbits32(_a_,_c_,_s_)	\
			 (ezspi_write32(_a_,(readl(_a_)&(~(_c_)) )|(_s_)))
#define ezspi_clrbits32(_a_,_c_)				\
			 (ezspi_write32(_a_,readl(_a_)&(~(_c_))))
#define ezspi_setbits32(_a_,_s_)				\
			 (ezspi_write32(_a_,readl(_a_)|(_s_)))

/*****************************************************************************/

/* SPI controller registers */
typedef	struct {
	u32	ctrlr0;			/* offset 0x00 */
	u32	ctrlr1;
	u32	ssienr;
	u32	mwcr;
	u32	ser;				/* offset 0x10 */
	u32	baudr;
	u32	txftlr;
	u32	rxftlr;
	u32	txflr;			/* offset 0x20 */
	u32	rxflr;
	u32	sr;
	u32	imr;
	u32	isr;				/* offset 0x30 */
	u32	risr;
	u32	txoicr;
	u32	rxoicr;
	u32	rxuicr;			/* offset 0x40 */
	u32	msticr;
	u32	icr;
	u32	dmacr;
	u32	dmatdlr;			/* offset 0x50 */
	u32	dmardlr;
	u32	idr;
	u32	ssi_comp_version;
	/* Each address location is 16-bit only */
	u32	dr[36];			/* offset 0x60 */
	/* This register is 8-bit length */
	u32	rx_sample_dly;	/* offset 0xF0 */
	u32	rsvd_0;
	u32	rsvd_1;
	u32	rsvd_2;
} ezspi_regs;

#define EZ_SPI_FIFO_SIZE 16

/*****************************************************************************/

static void ezspi_sendRecv(int byteLen,u8 *txd,u8 *rxd,ezspi_regs  *regs);

/*****************************************************************************/


void spi_init()
{
	printf( "freq: %d Hz ", CONFIG_SF_DEFAULT_SPEED );
}

static int get_cpu_id(void)
{
	int id;

	id = read_new_aux_reg(0x004);
	id = (id >> 8) & 0x0FF;

	return id;
}

/*****************************************************************************/

struct spi_slave *spi_setup_slave( u32 bus,  u32 cs,  u32 max_hz,  u32 mode )
{
	struct ezspi_slave  *ds;
	int i;
	int cpu_id;

	cpu_id = get_cpu_id();
	if(cpu_id != 0) {
		printf("Cant setup spi from proccessor %d\n",cpu_id);
		printf("This can be done only from proccessor 0\n");
		return NULL;
	}
	DPRINT( "%s: bus %u, CS %u, MAX_HZ %u, mode 0x%X\n",
			  __FUNCTION__, bus, cs, max_hz, mode );

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	ds = (struct ezspi_slave*)malloc(sizeof(struct ezspi_slave));
	if (!ds)
	return NULL;

	ds->slave.bus = bus;
	ds->slave.cs = cs;
	ds->base = SSI_BASE;

	/* Calculate baudrate */
	i = EZSPI_IN_FREQ_HZ / max_hz;
	if ( (i > 0) && ( (EZSPI_IN_FREQ_HZ / i ) > max_hz ) ) {
		i++;
	}
	if ( i < 1 ) {
		i = 1;
	}
	/* In worst case we set max divider -> minimum freq */
	if ( i > 0xFFFE ) {
		i = 0xFFFE;
	}
	ds->freq = i;
	ds->mode = mode; /* Motorola mode */
	ds->txthrs = 0;
	ds->rxthrs = 0;
	REG_CPU_SSI_MASK |= CS_FLASH_OVER;
	return &ds->slave;
}

/*****************************************************************************/

void spi_free_slave(struct spi_slave *slave)
{
	struct ezspi_slave *ds = to_ezspi(slave);
	free(ds);
}

/*****************************************************************************/

int spi_claim_bus(struct spi_slave *slave)
{
	/* Nothing to do */
	return 0;
}

/*****************************************************************************/

void spi_release_bus(struct spi_slave *slave)
{
	/* Nothing to do */
	return;
}

/*-----------------------------------------------------------------------
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
int  spi_xfer( struct spi_slave	*slave,	unsigned int bitlen,const void *dout,
		void *din,unsigned long flags )
{
	struct   ezspi_slave *ss;
	volatile ezspi_regs  *regs;
	u8 *txd = (u8*)dout;
	u8       *rxd = (u8*)din;
	u32      i;
	int cpu_id;
	int byteLen = bitlen/EZSPI_DATA_FRAME_SIZE;

	cpu_id = get_cpu_id();
	if(cpu_id != 0) {
		printf("Cant transfer to/from spi from proccessor %d\n",cpu_id);
		printf("This can be done only from  proccessor 0\n");
		return -1;
	}

	DPRINT( "%s: slave %u:%u, dout 0x%08X, din 0x%08X, bitlen %u, flags 0x%X\n",
			  __FUNCTION__, slave->bus, slave->cs,
			  txd == NULL ? -1 : *(uint *)txd,
			  (int)rxd,
			  bitlen, (int)flags );

	ss = to_ezspi(slave);
	regs = (ezspi_regs *)ss->base;

	DPRINT( "%s: checking if the bus is busy\n",
			  __FUNCTION__ );

	i = 0;
	while ( ezspi_read32( &regs->sr ) & (EZSPI_SR_BUSY) ) {
		if ( i++ > EZSPI_WAIT_FOR_BUSY_RETRY ) {
			printf( "ERROR: Bus busy timeout\n" );
			return -1;
		}
		udelay(1);
	}
	DPRINT( "%s: bus is not busy\n",
			  __FUNCTION__ );

	if (flags & SPI_XFER_BEGIN) {
		spi_cs_activate(slave);
	}

	DPRINT( "%s: starting transfer\n",
			  __FUNCTION__ );

	while (byteLen > 0 ) {
		i=EZ_SPI_FIFO_SIZE;
		if (i > byteLen) {
			i = byteLen;
		}
		ezspi_sendRecv(i,txd,rxd,(ezspi_regs  *)regs);
		if (txd != NULL ) {
			txd += i;
		}
		if (rxd != NULL) {
			rxd += i;
		}
		byteLen -= i;
	}

	if (flags & SPI_XFER_END) {
		spi_cs_deactivate(slave);
	}

	return(0);
}

/*****************************************************************************/

void ezspi_sendRecv(int byteLen,u8 *txd,u8 *rxd,ezspi_regs  *regs)
{
	int k = byteLen;
	int n;
	volatile u32 tmp;
	u32 *fifo = &regs->dr[0];
	u32 *status = &regs->sr;

	if (txd != NULL) {
		for ( n = 0 ; n < k ; n++ ) {
			tmp = txd[n];
			ezspi_write32( fifo, tmp );
		}
	}
	else {
		tmp = 0;
		for ( n = 0 ; n < k ; n++ ) {
			ezspi_write32( fifo, tmp ); /* only to create clocks */
		}
	}
	if (rxd != NULL) {
		for (n = 0 ; n < k ; n++ , rxd++ ) {
			while( ( ezspi_read32( status ) & EZSPI_SR_RFNE ) == 0 );
			tmp = ezspi_read32( fifo );
			*rxd = (u8)tmp;
		}
	}
	else {
		for (n = 0 ; n < k ; n++ , rxd++ ) {
			while( ( ezspi_read32( status ) & EZSPI_SR_RFNE ) == 0 );
			tmp = ezspi_read32( fifo );  /* only to clean FIFO */
		}
	}
	return;
}

/*****************************************************************************/

int spi_cs_is_valid(u32 bus, u32 cs)
{
	return ((bus == 0) && ( cs < 4 ));
}


/*****************************************************************************/


void spi_cs_activate(struct spi_slave *slave)
{
	struct ezspi_slave  *ss = to_ezspi(slave);
	volatile ezspi_regs *regs = (ezspi_regs *)ss->base;
	u32                 mask = (1<<slave->cs) & 0xFFFF;

	/* Disable SSI to setup transfer parameters */
	ezspi_write32( &regs->ssienr, 0 );

	/* Enable slave */
	ezspi_write32( &regs->ser, mask );
	/* Set slave mode */
	/* Transiver mode = Transmit & Receive - bits 9:8, value 0
	   Frame Format   = Motorola SPI - bits 5:4, value 0 */
	mask = 0;
	if ( ss->mode & SPI_CPOL )
		mask |= EZSPI_CTRLR0_CPOL;

	if ( ss->mode & SPI_CPHA )
		mask |= EZSPI_CTRLR0_CPHA;

	/* Set Data Frame Size */
	mask |= (EZSPI_DATA_FRAME_SIZE - 1);
	ezspi_clrsetbits32( &regs->ctrlr0,
	                    EZSPI_CTRLR0_TMOD |
	                    EZSPI_CTRLR0_CPOL |
	                    EZSPI_CTRLR0_CPHA |
	                    EZSPI_CTRLR0_FRF  |
	                    EZSPI_CTRLR0_DFS,
	                    mask );

	/* Set baudrate */
	ezspi_write32( &regs->baudr, ss->freq );
	ezspi_write32( &regs->ctrlr1, 0x0F );

	/* Set FIFO thresholds */
	ezspi_write32( &regs->txftlr, ss->txthrs );
	ezspi_write32( &regs->rxftlr, ss->rxthrs );
	ezspi_write32( &regs->ssienr, 1 );
	REG_CPU_SSI_MASK &= ~(CS_FLASH_OVER);
}

/*****************************************************************************/

void spi_cs_deactivate(struct spi_slave *slave)
{
	struct ezspi_slave   *ss = to_ezspi(slave);
	volatile ezspi_regs  *regs = (ezspi_regs *)ss->base;

	/* Disable SSI to setup transfer parameters */
	ezspi_write32( &regs->ssienr, 0 );
	/* Disable slave */
	ezspi_write32( &regs->ser, 0 );
	REG_CPU_SSI_MASK |= CS_FLASH_OVER;
}

/*****************************************************************************/

