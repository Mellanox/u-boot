OUTPUT_FORMAT("elf32-littlearc", "elf32-bigarc", "elf32-littlearc")
OUTPUT_ARCH(arc)
ENTRY(__start)
SECTIONS {

	.text : {*(.text)}
	.lit : {*(.lit)}
	.sdata : {*(.sdata)}
	.sbss : {*(.sbss)}
	.data : {*(.data)}
	.bss : {*(.bss)}

}
