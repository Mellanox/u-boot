/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <common.h>
#include <linux/types.h>
#include <asm/uboot-arc.h>
#include <asm/global_data.h>
#include <asm/arch/memory.h>
#include <stdio_dev.h>
#include <version.h>
#include <config.h>
#include <net.h>
#include <mmc.h>
#include <nand.h>

const char version_string[] =
	U_BOOT_VERSION" (" __DATE__ " - " __TIME__ ")";

unsigned boot_flash_base;

static int display_banner(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	printf("Global data pointer %x\n", (ulong)gd);

	printf("\n\n%s\n\n", version_string);
	printf("SYS CPU running at %dHZ\n", (int)(gd->cpu_clk));
	return 0;
}

static int init_baudrate(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	char tmp[64];	/* long enough for environment variables */
	int i = getenv_f("baudrate", tmp, sizeof(tmp));
	gd->bd->bi_baudrate = gd->baudrate = (i > 0)
			? (int) simple_strtoul(tmp, NULL, 10)
			: CONFIG_BAUDRATE;

	return 0;
}

DECLARE_GLOBAL_DATA_PTR;

static int display_dram_config(void)
{
	int i;
#ifdef NOT_NOW_IN_USE
#ifdef DEBUG
	puts("RAM Configuration:\n");

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		printf("Bank #%d: %08lx ", i, gd->bd->bi_dram[i].start);
		print_size(gd->bd->bi_dram[i].size, "\n");
	}
#else
	ulong size = 0;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		size += gd->bd->bi_dram[i].size;

	puts("DRAM:  ");
	print_size(size, "\n");
#endif
#endif
	return 0;
}

typedef int (init_fnc_t) (void);

int board_init(void);

extern int init_interrupt(void);

init_fnc_t *init_sequence[] = {

#if defined(CONFIG_ARCH_CPU_INIT)
	arch_cpu_init,		/* basic arch cpu dependent setup */
#endif
	board_init,			/* basic board dependent setup */
#if defined(CONFIG_USE_IRQ)
	init_interrupt,		/* set up exceptions */
#endif
	timer_init,			/* initialize timer */
#ifdef CONFIG_FSL_ESDHC
	get_clocks,
#endif
	env_init,			/* initialize environment */
	init_baudrate,		/* initialze baudrate settings */
	serial_init,		/* serial communications setup */
	console_init_f,		/* stage 1 init of console */
	display_banner,		/* say that we are here */
#if defined(CONFIG_DISPLAY_CPUINFO)
	print_cpuinfo,		/* display cpu info (and speed) */
#endif
#if defined(CONFIG_DISPLAY_BOARDINFO)
	checkboard,			/* display board info */
#endif
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
	init_func_i2c,
#endif
	display_dram_config,

	NULL,
};

/*
 *	Begin and End of memory area for malloc(), and current "brk"
 */
extern int __heap_start;
extern int __heap_end;

#ifndef CONFIG_SYS_NO_FLASH
static void display_flash_config(ulong size)
{
	puts("Flash: ");
	print_size(size, "\n");
}
#endif

static unsigned round_page(unsigned x)
{
	x += 0xfff;
	x &= ~0xfff;
	return x;
}

#if !defined(CONFIG_SYS_NO_FLASH)
static char *failed = "*** failed ***\n";
#endif

gd_t *global_data;
gd_t *gd;

extern int  init_osd_dev(void);
extern int _start;

void start_arcboot(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	static gd_t gd_data;
	static bd_t bd_data;
#if !defined(CONFIG_SYS_NO_FLASH)
	ulong flash_size;
#endif

	init_fnc_t **init_fnc_ptr;
	extern void *__text_end;

	unsigned stage = sizeof(init_sequence)/sizeof(init_sequence[0]);

	show_boot_progress(stage--);

	/* Init Global Data */
	gd = global_data = &gd_data;
	gd->bd = &bd_data;
	gd->cpu_clk = get_cpu_clk();
	gd->bd->bi_boot_params = BOOT_PARAMS_OFFSET;
	gd->bd->bi_memstart = PHYS_MEMORY_START;
	gd->bd->bi_memsize = PHYS_MEMORY_SIZE;
	gd->bd->bi_flashstart = CONFIG_SYS_FLASH_BASE;
	gd->bd->bi_flashoffset = 0;

	/* frame buffer will sit after end of program */
	gd->fb_base = CONFIG_SYS_TEXT_BASE;

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {

		show_boot_progress(stage--);

		if ((*init_fnc_ptr)() != 0) {
			printf("stage:%d", stage);
			hang();
		}
	}

	/* Setup malloc area */
	mem_malloc_init((ulong)&_start - CONFIG_SYS_MALLOC_LEN,
		CONFIG_SYS_MALLOC_LEN);

#if !defined(CONFIG_SYS_NO_FLASH)
	puts("Flash: ");
	flash_size = flash_init();

	if (flash_size > 0) {
# ifdef CONFIG_SYS_FLASH_CHECKSUM
		print_size(flash_size, "");
		/*
		 * Compute and print flash CRC if flashchecksum is set to 'y'
		 *
		 * NOTE: Maybe we should add some WATCHDOG_RESET()? XXX
		 */
		s = getenv("flashchecksum");
		if (s && (*s == 'y')) {
			printf("  CRC: %08X", crc32(0,
				(const unsigned char *) CONFIG_SYS_FLASH_BASE,
				flash_size));
		}
		putc('\n');
# else	/* !CONFIG_SYS_FLASH_CHECKSUM */
		print_size(flash_size, "\n");
# endif /* CONFIG_SYS_FLASH_CHECKSUM */
	} else {
		puts(failed);
		hang();
	}
#endif

#ifdef CONFIG_CMD_NAND
	puts("NAND:  ");
	nand_init();
#endif
	env_relocate();
	stdio_init();	/* get the devices list going. */
	jumptable_init();
	console_init_r();

#ifdef CONFIG_MMC
	mmc_initialize(&bd_data);
#endif

#ifdef CONFIG_CMD_NET
	puts("Net:   ");
	gd->bd->bi_ip_addr = getenv_IPaddr("ipaddr");
	/* eth_io_init(); */
	eth_initialize(gd->bd);
#if defined(CONFIG_RESET_PHY_R)
	debug("Reset Ethernet PHY\n");
	reset_phy();
#endif
#endif
#if defined(CONFIG_ARCH_MISC_INIT)
	/* miscellaneous arch dependent initialisations */
	arch_misc_init();
#endif
#if defined(CONFIG_MISC_INIT_R)
	/* miscellaneous platform dependent initialisations */
	misc_init_r();
#endif
#if defined(CONFIG_CMD_KGDB)
	puts("KGDB:  ");
	kgdb_init();
#endif
#if defined(CONFIG_USE_IRQ)
	/* enable exceptions */
	enable_interrupts();
#endif

	icache_enable();
	dcache_enable();
	printf("Dcache status %d\n", dcache_status());
	printf("Icache status %d\n", icache_status());

#ifdef CONFIG_HAPS51_DISPLAY
	osd_display();
#endif
#ifdef CONFIG_HAPS51_I2C
		disable_i2c_pinmux();
#endif
	for (;;)
		main_loop();
}

void hang()
{
	printf("please reset....\n");
	for (;;)
		/* Nothing to do */;
}

unsigned long do_go_exec(ulong (*entry)(int, char * []), int argc, char *argv[])
{
	/*
	 * x86 does not use a dedicated register to pass the pointer
	 * to the global_data
	 */
	argv[-1] = (char *)gd;

	return entry(argc, argv);
}
