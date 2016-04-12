/* { dg-do compile } */
/* { dg-options "-O2 -mno-mpx -mno-sse -mno-mmx -mno-80387 -mcld" } */

extern void bar (void);

void
__attribute__ ((interrupt))
foo (void *frame)
{
  bar ();
}

/* { dg-final { scan-assembler-times "iret" 1 { target ia32 } } } */
/* { dg-final { scan-assembler-times "iretq" 1 { target { ! ia32 } } } } */
/* { dg-final { scan-assembler-not "\tcld" } } */
