#ifndef JFDCTINT_H
#define JFDCTINT_H

/*
 * jfdctint.c
 *
 * Copyright (C) 1991-1996, Thomas G. Lane.
 * Modification developed 2003-2009 by Guido Vollbeding.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains a slow-but-accurate integer implementation of the
 * forward DCT (Discrete Cosine Transform).
 *
 * A 2-D DCT can be done by 1-D DCT on each row followed by 1-D DCT
 * on each column.  Direct algorithms are also available, but they are
 * much more complex and seem not to be any faster when reduced to code.
 *
 * This implementation is based on an algorithm described in
 *   C. Loeffler, A. Ligtenberg and G. Moschytz, "Practical Fast 1-D DCT
 *   Algorithms with 11 Multiplications", Proc. Int'l. Conf. on Acoustics,
 *   Speech, and Signal Processing 1989 (ICASSP '89), pp. 988-991.
 * The primary algorithm described there uses 11 multiplies and 29 adds.
 * We use their alternate method with 12 multiplies and 32 adds.
 * The advantage of this method is that no data path contains more than one
 * multiplication; this allows a very simple and accurate implementation in
 * scaled fixed-point arithmetic, with a minimal number of shifts.
 *
 * We also provide FDCT routines with various input sample block sizes for
 * direct resolution reduction or enlargement and for direct resolving the
 * common 2x1 and 1x2 subsampling cases without additional resampling: NxN
 * (N=1...16), 2NxN, and Nx2N (N=1...8) pixels for one 8x8 output DCT block.
 *
 * For N<8 we fill the remaining block coefficients with zero.
 * For N>8 we apply a partial N-point FDCT on the input samples, computing
 * just the lower 8 frequency coefficients and discarding the rest.
 *
 * We must scale the output coefficients of the N-point FDCT appropriately
 * to the standard 8-point FDCT level by 8/N per 1-D pass.  This scaling
 * is folded into the constant multipliers (pass 2) and/or final/initial
 * shifting.
 *
 * CAUTION: We rely on the FIX() macro except for the N=1,2,4,8 cases
 * since there would be too many additional constants to pre-calculate.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "LibJpeg.h"
#include "jdct.h"		/* Private declarations for DCT subsystem */

#ifdef DCT_ISLOW_SUPPORTED


/*
 * This module is specialized to the case DCTSIZE = 8.
 */

#if DCTSIZE != 8
  Sorry, this code only copes with 8x8 DCT blocks. /* deliberate syntax err */
#endif


/*
 * The poop on this scaling stuff is as follows:
 *
 * Each 1-D DCT step produces outputs which are a factor of sqrt(N)
 * larger than the true DCT outputs.  The final outputs are therefore
 * a factor of N larger than desired; since N=8 this can be cured by
 * a simple right shift at the end of the algorithm.  The advantage of
 * this arrangement is that we save two multiplications per 1-D DCT,
 * because the y0 and y4 outputs need not be divided by sqrt(N).
 * In the IJG code, this factor of 8 is removed by the quantization step
 * (in jcdctmgr.c), NOT in this module.
 *
 * We have to do addition and subtraction of the integer inputs, which
 * is no problem, and multiplication by fractional constants, which is
 * a problem to do in integer arithmetic.  We multiply all the constants
 * by CONST_SCALE and convert them to integer constants (thus retaining
 * CONST_BITS bits of precision in the constants).  After doing a
 * multiplication we have to divide the product by CONST_SCALE, with proper
 * rounding, to produce the correct output.  This division can be done
 * cheaply as a right shift of CONST_BITS bits.  We postpone shifting
 * as long as possible so that partial sums can be added together with
 * full fractional precision.
 *
 * The outputs of the first pass are scaled up by PASS1_BITS bits so that
 * they are represented to better-than-integral precision.  These outputs
 * require BITS_IN_JSAMPLE + PASS1_BITS + 3 bits; this fits in a 16-bit word
 * with the recommended scaling.  (For 12-bit sample data, the intermediate
 * array is INT32 anyway.)
 *
 * To avoid overflow of the 32-bit intermediate results in pass 2, we must
 * have BITS_IN_JSAMPLE + CONST_BITS + PASS1_BITS <= 26.  Error analysis
 * shows that the values given below are the most effective.
 */

