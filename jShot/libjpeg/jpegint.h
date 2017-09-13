#ifndef JPEGINT_H
#define JPEGINT_H

#include "LibJpeg.h"
#include "jinclude.h"

// This file provides common declarations for the various JPEG modules.
// These declarations are considered internal to the JPEG library; most
// applications using the library shouldn't need to include this file.


/* Declarations for both compression & decompression */

enum J_BUF_MODE
{			/* Operating modes for buffer controllers */
	JBUF_PASS_THRU,		/* Plain stripwise operation */
	/* Remaining modes require a full-image buffer to have been created */
	JBUF_SAVE_SOURCE,	/* Run source subobject only, save output */
	JBUF_CRANK_DEST,	/* Run dest subobject only, using saved data */
	JBUF_SAVE_AND_PASS	/* Run both subobjects, save output */
};

/* Values of global_state field (jdapi.c has some dependencies on ordering!) */
#define CSTATE_START	100	/* after create_compress */
#define CSTATE_SCANNING	101	/* start_compress done, write_scanlines OK */
#define CSTATE_RAW_OK	102	/* start_compress done, write_raw_data OK */
#define CSTATE_WRCOEFS	103	/* jpeg_write_coefficients done */
#define DSTATE_START	200	/* after create_decompress */
#define DSTATE_INHEADER	201	/* reading header markers, no SOS yet */
#define DSTATE_READY	202	/* found SOS, ready for start_decompress */
#define DSTATE_PRELOAD	203	/* reading multiscan file in start_decompress*/
#define DSTATE_PRESCAN	204	/* performing dummy pass for 2-pass quant */
#define DSTATE_SCANNING	205	/* start_decompress done, read_scanlines OK */
#define DSTATE_RAW_OK	206	/* start_decompress done, read_raw_data OK */
#define DSTATE_BUFIMAGE	207	/* expecting jpeg_start_output */
#define DSTATE_BUFPOST	208	/* looking for SOS/EOI in jpeg_finish_output */
#define DSTATE_RDCOEFS	209	/* reading file in jpeg_read_coefficients */
#define DSTATE_STOPPING	210	/* looking for EOI in jpeg_finish_decompress */


struct sJpeg_compress_struct;
struct sJpeg_component_info;
/* Declarations for compression modules */

/* Master control module */
struct sJpeg_comp_master 
{
  void (*prepare_for_pass) ( sJpeg_compress_struct *cinfo );
  void (*pass_startup) ( sJpeg_compress_struct *cinfo );
  void (*finish_pass) ( sJpeg_compress_struct *cinfo);

  /* State variables made visible to other modules */
  boolean call_pass_startup;	/* True if pass_startup must be called */
  boolean is_last_pass;		/* True during last pass */
};

/* Main buffer control (downsampled-data buffer) */
struct sJpeg_c_main_controller 
{
  void ( *start_pass ) ( sJpeg_compress_struct *cinfo, J_BUF_MODE pass_mode );
  void ( *process_data ) ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, unsigned int *in_row_ctr, unsigned int in_rows_avail );
};

/* Compression preprocessing (downsampling input buffer control) */
struct sJpeg_c_prep_controller 
{
  void ( *start_pass ) ( sJpeg_compress_struct *cinfo, J_BUF_MODE pass_mode );
  void ( *pre_process_data) ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, unsigned int *in_row_ctr, unsigned int in_rows_avail, JSAMPIMAGE output_buf, unsigned int *out_row_group_ctr, unsigned int out_row_groups_avail );
};

/* Coefficient buffer control */
struct sJpeg_c_coef_controller 
{
  void ( *start_pass ) ( sJpeg_compress_struct *cinfo, J_BUF_MODE pass_mode );
  boolean ( *compress_data ) ( sJpeg_compress_struct *cinfo, JSAMPIMAGE input_buf );
};

/* Colorspace conversion */
struct sJpeg_color_converter 
{
  void ( *start_pass ) ( sJpeg_compress_struct *cinfo );
  void ( *color_convert ) ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf,  JSAMPIMAGE output_buf, unsigned int output_row, int num_rows );
};

