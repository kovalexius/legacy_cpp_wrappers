#include "jcsample.h"
#include "jutils.h"

static void start_pass_downsample ( sJpeg_compress_struct *cinfo )
{
}

void expand_right_edge (JSAMPARRAY image_data, int num_rows, JDIMENSION input_cols, JDIMENSION output_cols )
{
	register JSAMPROW ptr;
  register JSAMPLE pixval;
  register int count;
  int row;
  int numcols = (int) (output_cols - input_cols);

  if (numcols > 0) {
    for (row = 0; row < num_rows; row++) 
		{
      ptr = image_data[row] + input_cols;
      pixval = ptr[-1];		/* don't need GETJSAMPLE() here */
      for (count = numcols; count > 0; count--)
				*ptr++ = pixval;
    }
  }
}

static void sep_downsample ( sJpeg_compress_struct *cinfo, JSAMPIMAGE input_buf, JDIMENSION in_row_index,JSAMPIMAGE output_buf, JDIMENSION out_row_group_index )
{
	my_downsampler *downsample = ( my_downsampler * ) cinfo->downsample;
  int ci;
  sJpeg_component_info * compptr;
  JSAMPARRAY in_ptr, out_ptr;

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components; ci++, compptr++) 
	{
    in_ptr = input_buf[ci] + in_row_index;
    out_ptr = output_buf[ci] + (out_row_group_index * downsample->rowgroup_height[ci]);
    (*downsample->methods[ci]) (cinfo, compptr, in_ptr, out_ptr);
  }
}

static void int_downsample ( sJpeg_compress_struct * cinfo, sJpeg_component_info * compptr,	JSAMPARRAY input_data, JSAMPARRAY output_data )
{
	my_downsampler * downsample = (my_downsampler *) cinfo->downsample;
  int inrow, outrow, h_expand, v_expand, numpix, numpix2, h, v;
  JDIMENSION outcol, outcol_h;	/* outcol_h == outcol*h_expand */
  JDIMENSION output_cols = compptr->width_in_blocks * compptr->DCT_h_scaled_size;
  JSAMPROW inptr, outptr;
  INT32 outvalue;

  h_expand = downsample->h_expand[compptr->component_index];
  v_expand = downsample->v_expand[compptr->component_index];
  numpix = h_expand * v_expand;
  numpix2 = numpix/2;

  /* Expand input data enough to let all the output samples be generated
   * by the standard loop.  Special-casing padded output would be more
   * efficient.
   */
  expand_right_edge(input_data, cinfo->max_v_samp_factor,
		    cinfo->image_width, output_cols * h_expand);

  inrow = outrow = 0;
  while (inrow < cinfo->max_v_samp_factor) 
	{
    outptr = output_data[outrow];
    for (outcol = 0, outcol_h = 0; outcol < output_cols; outcol++, outcol_h += h_expand) 
		{
      outvalue = 0;
      for (v = 0; v < v_expand; v++) 
			{
				inptr = input_data[inrow+v] + outcol_h;
				for (h = 0; h < h_expand; h++) 
				{
				  outvalue += (INT32) GETJSAMPLE(*inptr++);
				}
      }
      *outptr++ = (JSAMPLE) ((outvalue + numpix2) / numpix);
    }
    inrow += v_expand;
    outrow++;
  }
}

static void fullsize_downsample ( sJpeg_compress_struct *cinfo, sJpeg_component_info * compptr, JSAMPARRAY input_data, JSAMPARRAY output_data )
{
	/* Copy the data */
  jcopy_sample_rows(input_data, 0, output_data, 0,
		    cinfo->max_v_samp_factor, cinfo->image_width);
  /* Edge-expand */
  expand_right_edge(output_data, cinfo->max_v_samp_factor, cinfo->image_width,
		    compptr->width_in_blocks * compptr->DCT_h_scaled_size);
}

static void h2v1_downsample ( sJpeg_compress_struct *cinfo, sJpeg_component_info * compptr, JSAMPARRAY input_data, JSAMPARRAY output_data )
{
	int inrow;
  JDIMENSION outcol;
  JDIMENSION output_cols = compptr->width_in_blocks * compptr->DCT_h_scaled_size;
  register JSAMPROW inptr, outptr;
  register int bias;

  /* Expand input data enough to let all the output samples be generated
   * by the standard loop.  Special-casing padded output would be more
   * efficient.
   */
  expand_right_edge(input_data, cinfo->max_v_samp_factor,
		    cinfo->image_width, output_cols * 2);

  for (inrow = 0; inrow < cinfo->max_v_samp_factor; inrow++) 
	{
    outptr = output_data[inrow];
    inptr = input_data[inrow];
    bias = 0;			/* bias = 0,1,0,1,... for successive samples */
    for (outcol = 0; outcol < output_cols; outcol++) 
		{
      *outptr++ = (JSAMPLE) ((GETJSAMPLE(*inptr) + GETJSAMPLE(inptr[1])
			      + bias) >> 1);
      bias ^= 1;		/* 0=>1, 1=>0 */
      inptr += 2;
    }
  }
}