#if BITS_IN_JSAMPLE == 8
#define CONST_BITS  13
#define PASS1_BITS  2
#else
#define CONST_BITS  13
#define PASS1_BITS  1		/* lose a little precision to avoid overflow */
#endif

/* Some C compilers fail to reduce "FIX(constant)" at compile time, thus
 * causing a lot of useless floating-point operations at run time.
 * To get around this we use the following pre-calculated constants.
 * If you change CONST_BITS you may want to add appropriate values.
 * (With a reasonable C compiler, you can just rely on the FIX() macro...)
 */

#if CONST_BITS == 13
#define FIX_0_298631336  ((INT32)  2446)	/* FIX(0.298631336) */
#define FIX_0_390180644  ((INT32)  3196)	/* FIX(0.390180644) */
#define FIX_0_541196100  ((INT32)  4433)	/* FIX(0.541196100) */
#define FIX_0_765366865  ((INT32)  6270)	/* FIX(0.765366865) */
#define FIX_0_899976223  ((INT32)  7373)	/* FIX(0.899976223) */
#define FIX_1_175875602  ((INT32)  9633)	/* FIX(1.175875602) */
#define FIX_1_501321110  ((INT32)  12299)	/* FIX(1.501321110) */
#define FIX_1_847759065  ((INT32)  15137)	/* FIX(1.847759065) */
#define FIX_1_961570560  ((INT32)  16069)	/* FIX(1.961570560) */
#define FIX_2_053119869  ((INT32)  16819)	/* FIX(2.053119869) */
#define FIX_2_562915447  ((INT32)  20995)	/* FIX(2.562915447) */
#define FIX_3_072711026  ((INT32)  25172)	/* FIX(3.072711026) */
#else
#define FIX_0_298631336  FIX(0.298631336)
#define FIX_0_390180644  FIX(0.390180644)
#define FIX_0_541196100  FIX(0.541196100)
#define FIX_0_765366865  FIX(0.765366865)
#define FIX_0_899976223  FIX(0.899976223)
#define FIX_1_175875602  FIX(1.175875602)
#define FIX_1_501321110  FIX(1.501321110)
#define FIX_1_847759065  FIX(1.847759065)
#define FIX_1_961570560  FIX(1.961570560)
#define FIX_2_053119869  FIX(2.053119869)
#define FIX_2_562915447  FIX(2.562915447)
#define FIX_3_072711026  FIX(3.072711026)
#endif


/* Multiply an INT32 variable by an INT32 constant to yield an INT32 result.
 * For 8-bit samples with the recommended scaling, all the variable
 * and constant values involved are no more than 16 bits wide, so a
 * 16x16->32 bit multiply can be used instead of a full 32x32 multiply.
 * For 12-bit samples, a full 32-bit multiplication will be needed.
 */

#if BITS_IN_JSAMPLE == 8
#define MULTIPLY(var,const)  MULTIPLY16C16(var,const)
#else
#define MULTIPLY(var,const)  ((var) * (const))
#endif


 /*
 * Perform the forward DCT on one block of samples.
 */
void jpeg_fdct_islow ( DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col );


#ifdef DCT_SCALING_SUPPORTED

/*
 * Perform the forward DCT on a 7x7 sample block.
 */
void jpeg_fdct_7x7 ( DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col );

/*
 * Perform the forward DCT on a 6x6 sample block.
 */
void jpeg_fdct_6x6 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on a 5x5 sample block.
 */
void jpeg_fdct_5x5 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on a 4x4 sample block.
 */
void jpeg_fdct_4x4 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on a 3x3 sample block.
 */
