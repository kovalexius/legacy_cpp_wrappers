#ifndef JCMASTER_H
#define JCMASTER_H

/*
 * jcmaster.c
 *
 * Copyright (C) 1991-1997, Thomas G. Lane.
 * Modified 2003-2011 by Guido Vollbeding.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains master control logic for the JPEG compressor.
 * These routines are concerned with parameter validation, initial setup,
 * and inter-pass control (determining the number of passes and the work 
 * to be done in each pass).
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpegint.h"


struct sJpeg_compress_struct;
/* Private state */

enum c_pass_type
{
	main_pass,		/* input data, also do first output step */
	huff_opt_pass,		/* Huffman code optimization pass */
	output_pass		/* data output pass */
};

struct  my_comp_master
{
  sJpeg_comp_master pub;	/* public fields */

  c_pass_type pass_type;	/* the type of the current pass */

  int pass_number;		/* # of passes completed */
  int total_passes;		/* total # of passes needed */

  int scan_number;		/* current index in scan_info[] */
};


/*
 * Support routines that do various essential calculations.
 */

/*
 * Compute JPEG image dimensions and related values.
 * NOTE: this is exported for possible use by application.
 * Hence it mustn't do anything that can't be done twice.
 */
void jpeg_calc_jpeg_dimensions ( sJpeg_compress_struct *cinfo );

static void jpeg_calc_trans_dimensions ( sJpeg_compress_struct *cinfo);

static void initial_setup ( sJpeg_compress_struct *cinfo, boolean transcode_only );
/* Do computations that are needed before master selection phase */

#ifdef C_MULTISCAN_FILES_SUPPORTED
static void validate_script ( sJpeg_compress_struct *cinfo );
/* Verify that the scan script in cinfo->scan_info[] is valid; also
 * determine whether it uses progressive JPEG, and set cinfo->progressive_mode.
 */

static void reduce_script ( sJpeg_compress_struct *cinfo );
/* Adapt scan script for use with reduced block size;
 * assume that script has been validated before.
 */
#endif /* C_MULTISCAN_FILES_SUPPORTED */

static void select_scan_parameters ( sJpeg_compress_struct * cinfo );
/* Set up the scan parameters for the current scan */

static void per_scan_setup ( sJpeg_compress_struct *cinfo );
/* Do computations that are needed before processing a JPEG scan */
/* cinfo->comps_in_scan and cinfo->cur_comp_info[] are already set */

/*
 * Per-pass setup.
 * This is called at the beginning of each pass.  We determine which modules
 * will be active during this pass and give them appropriate start_pass calls.
 * We also set is_last_pass to indicate whether any more passes will be
 * required.
 */
static void prepare_for_pass ( sJpeg_compress_struct *cinfo);

/*
 * Special start-of-pass hook.
 * This is called by jpeg_write_scanlines if call_pass_startup is TRUE.
 * In single-pass processing, we need this hook because we don't want to
 * write frame/scan headers during jpeg_start_compress; we want to let the
 * application write COM markers etc. between jpeg_start_compress and the
 * jpeg_write_scanlines loop.
 * In multi-pass processing, this routine is not used.
 */
static void pass_startup ( sJpeg_compress_struct *cinfo);

/*
 * Finish up at end of pass.
 */
static void finish_pass_master ( sJpeg_compress_struct *cinfo );

/*
 * Initialize master compression control.
 */
void jinit_c_master_control ( sJpeg_compress_struct *cinfo, boolean transcode_only );






#endif