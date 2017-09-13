#ifndef JCDCTMGR_H
#define JCDCTMGR_H

/*
 * jcdctmgr.c
 *
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains the forward-DCT management logic.
 * This code selects a particular DCT implementation to be used,
 * and it performs related housekeeping chores including coefficient
 * quantization.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "LibJpeg.h"
#include "jdct.h"		/* Private declarations for DCT subsystem */



/* Private subobject for this module */

struct  my_fdct_controller
{
  sJpeg_forward_dct pub;	/* public fields */

  /* Pointer to the DCT routine actually in use */
  forward_DCT_method_ptr do_dct[MAX_COMPONENTS];

  /* The actual post-DCT divisors --- not identical to the quant table
   * entries, because of scaling (especially for an unnormalized DCT).
   * Each table is given in normal array order.
   */
  DCTELEM * divisors[NUM_QUANT_TBLS];

#ifdef DCT_FLOAT_SUPPORTED
  /* Same as above for the floating-point case. */
  float_DCT_method_ptr do_float_dct[MAX_COMPONENTS];
  FAST_FLOAT * float_divisors[NUM_QUANT_TBLS];
#endif
};

typedef my_fdct_controller * my_fdct_ptr;


/* The current scaled-DCT routines require ISLOW-style divisor tables,
 * so be sure to compile that code if either ISLOW or SCALING is requested.
 */
#ifdef DCT_ISLOW_SUPPORTED
#define PROVIDE_ISLOW_TABLES
#else
#ifdef DCT_SCALING_SUPPORTED
#define PROVIDE_ISLOW_TABLES
#endif
#endif


/*
 * Perform forward DCT on one or more blocks of a component.
 *
 * The input samples are taken from the sample_data[] array starting at
 * position start_row/start_col, and moving to the right for any additional
 * blocks. The quantized coefficients are returned in coef_blocks[].
 */
static void forward_DCT ( sJpeg_compress_struct *cinfo, sJpeg_component_info * compptr, JSAMPARRAY sample_data, JBLOCKROW coef_blocks, JDIMENSION start_row, JDIMENSION start_col, JDIMENSION num_blocks );
/* This version is used for integer DCT implementations. */

#ifdef DCT_FLOAT_SUPPORTED
static void forward_DCT_float ( sJpeg_compress_struct *cinfo, sJpeg_component_info * compptr, JSAMPARRAY sample_data, JBLOCKROW coef_blocks,JDIMENSION start_row, JDIMENSION start_col, JDIMENSION num_blocks );
/* This version is used for floating-point DCT implementations. */
#endif /* DCT_FLOAT_SUPPORTED */

/*
 * Initialize for a processing pass.
 * Verify that all referenced Q-tables are present, and set up
 * the divisor table for each one.
 * In the current implementation, DCT of all components is done during
 * the first pass, even if only some components will be output in the
 * first scan.  Hence all components should be examined here.
 */
static void start_pass_fdctmgr ( sJpeg_compress_struct *cinfo );


/*
 * Initialize FDCT manager.
 */
void jinit_forward_dct ( sJpeg_compress_struct *cinfo );

#endif