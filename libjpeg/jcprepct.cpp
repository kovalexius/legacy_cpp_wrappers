#include "jcprepct.h"
#include "jutils.h"

static void start_pass_prep ( sJpeg_compress_struct *cinfo, J_BUF_MODE pass_mode )
{
	my_prep_controller *prep = (my_prep_controller *) cinfo->prep;

  if (pass_mode != JBUF_PASS_THRU)
    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);

  /* Initialize total-height counter for detecting bottom of image */
  prep->rows_to_go = cinfo->image_height;
  /* Mark the conversion buffer empty */
  prep->next_buf_row = 0;
#ifdef CONTEXT_ROWS_SUPPORTED
  /* Preset additional state variables for context mode.
   * These aren't used in non-context mode, so we needn't test which mode.
   */
  prep->this_row_group = 0;
  /* Set next_buf_stop to stop after two row groups have been read in. */
  prep->next_buf_stop = 2 * cinfo->max_v_samp_factor;
#endif
}

static void expand_bottom_edge (JSAMPARRAY image_data, JDIMENSION num_cols, int input_rows, int output_rows )
{
	register int row;

  for (row = input_rows; row < output_rows; row++) 
	{
    jcopy_sample_rows(image_data, input_rows-1, image_data, row, 1, num_cols);
  }
}

static void pre_process_data ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JDIMENSION *in_row_ctr, JDIMENSION in_rows_avail,
		  JSAMPIMAGE output_buf, JDIMENSION *out_row_group_ctr, JDIMENSION out_row_groups_avail )
{
	my_prep_controller *prep = (my_prep_controller *) cinfo->prep;
  int numrows, ci;
  JDIMENSION inrows;
  sJpeg_component_info * compptr;

  while (*in_row_ctr < in_rows_avail && *out_row_group_ctr < out_row_groups_avail ) 
	{
    /* Do color conversion to fill the conversion buffer. */
    inrows = in_rows_avail - *in_row_ctr;
    numrows = cinfo->max_v_samp_factor - prep->next_buf_row;
    numrows = (int) MIN((JDIMENSION) numrows, inrows);
    (*cinfo->cconvert->color_convert) (cinfo, input_buf + *in_row_ctr, prep->color_buf, (JDIMENSION) prep->next_buf_row, numrows );
    *in_row_ctr += numrows;
    prep->next_buf_row += numrows;
    prep->rows_to_go -= numrows;
    /* If at bottom of image, pad to fill the conversion buffer. */
    if (prep->rows_to_go == 0 && prep->next_buf_row < cinfo->max_v_samp_factor) 
		{
      for (ci = 0; ci < cinfo->num_components; ci++) 
			{
					expand_bottom_edge(prep->color_buf[ci], cinfo->image_width,
			   prep->next_buf_row, cinfo->max_v_samp_factor);
      }
      prep->next_buf_row = cinfo->max_v_samp_factor;
    }
    /* If we've filled the conversion buffer, empty it. */
    if (prep->next_buf_row == cinfo->max_v_samp_factor) 
		{
      (*cinfo->downsample->downsample) ( cinfo,prep->color_buf, (JDIMENSION) 0,output_buf, *out_row_group_ctr );
      prep->next_buf_row = 0;
      (*out_row_group_ctr)++;
    }
    /* If at bottom of image, pad the output to a full iMCU height.
     * Note we assume the caller is providing a one-iMCU-height output buffer!
     */
    if (prep->rows_to_go == 0 && *out_row_group_ctr < out_row_groups_avail) 
		{
      for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components; ci++, compptr++) 
			{
				numrows = (compptr->v_samp_factor * compptr->DCT_v_scaled_size) / cinfo->min_DCT_v_scaled_size;
				expand_bottom_edge(output_buf[ci], compptr->width_in_blocks * compptr->DCT_h_scaled_size, (int) (*out_row_group_ctr * numrows), (int) (out_row_groups_avail * numrows));
      }
      *out_row_group_ctr = out_row_groups_avail;
      break;			/* can exit outer loop without test */
    }
  }
}

