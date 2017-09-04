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

#include <common.h>
#include <netdev.h>
#ifdef CONFIG_OF_LIBFDT
#include <libfdt.h>
#include <fdt_support.h>
#include "../../../drivers/net/nps_eth.h"
#endif
#include "common.h"
#include "nps.h"
#include "serdes.h"
#include "chip.h"
#ifdef CONFIG_TARGET_NPS_HE
#include "ddr_he.h"
#else
#include "ddr.h"
#include "bist.h"
#ifdef CONFIG_NPS_DDR_DEBUG
#include "ddr_debug.h"
#endif
#endif

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)

#define REG_FIELD_LEN	8

int ft_board_setup(void *blob, bd_t *bd)
{
	int rc;
	char *env_string;

	env_string = getenv("krn_present_cpus");
	if (env_string) {
		rc = fdt_find_and_setprop(blob,
				"/", "present-cpus",
				env_string,
				strlen(env_string) + 1, 1);
		if (rc)
			printf("Unable to update property present-cpus in fdt, "
				"err=%s\n",
				fdt_strerror(rc));
	}

	env_string = getenv("krn_possible_cpus");
	if (env_string) {
		rc = fdt_find_and_setprop(blob,
				"/", "possible-cpus",
				env_string,
				strlen(env_string) + 1, 1);
		if (rc)
			printf("Unable to update property possible-cpus in fdt, "
				"err=%s\n",
				fdt_strerror(rc));
	}

	/* dbg_lan is set to west side by default.
	* in case east dbg lan was configured, overwrite the bdg_lan
	* block address in the dtb*/
	if(is_east_dgb_lan()) {
		unsigned long val;
		char buffer[128];
		char *data;
		int node;

		data = &buffer[0];
	        node = fdt_path_offset (blob, "ethernet0");
		if (node < 0) {
			printf ("ethernet0 not found in fdt\n");
			return node;
		}

		val = NPS_ETH_EAST_DBG_LAN_BLOCK_ADDR;
		*(__be32 *)data = __cpu_to_be32(val);
		data +=4;
		val = NPS_ETH_DBG_LAN_BLOCK_SIZE;
		*(__be32 *)data = __cpu_to_be32(val);
		rc= fdt_setprop(blob, node, "reg", buffer, REG_FIELD_LEN);
		if (rc) {
			printf ("set property failed err=%s\n", fdt_strerror(rc));
			return rc;
		}
	}
	return 0;
}
#endif

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bd)
{
	int rc = 0;

#ifdef CONFIG_NPS_ETH
#ifdef CONFIG_TARGET_NPS_MINIHE
	rc += nps_miniHE_eth_initialize();
#else
	if (nps_dbg_lan_serdes_init()) {
		union crg_gen_purp_2 gen_purp_2;

		rc = nps_eth_initialize();
		/* report dbg_lan side to cp*/
		gen_purp_2.reg = read_non_cluster_reg(CRG_BLOCK_ID, CRG_REG_GEN_PURP_2);
		gen_purp_2.fields.dbg_lan_side = is_east_dgb_lan() ? 1:0;
		write_non_cluster_reg(CRG_BLOCK_ID, CRG_REG_GEN_PURP_2, gen_purp_2.reg);
	}
#endif
#endif
	return rc;
}
#endif

#ifdef CONFIG_TARGET_NPS_SIM
int misc_init_r(void)
{
	unsigned char mac_addr[6];
	char krn_string[1024];
	char *krn_p = krn_string;
	char *env_string;
	char *extra_bootargs;

	setenv("krn_present_cpus", (char *)PRESENT_CPUS_LRAM_ADDRESS);
	setenv("krn_possible_cpus", (char *)POSSIBLE_CPUS_LRAM_ADDRESS);

	strcpy(krn_p, (char *)(KRN_CMDLINE_LRAM_ADDRESS));

	/* override ipaddr in env */
	env_string = strsep(&krn_p, ":");
	strsep(&env_string, "=");
	setenv("ipaddr", env_string);

	/* override serverip in env */
	env_string = strsep(&krn_p, ":");
	setenv("serverip", env_string);

	/* override netmask in env */
	strsep(&krn_p, ":");
	env_string = strsep(&krn_p, ":");
	setenv("netmask", env_string);

	/* override ethaddr in env */
	env_string = strsep(&krn_p, "=");
	eth_parse_enetaddr(krn_p, mac_addr);
	if (is_valid_ethaddr(mac_addr))
		eth_setenv_enetaddr("ethaddr", mac_addr);

	/* check extra kernel parameters */
	strsep(&krn_p, " ");
	env_string = strsep(&krn_p, " ");
	if (strlen(env_string) > 0) {
		extra_bootargs = getenv("extra_bootargs");
		if (extra_bootargs) {
			strcat(env_string, " ");
			strcat(env_string, extra_bootargs);
		}
		setenv("extra_bootargs", env_string);
	}

	return 0;
}
#endif

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#if defined(CONFIG_TARGET_NPS_SOC) || defined(CONFIG_TARGET_NPS_HE) || defined(CONFIG_TARGET_NPS_IDG4400)
	configure_emem();
