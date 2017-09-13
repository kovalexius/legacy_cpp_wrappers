#ifndef JCPREPCT_H
#define JCPREPCT_H

/*
 * jcprepct.c
 *
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains the compression preprocessing controller.
 * This controller manages the color conversion, downsampling,
 * and edge expansion steps.
 *
 * Most of the complexity here is associated with buffering input rows
 * as required by the downsampler.  See the comments at the head of
 * jcsample.c for the downsampler's needs.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "LibJpeg.h"


/* At present, jcsample.c can request context rows only for smoothing.
 * In the future, we might also need context rows for CCIR601 sampling
 * or other more-complex downsampling procedures.  The code to support
 * context rows should be compiled only if needed.
 */
#ifdef INPUT_SMOOTHING_SUPPORTED
#define CONTEXT_ROWS_SUPPORTED
#endif


/*
 * For the simple (no-context-row) case, we just need to buffer one
 * row group's worth of pixels for the downsampling step.  At the bottom of
 * the image, we pad to a full row group by replicating the last pixel row.
 * The downsampler's last output row is then replicated if needed to pad
 * out to a full iMCU row.
 *
 * When providing context rows, we must buffer three row groups' worth of
 * pixels.  Three row groups are physically allocated, but the row pointer
 * arrays are made five row groups high, with the extra pointers above and
 * below "wrapping around" to point to the last and first real row groups.
 * This allows the downsampler to access the proper context rows.
 * At the top and bottom of the image, we create dummy context rows by
 * copying the first or last real pixel row.  This copying could be avoided
 * by pointer hacking as is done in jdmainct.c, but it doesn't seem worth the
 * trouble on the compression side.
 */


/* Private buffer controller object */
struct  my_prep_controller
{
  sJpeg_c_prep_controller pub; /* public fields */

  /* Downsampling input buffer.  This buffer holds color-converted data
   * until we have enough to do a downsample step.
   */
  JSAMPARRAY color_buf[MAX_COMPONENTS];

  JDIMENSION rows_to_go;	/* counts rows remaining in source image */
  int next_buf_row;		/* index of next row to store in color_buf */

#ifdef CONTEXT_ROWS_SUPPORTED	/* only needed for context case */
  int this_row_group;		/* starting row index of group to process */
  int next_buf_stop;		/* downsample when we reach this index */
#endif
};


/*
 * Initialize for a processing pass.
 */
static void start_pass_prep ( sJpeg_compress_struct *cinfo, J_BUF_MODE pass_mode );

/*
 * Expand an image vertically from height input_rows to height output_rows,
 * by duplicating the bottom row.
 */
static void expand_bottom_edge (JSAMPARRAY image_data, JDIMENSION num_cols, int input_rows, int output_rows );

/*
 * Process some data in the simple no-context case.
 *
 * Preprocessor output data is counted in "row groups".  A row group
 * is defined to be v_samp_factor sample rows of each component.
 * Downsampling will produce this much data from each max_v_samp_factor
 * input rows.
 */
static void pre_process_data ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JDIMENSION *in_row_ctr, JDIMENSION in_rows_avail,
		  JSAMPIMAGE output_buf, JDIMENSION *out_row_group_ctr, JDIMENSION out_row_groups_avail );

#ifdef CONTEXT_ROWS_SUPPORTED
/*
 * Process some data in the context case.
 */
static void pre_process_context ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JDIMENSION *in_row_ctr, JDIMENSION in_rows_avail, JSAMPIMAGE output_buf, JDIMENSION *out_row_group_ctr, JDIMENSION out_row_groups_avail );

/*
 * Create the wrapped-around downsampling input buffer needed for context mode.
 */
static void create_context_buffer ( sJpeg_compress_struct *cinfo );
#endif /* CONTEXT_ROWS_SUPPORTED */

/*
 * Initialize preprocessing controller.
 */
void jinit_c_prep_controller ( sJpeg_compress_struct *cinfo, boolean need_full_buffer );

#endif