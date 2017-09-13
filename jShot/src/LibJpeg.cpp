#include <stdlib.h>
#include "LibJpeg.h"
#include "jinclude.h"
#include "jmemmgr.h"
#include "jutils.h"
#include "jpegint.h"
#include "jcparam.h"
#include "jcapimin.h"
#include "jdatadst.h"
#include "jcinit.h"
#include "jcomapi.h"
#include "jdatadst.h"

void CLibJpeg::jpeg_create_compress (  sJpeg_compress_struct * cinfo, int version, size_t structsize )
{
  int i;

  /* Guard against version mismatches between library and caller. */
  cinfo->mem = NULL;		/* so jpeg_destroy knows mem mgr not called */

  if ( version != JPEG_LIB_VERSION )
    ERREXIT2( cinfo, JERR_BAD_LIB_VERSION, JPEG_LIB_VERSION, version );
	if ( structsize != (size_t)sizeof( sJpeg_compress_struct ) )
    ERREXIT2( cinfo, JERR_BAD_STRUCT_SIZE, (int) SIZEOF( sJpeg_compress_struct ), (int) structsize );

  /* For debugging purposes, we zero the whole master structure.
   * But the application has already set the err pointer, and may have set
   * client_data, so we have to save and restore those fields.
   * Note: if application hasn't set client_data, tools like Purify may
   * complain here.
   */
  {
    sJpeg_error_mgr * err = cinfo->err;
    void * client_data = cinfo->client_data; /* ignore Purify complaint here */
    MEMZERO( cinfo, SIZEOF( sJpeg_compress_struct ) );
    cinfo->err = err;
    cinfo->client_data = client_data;
  }
  cinfo->is_decompressor = FALSE;

  /* Initialize a memory manager instance for this object */
  jinit_memory_mgr( ( sJpeg_common_struct * ) cinfo );

  /* Zero out pointers to permanent structures. */
  cinfo->progress = NULL;
  cinfo->dest = NULL;

  cinfo->comp_info = NULL;

  for (i = 0; i < NUM_QUANT_TBLS; i++) {
    cinfo->quant_tbl_ptrs[i] = NULL;
    cinfo->q_scale_factor[i] = 100;
  }

  for (i = 0; i < NUM_HUFF_TBLS; i++) {
    cinfo->dc_huff_tbl_ptrs[i] = NULL;
    cinfo->ac_huff_tbl_ptrs[i] = NULL;
  }

  /* Must do it here for emit_dqt in case jpeg_write_tables is used */
  cinfo->block_size = DCTSIZE;
  cinfo->natural_order = jpeg_natural_order;
  cinfo->lim_Se = DCTSIZE2-1;

  cinfo->script_space = NULL;

  cinfo->input_gamma = 1.0;	/* in case application forgets */

  /* OK, I'm ready */
  cinfo->global_state = CSTATE_START;
}

void CLibJpeg::jpeg_stdio_dest ( sJpeg_compress_struct *cinfo, FILE * outfile )
{
	sMy_destination_mgr * dest;

  /* The destination object is made permanent so that multiple JPEG images
   * can be written to the same file without re-executing jpeg_stdio_dest.
   * This makes it dangerous to use this manager and a different destination
   * manager serially with the same JPEG object, because their private object
   * sizes may be different.  Caveat programmer.
   */
  if (cinfo->dest == NULL) 
	{	/* first time for this JPEG object? */
    cinfo->dest = ( sJpeg_destination_mgr *) (*cinfo->mem->alloc_small) (( sJpeg_common_struct *) cinfo, JPOOL_PERMANENT,  SIZEOF( sMy_destination_mgr));
  }

  dest = (sMy_destination_mgr *) cinfo->dest;
  dest->pub.init_destination = init_destination;
  dest->pub.empty_output_buffer = empty_output_buffer;
  dest->pub.term_destination = term_destination;
  dest->outfile = outfile;
}

