
	.section .text , "ax" , @progbits
	.type _start, @function
	.align 4
	.globl __start
	.extern loadRunUboot

	/* ppboot header */
___start:
	b __start
	.word __text_start
	.word __data_end
	.word __start

__start:
; disable I cache
		lr  r1,[0x11]
		or  r1,r1,1
		sr  r1,[0x11]
		and.f  r20,r0,r0
		jnz  core_nz
		mov  sp,stackBot
;;;;; Pre boot initialization area start
;;;;; Pre boot initialization area end
		mov  r0,0x00500000
		b  loadRunUboot

core_nz:
		mov  r2,0xFFFFFFF8	;secondary cpu kernel entry
		ld  r0,[r2,0]
		ld  r1,[r2,4]
		j  [r1]

	.section .bss

stackTop:
	.space    0x400
stackBot:


