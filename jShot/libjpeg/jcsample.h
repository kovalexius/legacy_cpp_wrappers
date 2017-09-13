#ifndef JCSAMPLE_H
#define JCSAMPLE_H

/*
 * jcsample.c
 *
 * Copyright (C) 1991-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains downsampling routines.
 *
 * Downsampling input data is counted in "row groups".  A row group
 * is defined to be max_v_samp_factor pixel rows of each component,
 * from which the downsampler produces v_samp_factor sample rows.
 * A single row group is processed in each call to the downsampler module.
 *
 * The downsampler is responsible for edge-expansion of its output data
 * to fill an integral number of DCT blocks horizontally.  The source buffer
 * may be modified if it is helpful for this purpose (the source buffer is
 * allocated wide enough to correspond to the desired output width).
 * The caller (the prep controller) is responsible for vertical padding.
 *
 * The downsampler may request "context rows" by setting need_context_rows
 * during startup.  In this case, the input arrays will contain at least
 * one row group's worth of pixels above and below the passed-in data;
 * the caller will create dummy rows at image top and bottom by replicating
 * the first or last real pixel row.
 *
 * An excellent reference for image resampling is
 *   Digital Image Warping, George Wolberg, 1990.
 *   Pub. by IEEE Computer Society Press, Los Alamitos, CA. ISBN 0-8186-8944-7.
 *
 * The downsampling algorithm used here is a simple average of the source
 * pixels covered by the output pixel.  The hi-falutin sampling literature
 * refers to this as a "box filter".  In general the characteristics of a box
 * filter are not very good, but for the specific cases we normally use (1:1
 * and 2:1 ratios) the box is equivalent to a "triangle filter" which is not
 * nearly so bad.  If you intend to use other sampling ratios, you'd be well
 * advised to improve this code.
 *
 * A simple input-smoothing capability is provided.  This is mainly intended
 * for cleaning up color-dithered GIF input files (if you find it inadequate,
 * we suggest using an external filtering program such as pnmconvol).  When
 * enabled, each input pixel P is replaced by a weighted sum of itself and its
 * eight neighbors.  P's weight is 1-8*SF and each neighbor's weight is SF,
 * where SF = (smoothing_factor / 1024).
 * Currently, smoothing is only supported for 2h2v sampling factors.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "LibJpeg.h"


/* Pointer to routine to downsample a single component */
typedef void ( * downsample1_ptr)( sJpeg_compress_struct *cinfo, sJpeg_component_info * compptr, JSAMPARRAY input_data, JSAMPARRAY output_data );

/* Private subobject */

struct my_downsampler
{
  sJpeg_downsampler pub;	/* public fields */

  /* Downsampling method pointers, one per component */
  downsample1_ptr methods[MAX_COMPONENTS];

  /* Height of an output row group for each component. */
  int rowgroup_height[MAX_COMPONENTS];

  /* These arrays save pixel expansion factors so that int_downsample need not
   * recompute them each time.  They are unused for other downsampling methods.
   */
  UINT8 h_expand[MAX_COMPONENTS];
  UINT8 v_expand[MAX_COMPONENTS];
};

/*
 * Initialize for a downsampling pass.
 */
static void start_pass_downsample ( sJpeg_compress_struct *cinfo );

/*
 * Expand a component horizontally from width input_cols to width output_cols,
 * by duplicating the rightmost samples.
 */
void expand_right_edge (JSAMPARRAY image_data, int num_rows, JDIMENSION input_cols, JDIMENSION output_cols );

/*
 * Do downsampling for a whole row group (all components).
 *
 * In this version we simply downsample each component independently.
 */
static void sep_downsample ( sJpeg_compress_struct *cinfo, JSAMPIMAGE input_buf, JDIMENSION in_row_index,JSAMPIMAGE output_buf, JDIMENSION out_row_group_index );

/*
 * Downsample pixel values of a single component.
 * One row group is processed per call.
 * This version handles arbitrary integral sampling ratios, without smoothing.
 * Note that this version is not actually used for customary sampling ratios.
 */
static void int_downsample ( sJpeg_compress_struct * cinfo, sJpeg_component_info * compptr,	JSAMPARRAY input_data, JSAMPARRAY output_data );


/*
 * Downsample pixel values of a single component.
 * This version handles the special case of a full-size component,
 * without smoothing.
 */
static void fullsize_downsample ( sJpeg_compress_struct *cinfo, sJpeg_component_info * compptr, JSAMPARRAY input_data, JSAMPARRAY output_data );


/*
 * Downsample pixel values of a single component.
 * This version handles the common case of 2:1 horizontal and 1:1 vertical,
 * without smoothing.
 *
 * A note about the "bias" calculations: when rounding fractional values to
 * integer, we do not want to always round 0.5 up to the next integer.
 * If we did that, we'd introduce a noticeable bias towards larger values.
 * Instead, this code is arranged so that 0.5 will be rounded up or down at
 * alternate pixel locations (a simple ordered dither pattern).
 */
static void h2v1_downsample ( sJpeg_compress_struct *cinfo, sJpeg_component_info *compptr, JSAMPARRAY input_data, JSAMPARRAY output_data );


/*
 * Downsample pixel values of a single component.
 * This version handles the standard case of 2:1 horizontal and 2:1 vertical,
 * without smoothing.
 */
static void h2v2_downsample ( sJpeg_compress_struct *cinfo, sJpeg_component_info *compptr, JSAMPARRAY input_data, JSAMPARRAY output_data );

#ifdef INPUT_SMOOTHING_SUPPORTED

/*
 * Downsample pixel values of a single component.
 * This version handles the standard case of 2:1 horizontal and 2:1 vertical,
 * with smoothing.  One row of context is required.
 */
static void h2v2_smooth_downsample ( sJpeg_compress_struct *cinfo, sJpeg_component_info *compptr, JSAMPARRAY input_data, JSAMPARRAY output_data );

/*
 * Downsample pixel values of a single component.
 * This version handles the special case of a full-size component,
 * with smoothing.  One row of context is required.
 */
static void fullsize_smooth_downsample ( sJpeg_compress_struct *cinfo, sJpeg_component_info *compptr, JSAMPARRAY input_data, JSAMPARRAY output_data );

#endif /* INPUT_SMOOTHING_SUPPORTED */


/*
 * Module initialization routine for downsampling.
 * Note that we must select a routine for each component.
 */
void jinit_downsampler ( sJpeg_compress_struct *cinfo );


#endif