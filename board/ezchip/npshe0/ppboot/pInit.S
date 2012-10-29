
     .extern spiFlash2RamGo
     .extern prebootParams
     .global __start
     .text
__start:
        b        start_400  ; 00
        .rept    0x3fc
        .byte    0
        .endr
start_400:
        lr      r0,[0x11] /* Disable Instruction Cache */
        or      r0,r0,1
        sr      r0,[0x11]
        sr      1,[0x47] /* Flush Data Cache */
        lr      r0,[0x48] /* Disable Data Cache */
        or      r0,r0,1
        sr      r0,[0x48]
        lr      r0,[0x04] /* IDENTITY register */
        lsr     r0,r0,8
        and.f   r0,r0,0x000000FF
        bnz     core_nz      /* all cores except core 0 don't have stack!!! */
        mov_s 	r1,0x00590000   /* SRC_FLASH_OFF */
        mov 	sp,stack_bot
/* int spiFlash2RamGo(int coreNum,int src); return coreNum */
        bl    	spiFlash2RamGo
core_nz:
        ld      r1,[prebootParams+0x0c] /* prebootParams[3] */
        j       [r1]
        nop_s

	.global __st_r13_to_r13
	.global __st_r13_to_r14
	.global __st_r13_to_r15
__st_r13_to_r15:
	st r15, [sp,8]
__st_r13_to_r14:
	st r14, [sp,4]
__st_r13_to_r13:
	j_s.d [%blink]
	st r13, [sp,0]



;	==================================
;	the loads

	.global __ld_r13_to_r13_ret
	.global __ld_r13_to_r14_ret
	.global __ld_r13_to_r15_ret
	.align 4

__ld_r13_to_r15_ret:
	ld r15, [sp,8]
__ld_r13_to_r14_ret:
	ld r14, [sp,4]
__ld_r13_to_r13_ret:
	ld_s r13, [sp,0]
	j [%blink]

	.section .bss

stack_top:
      .space   160
stack_bot:

