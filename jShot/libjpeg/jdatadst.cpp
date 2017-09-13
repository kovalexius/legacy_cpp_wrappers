#include "jdatadst.h"

void init_destination ( sJpeg_compress_struct * cinfo)
{
	sMy_destination_mgr * dest = (sMy_destination_mgr *) cinfo->dest;

  /* Allocate the output buffer --- it will be released when done with image */
  dest->buffer = (JOCTET *)(*cinfo->mem->alloc_small) (( sJpeg_common_struct *) cinfo, JPOOL_IMAGE,  OUTPUT_BUF_SIZE * SIZEOF(JOCTET));

  dest->pub.next_output_byte = dest->buffer;
  dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
}

void init_mem_destination ( sJpeg_compress_struct *cinfo )
{
  /* no work necessary here */
	sMy_mem_destination_mgr *dest = ( sMy_mem_destination_mgr *) cinfo->dest;

	dest->buffer = (JOCTET *)(*cinfo->mem->alloc_small) (( sJpeg_common_struct *) cinfo, JPOOL_IMAGE,  OUTPUT_BUF_SIZE * SIZEOF(JOCTET));

	dest->bufsize = 0;
	dest->outsize = 0;
	dest->pub.next_output_byte = dest->buffer;
  dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
}

boolean empty_output_buffer ( sJpeg_compress_struct * cinfo)
{
	sMy_destination_mgr * dest = (sMy_destination_mgr *) cinfo->dest;

  if (JFWRITE(dest->outfile, dest->buffer, OUTPUT_BUF_SIZE) != (size_t) OUTPUT_BUF_SIZE )
    ERREXIT(cinfo, JERR_FILE_WRITE);

  dest->pub.next_output_byte = dest->buffer;
  dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;

  return TRUE;
}

boolean empty_mem_output_buffer ( sJpeg_compress_struct *cinfo )
{
  size_t nextsize;
  unsigned char * nextbuffer;
	sMy_mem_destination_mgr *dest = (sMy_mem_destination_mgr *) cinfo->dest;

  /* Try to allocate new buffer with double size */
  nextsize = dest->bufsize * 2;
	nextbuffer = (unsigned char *) malloc(nextsize);

  if (nextbuffer == NULL)
    ERREXIT1(cinfo, JERR_OUT_OF_MEMORY, 10);

  MEMCOPY(nextbuffer, dest->buffer, dest->bufsize);

  if (dest->newbuffer != NULL)
    free(dest->newbuffer);

  dest->newbuffer = nextbuffer;

  dest->pub.next_output_byte = nextbuffer + dest->bufsize;
  dest->pub.free_in_buffer = dest->bufsize;

  dest->buffer = nextbuffer;
  dest->bufsize = nextsize;

  return TRUE;
}

void term_destination ( sJpeg_compress_struct * cinfo)
{
	sMy_destination_mgr * dest = (sMy_destination_mgr *) cinfo->dest;
  size_t datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;

  /* Write any data remaining in the buffer */
  if (datacount > 0) {
    if (JFWRITE(dest->outfile, dest->buffer, datacount) != datacount)
      ERREXIT(cinfo, JERR_FILE_WRITE);
  }
  fflush(dest->outfile);
  /* Make sure we wrote the output file OK */
  if (ferror(dest->outfile))
    ERREXIT(cinfo, JERR_FILE_WRITE);
}

void term_mem_destination ( sJpeg_compress_struct *cinfo )
{
  sMy_mem_destination_mgr *dest = (sMy_mem_destination_mgr *) cinfo->dest;

  *dest->outbuffer = dest->buffer;
  *dest->outsize = dest->bufsize - dest->pub.free_in_buffer;
}