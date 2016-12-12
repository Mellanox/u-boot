/*
* Copyright (c) 2016, Mellanox Technologies. All rights reserved.
*
* This software is available to you under a choice of one of two
* licenses.  You may choose to be licensed under the terms of the GNU
* General Public License (GPL) Version 2, available from the file
* COPYING in the main directory of this source tree, or the
* OpenIB.org BSD license below:
*
*     Redistribution and use in source and binary forms, with or
*     without modification, are permitted provided that the following
*     conditions are met:
*
*      - Redistributions of source code must retain the above
*        copyright notice, this list of conditions and the following
*        disclaimer.
*
*      - Redistributions in binary form must reproduce the above
*        copyright notice, this list of conditions and the following
*        disclaimer in the documentation and/or other materials
*        provided with the distribution.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
* BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
* ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#ifndef _SERDES_H_
#define _SERDES_H_

#define SERDES_WEST_BLOCK_ID			0x41d
#define SERDES_EAST_BLOCK_ID			0x51d

#define SERDES_NETIF_BROADCAST			0xe1
#define SERDES_SBUS_RECEIVER_SERDES_49		0x3b

#define SERDES_REG_GLOBAL_RESET			0x07
#define SERDES_REG_BROADCAST_GROUP		0xfd
#define SERDES_REG_DISABLE_INTERRUPTS		0x08
#define SERDES_REG_IMEM_CTRL			0x00
#define SERDES_REG_IMEM_BURST_CTRL		0x0a
#define SERDES_REG_ECC_CTRL			0x0b

#define SERDES_INTERRUPT_CODE_ENABLE		0x1
#define SERDES_INTERRUPT_CODE_TX_BITRATE	0x5
#define SERDES_INTERRUPT_CODE_RX_BITRATE	0x6
#define SERDES_INTERRUPT_CODE_WIDTH_MODE	0x14
#define SERDES_INTERRUPT_CODE_TX_EQ_CONTROL	0x15
#define SERDES_INTERRUPT_CODE_CRC		0x3C
#define SERDES_INTERRUPT_CODE_SET_DFE		0x26
#define SERDES_INTERRUPT_CODE_DFE_CONTROL	0xa


#define SERDES_UCODE_SIZE			11962

#define DBG_LAN_WEST_BLOCK_ID	0x41c
#define DBG_LAN_EAST_BLOCK_ID	0x51c
#define DBG_LAN_IND_DATA	0x0
#define DBG_LAN_IND_CMD		0x10
#define DBG_LAN_IND_ADDR	0x14
#define DBG_LAN_IND_STS		0x15
#define DBG_LAN_IND_SRD_STS	0x81
#define DBG_LAN_IND_STS_RDY	(1 << 0)
#define DBG_LAN_IND_STS_SUCCESS	(1 << 1)

int nps_dbg_lan_serdes_init(void);

#endif /* _SERDES_H_ */