void jpeg_fdct_3x3 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on a 2x2 sample block.
 */
void jpeg_fdct_2x2 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on a 1x1 sample block.
 */
void jpeg_fdct_1x1 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);


/*
 * Perform the forward DCT on a 9x9 sample block.
 */
void jpeg_fdct_9x9 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);


/*
 * Perform the forward DCT on a 10x10 sample block.
 */
void jpeg_fdct_10x10 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on an 11x11 sample block.
 */
void jpeg_fdct_11x11 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);


/*
 * Perform the forward DCT on a 12x12 sample block.
 */
void jpeg_fdct_12x12 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);


/*
 * Perform the forward DCT on a 13x13 sample block.
 */
void jpeg_fdct_13x13 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);


/*
 * Perform the forward DCT on a 14x14 sample block.
 */
void jpeg_fdct_14x14 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);


/*
 * Perform the forward DCT on a 15x15 sample block.
 */
void jpeg_fdct_15x15 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on a 16x16 sample block.
 */
void jpeg_fdct_16x16 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on a 16x8 sample block.
 *
 * 16-point FDCT in pass 1 (rows), 8-point in pass 2 (columns).
 */
void jpeg_fdct_16x8 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on a 14x7 sample block.
 *
 * 14-point FDCT in pass 1 (rows), 7-point in pass 2 (columns).
 */
void jpeg_fdct_14x7 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on a 12x6 sample block.
 *
 * 12-point FDCT in pass 1 (rows), 6-point in pass 2 (columns).
 */
void jpeg_fdct_12x6 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on a 10x5 sample block.
 *
 * 10-point FDCT in pass 1 (rows), 5-point in pass 2 (columns).
 */
void jpeg_fdct_10x5 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on an 8x4 sample block.
 *
 * 8-point FDCT in pass 1 (rows), 4-point in pass 2 (columns).
 */
void jpeg_fdct_8x4 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on a 6x3 sample block.
 *
 * 6-point FDCT in pass 1 (rows), 3-point in pass 2 (columns).
 */
void jpeg_fdct_6x3 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on a 4x2 sample block.
 *
 * 4-point FDCT in pass 1 (rows), 2-point in pass 2 (columns).
 */
void jpeg_fdct_4x2 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on a 2x1 sample block.
 *
 * 2-point FDCT in pass 1 (rows), 1-point in pass 2 (columns).
 */
void jpeg_fdct_2x1 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on an 8x16 sample block.
 *
 * 8-point FDCT in pass 1 (rows), 16-point in pass 2 (columns).
 */
void jpeg_fdct_8x16 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on a 7x14 sample block.
 *
 * 7-point FDCT in pass 1 (rows), 14-point in pass 2 (columns).
 */
void jpeg_fdct_7x14 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on a 6x12 sample block.
 *
 * 6-point FDCT in pass 1 (rows), 12-point in pass 2 (columns).
 */
void jpeg_fdct_6x12 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on a 5x10 sample block.
 *
 * 5-point FDCT in pass 1 (rows), 10-point in pass 2 (columns).
 */
void jpeg_fdct_5x10 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on a 4x8 sample block.
 *
 * 4-point FDCT in pass 1 (rows), 8-point in pass 2 (columns).
 */
void jpeg_fdct_4x8 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on a 3x6 sample block.
 *
 * 3-point FDCT in pass 1 (rows), 6-point in pass 2 (columns).
 */
void jpeg_fdct_3x6 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on a 2x4 sample block.
 *
 * 2-point FDCT in pass 1 (rows), 4-point in pass 2 (columns).
 */
void jpeg_fdct_2x4 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/*
 * Perform the forward DCT on a 1x2 sample block.
 *
 * 1-point FDCT in pass 1 (rows), 2-point in pass 2 (columns).
 */
void jpeg_fdct_1x2 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

#endif /* DCT_SCALING_SUPPORTED */
#endif /* DCT_ISLOW_SUPPORTED */
#endif