static void h2v2_downsample ( sJpeg_compress_struct *cinfo, sJpeg_component_info * compptr, JSAMPARRAY input_data, JSAMPARRAY output_data )
{
	int inrow, outrow;
  JDIMENSION outcol;
  JDIMENSION output_cols = compptr->width_in_blocks * compptr->DCT_h_scaled_size;
  register JSAMPROW inptr0, inptr1, outptr;
  register int bias;

  /* Expand input data enough to let all the output samples be generated
   * by the standard loop.  Special-casing padded output would be more
   * efficient.
   */
  expand_right_edge(input_data, cinfo->max_v_samp_factor,
		    cinfo->image_width, output_cols * 2);

  inrow = outrow = 0;
  while (inrow < cinfo->max_v_samp_factor) 
	{
    outptr = output_data[outrow];
    inptr0 = input_data[inrow];
    inptr1 = input_data[inrow+1];
    bias = 1;			/* bias = 1,2,1,2,... for successive samples */
    for (outcol = 0; outcol < output_cols; outcol++) 
		{
      *outptr++ = (JSAMPLE) ( (GETJSAMPLE(*inptr0) + GETJSAMPLE(inptr0[1]) + GETJSAMPLE(*inptr1) + GETJSAMPLE(inptr1[1]) + bias) >> 2 );
      bias ^= 3;		/* 1=>2, 2=>1 */
      inptr0 += 2; inptr1 += 2;
    }
    inrow += 2;
    outrow++;
  }
}

#ifdef INPUT_SMOOTHING_SUPPORTED

