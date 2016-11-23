/*
 * spiFlash.h
 *
 *  Created on: 05/12/2011
 *      Author: Avi R.
 */

#ifndef SPIFLASH_H_
#define SPIFLASH_H_

#include <linux/types.h>

#define UART_BASE       0xC0000000

#define REG_UART_THR             (* ( (volatile int*)(UART_BASE+0x00000000) ) )
#define REG_UART_DLL             (* ( (volatile int*)(UART_BASE+0x00000000) ) )
#define REG_UART_FCR             (* ( (volatile int*)(UART_BASE+0x00000008) ) )
#define REG_UART_LCR             (* ( (volatile int*)(UART_BASE+0x0000000C) ) )
#define REG_UART_LSR             (* ( (volatile int*)(UART_BASE+0x00000014) ) )

#define SSI_BASE        0xC0001000

#define CPU_REGS_BASE   0xC0002000
#define REG_CPU_SSI_MASK  (* ( (volatile int*)(CPU_REGS_BASE+0x00000040 ) ) )

#define CS_FLASH_OVER        0x01

#define CMD_READ_ARRAY_FAST		0x0b

/* SPI transfer flags */
#define SPI_XFER_BEGIN	0x01			/* Assert CS before transfer */
#define SPI_XFER_END	0x02			/* Deassert CS after transfer */

//#define u32 unsigned int
//#define u16 unsigned short int
//#define u8  unsigned char
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
#define	EZSPI_SR_RFNE			EZBIT(3)
#define	EZSPI_SR_BUSY			EZBIT(0)

/* CTRLR0 register defines */
#define	EZSPI_CTRLR0_TMOD		EZBIT_MASK(9,8)
#define	EZSPI_CTRLR0_CPOL		EZBIT(7)
#define	EZSPI_CTRLR0_CPHA		EZBIT(6)
#define	EZSPI_CTRLR0_FRF		EZBIT_MASK(5,4)
#define	EZSPI_CTRLR0_DFS		EZBIT_MASK(3,0)

#endif /* SPIFLASH_H_ */
