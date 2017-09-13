#ifndef JCCOLOR_H
#define JCCOLOR_H

#define JPEG_INTERNALS
#include "jinclude.h"
#include "LibJpeg.h"

/* Private subobject */

struct  my_color_converter
{
  struct sJpeg_color_converter pub; /* public fields */

  /* Private state for RGB->YCC conversion */
  INT32 * rgb_ycc_tab;		/* => table for RGB to YCbCr conversion */
};

/**************** RGB -> YCbCr conversion: most common case **************/

/*
 * YCbCr is defined per CCIR 601-1, except that Cb and Cr are
 * normalized to the range 0..MAXJSAMPLE rather than -0.5 .. 0.5.
 * The conversion equations to be implemented are therefore
 *	Y  =  0.29900 * R + 0.58700 * G + 0.11400 * B
 *	Cb = -0.16874 * R - 0.33126 * G + 0.50000 * B  + CENTERJSAMPLE
 *	Cr =  0.50000 * R - 0.41869 * G - 0.08131 * B  + CENTERJSAMPLE
 * (These numbers are derived from TIFF 6.0 section 21, dated 3-June-92.)
 * Note: older versions of the IJG code used a zero offset of MAXJSAMPLE/2,
 * rather than CENTERJSAMPLE, for Cb and Cr.  This gave equal positive and
 * negative swings for Cb/Cr, but meant that grayscale values (Cb=Cr=0)
 * were not represented exactly.  Now we sacrifice exact representation of
 * maximum red and maximum blue in order to get exact grayscales.
 *
 * To avoid floating-point arithmetic, we represent the fractional constants
 * as integers scaled up by 2^16 (about 4 digits precision); we have to divide
 * the products by 2^16, with appropriate rounding, to get the correct answer.
 *
 * For even more speed, we avoid doing any multiplications in the inner loop
 * by precalculating the constants times R,G,B for all possible values.
 * For 8-bit JSAMPLEs this is very reasonable (only 256 entries per table);
 * for 12-bit samples it is still acceptable.  It's not very reasonable for
 * 16-bit samples, but if you want lossless storage you shouldn't be changing
 * colorspace anyway.
 * The CENTERJSAMPLE offsets and the rounding fudge-factor of 0.5 are included
 * in the tables to save adding them separately in the inner loop.
 */
#define SCALEBITS	16	/* speediest right-shift on some machines */
#define CBCR_OFFSET	((INT32) CENTERJSAMPLE << SCALEBITS)
#define ONE_HALF	((INT32) 1 << (SCALEBITS-1))
#define FIX(x)		((INT32) ((x) * (1L<<SCALEBITS) + 0.5))


/* We allocate one big table and divide it up into eight parts, instead of
 * doing eight alloc_small requests.  This lets us use a single table base
 * address, which can be held in a register in the inner loops on many
 * machines (more than can hold all eight addresses, anyway).
 */
#define R_Y_OFF		0			/* offset to R => Y section */
#define G_Y_OFF		(1*(MAXJSAMPLE+1))	/* offset to G => Y section */
#define B_Y_OFF		(2*(MAXJSAMPLE+1))	/* etc. */
#define R_CB_OFF	(3*(MAXJSAMPLE+1))
#define G_CB_OFF	(4*(MAXJSAMPLE+1))
#define B_CB_OFF	(5*(MAXJSAMPLE+1))
#define R_CR_OFF	B_CB_OFF		/* B=>Cb, R=>Cr are the same */
#define G_CR_OFF	(6*(MAXJSAMPLE+1))
#define B_CR_OFF	(7*(MAXJSAMPLE+1))
#define TABLE_SIZE	(8*(MAXJSAMPLE+1))


/*
 * Initialize for RGB->YCC colorspace conversion.
 */
static void rgb_ycc_start ( sJpeg_compress_struct *cinfo );

/*
 * Convert some rows of samples to the JPEG colorspace.
 *
 * Note that we change from the application's interleaved-pixel format
 * to our internal noninterleaved, one-plane-per-component format.
 * The input buffer is therefore three times as wide as the output buffer.
 *
 * A starting row offset is provided only for the output buffer.  The caller
 * can easily adjust the passed input_buf value to accommodate any row
 * offset required on that side.
 */
static void rgb_ycc_convert ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JSAMPIMAGE output_buf, JDIMENSION output_row, int num_rows );


/**************** Cases other than RGB -> YCbCr **************/

/*
 * Convert some rows of samples to the JPEG colorspace.
 * This version handles RGB->grayscale conversion, which is the same
 * as the RGB->Y portion of RGB->YCbCr.
 * We assume rgb_ycc_start has been called (we only use the Y tables).
 */
static void rgb_gray_convert ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JSAMPIMAGE output_buf, JDIMENSION output_row, int num_rows );

/*
 * Convert some rows of samples to the JPEG colorspace.
 * This version handles Adobe-style CMYK->YCCK conversion,
 * where we convert R=1-C, G=1-M, and B=1-Y to YCbCr using the same
 * conversion as above, while passing K (black) unchanged.
 * We assume rgb_ycc_start has been called.
 */
static void cmyk_ycck_convert ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JSAMPIMAGE output_buf, JDIMENSION output_row, int num_rows );

/*
 * Convert some rows of samples to the JPEG colorspace.
 * [R,G,B] to [R-G,G,B-G] conversion with modulo calculation
 * (forward reversible color transform).
 */
static void rgb_rgb1_convert ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JSAMPIMAGE output_buf, JDIMENSION output_row, int num_rows );

/*
 * Convert some rows of samples to the JPEG colorspace.
 * This version handles grayscale output with no conversion.
 * The source can be either plain grayscale or YCbCr (since Y == gray).
 */
static void grayscale_convert ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JSAMPIMAGE output_buf, JDIMENSION output_row, int num_rows );

/*
 * Convert some rows of samples to the JPEG colorspace.
 * No colorspace conversion, but change from interleaved
 * to separate-planes representation.
 */
static void rgb_convert ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JSAMPIMAGE output_buf, JDIMENSION output_row, int num_rows );

/*
 * Convert some rows of samples to the JPEG colorspace.
 * This version handles multi-component colorspaces without conversion.
 * We assume input_components == num_components.
 */
static void null_convert ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JSAMPIMAGE output_buf, JDIMENSION output_row, int num_rows );

/*
 * Empty method for start_pass.
 */
static void null_method ( sJpeg_compress_struct *cinfo);

/*
 * Module initialization routine for input colorspace conversion.
 */
void jinit_color_converter ( sJpeg_compress_struct *cinfo);

#endif