#endif
	configure_l2c();
	configure_ciu();
	configure_mtm();
	check_mbist_result();

	/* Before loading uImage, disable all interrupts in order to avoid 
	 * unexpected ones before the relevant linux driver is ready.
	 * Bits 0-2 are reserved and should be set to 0b111 by the ISA. */
	write_aux_reg(ARC_AUX_IENABLE, 0x7);

#ifdef CONFIG_TARGET_NPS_SIM
	/* Copy kernel from flash to external memory */
	/* TODO remove this once simulator support flash operations */
	flash_to_mem(CONFIG_NPS_UIMAGE_FLASH_OFFS, CONFIG_NPS_UIMAGE_EMEM_ADDR, CONFIG_NPS_UIMAGE_SIZE);

#ifdef CONFIG_OF_LIBFDT
	/* copy dtb from flash to external memory */
	/* TODO remove this once simulator support flash operations */
	flash_to_mem(CONFIG_NPS_DTB_FLASH_OFFS, CONFIG_NPS_DTB_EMEM_ADDR, CONFIG_NPS_DTB_SIZE);
#endif
#endif /* CONFIG_TARGET_NPS_SIM */

	return 0;
}
#endif /* CONFIG_BOARD_LATE_INIT */

/*
 * Commands section
 */
#ifndef CONFIG_ENV_IS_NOWHERE
#include "environment.h"
static int do_resetenv(cmd_tbl_t *cmdtp, int flag, int argc,
					char * const argv[])
{
	char ipaddr[32];
	char etheddr[32];

	if (argc > 2)
		goto usage;

	strcpy(ipaddr, getenv("ipaddr"));
	strcpy(etheddr, getenv("ethaddr"));

	if (argc == 2) {
		if (!argv[1] && (strcmp(argv[1], "full") != 0))
			goto usage;
	}

	set_default_env(NULL);

	if (argc == 1) {
		setenv("ipaddr", ipaddr);
		setenv("ethaddr", etheddr);
	}

	if (saveenv()) {
		printf("Writing env to flash failed\n");
		return 1;
	}

	return 0;

usage:
	cmd_usage(cmdtp);

	return 1;
}
#endif

static int do_halt_cpu(cmd_tbl_t *cmdtp, int flag, int argc,
					char * const argv[])
{
	unsigned int value;

	/* Signal CP that we are done */
	value = read_non_cluster_reg(CRG_BLOCK_ID, CRG_REG_GEN_PURP_0);
	value |= CRG_GEN_PURP_0_SYNC_BIT;
	write_non_cluster_reg(CRG_BLOCK_ID, CRG_REG_GEN_PURP_0, value);
	asm volatile ("sync");

	value = read_cluster_reg(0, 0, 0, 0x81);
	value &= ~1;
	write_cluster_reg(0, 0, 0, 0x81, value);
	asm volatile ("sync");

	/* We are done Halt!!! */
	asm volatile ("flag 1");

	return 0;
}

#ifndef CONFIG_ENV_IS_NOWHERE
U_BOOT_CMD(
	resetenv, 2, 0, do_resetenv,
	"reset environment to default settings",
	"Usage: resetenv [full]"
);
#endif

U_BOOT_CMD(
	haltcpu, 1, 0, do_halt_cpu,
	"Halt cpu gracefully",
	"Halt cpu gracefully"
);

