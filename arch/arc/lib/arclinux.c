/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <common.h>
#include <command.h>
#include <image.h>
#include <u-boot/zlib.h>
#include <asm/byteorder.h>
#include <asm/setup.h>

DECLARE_GLOBAL_DATA_PTR;

#define CONFIG_CMDLINE_TAG 1

#if defined(CONFIG_SETUP_MEMORY_TAGS) || \
	defined(CONFIG_CMDLINE_TAG) || \
	defined(CONFIG_INITRD_TAG) || \
	defined(CONFIG_SERIAL_TAG) || \
	defined(CONFIG_REVISION_TAG) || \
	defined(CONFIG_VFD) || \
	defined(CONFIG_LCD)

static void setup_start_tag(bd_t *bd);

# ifdef CONFIG_SETUP_MEMORY_TAGS
static void setup_memory_tags(bd_t *bd);
# endif
static void setup_commandline_tag(bd_t *bd, char *commandline);

# ifdef CONFIG_INITRD_TAG
static void setup_initrd_tag(bd_t *bd, ulong initrd_start,
			      ulong initrd_end);
# endif
static void setup_end_tag(bd_t *bd);

# if defined(CONFIG_VFD) || defined(CONFIG_LCD)
static void setup_videolfb_tag(gd_t *gd);
# endif

static struct tag *params;
#endif /* CONFIG_SETUP_MEMORY_TAGS || CONFIG_CMDLINE_TAG || ... */

void do_bootm_linux(int flag, int argc, char *argv[],
		     bootm_headers_t *images)
{
	ulong  initrd_start, initrd_end;
	ulong  ep = 0;
	bd_t   *bd = gd->bd;
	char   *s;
	int    machid; /* = bd->bi_arch_number; */
	void   (*theKernel)(int zero, int arch, uint params);
	int	   ret;

#ifdef CONFIG_CMDLINE_TAG
	char *commandline = getenv("bootargs");
#endif

	/* find kernel entry point */
	if (images->legacy_hdr_valid) {
		ep = image_get_ep(&images->legacy_hdr_os_copy);
#if defined(CONFIG_FIT)
	} else if (images->fit_uname_os) {
		ret = fit_image_get_entry(images->fit_hdr_os,
					images->fit_noffset_os, &ep);
		if (ret) {
			puts("Can't get entry point property!\n");
			goto error;
		}
#endif
	} else {
		puts("Could not find kernel entry point!\n");
		goto error;
	}

	theKernel = (void (*)(int, int, uint))ep;

	s = getenv("machid");

	if (s) {
		machid = simple_strtoul(s, NULL, 16);
		printf("Using machid 0x%x from environment\n", machid);
	} else
		machid = 0x7b0033;

	ret = boot_get_ramdisk(argc, argv, images, IH_ARCH_ARC,
			&initrd_start, &initrd_end);

	if (ret)
		goto error;

	show_boot_progress(15);

	debug("## Transferring control to Linux (at address %08lx) ...\n",
	       (ulong) theKernel);

#if defined(CONFIG_SETUP_MEMORY_TAGS) || \
	defined(CONFIG_CMDLINE_TAG) || \
	defined(CONFIG_INITRD_TAG) || \
	defined(CONFIG_SERIAL_TAG) || \
	defined(CONFIG_REVISION_TAG) || \
	defined(CONFIG_LCD) || \
	defined(CONFIG_VFD)

	setup_start_tag(bd);
#ifdef CONFIG_SERIAL_TAG
	setup_serial_tag(&params);
#endif
#ifdef CONFIG_REVISION_TAG
	setup_revision_tag(&params);
#endif
#ifdef CONFIG_SETUP_MEMORY_TAGS
	setup_memory_tags(bd);
#endif
#ifdef CONFIG_CMDLINE_TAG
	setup_commandline_tag(bd, commandline);
#endif
#ifdef CONFIG_INITRD_TAG
	if (initrd_start && initrd_end)
		setup_initrd_tag(bd, initrd_start, initrd_end);
#endif
#if defined(CONFIG_VFD) || defined(CONFIG_LCD)
	setup_videolfb_tag((gd_t *) gd);
#endif
	setup_end_tag(bd);
#endif

	/* we assume that the kernel is in place */
	printf("\nStarting kernel ...\n\n");

#ifdef CONFIG_USB_DEVICE
	{
		extern void udc_disconnect(void);
		udc_disconnect();
	}
#endif

	/* cleanup_before_linux (); */

	theKernel(1, machid, commandline);

	/* does not return */
	return;
error:
	return;
}

#if defined(CONFIG_SETUP_MEMORY_TAGS) || \
	defined(CONFIG_CMDLINE_TAG) || \
	defined(CONFIG_INITRD_TAG) || \
	defined(CONFIG_SERIAL_TAG) || \
	defined(CONFIG_REVISION_TAG) || \
	defined(CONFIG_LCD) || \
	defined(CONFIG_VFD)

static void setup_start_tag(bd_t *bd)
{
	params = (struct tag *) bd->bi_boot_params;

	params->hdr.tag = ATAG_CORE;
	params->hdr.size = tag_size(tag_core);

	params = tag_next(params);
}

#ifdef CONFIG_SETUP_MEMORY_TAGS
static void setup_memory_tags(bd_t *bd)
{
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		params->hdr.tag = ATAG_MEM;
		params->hdr.size = tag_size(tag_mem32);
		params->u.mem.size = bd->bi_memsize;

		params = tag_next(params);
	}
}
#endif /* CONFIG_SETUP_MEMORY_TAGS */

static void setup_commandline_tag(bd_t *bd, char *commandline)
{
	char *p;

	if (!commandline)
		return;

	/* eat leading white space */
	for (p = commandline; *p == ' '; p++)
		/* Nothing to do */;

	/* skip non-existent command lines so the kernel will still
	 * use its default command line.
	 */
	if (*p == '\0')
		return;

	printf("Command line TAG setup\n");

	params->hdr.tag = ATAG_CMDLINE;
	params->hdr.size =
		(sizeof(struct tag_header) + strlen(p) + 1 + 4) >> 2;

	strcpy(params->u.cmdline.cmdline, p);

	printf("Params->u.cmdline.cmdline %s\n", params->u.cmdline.cmdline);
	printf("p %s\n", p);

	params = tag_next(params);
}


#ifdef CONFIG_INITRD_TAG
static void setup_initrd_tag(bd_t *bd, ulong initrd_start, ulong initrd_end)
{
	/* an ATAG_INITRD node tells the kernel where the compressed
	 * ramdisk can be found. ATAG_RDIMG is a better name, actually.
	 */
	params->hdr.tag = ATAG_INITRD2;
	params->hdr.size = tag_size(tag_initrd);

	params->u.initrd.start = initrd_start;
	params->u.initrd.size = initrd_end - initrd_start;

	params = tag_next(params);
}
#endif /* CONFIG_INITRD_TAG */

#if defined(CONFIG_VFD) || defined(CONFIG_LCD)
extern ulong calc_fbsize(void);

static void setup_videolfb_tag(gd_t *gd)
{
	/* An ATAG_VIDEOLFB node tells the kernel where and how large
	 * the framebuffer for video was allocated (among other things).
	 * Note that a _physical_ address is passed !
	 *
	 * We only use it to pass the address and size, the other entries
	 * in the tag_videolfb are not of interest.
	 */
	params->hdr.tag = ATAG_VIDEOLFB;
	params->hdr.size = tag_size(tag_videolfb);

	params->u.videolfb.lfb_base = (u32)gd->fb_base;
	/* Fb size is calculated according to parameters for our panel
	 */
	params->u.videolfb.lfb_size = calc_fbsize();

	params = tag_next(params);
}
#endif /* CONFIG_VFD || CONFIG_LCD */

#ifdef CONFIG_SERIAL_TAG
void setup_serial_tag(struct tag **tmp)
{
	struct tag *params = *tmp;
	struct tag_serialnr serialnr;
	void get_board_serial(struct tag_serialnr *serialnr);

	get_board_serial(&serialnr);
	params->hdr.tag = ATAG_SERIAL;
	params->hdr.size = tag_size(tag_serialnr);
	params->u.serialnr.low = serialnr.low;
	params->u.serialnr.high = serialnr.high;
	params = tag_next(params);
	*tmp = params;

	printf("config_serial_tag\n");
}
#endif

#ifdef CONFIG_REVISION_TAG
void setup_revision_tag(struct tag **in_params)
{
	u32 rev = 0;
	u32 get_board_rev(void);

	rev = get_board_rev();

	params->hdr.size = tag_size(tag_revision);
	params->u.revision.rev = rev;
	params = tag_next(params);
}
#endif  /* CONFIG_REVISION_TAG */

static void setup_end_tag(bd_t *bd)
{
	params->hdr.tag = ATAG_NONE;
	params->hdr.size = 0;
}

#endif /* CONFIG_SETUP_MEMORY_TAGS || CONFIG_CMDLINE_TAG || ... */
