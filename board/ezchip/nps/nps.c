/*
*	Copyright (C) 2015 EZchip, Inc. (www.ezchip.com)
*
* 	This program is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License version 2 as
*	published by the Free Software Foundation.
*/

#include "nps.h"
#include <netdev.h>

/*
 * Board defines dependent configurations section
 */
#ifdef CONFIG_BOARD_EARLY_INIT_R
int board_early_init_r(void)
{
	configure_emem();

#ifdef CONFIG_OF_LIBFDT

	flash_to_mem(DTB_FLASH_OFFSET, DTB_EMEM_ADDRESS, DTB_MAX_SIZE);

	fdt_set_totalsize((void*)DTB_EMEM_ADDRESS, fdt_totalsize(DTB_EMEM_ADDRESS) + FDT_PAD_SIZE);

	if (fdt_setprop((void*)DTB_EMEM_ADDRESS, FDT_ROOT_NODE_OFFSET, "present-cpus",(char*)PRESENT_CPUS_LRAM_ADDRESS, CPU_MAP_MAX_SIZE))
		printf("ERR: Failed to update present cpus in fdt\n");

	if (fdt_setprop((void*)DTB_EMEM_ADDRESS, FDT_ROOT_NODE_OFFSET, "possible-cpus",(char*)POSSIBLE_CPUS_LRAM_ADDRESS, CPU_MAP_MAX_SIZE))
			printf("ERR: Failed to update possible cpus in fdt\n");

#endif /* CONFIG_OF_LIBFDT */

	return 0;
}
#endif /* CONFIG_BOARD_EARLY_INIT_R */

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bd)
{
	int rc = 0;

#ifdef CONFIG_NPS_ETH
	rc += nps_eth_initialize();
#endif
#ifdef CONFIG_NPS_MINI_HE
	rc += nps_miniHE_eth_initialize();
#endif
	return rc;
}
#endif

/*
 * Commands section
 */
