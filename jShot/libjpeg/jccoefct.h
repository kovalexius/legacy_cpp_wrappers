#ifndef JCCOEFCT_H
#define JCCOEFCT_H

/*
 * jccoefct.c
 *
 * Copyright (C) 1994-1997, Thomas G. Lane.
 * Modified 2003-2011 by Guido Vollbeding.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains the coefficient buffer controller for compression.
 * This controller is the top level of the JPEG compressor proper.
 * The coefficient buffer lies between forward-DCT and entropy encoding steps.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "LibJpeg.h"


/* We use a full-image coefficient buffer when doing Huffman optimization,
 * and also for writing multiple-scan JPEG files.  In all cases, the DCT
 * step is run during the first pass, and subsequent passes need only read
 * the buffered coefficients.
 */
#ifdef ENTROPY_OPT_SUPPORTED
#define FULL_COEF_BUFFER_SUPPORTED
#else
#ifdef C_MULTISCAN_FILES_SUPPORTED
#define FULL_COEF_BUFFER_SUPPORTED
#endif
#endif


/* Private buffer controller object */
struct sJpeg_c_coef_controller;
struct my_coef_controller
{
  sJpeg_c_coef_controller pub; /* public fields */

  JDIMENSION iMCU_row_num;	/* iMCU row # within image */
  JDIMENSION mcu_ctr;		/* counts MCUs processed in current row */
  int MCU_vert_offset;		/* counts MCU rows within iMCU row */
  int MCU_rows_per_iMCU_row;	/* number of such rows needed */

  /* For single-pass compression, it's sufficient to buffer just one MCU
   * (although this may prove a bit slow in practice).  We allocate a
   * workspace of C_MAX_BLOCKS_IN_MCU coefficient blocks, and reuse it for each
   * MCU constructed and sent.  (On 80x86, the workspace is FAR even though
   * it's not really very big; this is to keep the module interfaces unchanged
   * when a large coefficient buffer is necessary.)
   * In multi-pass modes, this array points to the current MCU's blocks
   * within the virtual arrays.
   */
  JBLOCKROW MCU_buffer[C_MAX_BLOCKS_IN_MCU];

  /* In multi-pass modes, we need a virtual block array for each component. */
  sJvirt_barray_control *whole_image[MAX_COMPONENTS];
};


static void start_iMCU_row ( sJpeg_compress_struct *cinfo );
/* Reset within-iMCU-row counters for a new row */

/*
 * Initialize for a processing pass.
 */
static void start_pass_coef ( sJpeg_compress_struct *cinfo, J_BUF_MODE pass_mode );

/*
 * Process some data in the single-pass case.
 * We process the equivalent of one fully interleaved MCU row ("iMCU" row)
 * per call, ie, v_samp_factor block rows for each component in the image.
 * Returns TRUE if the iMCU row is completed, FALSE if suspended.
 *
 * NB: input_buf contains a plane for each component in image,
 * which we index according to the component's SOF position.
 */
static boolean compress_data ( sJpeg_compress_struct *cinfo, JSAMPIMAGE input_buf );

#ifdef FULL_COEF_BUFFER_SUPPORTED

/*
 * Process some data in the first pass of a multi-pass case.
 * We process the equivalent of one fully interleaved MCU row ("iMCU" row)
 * per call, ie, v_samp_factor block rows for each component in the image.
 * This amount of data is read from the source buffer, DCT'd and quantized,
 * and saved into the virtual arrays.  We also generate suitable dummy blocks
 * as needed at the right and lower edges.  (The dummy blocks are constructed
 * in the virtual arrays, which have been padded appropriately.)  This makes
 * it possible for subsequent passes not to worry about real vs. dummy blocks.
 *
 * We must also emit the data to the entropy encoder.  This is conveniently
 * done by calling compress_output() after we've loaded the current strip
 * of the virtual arrays.
 *
 * NB: input_buf contains a plane for each component in image.  All
 * components are DCT'd and loaded into the virtual arrays in this pass.
 * However, it may be that only a subset of the components are emitted to
 * the entropy encoder during this first pass; be careful about looking
 * at the scan-dependent variables (MCU dimensions, etc).
 */
static boolean compress_first_pass ( sJpeg_compress_struct *cinfo, JSAMPIMAGE input_buf );

/*
 * Process some data in subsequent passes of a multi-pass case.
 * We process the equivalent of one fully interleaved MCU row ("iMCU" row)
 * per call, ie, v_samp_factor block rows for each component in the scan.
 * The data is obtained from the virtual arrays and fed to the entropy coder.
 * Returns TRUE if the iMCU row is completed, FALSE if suspended.
 *
 * NB: input_buf is ignored; it is likely to be a NULL pointer.
 */
static boolean compress_output ( sJpeg_compress_struct *cinfo, JSAMPIMAGE input_buf );
#endif /* FULL_COEF_BUFFER_SUPPORTED */

/*
 * Initialize coefficient buffer controller.
 */
void jinit_c_coef_controller ( sJpeg_compress_struct *cinfo, boolean need_full_buffer );

#endif