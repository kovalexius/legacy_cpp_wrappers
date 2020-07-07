#ifndef JCARITH_H
#define JCARITH_H

#define JPEG_INTERNALS
#include "jinclude.h"
#include "LibJpeg.h"


/* Expanded entropy encoder object for arithmetic encoding. */
struct arith_entropy_encoder
{
  struct sJpeg_entropy_encoder pub; /* public fields */

  INT32 c; /* C register, base of coding interval, layout as in sec. D.1.3 */
  INT32 a;               /* A register, normalized size of coding interval */
  INT32 sc;        /* counter for stacked 0xFF values which might overflow */
  INT32 zc;          /* counter for pending 0x00 output values which might *
                          * be discarded at the end ("Pacman" termination) */
  int ct;  /* bit shift counter, determines when next byte will be written */
  int buffer;                /* buffer for most recent output byte != 0xFF */

  int last_dc_val[MAX_COMPS_IN_SCAN]; /* last DC coef for each component */
  int dc_context[MAX_COMPS_IN_SCAN]; /* context index for DC conditioning */

  unsigned int restarts_to_go;	/* MCUs left in this restart interval */
  int next_restart_num;		/* next restart number to write (0-7) */

  /* Pointers to statistics areas (these workspaces have image lifespan) */
  unsigned char * dc_stats[NUM_ARITH_TBLS];
  unsigned char * ac_stats[NUM_ARITH_TBLS];

  /* Statistics bin for coding with fixed probability 0.5 */
  unsigned char fixed_bin[4];
};

/* The following two definitions specify the allocation chunk size
 * for the statistics area.
 * According to sections F.1.4.4.1.3 and F.1.4.4.2, we need at least
 * 49 statistics bins for DC, and 245 statistics bins for AC coding.
 *
 * We use a compact representation with 1 byte per statistics bin,
 * thus the numbers directly represent byte sizes.
 * This 1 byte per statistics bin contains the meaning of the MPS
 * (more probable symbol) in the highest bit (mask 0x80), and the
 * index into the probability estimation state machine table
 * in the lower bits (mask 0x7F).
 */

#define DC_STAT_BINS 64
#define AC_STAT_BINS 256

/* NOTE: Uncomment the following #define if you want to use the
 * given formula for calculating the AC conditioning parameter Kx
 * for spectral selection progressive coding in section G.1.3.2
 * of the spec (Kx = Kmin + SRL (8 + Se - Kmin) 4).
 * Although the spec and P&M authors claim that this "has proven
 * to give good results for 8 bit precision samples", I'm not
 * convinced yet that this is really beneficial.
 * Early tests gave only very marginal compression enhancements
 * (a few - around 5 or so - bytes even for very large files),
 * which would turn out rather negative if we'd suppress the
 * DAC (Define Arithmetic Conditioning) marker segments for
 * the default parameters in the future.
 * Note that currently the marker writing module emits 12-byte
 * DAC segments for a full-component scan in a color image.
 * This is not worth worrying about IMHO. However, since the
 * spec defines the default values to be used if the tables
 * are omitted (unlike Huffman tables, which are required
 * anyway), one might optimize this behaviour in the future,
 * and then it would be disadvantageous to use custom tables if
 * they don't provide sufficient gain to exceed the DAC size.
 *
 * On the other hand, I'd consider it as a reasonable result
 * that the conditioning has no significant influence on the
 * compression performance. This means that the basic
 * statistical model is already rather stable.
 *
 * Thus, at the moment, we use the default conditioning values
 * anyway, and do not use the custom formula.
 *
#define CALCULATE_SPECTRAL_CONDITIONING
 */

/* IRIGHT_SHIFT is like RIGHT_SHIFT, but works on int rather than INT32.
 * We assume that int right shift is unsigned if INT32 right shift is,
 * which should be safe.
 */

#ifdef RIGHT_SHIFT_IS_UNSIGNED
#define ISHIFT_TEMPS	int ishift_temp;
#define IRIGHT_SHIFT(x,shft)  \
	((ishift_temp = (x)) < 0 ? \
	 (ishift_temp >> (shft)) | ((~0) << (16-(shft))) : \
	 (ishift_temp >> (shft)))
#else
#define ISHIFT_TEMPS
#define IRIGHT_SHIFT(x,shft)	((x) >> (shft))
#endif


static void emit_byte ( int val, sJpeg_compress_struct *cinfo );
/* Write next output byte; we do not support suspension in this module. */


/*
 * Module initialization routine for arithmetic entropy encoding.
 */
void jinit_arith_encoder ( sJpeg_compress_struct *cinfo);

/*
 * Initialize for an arithmetic-compressed scan.
 */
static void start_pass ( sJpeg_compress_struct *cinfo, boolean gather_statistics );

/*
 * MCU encoding for DC initial scan (either spectral selection,
 * or first pass of successive approximation).
 */
static boolean encode_mcu_DC_first ( sJpeg_compress_struct *cinfo, JBLOCKROW *MCU_data);

/*
 * MCU encoding for AC initial scan (either spectral selection,
 * or first pass of successive approximation).
 */
static boolean encode_mcu_AC_first ( sJpeg_compress_struct *cinfo, JBLOCKROW *MCU_data);

/*
 * MCU encoding for AC successive approximation refinement scan.
 */
static boolean encode_mcu_AC_refine ( sJpeg_compress_struct *cinfo, JBLOCKROW *MCU_data );

/*
 * MCU encoding for DC successive approximation refinement scan.
 */
static boolean encode_mcu_DC_refine ( sJpeg_compress_struct *cinfo, JBLOCKROW *MCU_data );

/*
 * Encode and output one MCU's worth of arithmetic-compressed coefficients.
 */
static boolean encode_mcu ( sJpeg_compress_struct *cinfo, JBLOCKROW *MCU_data );


/*
 * Emit a restart marker & resynchronize predictions.
 */
static void emit_restart ( sJpeg_compress_struct *cinfo, int restart_num );

/*
 * Finish up at the end of an arithmetic-compressed scan.
 */
static void finish_pass ( sJpeg_compress_struct *cinfo );

/*
 * The core arithmetic encoding routine (common in JPEG and JBIG).
 * This needs to go as fast as possible.
 * Machine-dependent optimization facilities
 * are not utilized in this portable implementation.
 * However, this code should be fairly efficient and
 * may be a good base for further optimizations anyway.
 *
 * Parameter 'val' to be encoded may be 0 or 1 (binary decision).
 *
 * Note: I've added full "Pacman" termination support to the
 * byte output routines, which is equivalent to the optional
 * Discard_final_zeros procedure (Figure D.15) in the spec.
 * Thus, we always produce the shortest possible output
 * stream compliant to the spec (no trailing zero bytes,
 * except for FF stuffing).
 *
 * I've also introduced a new scheme for accessing
 * the probability estimation state machine table,
 * derived from Markus Kuhn's JBIG implementation.
 */
static void arith_encode ( sJpeg_compress_struct *cinfo, unsigned char *st, int val );


#endif