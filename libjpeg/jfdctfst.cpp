#include "jfdctfst.h"

#ifdef DCT_IFAST_SUPPORTED
void jpeg_fdct_ifast (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col)
{
	DCTELEM tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  DCTELEM tmp10, tmp11, tmp12, tmp13;
  DCTELEM z1, z2, z3, z4, z5, z11, z13;
  DCTELEM *dataptr;
  JSAMPROW elemptr;
  int ctr;
  SHIFT_TEMPS

  /* Pass 1: process rows. */

  dataptr = data;
  for (ctr = 0; ctr < DCTSIZE; ctr++) {
    elemptr = sample_data[ctr] + start_col;

    /* Load data into workspace */
    tmp0 = GETJSAMPLE(elemptr[0]) + GETJSAMPLE(elemptr[7]);
    tmp7 = GETJSAMPLE(elemptr[0]) - GETJSAMPLE(elemptr[7]);
    tmp1 = GETJSAMPLE(elemptr[1]) + GETJSAMPLE(elemptr[6]);
    tmp6 = GETJSAMPLE(elemptr[1]) - GETJSAMPLE(elemptr[6]);
    tmp2 = GETJSAMPLE(elemptr[2]) + GETJSAMPLE(elemptr[5]);
    tmp5 = GETJSAMPLE(elemptr[2]) - GETJSAMPLE(elemptr[5]);
    tmp3 = GETJSAMPLE(elemptr[3]) + GETJSAMPLE(elemptr[4]);
    tmp4 = GETJSAMPLE(elemptr[3]) - GETJSAMPLE(elemptr[4]);

    /* Even part */

    tmp10 = tmp0 + tmp3;	/* phase 2 */
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    /* Apply unsigned->signed conversion */
    dataptr[0] = tmp10 + tmp11 - 8 * CENTERJSAMPLE; /* phase 3 */
    dataptr[4] = tmp10 - tmp11;

    z1 = MULTIPLY(tmp12 + tmp13, FIX_0_707106781); /* c4 */
    dataptr[2] = tmp13 + z1;	/* phase 5 */
    dataptr[6] = tmp13 - z1;

    /* Odd part */

    tmp10 = tmp4 + tmp5;	/* phase 2 */
    tmp11 = tmp5 + tmp6;
    tmp12 = tmp6 + tmp7;

    /* The rotator is modified from fig 4-8 to avoid extra negations. */
    z5 = MULTIPLY(tmp10 - tmp12, FIX_0_382683433); /* c6 */
    z2 = MULTIPLY(tmp10, FIX_0_541196100) + z5; /* c2-c6 */
    z4 = MULTIPLY(tmp12, FIX_1_306562965) + z5; /* c2+c6 */
    z3 = MULTIPLY(tmp11, FIX_0_707106781); /* c4 */

    z11 = tmp7 + z3;		/* phase 5 */
    z13 = tmp7 - z3;

    dataptr[5] = z13 + z2;	/* phase 6 */
    dataptr[3] = z13 - z2;
    dataptr[1] = z11 + z4;
    dataptr[7] = z11 - z4;

    dataptr += DCTSIZE;		/* advance pointer to next row */
  }

  /* Pass 2: process columns. */

  dataptr = data;
  for (ctr = DCTSIZE-1; ctr >= 0; ctr--) {
    tmp0 = dataptr[DCTSIZE*0] + dataptr[DCTSIZE*7];
    tmp7 = dataptr[DCTSIZE*0] - dataptr[DCTSIZE*7];
    tmp1 = dataptr[DCTSIZE*1] + dataptr[DCTSIZE*6];
    tmp6 = dataptr[DCTSIZE*1] - dataptr[DCTSIZE*6];
    tmp2 = dataptr[DCTSIZE*2] + dataptr[DCTSIZE*5];
    tmp5 = dataptr[DCTSIZE*2] - dataptr[DCTSIZE*5];
    tmp3 = dataptr[DCTSIZE*3] + dataptr[DCTSIZE*4];
    tmp4 = dataptr[DCTSIZE*3] - dataptr[DCTSIZE*4];

    /* Even part */

    tmp10 = tmp0 + tmp3;	/* phase 2 */
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    dataptr[DCTSIZE*0] = tmp10 + tmp11; /* phase 3 */
    dataptr[DCTSIZE*4] = tmp10 - tmp11;

    z1 = MULTIPLY(tmp12 + tmp13, FIX_0_707106781); /* c4 */
    dataptr[DCTSIZE*2] = tmp13 + z1; /* phase 5 */
    dataptr[DCTSIZE*6] = tmp13 - z1;

    /* Odd part */

    tmp10 = tmp4 + tmp5;	/* phase 2 */
    tmp11 = tmp5 + tmp6;
    tmp12 = tmp6 + tmp7;

    /* The rotator is modified from fig 4-8 to avoid extra negations. */
    z5 = MULTIPLY(tmp10 - tmp12, FIX_0_382683433); /* c6 */
    z2 = MULTIPLY(tmp10, FIX_0_541196100) + z5; /* c2-c6 */
    z4 = MULTIPLY(tmp12, FIX_1_306562965) + z5; /* c2+c6 */
    z3 = MULTIPLY(tmp11, FIX_0_707106781); /* c4 */

    z11 = tmp7 + z3;		/* phase 5 */
    z13 = tmp7 - z3;

    dataptr[DCTSIZE*5] = z13 + z2; /* phase 6 */
    dataptr[DCTSIZE*3] = z13 - z2;
    dataptr[DCTSIZE*1] = z11 + z4;
    dataptr[DCTSIZE*7] = z11 - z4;

    dataptr++;			/* advance pointer to next column */
  }
}

#endif /* DCT_IFAST_SUPPORTED */