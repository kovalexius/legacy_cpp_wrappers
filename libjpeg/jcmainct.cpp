#include "jcmainct.h"

static void start_pass_main ( sJpeg_compress_struct *cinfo, J_BUF_MODE pass_mode )
{
	my_main_controller *mainp = ( my_main_controller * ) cinfo->main;

  /* Do nothing in raw-data mode. */
  if (cinfo->raw_data_in)
    return;

  mainp->cur_iMCU_row = 0;	/* initialize counters */
  mainp->rowgroup_ctr = 0;
  mainp->suspended = FALSE;
  mainp->pass_mode = pass_mode;	/* save mode for use by process_data */

  switch (pass_mode) 
	{
  case JBUF_PASS_THRU:
#ifdef FULL_MAIN_BUFFER_SUPPORTED
    if (mainp->whole_image[0] != NULL)
      ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
#endif
    mainp->pub.process_data = process_data_simple_main;
    break;
#ifdef FULL_MAIN_BUFFER_SUPPORTED
  case JBUF_SAVE_SOURCE:
  case JBUF_CRANK_DEST:
  case JBUF_SAVE_AND_PASS:
    if (mainp->whole_image[0] == NULL)
      ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
    mainp->pub.process_data = process_data_buffer_main;
    break;
#endif
  default:
    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
    break;
  }
}

static void process_data_simple_main ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JDIMENSION *in_row_ctr, JDIMENSION in_rows_avail )
{
	my_main_controller *mainp = (my_main_controller *) cinfo->main;

  while (mainp->cur_iMCU_row < cinfo->total_iMCU_rows) 
	{
    /* Read input data if we haven't filled the main buffer yet */
    if (mainp->rowgroup_ctr < (JDIMENSION) cinfo->min_DCT_v_scaled_size)
      (*cinfo->prep->pre_process_data) (cinfo,
					input_buf, in_row_ctr, in_rows_avail,
					mainp->buffer, &mainp->rowgroup_ctr,
					(JDIMENSION) cinfo->min_DCT_v_scaled_size);

    /* If we don't have a full iMCU row buffered, return to application for
     * more data.  Note that preprocessor will always pad to fill the iMCU row
     * at the bottom of the image.
     */
    if (mainp->rowgroup_ctr != (JDIMENSION) cinfo->min_DCT_v_scaled_size)
      return;

    /* Send the completed row to the compressor */
    if (! (*cinfo->coef->compress_data) (cinfo, mainp->buffer)) 
		{
      /* If compressor did not consume the whole row, then we must need to
       * suspend processing and return to the application.  In this situation
       * we pretend we didn't yet consume the last input row; otherwise, if
       * it happened to be the last row of the image, the application would
       * think we were done.
       */
      if (! mainp->suspended) 
			{
				(*in_row_ctr)--;
				mainp->suspended = TRUE;
      }
      return;
    }
    /* We did finish the row.  Undo our little suspension hack if a previous
     * call suspended; then mark the main buffer empty.
     */
    if (mainp->suspended) 
		{
      (*in_row_ctr)++;
      mainp->suspended = FALSE;
    }
    mainp->rowgroup_ctr = 0;
    mainp->cur_iMCU_row++;
  }
}