U_BOOT_CMD(
	debug, 2, 0, do_debug,
	"Enable/Disable debug regDB prints",
	"1(Enable)/0(Disable)"
);

U_BOOT_CMD(
	read_reg, 5, 0, do_read_reg,
	"read one or more registers",
	"block reg\nread_reg block low - high\nread_reg block reg1,reg2\n"
	"read_reg block cluster_x cluster_y reg"
);

U_BOOT_CMD(
	write_reg, 6, 0, do_write_reg,
	"write register",
	"block reg value\nwrite_reg block cluster_x cluster_y reg value"
);

#ifdef CONFIG_NPS_DDR_DEBUG
U_BOOT_CMD(
	ddr_diagnostic, 2, 0, do_ddr_diagnostic,
	"dump ddr information into a table format",
	""
);

U_BOOT_CMD(
	pup_fail_dump, 2, 0, do_pup_fail_dump,
	"enable/disable dump in case of PUP fail",
	"Usage: pup_fail_dump 1 or up_fail_dump 0"
);

U_BOOT_CMD(
	print_ddr_config, 1, 0, do_print_ddr_config,
	"Prints current DDR configuration",
	""
);

U_BOOT_CMD(
        wl, 3, 0, do_sw_wl,
        "SW write leveling",
        "Usage: wl <wl_mr1_data>"
);

U_BOOT_CMD(
        calib, 1, 0, do_calib_fail_recovery,
        "Calibration recovery",
        ""
);

U_BOOT_CMD(
	ddr_pause, 2, 0, do_ddr_pause,
	"Enable/Disable ddr_pause",
	"1(Enable)/0(Disable)"
);

U_BOOT_CMD(
	phy_bist, 2, 0, do_phy_bist,
	" run & setup phy bist",
	" mc_mask"
);

U_BOOT_CMD(
	show_wsm, 2, 0, do_marg_and_d2,
	"Show Write Skew Margin for DDR PHY(s)",
	"Usage: show_wsm UM<side>_<num> or show_wsm ALL"
);

U_BOOT_CMD(
	show_rsm, 2, 0, do_marg_and_d2,
	"Show Read Skew Margin for DDR PHY(s)",
	"Usage: show_rsm UM<side>_<num> or show_rsm ALL"
);


U_BOOT_CMD(
	show_wcm, 2, 0, do_marg_and_d2,
	"Show Write Centralization Margin for DDR PHY(s)",
	"Usage: show_wcm UM<side>_<num> or show_wcm ALL"
);

U_BOOT_CMD(
	show_rcm, 2, 0, do_marg_and_d2,
	"Show Read Centralization Margin for DDR PHY(s)",
	"Usage: show_rcm UM<side>_<num> or show_rcm ALL"
);


U_BOOT_CMD(
	edit_wctap, 4, 0, do_edit_results,
	"Edit Write Centralization TAP for DDR PHY",
	"Usage: edit_wctap UM<side>_<num> bl_index tap_delay"
);

U_BOOT_CMD(
	edit_rctap, 4, 0, do_edit_results,
	"Edit Read Centralization TAP for DDR PHY",
	"Usage: edit_rctap UM<side>_<num> bl_index tap_delay"
);

U_BOOT_CMD(
	edit_wdtap, 5, 0, do_edit_results,
	"Edit single DQ Write Delay TAP for DDR PHY",
	"Usage: edit_wdtap UM<side>_<num> bl_index bit_index tap_delay"
);

U_BOOT_CMD(
	edit_rdtap, 5, 0, do_edit_results,
	"Edit single DQ Read Delay TAP for DDR PHY",
	"Usage: edit_rdtap UM<side>_<num> bl_index bit_index tap_delay"
);

U_BOOT_CMD(
	dto_probing, 4, 0, do_dto_probing,
	"Enable DTO signal probing",
	"UMx dto_msel dto_isel"
);

U_BOOT_CMD(
	ato_probing, 4, 0, do_ato_probing,
	"Enable ATO signal probing",
	"UMx ato_msel ato_isel"
);

U_BOOT_CMD(
	write_2d, 2, 0, do_marg_and_d2,
	"Per-Byte-Lane Write Data Eye Plotting for DDR PHY(s)",
	"Usage: write_2d UM<side>_<num> or write_2d ALL"
);