#ifdef CONTEXT_ROWS_SUPPORTED
static void pre_process_context ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JDIMENSION *in_row_ctr, JDIMENSION in_rows_avail, JSAMPIMAGE output_buf, JDIMENSION *out_row_group_ctr, JDIMENSION out_row_groups_avail )
{
	my_prep_controller *prep = (my_prep_controller *) cinfo->prep;
  int numrows, ci;
  int buf_height = cinfo->max_v_samp_factor * 3;
  JDIMENSION inrows;

  while (*out_row_group_ctr < out_row_groups_avail) 
	{
    if (*in_row_ctr < in_rows_avail) 
		{
      /* Do color conversion to fill the conversion buffer. */
      inrows = in_rows_avail - *in_row_ctr;
      numrows = prep->next_buf_stop - prep->next_buf_row;
      numrows = (int) MIN((JDIMENSION) numrows, inrows);
      (*cinfo->cconvert->color_convert) (cinfo, input_buf + *in_row_ctr, prep->color_buf, (JDIMENSION) prep->next_buf_row, numrows );
      /* Pad at top of image, if first time through */
      if (prep->rows_to_go == cinfo->image_height) 
			{
				for (ci = 0; ci < cinfo->num_components; ci++) 
				{
				  int row;
				  for ( row = 1; row <= cinfo->max_v_samp_factor; row++ ) 
					{
				    jcopy_sample_rows( prep->color_buf[ci], 0, prep->color_buf[ci], -row, 1, cinfo->image_width );
				  }
				}
      }
      *in_row_ctr += numrows;
      prep->next_buf_row += numrows;
      prep->rows_to_go -= numrows;
    } 
		else 
		{
      /* Return for more data, unless we are at the bottom of the image. */
      if (prep->rows_to_go != 0)
				break;
      /* When at bottom of image, pad to fill the conversion buffer. */
      if (prep->next_buf_row < prep->next_buf_stop) 
			{
				for (ci = 0; ci < cinfo->num_components; ci++) 
				{
				  expand_bottom_edge(prep->color_buf[ci], cinfo->image_width,
						     prep->next_buf_row, prep->next_buf_stop);
				}
				prep->next_buf_row = prep->next_buf_stop;
      }
    }
    /* If we've gotten enough data, downsample a row group. */
    if (prep->next_buf_row == prep->next_buf_stop) {
      (*cinfo->downsample->downsample) (cinfo,
					prep->color_buf,
					(JDIMENSION) prep->this_row_group,
					output_buf, *out_row_group_ctr);
      (*out_row_group_ctr)++;
      /* Advance pointers with wraparound as necessary. */
      prep->this_row_group += cinfo->max_v_samp_factor;
      if (prep->this_row_group >= buf_height)
	prep->this_row_group = 0;
      if (prep->next_buf_row >= buf_height)
	prep->next_buf_row = 0;
      prep->next_buf_stop = prep->next_buf_row + cinfo->max_v_samp_factor;
    }
  }
}

static void create_context_buffer ( sJpeg_compress_struct *cinfo )
{
	my_prep_controller *prep = (my_prep_controller *) cinfo->prep;
  int rgroup_height = cinfo->max_v_samp_factor;
  int ci, i;
  sJpeg_component_info * compptr;
  JSAMPARRAY true_buffer, fake_buffer;

  /* Grab enough space for fake row pointers for all the components;
   * we need five row groups' worth of pointers for each component.
   */
  fake_buffer = (JSAMPARRAY)
    (*cinfo->mem->alloc_small) ((sJpeg_common_struct *) cinfo, JPOOL_IMAGE, (cinfo->num_components * 5 * rgroup_height) * SIZEOF(JSAMPROW) );

  for ( ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components; ci++, compptr++ ) 
	{
    /* Allocate the actual buffer space (3 row groups) for this component.
     * We make the buffer wide enough to allow the downsampler to edge-expand
     * horizontally within the buffer, if it so chooses.
     */
    true_buffer = (*cinfo->mem->alloc_sarray) ((sJpeg_common_struct *) cinfo, JPOOL_IMAGE, (JDIMENSION) (((long) compptr->width_in_blocks * cinfo->min_DCT_h_scaled_size * cinfo->max_h_samp_factor) / compptr->h_samp_factor),
       (JDIMENSION) (3 * rgroup_height));
    /* Copy true buffer row pointers into the middle of the fake row array */
    MEMCOPY(fake_buffer + rgroup_height, true_buffer,
	    3 * rgroup_height * SIZEOF(JSAMPROW));
    /* Fill in the above and below wraparound pointers */
    for (i = 0; i < rgroup_height; i++) 
		{
      fake_buffer[i] = true_buffer[2 * rgroup_height + i];
      fake_buffer[4 * rgroup_height + i] = true_buffer[i];
    }
    prep->color_buf[ci] = fake_buffer + rgroup_height;
    fake_buffer += 5 * rgroup_height; /* point to space for next component */
  }
}
#endif /* CONTEXT_ROWS_SUPPORTED */

void jinit_c_prep_controller ( sJpeg_compress_struct *cinfo, boolean need_full_buffer )
{
	my_prep_controller *prep;
  int ci;
  sJpeg_component_info * compptr;

  if (need_full_buffer)		/* safety check */
    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);

  prep = (my_prep_controller *)(*cinfo->mem->alloc_small) (( sJpeg_common_struct* ) cinfo, JPOOL_IMAGE,	SIZEOF(my_prep_controller));
	cinfo->prep = (sJpeg_c_prep_controller *) prep;
  prep->pub.start_pass = start_pass_prep;

  /* Allocate the color conversion buffer.
   * We make the buffer wide enough to allow the downsampler to edge-expand
   * horizontally within the buffer, if it so chooses.
   */
  if (cinfo->downsample->need_context_rows) {
    /* Set up to provide context rows */
#ifdef CONTEXT_ROWS_SUPPORTED
    prep->pub.pre_process_data = pre_process_context;
    create_context_buffer(cinfo);
#else
    ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
  } 
	else 
	{
    /* No context, just make it tall enough for one row group */
    prep->pub.pre_process_data = pre_process_data;
    for ( ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components; ci++, compptr++ ) 
		{
      prep->color_buf[ci] = (*cinfo->mem->alloc_sarray)	((sJpeg_common_struct*) cinfo, JPOOL_IMAGE, (JDIMENSION) (((long) compptr->width_in_blocks * cinfo->min_DCT_h_scaled_size *
				cinfo->max_h_samp_factor) / compptr->h_samp_factor), (JDIMENSION) cinfo->max_v_samp_factor);
    }
  }
}