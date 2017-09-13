#include "jccolor.h"
#include "jmorecfg.h"
//#include "jerror.h"

static void rgb_ycc_start ( sJpeg_compress_struct *cinfo )
{
	my_color_converter *cconvert = ( my_color_converter * ) cinfo->cconvert;
  INT32 * rgb_ycc_tab;
  INT32 i;

  /* Allocate and fill in the conversion tables. */
  cconvert->rgb_ycc_tab = rgb_ycc_tab = (INT32 *)
    (*cinfo->mem->alloc_small) ( ( sJpeg_common_struct *) cinfo, JPOOL_IMAGE, (TABLE_SIZE * SIZEOF(INT32) ) );

  for (i = 0; i <= MAXJSAMPLE; i++) 
	{
    rgb_ycc_tab[i+R_Y_OFF] = FIX(0.29900) * i;
    rgb_ycc_tab[i+G_Y_OFF] = FIX(0.58700) * i;
    rgb_ycc_tab[i+B_Y_OFF] = FIX(0.11400) * i     + ONE_HALF;
    rgb_ycc_tab[i+R_CB_OFF] = (-FIX(0.16874)) * i;
    rgb_ycc_tab[i+G_CB_OFF] = (-FIX(0.33126)) * i;
    /* We use a rounding fudge-factor of 0.5-epsilon for Cb and Cr.
     * This ensures that the maximum output will round to MAXJSAMPLE
     * not MAXJSAMPLE+1, and thus that we don't have to range-limit.
     */
    rgb_ycc_tab[i+B_CB_OFF] = FIX(0.50000) * i    + CBCR_OFFSET + ONE_HALF-1;
/*  B=>Cb and R=>Cr tables are the same
    rgb_ycc_tab[i+R_CR_OFF] = FIX(0.50000) * i    + CBCR_OFFSET + ONE_HALF-1;
*/
    rgb_ycc_tab[i+G_CR_OFF] = (-FIX(0.41869)) * i;
    rgb_ycc_tab[i+B_CR_OFF] = (-FIX(0.08131)) * i;
  }
}

static void rgb_ycc_convert ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JSAMPIMAGE output_buf, JDIMENSION output_row, int num_rows )
{
	my_color_converter *cconvert = (my_color_converter *) cinfo->cconvert;
  register INT32 * ctab = cconvert->rgb_ycc_tab;
  register int r, g, b;
  register JSAMPROW inptr;
  register JSAMPROW outptr0, outptr1, outptr2;
  register JDIMENSION col;
  JDIMENSION num_cols = cinfo->image_width;

  while (--num_rows >= 0) 
	{
    inptr = *input_buf++;
    outptr0 = output_buf[0][output_row];
    outptr1 = output_buf[1][output_row];
    outptr2 = output_buf[2][output_row];
    output_row++;
    for (col = 0; col < num_cols; col++) 
		{
      r = GETJSAMPLE(inptr[RGB_RED]);
      g = GETJSAMPLE(inptr[RGB_GREEN]);
      b = GETJSAMPLE(inptr[RGB_BLUE]);
      /* If the inputs are 0..MAXJSAMPLE, the outputs of these equations
       * must be too; we do not need an explicit range-limiting operation.
       * Hence the value being shifted is never negative, and we don't
       * need the general RIGHT_SHIFT macro.
       */
      /* Y */
      outptr0[col] = (JSAMPLE)
		((ctab[r+R_Y_OFF] + ctab[g+G_Y_OFF] + ctab[b+B_Y_OFF])
		 >> SCALEBITS);
      /* Cb */
      outptr1[col] = (JSAMPLE)
		((ctab[r+R_CB_OFF] + ctab[g+G_CB_OFF] + ctab[b+B_CB_OFF])
		 >> SCALEBITS);
      /* Cr */
      outptr2[col] = (JSAMPLE)
		((ctab[r+R_CR_OFF] + ctab[g+G_CR_OFF] + ctab[b+B_CR_OFF])
		 >> SCALEBITS);
      inptr += RGB_PIXELSIZE;
    }
  }
}

