/*
 * EZchip Semiconductors (C) Copyright 2010
 * Arkady Gilinsky, arkadyg@ezchip.com
 * Based of DaVinci SPI Controller
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

#ifndef _EZCHIP_NPA0_SPI_H_
#define _EZCHIP_NPA0_SPI_H_

#include <common.h>
#include <spi.h>

/* CTRLR0 register defines */
#define	EZSPI_CTRLR0_TMOD		EZBIT_MASK(9,8)
#define	EZSPI_CTRLR0_CPOL		EZBIT(7)
#define	EZSPI_CTRLR0_CPHA		EZBIT(6)
#define	EZSPI_CTRLR0_FRF		EZBIT_MASK(5,4)
#define	EZSPI_CTRLR0_DFS		EZBIT_MASK(3,0)

/* SR register defines */
#define	EZSPI_SR_DCOL			EZBIT(6)
#define	EZSPI_SR_TXE			EZBIT(5)
#define	EZSPI_SR_RFF			EZBIT(4)
#define	EZSPI_SR_RFNE			EZBIT(3)
#define	EZSPI_SR_TFE			EZBIT(2)
#define	EZSPI_SR_TFNF			EZBIT(1)
#define	EZSPI_SR_BUSY			EZBIT(0)

struct ezspi_slave {
	struct spi_slave slave;
	u32             base;
	u32             freq; /* CLK_IN/CLK_OUT */
	u32             mode;
	u32             txthrs;
	u32             rxthrs;
};

static inline struct ezspi_slave *to_ezspi(struct spi_slave *slave)
{
	return container_of(slave, struct ezspi_slave, slave);
}

#endif /* _EZCHIP_NPA0_SPI_H_ */
