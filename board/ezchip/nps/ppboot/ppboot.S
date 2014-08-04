
     .extern spiFlash2RamGo
     .extern prebootParams
     .global __start
     .text

__start:
        lr      r0,[0x11] /* Disable Instruction Cache */
        or      r0,r0,1
        sr      r0,[0x11]
        sr      1,[0x47] /* Flush Data Cache */
        lr      r0,[0x48] /* Disable Data Cache */
        or      r0,r0,1
        sr      r0,[0x48]
        lr      r0,[0x04] /* IDENTITY register */
        lsr     r0,r0,8
        and.f   r0,r0,0xFF
        bnz     core_nz      /* all cores except core 0 don't have stack!!! */
        mov_s 	r1,0x00590000   /* SRC_FLASH_OFF */
        mov 	sp,stack_bot
/* int spiFlash2RamGo(int coreNum,int src); return coreNum */
        bl    	spiFlash2RamGo
core_nz:
        ld      r1,[prebootParams+0x0c] /* prebootParams[3] */
        j       [r1]
        nop_s

	.section .bss

stack_top:
      .space   160
stack_bot:

