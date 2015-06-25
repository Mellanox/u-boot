OUTPUT_FORMAT("elf32-littlearc", "elf32-bigarc", "elf32-littlearc")
OUTPUT_ARCH(arc)
ENTRY(__start)
SECTIONS
{

	. = ALIGN(4); 
	.text      :
	{

	 __text_start = .;
	  *(.text)	
	}
	. = ALIGN(4);
	__text_end = .;

  .sdata          :
  {
    __SDATA_BEGIN__ = .;
    *(.sdata .sdata.* .gnu.linkonce.s.*)
  }

	__bss_start = .;
	.sbss : { 
    PROVIDE (__sbss_start = .);
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    PROVIDE (__sbss_end = .);
       }

	.bss : { 
    *(.bss) 
       }

	__bss_end = .;

	__data_start = .;
	.rodata : {
        *(.rodata .rodata.*)

    }
	
	. = ALIGN(4);
	
	.data : { *(.data) }
	. = ALIGN(4);
	.got : { *(.got) }
	
	. = ALIGN(4);
	__u_boot_cmd_start = .;
	.u_boot_cmd : { *(.u_boot_cmd) }
	__u_boot_cmd_end = .;

	. = ALIGN(4);
	
	__data_end = .;


    /* following statement was added to be able to debug using mdb */
    /* DWARF 1 */
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }
	/* GNU DWARF 1 extensions */
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }
	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	/* DWARF 2 */
	.debug_info     0 : { *(.debug_info) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }

}