static void h2v2_smooth_downsample ( sJpeg_compress_struct *cinfo, sJpeg_component_info * compptr, JSAMPARRAY input_data, JSAMPARRAY output_data )
{
	int inrow, outrow;
  JDIMENSION colctr;
  JDIMENSION output_cols = compptr->width_in_blocks * compptr->DCT_h_scaled_size;
  register JSAMPROW inptr0, inptr1, above_ptr, below_ptr, outptr;
  INT32 membersum, neighsum, memberscale, neighscale;

  /* Expand input data enough to let all the output samples be generated
   * by the standard loop.  Special-casing padded output would be more
   * efficient.
   */
  expand_right_edge(input_data - 1, cinfo->max_v_samp_factor + 2, cinfo->image_width, output_cols * 2);

  /* We don't bother to form the individual "smoothed" input pixel values;
   * we can directly compute the output which is the average of the four
   * smoothed values.  Each of the four member pixels contributes a fraction
   * (1-8*SF) to its own smoothed image and a fraction SF to each of the three
   * other smoothed pixels, therefore a total fraction (1-5*SF)/4 to the final
   * output.  The four corner-adjacent neighbor pixels contribute a fraction
   * SF to just one smoothed pixel, or SF/4 to the final output; while the
   * eight edge-adjacent neighbors contribute SF to each of two smoothed
   * pixels, or SF/2 overall.  In order to use integer arithmetic, these
   * factors are scaled by 2^16 = 65536.
   * Also recall that SF = smoothing_factor / 1024.
   */

  memberscale = 16384 - cinfo->smoothing_factor * 80; /* scaled (1-5*SF)/4 */
  neighscale = cinfo->smoothing_factor * 16; /* scaled SF/4 */

  inrow = outrow = 0;
  while (inrow < cinfo->max_v_samp_factor) 
	{
    outptr = output_data[outrow];
    inptr0 = input_data[inrow];
    inptr1 = input_data[inrow+1];
    above_ptr = input_data[inrow-1];
    below_ptr = input_data[inrow+2];

    /* Special case for first column: pretend column -1 is same as column 0 */
    membersum = GETJSAMPLE(*inptr0) + GETJSAMPLE(inptr0[1]) +
		GETJSAMPLE(*inptr1) + GETJSAMPLE(inptr1[1]);
    neighsum = GETJSAMPLE(*above_ptr) + GETJSAMPLE(above_ptr[1]) +
	       GETJSAMPLE(*below_ptr) + GETJSAMPLE(below_ptr[1]) +
	       GETJSAMPLE(*inptr0) + GETJSAMPLE(inptr0[2]) +
	       GETJSAMPLE(*inptr1) + GETJSAMPLE(inptr1[2]);
    neighsum += neighsum;
    neighsum += GETJSAMPLE(*above_ptr) + GETJSAMPLE(above_ptr[2]) +
		GETJSAMPLE(*below_ptr) + GETJSAMPLE(below_ptr[2]);
    membersum = membersum * memberscale + neighsum * neighscale;
    *outptr++ = (JSAMPLE) ((membersum + 32768) >> 16);
    inptr0 += 2; inptr1 += 2; above_ptr += 2; below_ptr += 2;

    for (colctr = output_cols - 2; colctr > 0; colctr--) {
      /* sum of pixels directly mapped to this output element */
      membersum = GETJSAMPLE(*inptr0) + GETJSAMPLE(inptr0[1]) +
		  GETJSAMPLE(*inptr1) + GETJSAMPLE(inptr1[1]);
      /* sum of edge-neighbor pixels */
      neighsum = GETJSAMPLE(*above_ptr) + GETJSAMPLE(above_ptr[1]) +
		 GETJSAMPLE(*below_ptr) + GETJSAMPLE(below_ptr[1]) +
		 GETJSAMPLE(inptr0[-1]) + GETJSAMPLE(inptr0[2]) +
		 GETJSAMPLE(inptr1[-1]) + GETJSAMPLE(inptr1[2]);
      /* The edge-neighbors count twice as much as corner-neighbors */
      neighsum += neighsum;
      /* Add in the corner-neighbors */
      neighsum += GETJSAMPLE(above_ptr[-1]) + GETJSAMPLE(above_ptr[2]) +
		  GETJSAMPLE(below_ptr[-1]) + GETJSAMPLE(below_ptr[2]);
      /* form final output scaled up by 2^16 */
      membersum = membersum * memberscale + neighsum * neighscale;
      /* round, descale and output it */
      *outptr++ = (JSAMPLE) ((membersum + 32768) >> 16);
      inptr0 += 2; inptr1 += 2; above_ptr += 2; below_ptr += 2;
    }

    /* Special case for last column */
    membersum = GETJSAMPLE(*inptr0) + GETJSAMPLE(inptr0[1]) +
		GETJSAMPLE(*inptr1) + GETJSAMPLE(inptr1[1]);
    neighsum = GETJSAMPLE(*above_ptr) + GETJSAMPLE(above_ptr[1]) +
	       GETJSAMPLE(*below_ptr) + GETJSAMPLE(below_ptr[1]) +
	       GETJSAMPLE(inptr0[-1]) + GETJSAMPLE(inptr0[1]) +
	       GETJSAMPLE(inptr1[-1]) + GETJSAMPLE(inptr1[1]);
    neighsum += neighsum;
    neighsum += GETJSAMPLE(above_ptr[-1]) + GETJSAMPLE(above_ptr[1]) +
		GETJSAMPLE(below_ptr[-1]) + GETJSAMPLE(below_ptr[1]);
    membersum = membersum * memberscale + neighsum * neighscale;
    *outptr = (JSAMPLE) ((membersum + 32768) >> 16);

    inrow += 2;
    outrow++;
  }
}

static void fullsize_smooth_downsample ( sJpeg_compress_struct *cinfo, sJpeg_component_info *compptr, JSAMPARRAY input_data, JSAMPARRAY output_data )
{
	int inrow;
  JDIMENSION colctr;
  JDIMENSION output_cols = compptr->width_in_blocks * compptr->DCT_h_scaled_size;
  register JSAMPROW inptr, above_ptr, below_ptr, outptr;
  INT32 membersum, neighsum, memberscale, neighscale;
  int colsum, lastcolsum, nextcolsum;

  /* Expand input data enough to let all the output samples be generated
   * by the standard loop.  Special-casing padded output would be more
   * efficient.
   */
  expand_right_edge(input_data - 1, cinfo->max_v_samp_factor + 2,
		    cinfo->image_width, output_cols);

  /* Each of the eight neighbor pixels contributes a fraction SF to the
   * smoothed pixel, while the main pixel contributes (1-8*SF).  In order
   * to use integer arithmetic, these factors are multiplied by 2^16 = 65536.
   * Also recall that SF = smoothing_factor / 1024.
   */

  memberscale = 65536L - cinfo->smoothing_factor * 512L; /* scaled 1-8*SF */
  neighscale = cinfo->smoothing_factor * 64; /* scaled SF */

  for (inrow = 0; inrow < cinfo->max_v_samp_factor; inrow++) {
    outptr = output_data[inrow];
    inptr = input_data[inrow];
    above_ptr = input_data[inrow-1];
    below_ptr = input_data[inrow+1];

    /* Special case for first column */
    colsum = GETJSAMPLE(*above_ptr++) + GETJSAMPLE(*below_ptr++) +
	     GETJSAMPLE(*inptr);
    membersum = GETJSAMPLE(*inptr++);
    nextcolsum = GETJSAMPLE(*above_ptr) + GETJSAMPLE(*below_ptr) +
		 GETJSAMPLE(*inptr);
    neighsum = colsum + (colsum - membersum) + nextcolsum;
    membersum = membersum * memberscale + neighsum * neighscale;
    *outptr++ = (JSAMPLE) ((membersum + 32768) >> 16);
    lastcolsum = colsum; colsum = nextcolsum;

    for (colctr = output_cols - 2; colctr > 0; colctr--) {
      membersum = GETJSAMPLE(*inptr++);
      above_ptr++; below_ptr++;
      nextcolsum = GETJSAMPLE(*above_ptr) + GETJSAMPLE(*below_ptr) +
		   GETJSAMPLE(*inptr);
      neighsum = lastcolsum + (colsum - membersum) + nextcolsum;
      membersum = membersum * memberscale + neighsum * neighscale;
      *outptr++ = (JSAMPLE) ((membersum + 32768) >> 16);
      lastcolsum = colsum; colsum = nextcolsum;
    }

    /* Special case for last column */
    membersum = GETJSAMPLE(*inptr);
    neighsum = lastcolsum + (colsum - membersum) + colsum;
    membersum = membersum * memberscale + neighsum * neighscale;
    *outptr = (JSAMPLE) ((membersum + 32768) >> 16);

  }
}