/* Downsampling */
struct sJpeg_downsampler 
{
  void ( *start_pass ) ( sJpeg_compress_struct *cinfo );
  void ( *downsample ) ( sJpeg_compress_struct *cinfo, JSAMPIMAGE input_buf, unsigned int in_row_index, JSAMPIMAGE output_buf,  unsigned int out_row_group_index );

  boolean need_context_rows;	/* TRUE if need rows above & below */
};

/* Forward DCT (also controls coefficient quantization) */
typedef void ( *forward_DCT_ptr )( sJpeg_compress_struct *cinfo, sJpeg_component_info * compptr, JSAMPARRAY sample_data, JBLOCKROW coef_blocks, unsigned int start_row, unsigned int start_col, unsigned int num_blocks );

struct sJpeg_forward_dct 
{
  void ( *start_pass ) ( sJpeg_compress_struct *cinfo );
  /* It is useful to allow each component to have a separate FDCT method. */
  forward_DCT_ptr forward_DCT[MAX_COMPONENTS];
};

/* Entropy encoding */
struct sJpeg_compress_struct;
struct sJpeg_entropy_encoder 
{
  void ( *start_pass ) ( sJpeg_compress_struct *cinfo, boolean gather_statistics );
  boolean ( *encode_mcu ) ( sJpeg_compress_struct *cinfo, JBLOCKROW *MCU_data );
  void ( *finish_pass ) ( sJpeg_compress_struct *cinfo );
};

/* Marker writing */
struct sJpeg_marker_writer 
{
  void ( *write_file_header ) ( sJpeg_compress_struct *cinfo );
  void ( *write_frame_header ) ( sJpeg_compress_struct * cinfo );
  void ( *write_scan_header ) ( sJpeg_compress_struct * cinfo );
  void ( *write_file_trailer ) ( sJpeg_compress_struct * cinfo );
  void ( *write_tables_only ) ( sJpeg_compress_struct * cinfo );
  /* These routines are exported to allow insertion of extra markers */
  /* Probably only COM and APPn markers should be written this way */
  void ( *write_marker_header ) ( sJpeg_compress_struct * cinfo, int marker, unsigned int datalen );
  void ( *write_marker_byte ) ( sJpeg_compress_struct * cinfo, int val );
};

/* Miscellaneous useful macros */

#undef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#undef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))


/* We assume that right shift corresponds to signed division by 2 with
 * rounding towards minus infinity.  This is correct for typical "arithmetic
 * shift" instructions that shift in copies of the sign bit.  But some
 * C compilers implement >> with an unsigned shift.  For these machines you
 * must define RIGHT_SHIFT_IS_UNSIGNED.
 * RIGHT_SHIFT provides a proper signed right shift of an INT32 quantity.
 * It is only applied with constant shift counts.  SHIFT_TEMPS must be
 * included in the variables of any routine using RIGHT_SHIFT.
 */
#ifdef RIGHT_SHIFT_IS_UNSIGNED
#define SHIFT_TEMPS	INT32 shift_temp;
#define RIGHT_SHIFT(x,shft)  \
	((shift_temp = (x)) < 0 ? \
	 (shift_temp >> (shft)) | ((~((INT32) 0)) << (32-(shft))) : \
	 (shift_temp >> (shft)))
#else
#define SHIFT_TEMPS
#define RIGHT_SHIFT(x,shft)	((x) >> (shft))
#endif


/* On normal machines we can apply MEMCOPY() and MEMZERO() to sample arrays
 * and coefficient-block arrays.  This won't work on 80x86 because the arrays
 * are FAR and we're assuming a small-pointer memory model.  However, some
 * DOS compilers provide far-pointer versions of memcpy() and memset() even
 * in the small-model libraries.  These will be used if USE_FMEM is defined.
 * Otherwise, the routines in jutils.c do it the hard way.
 */
#ifndef NEED_FAR_POINTERS	/* normal case, same as regular macro */
#define FMEMZERO(target,size)	MEMZERO(target,size)
#else				/* 80x86 case */
#ifdef USE_FMEM
#define FMEMZERO(target,size)	_fmemset((void FAR *)(target), 0, (size_t)(size))
#else
EXTERN(void) jzero_far JPP((void FAR * target, size_t bytestozero));
#define FMEMZERO(target,size)	jzero_far(target, size)
#endif
#endif


#endif