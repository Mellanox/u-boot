/*
	setjmp.h
	stubs for future use.
*/

#ifndef _SETJMP_H_
#define _SETJMP_H_

//#include "_ansi.h"
// #include <../board/npshe0/setjmp2.h>

typedef int jmp_buf[36];

void longjmp(jmp_buf __jmpb, int __retval);
int	setjmp(jmp_buf __jmpb);


#endif /* _SETJMP_H_ */