#ifdef FULL_MAIN_BUFFER_SUPPORTED
static void process_data_buffer_main ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JDIMENSION *in_row_ctr, JDIMENSION in_rows_avail )
{
	my_main_controller *mainp = (my_main_controller *) cinfo->main;
  int ci;
  sJpeg_component_info *compptr;
  boolean writing = (mainp->pass_mode != JBUF_CRANK_DEST);

  while (mainp->cur_iMCU_row < cinfo->total_iMCU_rows) {
    /* Realign the virtual buffers if at the start of an iMCU row. */
    if (mainp->rowgroup_ctr == 0) {
      for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
	   ci++, compptr++) {
	mainp->buffer[ci] = (*cinfo->mem->access_virt_sarray)((sJpeg_common_struct*) cinfo, mainp->whole_image[ci], mainp->cur_iMCU_row *
	   ((JDIMENSION) (compptr->v_samp_factor * cinfo->min_DCT_v_scaled_size)),
	   (JDIMENSION) (compptr->v_samp_factor * cinfo->min_DCT_v_scaled_size),
	   writing);
      }
      /* In a read pass, pretend we just read some source data. */
      if (! writing) {
	*in_row_ctr += (JDIMENSION)
	  (cinfo->max_v_samp_factor * cinfo->min_DCT_v_scaled_size);
	mainp->rowgroup_ctr = (JDIMENSION) cinfo->min_DCT_v_scaled_size;
      }
    }

    /* If a write pass, read input data until the current iMCU row is full. */
    /* Note: preprocessor will pad if necessary to fill the last iMCU row. */
    if (writing) {
      (*cinfo->prep->pre_process_data) (cinfo,
					input_buf, in_row_ctr, in_rows_avail,
					mainp->buffer, &mainp->rowgroup_ctr,
					(JDIMENSION) cinfo->min_DCT_v_scaled_size);
      /* Return to application if we need more data to fill the iMCU row. */
      if (mainp->rowgroup_ctr < (JDIMENSION) cinfo->min_DCT_v_scaled_size)
	return;
    }

    /* Emit data, unless this is a sink-only pass. */
    if (mainp->pass_mode != JBUF_SAVE_SOURCE) {
      if (! (*cinfo->coef->compress_data) (cinfo, mainp->buffer)) {
	/* If compressor did not consume the whole row, then we must need to
	 * suspend processing and return to the application.  In this situation
	 * we pretend we didn't yet consume the last input row; otherwise, if
	 * it happened to be the last row of the image, the application would
	 * think we were done.
	 */
	if (! mainp->suspended) {
	  (*in_row_ctr)--;
	  mainp->suspended = TRUE;
	}
	return;
      }
      /* We did finish the row.  Undo our little suspension hack if a previous
       * call suspended; then mark the main buffer empty.
       */
      if (mainp->suspended) {
	(*in_row_ctr)++;
	mainp->suspended = FALSE;
      }
    }

    /* If get here, we are done with this iMCU row.  Mark buffer empty. */
    mainp->rowgroup_ctr = 0;
    mainp->cur_iMCU_row++;
  }
}
#endif /* FULL_MAIN_BUFFER_SUPPORTED */

/*
 * Initialize main buffer controller.
 */
void jinit_c_main_controller ( sJpeg_compress_struct *cinfo, boolean need_full_buffer )
{
	my_main_controller *mainp;
  int ci;
  sJpeg_component_info *compptr;

  mainp = (my_main_controller *) (*cinfo->mem->alloc_small) ((sJpeg_common_struct*) cinfo, JPOOL_IMAGE,	SIZEOF(my_main_controller));
  cinfo->main = &mainp->pub;
  mainp->pub.start_pass = start_pass_main;

  /* We don't need to create a buffer in raw-data mode. */
  if (cinfo->raw_data_in)
    return;

  /* Create the buffer.  It holds downsampled data, so each component
   * may be of a different size.
   */
  if (need_full_buffer) 
	{
#ifdef FULL_MAIN_BUFFER_SUPPORTED
    /* Allocate a full-image virtual array for each component */
    /* Note we pad the bottom to a multiple of the iMCU height */
    for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
	 ci++, compptr++) {
      mainp->whole_image[ci] = (*cinfo->mem->request_virt_sarray)
	((j_common_ptr) cinfo, JPOOL_IMAGE, FALSE,
	 compptr->width_in_blocks * ((JDIMENSION) compptr->DCT_h_scaled_size),
	 ((JDIMENSION) jround_up((long) compptr->height_in_blocks,
				 (long) compptr->v_samp_factor)) *
	 ((JDIMENSION) cinfo->min_DCT_v_scaled_size),
	 (JDIMENSION) (compptr->v_samp_factor * compptr->DCT_v_scaled_size));
    }
#else
    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
#endif
  } 
	else 
	{
#ifdef FULL_MAIN_BUFFER_SUPPORTED
    mainp->whole_image[0] = NULL; /* flag for no virtual arrays */
#endif
    /* Allocate a strip buffer for each component */
    for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components; ci++, compptr++) 
		{
      mainp->buffer[ci] = (*cinfo->mem->alloc_sarray)((sJpeg_common_struct*) cinfo, JPOOL_IMAGE, compptr->width_in_blocks * ((JDIMENSION) compptr->DCT_h_scaled_size), (JDIMENSION) (compptr->v_samp_factor * compptr->DCT_v_scaled_size));
    }
  }
}