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

#ifndef _NPS_H_
#define _NPS_H_

#include <common.h>
#include <command.h>
#include <net.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <linux/ctype.h>
#include <image.h>
#include "common.h"

/* LRAM images addresses and offsets */
#define KRN_CMDLINE_LRAM_OFFSET		0xB00000
#define PRESENT_CPUS_LRAM_OFFSET	0xB01000
#define POSSIBLE_CPUS_LRAM_OFFSET	0xB06000
#define DDR_CONFIG_LRAM_OFFSET		0x500000

#define NPS_IMAGE_LRAM_ADDRESS(lram_offset)\
			(CONFIG_SYS_SDRAM_BASE + lram_offset)
#define KRN_CMDLINE_LRAM_ADDRESS\
	NPS_IMAGE_LRAM_ADDRESS(KRN_CMDLINE_LRAM_OFFSET)
#define PRESENT_CPUS_LRAM_ADDRESS\
	NPS_IMAGE_LRAM_ADDRESS(PRESENT_CPUS_LRAM_OFFSET)
#define POSSIBLE_CPUS_LRAM_ADDRESS\
	NPS_IMAGE_LRAM_ADDRESS(POSSIBLE_CPUS_LRAM_OFFSET)
#define DDR_CONFIG_LRAM_ADDRESS\
		NPS_IMAGE_LRAM_ADDRESS(DDR_CONFIG_LRAM_OFFSET)

int do_file_load(char *file_name_var, char *mode);
int do_debug(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_write_reg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_read_reg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_dump_reg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

#endif /* _NPS_H_ */