void CLibJpeg::jpeg_mem_dest ( sJpeg_compress_struct *cinfo, unsigned char **outbuffer, unsigned long *outsize )
{
	sMy_mem_destination_mgr *dest;

  if (outbuffer == NULL || outsize == NULL)	/* sanity check */
    ERREXIT(cinfo, JERR_BUFFER_SIZE);

  /* The destination object is made permanent so that multiple JPEG images
   * can be written to the same buffer without re-executing jpeg_mem_dest.
   */
  if (cinfo->dest == NULL) 
  {	/* first time for this JPEG object? */
    cinfo->dest = ( sJpeg_destination_mgr *)(*cinfo->mem->alloc_small) ( (sJpeg_common_struct*) cinfo, JPOOL_PERMANENT, (size_t)sizeof(sMy_mem_destination_mgr) );
  }

  dest = (sMy_mem_destination_mgr *) cinfo->dest;
  dest->pub.init_destination = init_mem_destination;
  dest->pub.empty_output_buffer = empty_mem_output_buffer;
  dest->pub.term_destination = term_mem_destination;
  dest->outbuffer = outbuffer;
  dest->outsize = outsize;
  dest->newbuffer = NULL;

  if (*outbuffer == NULL || *outsize == 0) 
  {
    /* Allocate initial buffer */
    dest->newbuffer = *outbuffer = (unsigned char *) malloc(OUTPUT_BUF_SIZE);
    if (dest->newbuffer == NULL)
      ERREXIT1(cinfo, JERR_OUT_OF_MEMORY, 10);
    *outsize = OUTPUT_BUF_SIZE;
  }

  dest->pub.next_output_byte = dest->buffer = *outbuffer;
  dest->pub.free_in_buffer = dest->bufsize = *outsize;
}


void CLibJpeg::jpeg_set_quality ( sJpeg_compress_struct *cinfo, int quality, boolean force_baseline )
{
  /* Convert user 0-100 rating to percentage scaling */
  quality = jpeg_quality_scaling(quality);

  /* Set up standard quality tables */
  jpeg_set_linear_quality(cinfo, quality, force_baseline);
}

void CLibJpeg::jpeg_set_defaults ( sJpeg_compress_struct * cinfo)
{
	int i;

  /* Safety check to ensure start_compress not called yet. */
  if (cinfo->global_state != CSTATE_START)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);

  /* Allocate comp_info array large enough for maximum component count.
   * Array is made permanent in case application wants to compress
   * multiple images at same param settings.
   */
  if (cinfo->comp_info == NULL)
    cinfo->comp_info = ( sJpeg_component_info *)
      (*cinfo->mem->alloc_small) (( sJpeg_common_struct*) cinfo, JPOOL_PERMANENT,  MAX_COMPONENTS * SIZEOF( sJpeg_component_info));

  /* Initialize everything not dependent on the color space */

  cinfo->scale_num = 1;		/* 1:1 scaling */
  cinfo->scale_denom = 1;
  cinfo->data_precision = BITS_IN_JSAMPLE;
  /* Set up two quantization tables using default quality of 75 */
  jpeg_set_quality(cinfo, 75, TRUE);
  /* Set up two Huffman tables */
  std_huff_tables(cinfo);

  /* Initialize default arithmetic coding conditioning */
  for (i = 0; i < NUM_ARITH_TBLS; i++) {
    cinfo->arith_dc_L[i] = 0;
    cinfo->arith_dc_U[i] = 1;
    cinfo->arith_ac_K[i] = 5;
  }

  /* Default is no multiple-scan output */
  cinfo->scan_info = NULL;
  cinfo->num_scans = 0;

  /* Expect normal source image, not raw downsampled data */
  cinfo->raw_data_in = FALSE;

  /* Use Huffman coding, not arithmetic coding, by default */
  cinfo->arith_code = FALSE;

  /* By default, don't do extra passes to optimize entropy coding */
  cinfo->optimize_coding = FALSE;
  /* The standard Huffman tables are only valid for 8-bit data precision.
   * If the precision is higher, force optimization on so that usable
   * tables will be computed.  This test can be removed if default tables
   * are supplied that are valid for the desired precision.
   */
  if (cinfo->data_precision > 8)
    cinfo->optimize_coding = TRUE;

  /* By default, use the simpler non-cosited sampling alignment */
  cinfo->CCIR601_sampling = FALSE;

  /* By default, apply fancy downsampling */
  cinfo->do_fancy_downsampling = TRUE;

  /* No input smoothing */
  cinfo->smoothing_factor = 0;

  /* DCT algorithm preference */
  cinfo->dct_method = JDCT_DEFAULT;

  /* No restart markers */
  cinfo->restart_interval = 0;
  cinfo->restart_in_rows = 0;

  /* Fill in default JFIF marker parameters.  Note that whether the marker
   * will actually be written is determined by jpeg_set_colorspace.
   *
   * By default, the library emits JFIF version code 1.01.
   * An application that wants to emit JFIF 1.02 extension markers should set
   * JFIF_minor_version to 2.  We could probably get away with just defaulting
   * to 1.02, but there may still be some decoders in use that will complain
   * about that; saying 1.01 should minimize compatibility problems.
   */
  cinfo->JFIF_major_version = 1; /* Default JFIF version = 1.01 */
  cinfo->JFIF_minor_version = 1;
  cinfo->density_unit = 0;	/* Pixel size is unknown by default */
  cinfo->X_density = 1;		/* Pixel aspect ratio is square by default */
  cinfo->Y_density = 1;

  /* No color transform */
  cinfo->color_transform = JCT_NONE;

  /* Choose JPEG colorspace based on input space, set defaults accordingly */

  jpeg_default_colorspace(cinfo);
}

