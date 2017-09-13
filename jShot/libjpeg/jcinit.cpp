#include "jcinit.h"
#include "jcmaster.h"
#include "jcarith.h"
#include "jccoefct.h"
#include "jcmainct.h"
#include "jcprepct.h"
#include "jccolor.h"
#include "jcsample.h"
#include "jcdctmgr.h"
#include "jchuff.h"
#include "jcmarker.h"

void jinit_compress_master ( sJpeg_compress_struct *cinfo)
{
  /* Initialize master control (includes parameter checking/processing) */
  jinit_c_master_control(cinfo, FALSE /* full compression */);

  /* Preprocessing */
  if (! cinfo->raw_data_in) 
	{
    jinit_color_converter(cinfo);
    jinit_downsampler(cinfo);
    jinit_c_prep_controller(cinfo, FALSE /* never need full buffer here */);
  }
  /* Forward DCT */
  jinit_forward_dct(cinfo);
  /* Entropy encoding: either Huffman or arithmetic coding. */
  if (cinfo->arith_code)
    jinit_arith_encoder(cinfo);
  else {
    jinit_huff_encoder(cinfo);
  }

  /* Need a full-image coefficient buffer in any multi-pass mode. */
  jinit_c_coef_controller(cinfo, (boolean) (cinfo->num_scans > 1 || cinfo->optimize_coding));
  jinit_c_main_controller(cinfo, FALSE /* never need full buffer here */);

  jinit_marker_writer(cinfo);

  /* We can now tell the memory manager to allocate virtual arrays. */
  (*cinfo->mem->realize_virt_arrays) ( ( sJpeg_common_struct *) cinfo );

  /* Write the datastream header (SOI) immediately.
   * Frame and scan headers are postponed till later.
   * This lets application insert special markers after the SOI.
   */
  (*cinfo->marker->write_file_header) (cinfo);
}