U_BOOT_CMD(
	read_2d, 2, 0, do_marg_and_d2,
	"DDR PHY Per-Byte-Lane read Data Eye Plotting for DDR PHY(s)",
	"Usage: read_2d UM<side>_<num> or read_2d ALL"
);

U_BOOT_CMD(
	sdram_vref, 4, 0, do_vref,
	"Modify SDRAM VREF Setup",
	"Usage: sdram_vref UM<side>_<num> bl_index vref_val"
);

U_BOOT_CMD(
	phy_vref, 4, 0, do_vref,
	"Modify DDR PHY VREF Setup",
	"Usage: phy_vref UM<side>_<num> bl_index vref_val"
);

U_BOOT_CMD(
	phy_vref_prob, 3, 0, do_phy_vref_prob,
	"Modify DDR PHY VREF Setup",
	"Usage: phy_vref_prob UM<side>_<num> bl_index"
);

U_BOOT_CMD(
	pub_record, 3, 0, do_ddr_basic,
	"Recording DDR PHY Registers State",
	"Usage: pub_record UM<s>_<n> < 0x<addr> or name or ALL >"
);

U_BOOT_CMD(
	pub_restore, 3, 0, do_ddr_basic,
	"Restoring DDR PHY Registers State",
	"Usage: pub_restore UM<s>_<n> < 0x<addr> or name or ALL >"
);

U_BOOT_CMD(
	pub_read, 3, 0, do_ddr_basic,
	"Reading DDR PHY Register value",
	"Usage: pub_read UM<s>_<n> < 0x<addr> or name >"
);

U_BOOT_CMD(
	pub_dump, 2, 0, do_ddr_basic,
	"Dump All DDR PHY Registers value",
	"Usage: pub_dump UM<s>_<n>"
);

U_BOOT_CMD(
	mem_read_loop, 7, 0, do_mem_read_loop,
	" read 16/32 bytes from external memory",
	" mc_mask bank_group bank row column size"
);

U_BOOT_CMD(
	pub_write, 4, 0, do_ddr_basic,
	"Writing DDR PHY Register value",
	"Usage: pub_write UM<s>_<n> < 0x<addr> or name > 0x<val>"
);

U_BOOT_CMD(
	post_ddr, 2, 0, do_post_training,
	"do post ddr training",
	"size"
);

U_BOOT_CMD(
	ddr_training, 2, 0, do_ddr_training_steps,
	"do DDR training steps according to pir_val",
	"Usage: ddr_training pir_val (0xFFFFFFFF for default mode)"
);

U_BOOT_CMD(
	pup_rerun, 2, 0, do_configure_emem,
	"rerun DDR MC and PHY configuration",
	"Usage: pup_rerun"
);

U_BOOT_CMD(
	mem_write, 15, 0, do_mem_write,
	" write provided 16/32 bytes to external memory",
	" mc_mask bank_group bank row column size 0x<word0> 0x<word1>..."
);

U_BOOT_CMD(
	mem_write_loop, 15, 0, do_mem_write_loop,
	" write provided 16/32 "
	"bytes to external memory in loop",
	" mc_mask bank_group bank row column size 0x<word0> 0x<word1>..."
);

U_BOOT_CMD(
	mem_read, 7, 0, do_mem_read,
	" read 32 bytes from external memory",
	" mc_mask bank_group bank row column size"
);
#endif
U_BOOT_CMD(
	ddr_bist, 7, 0, do_ddr_bist,
	"run ddr bist commands",
	"show config\n"
	"ddr_bist run <mc_mask> 11 <seed> [rep_mode] [op_mode]\n"
	"ddr_bist run <mc_mask> <config_set_id> [rep_mode] [op_mode]\n"
	"ddr_bist dump <mc_mask> [stop]\n"
	"ddr_bist pattern <mc_mask> <config> [change byte]\n"
	"ddr_bist loop <mc_mask> <iterations> [start/fixed_seed] <seed>\n"
	"ddr_bist rd_loop <mc_mask> <iterations> <compare_num> start/fixed_seed <seed>"
);

U_BOOT_CMD(
	bus_test, 3, 0, do_bus_test,
	"test ddr address or data buses",
	"mc_mask bus(address/data) "
);

