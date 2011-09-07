/* { dg-do compile } */
/* { dg-options "-O2 -fdump-tree-ivopts" } */

void
f1 (char *p, unsigned long int i, unsigned long int n)
{
  p += i;
  do
    {
      *p = '\0';
      p += 1;
      i++;
    }
  while (i < n);
}

/* { dg-final { scan-tree-dump-times "PHI" 1 "ivopts"} } */
/* { dg-final { scan-tree-dump-times "PHI <p_" 1 "ivopts"} } */
/* { dg-final { scan-tree-dump-times "p_\[0-9\]* <" 1 "ivopts"} } */
/* { dg-final { cleanup-tree-dump "ivopts" } } */