#ifndef CONFIG_ENV_IS_NOWHERE
#include "environment.h"
static int do_resetenv(cmd_tbl_t *cmdtp, int flag,int argc, char * const argv[])
{
	char ipaddr[32];
	char etheddr[32];


	if (argc > 2)
		goto usage;

	strcpy(ipaddr,getenv("ipaddr"));
	strcpy(etheddr,getenv("ethaddr"));

	if (argc == 2) {
		if (!argv[1] && (strcmp(argv[1],"full") != 0 ))
			goto usage;
	}

	set_default_env(NULL);

	if (argc == 1) {
		setenv ("ipaddr", ipaddr);
		setenv ("ethaddr", etheddr);
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

int do_file_load(char *file_name_var, char* mode)
{
	char *file_name;
	char cmdbuf[CONFIG_SYS_CBSIZE];

	if ( !file_name_var || !(file_name_var[0]))
		goto usage;

	file_name = getenv(file_name_var);
	if (!file_name) {
		printf("%s variable is not defined\n",file_name_var);
		goto usage;
	}

	if ( (mode != NULL) && (strcmp(mode,"serial") == 0) ) {
		strcpy (cmdbuf, "loady");
	} else {
		strcpy (cmdbuf, "tftpboot ");
		strcat (cmdbuf, file_name);
	}

	if (run_command (cmdbuf, 0) < 0) /* tftpboot ${file_name} */
		goto usage;

	return 0;
usage:
	printf("Usage: do_file_load [file_name] \n");
	return 1;

}

static int do_uboot_write(void)
{
	int uboot_sf_size;
	char *uboot_offset;
	char *uboot_file_size;
	char *uboot_load_address;
	char cmdbuf[CONFIG_SYS_CBSIZE];

	uboot_offset = getenv("uboot_offs");
	uboot_file_size = getenv("filesize");
	uboot_load_address = getenv("loadaddr");

	/* Check environment variables */
	if (!uboot_offset || !uboot_file_size || !uboot_load_address) {
		printf("Environment variables are not defined:\n");
		printf("uboot_offs:%s\n",uboot_offset);
		printf("filesize:%s\n",uboot_file_size);
		printf("loadaddr:%s\n",uboot_load_address);
		goto usage;
	}

	/* Check file size */
	uboot_sf_size = simple_strtoul(uboot_file_size, NULL, 16);
	if (uboot_sf_size > UBOOT_SIZE) {
		printf("file size is larger then %x\n",UBOOT_SIZE);
		goto usage;
	}

	if (run_command ("sf probe 0", 0) < 0) /* sf probe 0 */
		goto usage;

	if (uboot_sf_size % CONFIG_ENV_SECT_SIZE)
		uboot_sf_size = ((uboot_sf_size / CONFIG_ENV_SECT_SIZE) + 1) * CONFIG_ENV_SECT_SIZE;

	sprintf(cmdbuf, "sf erase %s %x", uboot_offset, uboot_sf_size);

	if (run_command (cmdbuf, 0) < 0) /* sf erase ${uboot_offs} ${uboot_sf_size} */
		goto usage;

	sprintf(cmdbuf, "sf write %s %s %s", uboot_load_address, uboot_offset, uboot_file_size);

	if (run_command (cmdbuf, 0) < 0) /* sf  write ${loadaddr} ${uboot_offs} ${filesize} */
		goto usage;

	return 0;
usage:
	printf("Usage: uboot_write\n");
	return 1;
}

static int do_uboot_update(cmd_tbl_t *cmdtp, int flag,int argc, char * const argv[])
{
	int ret;
	char *arg = NULL;

	if (argc > 2)
		goto usage;

	if (argc == 2)
		arg = argv[1];

	ret = do_file_load("uboot_file",arg);
	if (ret)
		goto usage;

	ret = do_uboot_write();
	if (ret)
		goto usage;

	return 0;
usage:
	cmd_usage(cmdtp);
	return 1;
}

#ifdef CONFIG_OF_LIBFDT
static int do_boot_prepare(cmd_tbl_t *cmdtp, int flag,int argc, char * const argv[])
{
	char cmdline[KRN_COMMANDLINE_MAX_SIZE];
	int  nodeoffset;
	char *dtb_bootargs;
	char *env_bootargs;

	/* Copy flash images to emem */
	flash_to_mem(KERNEL_FLASH_OFFSET, UIMAGE_BASE_ADDRESS, UIMAGE_MAX_SIZE);
	flash_to_mem(ROOTFS_FLASH_OFFSET, ROOTFS_EMEM_ADDRESS, ROOTFS_MAX_SIZE);

	/* Update kernel command line */
	nodeoffset = fdt_path_offset((void*)DTB_EMEM_ADDRESS, "/chosen");
	if (nodeoffset < 0)
		return nodeoffset;
	dtb_bootargs = (char *)fdt_getprop((void*)DTB_EMEM_ADDRESS, nodeoffset, "bootargs", NULL);
	env_bootargs = getenv("bootargs");
	sprintf(cmdline,"%s %s %s %s", dtb_bootargs, (char*)KRN_CMDLINE_LRAM_ADDRESS, KERNEL_CMDLINE_FS_STR, (env_bootargs == NULL) ? " " : env_bootargs);
	setenv("bootargs",cmdline);

	return 0;
}
#endif

#ifndef CONFIG_ENV_IS_NOWHERE
U_BOOT_CMD(
	resetenv, 2, 0, do_resetenv,
	"reset environment to default settings",
	"Usage: resetenv [full]"
);
#endif

U_BOOT_CMD(
	ubootup, 2, 0, do_uboot_update,
	"Write uboot to SPI flash in ${uboot_offs}",
	"Usage: uboot_update [serial]"
);

#ifdef CONFIG_OF_LIBFDT
U_BOOT_CMD(
	boot_prepare, 2, 0, do_boot_prepare,
	"Configure emem, copy kernel and fs to emem",
	"Usage: boot_prepare"
);
#endif