#endif /* INPUT_SMOOTHING_SUPPORTED */

void jinit_downsampler ( sJpeg_compress_struct *cinfo )
{
	my_downsampler *downsample;
  int ci;
  sJpeg_component_info * compptr;
  boolean smoothok = TRUE;
  int h_in_group, v_in_group, h_out_group, v_out_group;

  downsample = (my_downsampler *) (*cinfo->mem->alloc_small) ( (sJpeg_common_struct*) cinfo, JPOOL_IMAGE,	SIZEOF(my_downsampler));
  cinfo->downsample = ( sJpeg_downsampler *) downsample;
  downsample->pub.start_pass = start_pass_downsample;
  downsample->pub.downsample = sep_downsample;
  downsample->pub.need_context_rows = FALSE;

  if (cinfo->CCIR601_sampling)
    ERREXIT(cinfo, JERR_CCIR601_NOTIMPL);

  /* Verify we can handle the sampling factors, and set up method pointers */
  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components; ci++, compptr++) 
	{
    /* Compute size of an "output group" for DCT scaling.  This many samples
     * are to be converted from max_h_samp_factor * max_v_samp_factor pixels.
     */
    h_out_group = (compptr->h_samp_factor * compptr->DCT_h_scaled_size) / cinfo->min_DCT_h_scaled_size;
    v_out_group = (compptr->v_samp_factor * compptr->DCT_v_scaled_size) / cinfo->min_DCT_v_scaled_size;
    h_in_group = cinfo->max_h_samp_factor;
    v_in_group = cinfo->max_v_samp_factor;
    downsample->rowgroup_height[ci] = v_out_group; /* save for use later */
    if (h_in_group == h_out_group && v_in_group == v_out_group) 
		{
#ifdef INPUT_SMOOTHING_SUPPORTED
      if (cinfo->smoothing_factor) 
			{
				downsample->methods[ci] = fullsize_smooth_downsample;
				downsample->pub.need_context_rows = TRUE;
      } 
			else
#endif
			downsample->methods[ci] = fullsize_downsample;
    } 
		else if (h_in_group == h_out_group * 2 && v_in_group == v_out_group) 
		{
      smoothok = FALSE;
      downsample->methods[ci] = h2v1_downsample;
    } 
		else if (h_in_group == h_out_group * 2 && v_in_group == v_out_group * 2) 
		{
#ifdef INPUT_SMOOTHING_SUPPORTED
      if (cinfo->smoothing_factor) 
			{
				downsample->methods[ci] = h2v2_smooth_downsample;
				downsample->pub.need_context_rows = TRUE;
      } 
			else
#endif
	downsample->methods[ci] = h2v2_downsample;
    } 
		else if ((h_in_group % h_out_group) == 0 && (v_in_group % v_out_group) == 0) 
		{
      smoothok = FALSE;
      downsample->methods[ci] = int_downsample;
      downsample->h_expand[ci] = (UINT8) (h_in_group / h_out_group);
      downsample->v_expand[ci] = (UINT8) (v_in_group / v_out_group);
    } 
		else
      ERREXIT(cinfo, JERR_FRACT_SAMPLE_NOTIMPL);
  }

#ifdef INPUT_SMOOTHING_SUPPORTED
  if (cinfo->smoothing_factor && !smoothok)
    TRACEMS(cinfo, 0, JTRC_SMOOTH_NOTIMPL);
#endif
}