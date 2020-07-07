#ifndef JCMARKER_H
#define JCMARKER_H

#define JPEG_INTERNALS
#include "jinclude.h"
#include "LibJpeg.h"


enum JPEG_MARKER
{			/* JPEG marker codes */
  M_SOF0  = 0xc0,
  M_SOF1  = 0xc1,
  M_SOF2  = 0xc2,
  M_SOF3  = 0xc3,

  M_SOF5  = 0xc5,
  M_SOF6  = 0xc6,
  M_SOF7  = 0xc7,

  M_JPG   = 0xc8,
  M_SOF9  = 0xc9,
  M_SOF10 = 0xca,
  M_SOF11 = 0xcb,

  M_SOF13 = 0xcd,
  M_SOF14 = 0xce,
  M_SOF15 = 0xcf,

  M_DHT   = 0xc4,

  M_DAC   = 0xcc,

  M_RST0  = 0xd0,
  M_RST1  = 0xd1,
  M_RST2  = 0xd2,
  M_RST3  = 0xd3,
  M_RST4  = 0xd4,
  M_RST5  = 0xd5,
  M_RST6  = 0xd6,
  M_RST7  = 0xd7,

  M_SOI   = 0xd8,
  M_EOI   = 0xd9,
  M_SOS   = 0xda,
  M_DQT   = 0xdb,
  M_DNL   = 0xdc,
  M_DRI   = 0xdd,
  M_DHP   = 0xde,
  M_EXP   = 0xdf,

  M_APP0  = 0xe0,
  M_APP1  = 0xe1,
  M_APP2  = 0xe2,
  M_APP3  = 0xe3,
  M_APP4  = 0xe4,
  M_APP5  = 0xe5,
  M_APP6  = 0xe6,
  M_APP7  = 0xe7,
  M_APP8  = 0xe8,
  M_APP9  = 0xe9,
  M_APP10 = 0xea,
  M_APP11 = 0xeb,
  M_APP12 = 0xec,
  M_APP13 = 0xed,
  M_APP14 = 0xee,
  M_APP15 = 0xef,

  M_JPG0  = 0xf0,
  M_JPG8  = 0xf8,
  M_JPG13 = 0xfd,
  M_COM   = 0xfe,

  M_TEM   = 0x01,

  M_ERROR = 0x100
};


/* Private state */
struct my_marker_writer
{
  struct sJpeg_marker_writer pub; /* public fields */

  unsigned int last_restart_interval; /* last DRI value emitted; 0 after SOI */
};



/*
 * Initialize the marker writer module.
 */
void jinit_marker_writer ( sJpeg_compress_struct *cinfo );


/*
 * Write scan header.
 * This consists of DHT or DAC markers, optional DRI, and SOS.
 * Compressed data will be written following the SOS.
 */
static void write_scan_header ( sJpeg_compress_struct *cinfo );

/*
 * These routines allow writing an arbitrary marker with parameters.
 * The only intended use is to emit COM or APPn markers after calling
 * write_file_header and before calling write_frame_header.
 * Other uses are not guaranteed to produce desirable results.
 * Counting the parameter bytes properly is the caller's responsibility.
 */
static void write_marker_header ( sJpeg_compress_struct *cinfo, int marker, unsigned int datalen );
/* Emit an arbitrary marker header */

static void write_marker_byte ( sJpeg_compress_struct *cinfo, int val );
/* Emit one byte of marker parameters following write_marker_header */


/*
 * Write datastream header.
 * This consists of an SOI and optional APPn markers.
 * We recommend use of the JFIF marker, but not the Adobe marker,
 * when using YCbCr or grayscale data.  The JFIF marker should NOT
 * be used for any other JPEG colorspace.  The Adobe marker is helpful
 * to distinguish RGB, CMYK, and YCCK colorspaces.
 * Note that an application can write additional header markers after
 * jpeg_start_compress returns.
 */
static void write_file_header ( sJpeg_compress_struct *cinfo );


/*
 * Write datastream trailer.
 */
static void write_file_trailer ( sJpeg_compress_struct *cinfo );

/*
 * Write an abbreviated table-specification datastream.
 * This consists of SOI, DQT and DHT tables, and EOI.
 * Any table that is defined and not marked sent_table = TRUE will be
 * emitted.  Note that all tables will be marked sent_table = TRUE at exit.
 */
static void write_tables_only ( sJpeg_compress_struct *cinfo );

/*
 * Write frame header.
 * This consists of DQT and SOFn markers,
 * a conditional LSE marker and a conditional pseudo SOS marker.
 * Note that we do not emit the SOF until we have emitted the DQT(s).
 * This avoids compatibility problems with incorrect implementations that
 * try to error-check the quant table numbers as soon as they see the SOF.
 */
static void write_frame_header ( sJpeg_compress_struct *cinfo);

/*
 * Basic output routines.
 *
 * Note that we do not support suspension while writing a marker.
 * Therefore, an application using suspension must ensure that there is
 * enough buffer space for the initial markers (typ. 600-700 bytes) before
 * calling jpeg_start_compress, and enough space to write the trailing EOI
 * (a few bytes) before calling jpeg_finish_compress.  Multipass compression
 * modes are not supported at all with suspension, so those two are the only
 * points where markers will be written.
 */
static void emit_byte ( sJpeg_compress_struct *cinfo, int val );
/* Emit a byte */

static void emit_marker ( sJpeg_compress_struct *cinfo, JPEG_MARKER mark );
/* Emit a marker code */

static void emit_2bytes ( sJpeg_compress_struct *cinfo, int value );
/* Emit a 2-byte integer; these are always MSB first in JPEG files */

/*
 * Routines to write specific marker types.
 */
static int emit_dqt ( sJpeg_compress_struct *cinfo, int index );
/* Emit a DQT marker */
/* Returns the precision used (0 = 8bits, 1 = 16bits) for baseline checking */

static void emit_dht ( sJpeg_compress_struct *cinfo, int index, boolean is_ac );
/* Emit a DHT marker */

static void emit_adobe_app14 ( sJpeg_compress_struct *cinfo );
/* Emit an Adobe APP14 marker */

static void emit_dac ( sJpeg_compress_struct *cinfo );
/* Emit a DAC marker */
/* Since the useful info is so small, we want to emit all the tables in */
/* one DAC marker.  Therefore this routine does its own scan of the table. */

static void emit_dri ( sJpeg_compress_struct *cinfo );
/* Emit a DRI marker */

static void emit_lse_ict ( sJpeg_compress_struct *cinfo );
/* Emit an LSE inverse color transform specification marker */

static void emit_jfif_app0 ( sJpeg_compress_struct *cinfo );
/* Emit a JFIF-compliant APP0 marker */

static void emit_pseudo_sos ( sJpeg_compress_struct *cinfo );
/* Emit a pseudo SOS marker */

static void emit_sof ( sJpeg_compress_struct *cinfo, JPEG_MARKER code );
/* Emit a SOF marker */

static void emit_sos ( sJpeg_compress_struct *cinfo );
/* Emit a SOS marker */

#endif