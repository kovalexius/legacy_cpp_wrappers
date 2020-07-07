#ifndef JCMAINCT_H
#define JCMAINCT_H

/*
 * jcmainct.c
 *
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * Modified 2003-2012 by Guido Vollbeding.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains the main buffer controller for compression.
 * The main buffer lies between the pre-processor and the JPEG
 * compressor proper; it holds downsampled data in the JPEG colorspace.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "LibJpeg.h"


/* Note: currently, there is no operating mode in which a full-image buffer
 * is needed at this step.  If there were, that mode could not be used with
 * "raw data" input, since this module is bypassed in that case.  However,
 * we've left the code here for possible use in special applications.
 */
#undef FULL_MAIN_BUFFER_SUPPORTED


/* Private buffer controller object */

struct  my_main_controller
{
  sJpeg_c_main_controller pub; /* public fields */

  JDIMENSION cur_iMCU_row;	/* number of current iMCU row */
  JDIMENSION rowgroup_ctr;	/* counts row groups received in iMCU row */
  boolean suspended;		/* remember if we suspended output */
  J_BUF_MODE pass_mode;		/* current operating mode */

  /* If using just a strip buffer, this points to the entire set of buffers
   * (we allocate one for each component).  In the full-image case, this
   * points to the currently accessible strips of the virtual arrays.
   */
  JSAMPARRAY buffer[MAX_COMPONENTS];

#ifdef FULL_MAIN_BUFFER_SUPPORTED
  /* If using full-image storage, this array holds pointers to virtual-array
   * control blocks for each component.  Unused if not full-image storage.
   */
  jvirt_sarray_ptr whole_image[MAX_COMPONENTS];
#endif
};


/*
 * Initialize for a processing pass.
 */
static void start_pass_main ( sJpeg_compress_struct *cinfo, J_BUF_MODE pass_mode );


/*
 * Process some data.
 * This routine handles the simple pass-through mode,
 * where we have only a strip buffer.
 */
static void process_data_simple_main ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JDIMENSION *in_row_ctr, JDIMENSION in_rows_avail );

#ifdef FULL_MAIN_BUFFER_SUPPORTED
/*
 * Process some data.
 * This routine handles all of the modes that use a full-size buffer.
 */
static void process_data_buffer_main ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JDIMENSION *in_row_ctr, JDIMENSION in_rows_avail );
#endif /* FULL_MAIN_BUFFER_SUPPORTED */

void jinit_c_main_controller ( sJpeg_compress_struct *cinfo, boolean need_full_buffer );

#endif