static void rgb_gray_convert ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JSAMPIMAGE output_buf, JDIMENSION output_row, int num_rows )
{
	my_color_converter *cconvert = (my_color_converter *) cinfo->cconvert;
  register INT32 * ctab = cconvert->rgb_ycc_tab;
  register int r, g, b;
  register JSAMPROW inptr;
  register JSAMPROW outptr;
  register JDIMENSION col;
  JDIMENSION num_cols = cinfo->image_width;

  while (--num_rows >= 0) {
    inptr = *input_buf++;
    outptr = output_buf[0][output_row++];
    for (col = 0; col < num_cols; col++) 
		{
      r = GETJSAMPLE(inptr[RGB_RED]);
      g = GETJSAMPLE(inptr[RGB_GREEN]);
      b = GETJSAMPLE(inptr[RGB_BLUE]);
      /* Y */
      outptr[col] = (JSAMPLE)
		((ctab[r+R_Y_OFF] + ctab[g+G_Y_OFF] + ctab[b+B_Y_OFF])
		 >> SCALEBITS);
      inptr += RGB_PIXELSIZE;
    }
  }
}

static void cmyk_ycck_convert ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JSAMPIMAGE output_buf, JDIMENSION output_row, int num_rows )
{
	my_color_converter *cconvert = (my_color_converter *) cinfo->cconvert;
  register INT32 * ctab = cconvert->rgb_ycc_tab;
  register int r, g, b;
  register JSAMPROW inptr;
  register JSAMPROW outptr0, outptr1, outptr2, outptr3;
  register JDIMENSION col;
  JDIMENSION num_cols = cinfo->image_width;

  while (--num_rows >= 0) {
    inptr = *input_buf++;
    outptr0 = output_buf[0][output_row];
    outptr1 = output_buf[1][output_row];
    outptr2 = output_buf[2][output_row];
    outptr3 = output_buf[3][output_row];
    output_row++;
    for (col = 0; col < num_cols; col++) 
		{
      r = MAXJSAMPLE - GETJSAMPLE(inptr[0]);
      g = MAXJSAMPLE - GETJSAMPLE(inptr[1]);
      b = MAXJSAMPLE - GETJSAMPLE(inptr[2]);
      /* K passes through as-is */
      outptr3[col] = inptr[3];	/* don't need GETJSAMPLE here */
      /* If the inputs are 0..MAXJSAMPLE, the outputs of these equations
       * must be too; we do not need an explicit range-limiting operation.
       * Hence the value being shifted is never negative, and we don't
       * need the general RIGHT_SHIFT macro.
       */
      /* Y */
      outptr0[col] = (JSAMPLE)((ctab[r+R_Y_OFF] + ctab[g+G_Y_OFF] + ctab[b+B_Y_OFF]) >> SCALEBITS);
      /* Cb */
      outptr1[col] = (JSAMPLE)((ctab[r+R_CB_OFF] + ctab[g+G_CB_OFF] + ctab[b+B_CB_OFF]) >> SCALEBITS);
      /* Cr */
      outptr2[col] = (JSAMPLE)((ctab[r+R_CR_OFF] + ctab[g+G_CR_OFF] + ctab[b+B_CR_OFF]) >> SCALEBITS);
      inptr += 4;
    }
  }
}