void CLibJpeg::jpeg_start_compress ( sJpeg_compress_struct *cinfo, boolean write_all_tables )
{
  if (cinfo->global_state != CSTATE_START)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);

  if (write_all_tables)
    jpeg_suppress_tables( cinfo, FALSE );	/* mark all tables to be written */

  /* (Re)initialize error mgr and destination modules */
	(*cinfo->err->reset_error_mgr) ((sJpeg_common_struct*) cinfo);
  (*cinfo->dest->init_destination) (cinfo);
  /* Perform master selection of active modules */
  jinit_compress_master(cinfo);
  /* Set up for the first pass */
  (*cinfo->master->prepare_for_pass) (cinfo);
  /* Ready for application to drive first pass through jpeg_write_scanlines
   * or jpeg_write_raw_data.
   */
  cinfo->next_scanline = 0;
  cinfo->global_state = (cinfo->raw_data_in ? CSTATE_RAW_OK : CSTATE_SCANNING);
}

JDIMENSION CLibJpeg::jpeg_write_scanlines ( sJpeg_compress_struct *cinfo, JSAMPARRAY scanlines, JDIMENSION num_lines )
{
	JDIMENSION row_ctr, rows_left;

  if (cinfo->global_state != CSTATE_SCANNING)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
  if (cinfo->next_scanline >= cinfo->image_height)
    WARNMS(cinfo, JWRN_TOO_MUCH_DATA);

  /* Call progress monitor hook if present */
  if (cinfo->progress != NULL) {
    cinfo->progress->pass_counter = (long) cinfo->next_scanline;
    cinfo->progress->pass_limit = (long) cinfo->image_height;
    (*cinfo->progress->progress_monitor) (( sJpeg_common_struct *) cinfo);
  }

  /* Give master control module another chance if this is first call to
   * jpeg_write_scanlines.  This lets output of the frame/scan headers be
   * delayed so that application can write COM, etc, markers between
   * jpeg_start_compress and jpeg_write_scanlines.
   */
  if (cinfo->master->call_pass_startup)
    (*cinfo->master->pass_startup) (cinfo);

  /* Ignore any extra scanlines at bottom of image. */
  rows_left = cinfo->image_height - cinfo->next_scanline;
  if (num_lines > rows_left)
    num_lines = rows_left;

  row_ctr = 0;
  (*cinfo->main->process_data) (cinfo, scanlines, &row_ctr, num_lines);
  cinfo->next_scanline += row_ctr;
  return row_ctr;
}

void CLibJpeg::jpeg_finish_compress ( sJpeg_compress_struct * cinfo)
{
	JDIMENSION iMCU_row;

  if (cinfo->global_state == CSTATE_SCANNING ||
      cinfo->global_state == CSTATE_RAW_OK) {
    /* Terminate first pass */
    if (cinfo->next_scanline < cinfo->image_height)
      ERREXIT(cinfo, JERR_TOO_LITTLE_DATA);
    (*cinfo->master->finish_pass) (cinfo);
  } else if (cinfo->global_state != CSTATE_WRCOEFS)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
  /* Perform any remaining passes */
  while (! cinfo->master->is_last_pass) {
    (*cinfo->master->prepare_for_pass) (cinfo);
    for (iMCU_row = 0; iMCU_row < cinfo->total_iMCU_rows; iMCU_row++) 
		{
      if (cinfo->progress != NULL) 
			{
				cinfo->progress->pass_counter = (long) iMCU_row;
				cinfo->progress->pass_limit = (long) cinfo->total_iMCU_rows;
				(*cinfo->progress->progress_monitor) (( sJpeg_common_struct*) cinfo);
      }
      /* We bypass the main controller and invoke coef controller directly;
       * all work is being done from the coefficient buffer.
       */
      if (! (*cinfo->coef->compress_data) (cinfo, (JSAMPIMAGE) NULL))
				ERREXIT(cinfo, JERR_CANT_SUSPEND);
    }
    (*cinfo->master->finish_pass) (cinfo);
  }
  /* Write EOI, do final cleanup */
  (*cinfo->marker->write_file_trailer) (cinfo);
  (*cinfo->dest->term_destination) (cinfo);
  /* We can use jpeg_abort to release memory and reset global_state */
  jpeg_abort((sJpeg_common_struct*) cinfo);
}

void CLibJpeg::jpeg_destroy_compress ( sJpeg_compress_struct * cinfo)
{
	jpeg_destroy( ( sJpeg_common_struct *) cinfo); /* use common routine */
}