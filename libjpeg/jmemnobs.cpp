#include "jmemnobs.h"
#include "jerror.h"

void* jpeg_get_small ( sJpeg_common_struct *cinfo, size_t sizeofobject )
{
	return (void *) malloc(sizeofobject);
}

void  jpeg_free_small ( sJpeg_common_struct *cinfo, void * object, size_t sizeofobject )
{
	free(object);
}

void * jpeg_get_large( sJpeg_common_struct *cinfo, size_t sizeofobject )
{
	return (void *) malloc(sizeofobject);
}

void  jpeg_free_large ( sJpeg_common_struct *cinfo, void * object, size_t sizeofobject )
{
	free(object);
}

long	jpeg_mem_available ( sJpeg_common_struct *cinfo, long min_bytes_needed, long max_bytes_needed, long already_allocated )
{
  return max_bytes_needed;
}

void jpeg_open_backing_store ( sJpeg_common_struct *cinfo, sBacking_store_struct *info, long total_bytes_needed)
{
	ERREXIT(cinfo, JERR_NO_BACKING_STORE);
}



long jpeg_mem_init ( sJpeg_common_struct *cinfo )
{
  return 0;			/* just set max_memory_to_use to 0 */
}

void jpeg_mem_term ( sJpeg_common_struct *cinfo )
{
  /* no work */
}