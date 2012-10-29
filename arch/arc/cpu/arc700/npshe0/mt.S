
#include <asm/arcregs.h>
#include <config.h>
 
		.global rt_bwl
		.global rtsc32
		.global rtsc64

rt_bwl:
;
        mov        r2,0x01234567
        mov        r3,0x89abcdef
        mov        r4,0x02468ace
        mov        r5,0x13579bdf
	st.di      r2,[r0,0]
	st.di      r3,[r0,4]
	st.di      r4,[r0,8]
	st.di      r5,[r0,12]
;
	ldw.di     r2,[r0,0]
	ldw.di     r3,[r0,2]
	ldw.di     r4,[r0,4]
	ldw.di     r5,[r0,6]
	ldw.di     r6,[r0,8]
	ldw.di     r7,[r0,10]
	ldw.di     r8,[r0,12]
	ldw.di     r9,[r0,14]
;
	stw.di     r2,[r0,32]
	stw.di     r3,[r0,34]
	stw.di     r4,[r0,36]
	stw.di     r5,[r0,38]
	stw.di     r6,[r0,40]
	stw.di     r7,[r0,42]
	stw.di     r8,[r0,44]
	stw.di     r9,[r0,46]
;
	ldb.di     r2,[r0,32]
	ldb.di     r3,[r0,33]
	ldb.di     r4,[r0,34]
	ldb.di     r5,[r0,35]
	ldb.di     r6,[r0,36]
	ldb.di     r7,[r0,37]
	ldb.di     r8,[r0,38]
	ldb.di     r9,[r0,39]
;
	stb.di     r2,[r0,64]
	stb.di     r3,[r0,65]
	stb.di     r4,[r0,66]
	stb.di     r5,[r0,67]
	stb.di     r6,[r0,68]
	stb.di     r7,[r0,69]
	stb.di     r8,[r0,70]
	stb.di     r9,[r0,71]
;
	ldb.di     r2,[r0,40]
	ldb.di     r3,[r0,41]
	ldb.di     r4,[r0,42]
	ldb.di     r5,[r0,43]
	ldb.di     r6,[r0,44]
	ldb.di     r7,[r0,45]
	ldb.di     r8,[r0,46]
	ldb.di     r9,[r0,47]
;
	stb.di     r2,[r0,72]
	stb.di     r3,[r0,73]
	stb.di     r4,[r0,74]
	stb.di     r5,[r0,75]
	stb.di     r6,[r0,76]
	stb.di     r7,[r0,77]
	stb.di     r8,[r0,78]
	stb.di     r9,[r0,79]
;
        ld.di	   r2,[r0,0]
        ld.di      r3,[r0,64]
        ld.di	   r4,[r0,4]
        ld.di      r5,[r0,68]
        ld.di	   r6,[r0,8]
        ld.di      r7,[r0,72]
        ld.di	   r8,[r0,12]
        ld.di      r9,[r0,76]
;
        brne       r2,r3,delta1
        brne       r4,r5,delta2
        brne       r6,r7,delta3
        brne       r8,r9,delta4
        mov        r0,0
exit_bwl:
        j_s        [blink]
;
delta1:
        b.d        exit_e_bwl
        mov        r0,r2
delta2:
        b.d        exit_e_bwl
        mov        r0,r4
delta3:
        b.d        exit_e_bwl
        mov        r0,r6
delta4:
        mov        r0,r8
exit_e_bwl:
        mov_s      r2,0x90000000
        b.d        exit_bwl
        or_s       r0,r0,r2

        .align 4
; unsigned long rtsc32(void)
rtsc32:
;        rtsc	   r0,0
        .long      0x306f001a
        j_s        [blink]

        .align 4
; unsigned long long rtsc64(void)
rtsc64:
;        rtsc	   r1,0
        .long      0x316f001a
;        mov        r0,r58
        .long      0x200a0e80
        j_s        [blink]


; int swapAsm(int firstValue,int *vec,int loops)

	.global swapAsm
        .align 4
swapAsm:
        mov	       lp_count,r2
        lp	       swap_10
;        rtsc	   r2,0
        .long      0x326f001a
        and        r2,r2,3
        add        r3,r2,1
        and        r3,r3,3
        add2       r3,r1,r3
        add2       r2,r1,r2
        ex         r0,[r3]
        ex         r0,[r2]
swap_10:
        j_s        [blink]

; int dummyCall(int value);  - return value to the caller

	.global dummyCall
        .align 4
dummyCall:
        j_s        [blink]