static void rgb_rgb1_convert ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JSAMPIMAGE output_buf, JDIMENSION output_row, int num_rows )
{
	register int r, g, b;
  register JSAMPROW inptr;
  register JSAMPROW outptr0, outptr1, outptr2;
  register JDIMENSION col;
  JDIMENSION num_cols = cinfo->image_width;

  while (--num_rows >= 0) 
	{
    inptr = *input_buf++;
    outptr0 = output_buf[0][output_row];
    outptr1 = output_buf[1][output_row];
    outptr2 = output_buf[2][output_row];
    output_row++;
    for (col = 0; col < num_cols; col++) 
		{
      r = GETJSAMPLE(inptr[RGB_RED]);
      g = GETJSAMPLE(inptr[RGB_GREEN]);
      b = GETJSAMPLE(inptr[RGB_BLUE]);
      /* Assume that MAXJSAMPLE+1 is a power of 2, so that the MOD
       * (modulo) operator is equivalent to the bitmask operator AND.
       */
      outptr0[col] = (JSAMPLE) ((r - g + CENTERJSAMPLE) & MAXJSAMPLE);
      outptr1[col] = (JSAMPLE) g;
      outptr2[col] = (JSAMPLE) ((b - g + CENTERJSAMPLE) & MAXJSAMPLE);
      inptr += RGB_PIXELSIZE;
    }
  }
}

static void grayscale_convert ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JSAMPIMAGE output_buf, JDIMENSION output_row, int num_rows )
{
	int instride = cinfo->input_components;
  register JSAMPROW inptr;
  register JSAMPROW outptr;
  register JDIMENSION col;
  JDIMENSION num_cols = cinfo->image_width;

  while (--num_rows >= 0) 
	{
    inptr = *input_buf++;
    outptr = output_buf[0][output_row++];
    for (col = 0; col < num_cols; col++) {
      outptr[col] = inptr[0];	/* don't need GETJSAMPLE() here */
      inptr += instride;
    }
  }
}

static void rgb_convert ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JSAMPIMAGE output_buf, JDIMENSION output_row, int num_rows )
{
	register JSAMPROW inptr;
  register JSAMPROW outptr0, outptr1, outptr2;
  register JDIMENSION col;
  JDIMENSION num_cols = cinfo->image_width;

  while (--num_rows >= 0) 
	{
    inptr = *input_buf++;
    outptr0 = output_buf[0][output_row];
    outptr1 = output_buf[1][output_row];
    outptr2 = output_buf[2][output_row];
    output_row++;
    for (col = 0; col < num_cols; col++) 
		{
      /* We can dispense with GETJSAMPLE() here */
      outptr0[col] = inptr[RGB_RED];
      outptr1[col] = inptr[RGB_GREEN];
      outptr2[col] = inptr[RGB_BLUE];
      inptr += RGB_PIXELSIZE;
    }
  }
}

static void null_convert ( sJpeg_compress_struct *cinfo, JSAMPARRAY input_buf, JSAMPIMAGE output_buf, JDIMENSION output_row, int num_rows )
{
	int ci;
  register int nc = cinfo->num_components;
  register JSAMPROW inptr;
  register JSAMPROW outptr;
  register JDIMENSION col;
  JDIMENSION num_cols = cinfo->image_width;

  while (--num_rows >= 0) 
	{
    /* It seems fastest to make a separate pass for each component. */
    for (ci = 0; ci < nc; ci++) 
		{
      inptr = input_buf[0] + ci;
      outptr = output_buf[ci][output_row];
      for (col = 0; col < num_cols; col++) 
			{
				*outptr++ = *inptr;	/* don't need GETJSAMPLE() here */
				inptr += nc;
      }
    }
    input_buf++;
    output_row++;
  }
}

static void null_method ( sJpeg_compress_struct *cinfo)
{
	/* no work needed */
}

void jinit_color_converter ( sJpeg_compress_struct *cinfo )
{
	my_color_converter *cconvert;

  cconvert = (my_color_converter *)(*cinfo->mem->alloc_small) (( sJpeg_common_struct*) cinfo, JPOOL_IMAGE,SIZEOF(my_color_converter));
  cinfo->cconvert = &cconvert->pub;
  /* set start_pass to null method until we find out differently */
  cconvert->pub.start_pass = null_method;

  /* Make sure input_components agrees with in_color_space */
  switch (cinfo->in_color_space) {
  case JCS_GRAYSCALE:
    if (cinfo->input_components != 1)
      ERREXIT(cinfo, JERR_BAD_IN_COLORSPACE);
    break;

  case JCS_RGB:
    if (cinfo->input_components != RGB_PIXELSIZE)
      ERREXIT(cinfo, JERR_BAD_IN_COLORSPACE);
    break;

  case JCS_YCbCr:
    if (cinfo->input_components != 3)
      ERREXIT(cinfo, JERR_BAD_IN_COLORSPACE);
    break;

  case JCS_CMYK:
  case JCS_YCCK:
    if (cinfo->input_components != 4)
      ERREXIT(cinfo, JERR_BAD_IN_COLORSPACE);
    break;

  default:			/* JCS_UNKNOWN can be anything */
    if (cinfo->input_components < 1)
      ERREXIT(cinfo, JERR_BAD_IN_COLORSPACE);
    break;
  }

  /* Support color transform only for RGB colorspace */
  if (cinfo->color_transform && cinfo->jpeg_color_space != JCS_RGB)
    ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);

  /* Check num_components, set conversion method based on requested space */
  switch (cinfo->jpeg_color_space) {
  case JCS_GRAYSCALE:
    if (cinfo->num_components != 1)
      ERREXIT(cinfo, JERR_BAD_J_COLORSPACE);
    if (cinfo->in_color_space == JCS_GRAYSCALE ||
	cinfo->in_color_space == JCS_YCbCr)
      cconvert->pub.color_convert = grayscale_convert;
    else if (cinfo->in_color_space == JCS_RGB) {
      cconvert->pub.start_pass = rgb_ycc_start;
      cconvert->pub.color_convert = rgb_gray_convert;
    } else
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    break;

  case JCS_RGB:
    if (cinfo->num_components != 3)
      ERREXIT(cinfo, JERR_BAD_J_COLORSPACE);
    if (cinfo->in_color_space == JCS_RGB) {
      switch (cinfo->color_transform) {
      case JCT_NONE:
	cconvert->pub.color_convert = rgb_convert;
	break;
      case JCT_SUBTRACT_GREEN:
	cconvert->pub.color_convert = rgb_rgb1_convert;
	break;
      default:
	ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
	break;
      }
    } else
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    break;

  case JCS_YCbCr:
    if (cinfo->num_components != 3)
      ERREXIT(cinfo, JERR_BAD_J_COLORSPACE);
    if (cinfo->in_color_space == JCS_RGB) {
      cconvert->pub.start_pass = rgb_ycc_start;
      cconvert->pub.color_convert = rgb_ycc_convert;
    } else if (cinfo->in_color_space == JCS_YCbCr)
      cconvert->pub.color_convert = null_convert;
    else
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    break;

  case JCS_CMYK:
    if (cinfo->num_components != 4)
      ERREXIT(cinfo, JERR_BAD_J_COLORSPACE);
    if (cinfo->in_color_space == JCS_CMYK)
      cconvert->pub.color_convert = null_convert;
    else
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    break;

  case JCS_YCCK:
    if (cinfo->num_components != 4)
      ERREXIT(cinfo, JERR_BAD_J_COLORSPACE);
    if (cinfo->in_color_space == JCS_CMYK) {
      cconvert->pub.start_pass = rgb_ycc_start;
      cconvert->pub.color_convert = cmyk_ycck_convert;
    } else if (cinfo->in_color_space == JCS_YCCK)
      cconvert->pub.color_convert = null_convert;
    else
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    break;

  default:			/* allow null conversion of JCS_UNKNOWN */
    if (cinfo->jpeg_color_space != cinfo->in_color_space ||
	cinfo->num_components != cinfo->input_components)
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    cconvert->pub.color_convert = null_convert;
    break;
  }
}