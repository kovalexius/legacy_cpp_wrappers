#ifndef JCPARAM_H
#define JCPARAM_H

#define JPEG_INTERNALS
#include "jpegint.h"
#include "LibJpeg.h"


/* These are the sample quantization tables given in JPEG spec section K.1.
 * The spec says that the values given produce "good" quality, and
 * when divided by 2, "very good" quality.
 */
static const unsigned int std_luminance_quant_tbl[DCTSIZE2] = {
  16,  11,  10,  16,  24,  40,  51,  61,
  12,  12,  14,  19,  26,  58,  60,  55,
  14,  13,  16,  24,  40,  57,  69,  56,
  14,  17,  22,  29,  51,  87,  80,  62,
  18,  22,  37,  56,  68, 109, 103,  77,
  24,  35,  55,  64,  81, 104, 113,  92,
  49,  64,  78,  87, 103, 121, 120, 101,
  72,  92,  95,  98, 112, 100, 103,  99
};
static const unsigned int std_chrominance_quant_tbl[DCTSIZE2] = {
  17,  18,  24,  47,  99,  99,  99,  99,
  18,  21,  26,  66,  99,  99,  99,  99,
  24,  26,  56,  99,  99,  99,  99,  99,
  47,  66,  99,  99,  99,  99,  99,  99,
  99,  99,  99,  99,  99,  99,  99,  99,
  99,  99,  99,  99,  99,  99,  99,  99,
  99,  99,  99,  99,  99,  99,  99,  99,
  99,  99,  99,  99,  99,  99,  99,  99
};


int jpeg_quality_scaling ( int quality );
/* Convert a user-specified quality rating to a percentage scaling factor
 * for an underlying quantization table, using our recommended scaling curve.
 * The input 'quality' factor should be 0 (terrible) to 100 (very good).
 */

void jpeg_add_quant_table ( sJpeg_compress_struct *cinfo, int which_tbl, const unsigned int *basic_table, int scale_factor, boolean force_baseline );
/* Define a quantization table equal to the basic_table times
 * a scale factor (given as a percentage).
 * If force_baseline is TRUE, the computed quantization table entries
 * are limited to 1..255 for JPEG baseline compatibility.
 */

/*
 * Quantization table setup routines
 */
void jpeg_set_linear_quality ( sJpeg_compress_struct *cinfo, int scale_factor, boolean force_baseline );
/* Set or change the 'quality' (quantization) setting, using default tables
 * and a straight percentage-scaling quality scale.  In most cases it's better
 * to use jpeg_set_quality (below); this entry point is provided for
 * applications that insist on a linear percentage scaling.
 */

/*
 * Select an appropriate JPEG colorspace for in_color_space.
 */
void jpeg_default_colorspace ( sJpeg_compress_struct *cinfo);

/*
 * Set the JPEG colorspace, and choose colorspace-dependent default values.
 */
void jpeg_set_colorspace ( sJpeg_compress_struct *cinfo, J_COLOR_SPACE colorspace );

void jpeg_set_quality ( sJpeg_compress_struct *cinfo, int quality, boolean force_baseline );
/* Set or change the 'quality' (quantization) setting, using default tables.
 * This is the standard quality-adjusting entry point for typical user
 * interfaces; only those who want detailed control over quantization tables
 * would use the preceding routines directly.
 */

/*
 * Huffman table setup routines
 */
static void add_huff_table ( sJpeg_compress_struct * cinfo,	JHUFF_TBL **htblptr, const UINT8 *bits, const UINT8 *val);
/* Define a Huffman table */

void std_huff_tables ( sJpeg_compress_struct * cinfo);
/* Set up the standard Huffman tables (cf. JPEG standard section K.3) */
/* IMPORTANT: these are only valid for 8-bit data precision! */

#endif