/*
 * spiFlash.h
 *
 *  Created on: 05/12/2011
 *      Author: Avi R.
 */

#ifndef SPIFLASH_H_
#define SPIFLASH_H_

#define PAGE_SIZE               256
#define SECTOR_SIZE             0x00040000
#define CMD_READ_ID			    0x9f

#define CMD_READ_ARRAY_FAST		0x0b
#define CMD_S25FLXX_WREN	    0x06	/* Write Enable */
#define CMD_S25FLXX_RDSR	    0x05	/* Read Status Register */
#define CMD_S25FLXX_PP		    0x02	/* Page Program */
#define CMD_S25FLXX_SE		    0xd8	/* Sector Erase */


#define SPANSION_SR_WIP		(1 << 0)	/* Write-in-Progress */

/* SPI transfer flags */
#define SPI_XFER_BEGIN	0x01			/* Assert CS before transfer */
#define SPI_XFER_END	0x02			/* Deassert CS after transfer */


#define u32 unsigned int
#define u16 unsigned short int
#define u8  unsigned char
#define	EZBIT(_x_)					(1<<(_x_))
#define	EZBIT_MASK(h,l)			    (EZBIT(h) | (EZBIT(h) - EZBIT(l)))

struct ezspi_slave {
//	struct spi_slave slave;
	u32             base;
	u32             freq; /* CLK_IN/CLK_OUT */
	u32             mode;
	u32             txthrs;
	u32             rxthrs;
    u32            *gpioDataReg;
    u32             spiCsExt;
};

/* SR register defines */
#define	EZSPI_SR_DCOL			EZBIT(6)
#define	EZSPI_SR_TXE			EZBIT(5)
#define	EZSPI_SR_RFF			EZBIT(4)
#define	EZSPI_SR_RFNE			EZBIT(3)
#define	EZSPI_SR_TFE			EZBIT(2)
#define	EZSPI_SR_TFNF			EZBIT(1)
#define	EZSPI_SR_BUSY			EZBIT(0)

/* CTRLR0 register defines */
#define	EZSPI_CTRLR0_TMOD		EZBIT_MASK(9,8)
#define	EZSPI_CTRLR0_CPOL		EZBIT(7)
#define	EZSPI_CTRLR0_CPHA		EZBIT(6)
#define	EZSPI_CTRLR0_FRF		EZBIT_MASK(5,4)
#define	EZSPI_CTRLR0_DFS		EZBIT_MASK(3,0)

#endif /* SPIFLASH_H_ */
