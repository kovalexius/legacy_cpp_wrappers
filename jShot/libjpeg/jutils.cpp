#include "jutils.h"



long jdiv_round_up (long a, long b)
/* Compute a/b rounded up to next integer, ie, ceil(a/b) */
/* Assumes a >= 0, b > 0 */
{
  return (a + b - 1L) / b;
}


long jround_up (long a, long b)
/* Compute a rounded up to next multiple of b, ie, ceil(a/b)*b */
/* Assumes a >= 0, b > 0 */
{
  a += b - 1L;
  return a - (a % b);
}

#ifdef NEED_FAR_POINTERS
#ifndef USE_FMEM
void jzero_far ( void FAR * target, size_t bytestozero )
{
	register char FAR * ptr = (char FAR *) target;
  register size_t count;

  for (count = bytestozero; count > 0; count--) 
	{
    *ptr++ = 0;
  }
}
#endif
#endif

void jcopy_sample_rows ( JSAMPARRAY input_array, int source_row, JSAMPARRAY output_array, int dest_row, int num_rows, JDIMENSION num_cols )
{
	register JSAMPROW inptr, outptr;
#ifdef FMEMCOPY
  register size_t count = (size_t) (num_cols * SIZEOF(JSAMPLE));
#else
  register JDIMENSION count;
#endif
  register int row;

  input_array += source_row;
  output_array += dest_row;

  for (row = num_rows; row > 0; row--) 
	{
    inptr = *input_array++;
    outptr = *output_array++;
#ifdef FMEMCOPY
    FMEMCOPY(outptr, inptr, count);
#else
    for (count = num_cols; count > 0; count--)
      *outptr++ = *inptr++;	/* needn't bother with GETJSAMPLE() here */
#endif
  }
}

void jcopy_block_row (JBLOCKROW input_row, JBLOCKROW output_row, JDIMENSION num_blocks )
{
	#ifdef FMEMCOPY
  FMEMCOPY(output_row, input_row, num_blocks * (DCTSIZE2 * SIZEOF(JCOEF)));
#else
  register JCOEFPTR inptr, outptr;
  register long count;

  inptr = (JCOEFPTR) input_row;
  outptr = (JCOEFPTR) output_row;
  for (count = (long) num_blocks * DCTSIZE2; count > 0; count--) {
    *outptr++ = *inptr++;
  }
#endif
}