#ifndef JMEMNOBS_H
#define JMEMNOBS_H

#include <stdlib.h>
#include "LibJpeg.h"
#include "jmemsys.h"


/*
 * Memory allocation and freeing are controlled by the regular library
 * routines malloc() and free().
 */
void* jpeg_get_small( sJpeg_common_struct *cinfo, size_t sizeofobject );
void  jpeg_free_small( sJpeg_common_struct *cinfo, void * object, size_t sizeofobject );

/*
 * "Large" objects are treated the same as "small" ones.
 * NB: although we include FAR keywords in the routine declarations,
 * this file won't actually work in 80x86 small/medium model; at least,
 * you probably won't be able to process useful-size images in only 64KB.
 */
void * jpeg_get_large( sJpeg_common_struct *cinfo, size_t sizeofobject );
void   jpeg_free_large ( sJpeg_common_struct *cinfo, void * object, size_t sizeofobject );

/*
 * This routine computes the total memory space available for allocation.
 * Here we always say, "we got all you want bud!"
 */

long	jpeg_mem_available ( sJpeg_common_struct *cinfo, long min_bytes_needed, long max_bytes_needed, long already_allocated );


/*
 * Backing store (temporary file) management.
 * Since jpeg_mem_available always promised the moon,
 * this should never be called and we can just error out.
 */

void jpeg_open_backing_store ( sJpeg_common_struct *cinfo, sBacking_store_struct *info, long total_bytes_needed);

/*
 * These routines take care of any system-dependent initialization and
 * cleanup required.  Here, there isn't any.
 */

long jpeg_mem_init ( sJpeg_common_struct *cinfo );
void jpeg_mem_term ( sJpeg_common_struct *cinfo );

#endif