#include <stdlib.h>
#include "jmemmgr.h"
#include "jerror.h"
#include "jpegint.h"
#include "jmemnobs.h"


static JBLOCKARRAY access_virt_barray ( sJpeg_common_struct *cinfo, sJvirt_barray_control *ptr, unsigned int start_row, unsigned int num_rows, boolean writable)
{
  unsigned int end_row = start_row + num_rows;
  unsigned int undef_row;

  /* debugging check */
  if ( end_row > ptr->rows_in_array || num_rows > ptr->maxaccess || ptr->mem_buffer == NULL )
    ERREXIT(cinfo, JERR_BAD_VIRTUAL_ACCESS);

  /* Make the desired part of the virtual array accessible */
  if (start_row < ptr->cur_start_row || end_row > ptr->cur_start_row+ptr->rows_in_mem) 
	{
    if ( ! ptr->b_s_open )
      ERREXIT( cinfo, JERR_VIRTUAL_BUG );
    /* Flush old buffer contents if necessary */
    if ( ptr->dirty ) 
		{
      do_barray_io(cinfo, ptr, TRUE);
      ptr->dirty = FALSE;
    }
    /* Decide what part of virtual array to access.
     * Algorithm: if target address > current window, assume forward scan,
     * load starting at target address.  If target address < current window,
     * assume backward scan, load so that target area is top of window.
     * Note that when switching from forward write to forward read, will have
     * start_row = 0, so the limiting case applies and we load from 0 anyway.
     */
    if (start_row > ptr->cur_start_row) 
      ptr->cur_start_row = start_row;
		else 
		{
      /* use long arithmetic here to avoid overflow & unsigned problems */
      long ltemp;

      ltemp = (long) end_row - (long) ptr->rows_in_mem;
      if (ltemp < 0)
				ltemp = 0;		/* don't fall off front end of file */
      ptr->cur_start_row = (unsigned int) ltemp;
    }
    /* Read in the selected part of the array.
     * During the initial write pass, we will do no actual read
     * because the selected part is all undefined.
     */
    do_barray_io(cinfo, ptr, FALSE);
  }
  /* Ensure the accessed part of the array is defined; prezero if needed.
   * To improve locality of access, we only prezero the part of the array
   * that the caller is about to access, not the entire in-memory array.
   */
  if (ptr->first_undef_row < end_row) 
	{
    if (ptr->first_undef_row < start_row) 
		{
      if (writable)		/* writer skipped over a section of array */
				ERREXIT(cinfo, JERR_BAD_VIRTUAL_ACCESS);
      undef_row = start_row;	/* but reader is allowed to read ahead */
    } 
		else 
		{
      undef_row = ptr->first_undef_row;
    }
    if (writable)
      ptr->first_undef_row = end_row;
    if (ptr->pre_zero) 
		{
      size_t bytesperrow = (size_t) ptr->blocksperrow * (size_t)sizeof( short[DCTSIZE2] );
      undef_row -= ptr->cur_start_row; /* make indexes relative to buffer */
      end_row -= ptr->cur_start_row;
      while (undef_row < end_row) 
			{
				FMEMZERO((void *) ptr->mem_buffer[undef_row], bytesperrow);
				undef_row++;
      }
    }
		else 
		{
      if (! writable)		/* reader looking at undefined data */
			ERREXIT(cinfo, JERR_BAD_VIRTUAL_ACCESS);
    }
  }
  /* Flag the buffer dirty if caller will write in it */
  if (writable)
    ptr->dirty = TRUE;
  /* Return address of proper part of the buffer */
  return ptr->mem_buffer + (start_row - ptr->cur_start_row);
}


static void do_barray_io ( sJpeg_common_struct *cinfo, sJvirt_barray_control *ptr, boolean writing)
/* Do backing store read or write of a virtual coefficient-block array */
{
  long bytesperrow, file_offset, byte_count, rows, thisrow, i;

	bytesperrow = (long) ptr->blocksperrow * (size_t)sizeof( short[DCTSIZE2] );
  file_offset = ptr->cur_start_row * bytesperrow;
  /* Loop to read or write each allocation chunk in mem_buffer */
  for (i = 0; i < (long) ptr->rows_in_mem; i += ptr->rowsperchunk) 
	{
    /* One chunk, but check for short chunk at end of buffer */
    rows = MIN((long) ptr->rowsperchunk, (long) ptr->rows_in_mem - i);
    /* Transfer no more than is currently defined */
    thisrow = (long) ptr->cur_start_row + i;
    rows = MIN(rows, (long) ptr->first_undef_row - thisrow);
    /* Transfer no more than fits in file */
    rows = MIN(rows, (long) ptr->rows_in_array - thisrow);
    if (rows <= 0)		/* this chunk might be past end of file! */
      break;
    byte_count = rows * bytesperrow;
    if (writing)
      ( *ptr->b_s_info.write_backing_store ) ( cinfo, & ptr->b_s_info, (void *) ptr->mem_buffer[i], file_offset, byte_count );
    else
      ( *ptr->b_s_info.read_backing_store ) ( cinfo, & ptr->b_s_info, (void *) ptr->mem_buffer[i], file_offset, byte_count );
    file_offset += byte_count;
  }
}



static void do_sarray_io ( sJpeg_common_struct *cinfo, sJvirt_sarray_control *ptr, boolean writing )
/* Do backing store read or write of a virtual sample array */
{
  long bytesperrow, file_offset, byte_count, rows, thisrow, i;

	bytesperrow = (long) ptr->samplesperrow * (size_t)sizeof( unsigned char );
  file_offset = ptr->cur_start_row * bytesperrow;
  /* Loop to read or write each allocation chunk in mem_buffer */
  for (i = 0; i < (long) ptr->rows_in_mem; i += ptr->rowsperchunk) 
	{
    /* One chunk, but check for short chunk at end of buffer */
    rows = MIN((long) ptr->rowsperchunk, (long) ptr->rows_in_mem - i);
    /* Transfer no more than is currently defined */
    thisrow = (long) ptr->cur_start_row + i;
    rows = MIN(rows, (long) ptr->first_undef_row - thisrow);
    /* Transfer no more than fits in file */
    rows = MIN(rows, (long) ptr->rows_in_array - thisrow);
    if (rows <= 0)		/* this chunk might be past end of file! */
      break;
    byte_count = rows * bytesperrow;
    if (writing)
      ( *ptr->b_s_info.write_backing_store ) ( cinfo, & ptr->b_s_info, (void *) ptr->mem_buffer[i], file_offset, byte_count );
    else
      (*ptr->b_s_info.read_backing_store) ( cinfo, & ptr->b_s_info, (void *) ptr->mem_buffer[i], file_offset, byte_count );
    file_offset += byte_count;
  }
}

static unsigned char** access_virt_sarray ( sJpeg_common_struct *cinfo, sJvirt_sarray_control *ptr, unsigned int start_row, unsigned int num_rows, boolean writable )
/* Access the part of a virtual sample array starting at start_row */
/* and extending for num_rows rows.  writable is true if  */
/* caller intends to modify the accessed area. */
{
  unsigned int end_row = start_row + num_rows;
  unsigned int undef_row;

  /* debugging check */
  if ( end_row > ptr->rows_in_array || num_rows > ptr->maxaccess || ptr->mem_buffer == NULL )
    ERREXIT(cinfo, JERR_BAD_VIRTUAL_ACCESS);

  /* Make the desired part of the virtual array accessible */
  if ( start_row < ptr->cur_start_row || end_row > ptr->cur_start_row+ptr->rows_in_mem ) 
	{
    if ( !ptr->b_s_open )
      ERREXIT( cinfo, JERR_VIRTUAL_BUG );
    /* Flush old buffer contents if necessary */
    if (ptr->dirty) 
		{
      do_sarray_io(cinfo, ptr, TRUE);
      ptr->dirty = FALSE;
    }
    /* Decide what part of virtual array to access.
     * Algorithm: if target address > current window, assume forward scan,
     * load starting at target address.  If target address < current window,
     * assume backward scan, load so that target area is top of window.
     * Note that when switching from forward write to forward read, will have
     * start_row = 0, so the limiting case applies and we load from 0 anyway.
     */
    if (start_row > ptr->cur_start_row) {
      ptr->cur_start_row = start_row;
    } else {
      /* use long arithmetic here to avoid overflow & unsigned problems */
      long ltemp;

      ltemp = (long) end_row - (long) ptr->rows_in_mem;
      if (ltemp < 0)
	ltemp = 0;		/* don't fall off front end of file */
      ptr->cur_start_row = (unsigned int) ltemp;
    }
    /* Read in the selected part of the array.
     * During the initial write pass, we will do no actual read
     * because the selected part is all undefined.
     */
    do_sarray_io(cinfo, ptr, FALSE);
  }
  /* Ensure the accessed part of the array is defined; prezero if needed.
   * To improve locality of access, we only prezero the part of the array
   * that the caller is about to access, not the entire in-memory array.
   */
  if (ptr->first_undef_row < end_row) 
	{
    if (ptr->first_undef_row < start_row) 
		{
      if (writable)		/* writer skipped over a section of array */
				ERREXIT(cinfo, JERR_BAD_VIRTUAL_ACCESS);
      undef_row = start_row;	/* but reader is allowed to read ahead */
    } 
		else 
		{
      undef_row = ptr->first_undef_row;
    }
    if (writable)
      ptr->first_undef_row = end_row;
    if (ptr->pre_zero) 
		{
			size_t bytesperrow = (size_t) ptr->samplesperrow * (size_t)sizeof( unsigned char );
      undef_row -= ptr->cur_start_row; /* make indexes relative to buffer */
      end_row -= ptr->cur_start_row;
      while ( undef_row < end_row ) 
			{
				FMEMZERO( (void *) ptr->mem_buffer[undef_row], bytesperrow );
				undef_row++;
      }
    } 
		else 
		{
      if (! writable)		/* reader looking at undefined data */
			ERREXIT(cinfo, JERR_BAD_VIRTUAL_ACCESS);
    }
  }
  /* Flag the buffer dirty if caller will write in it */
  if (writable)
    ptr->dirty = TRUE;
  /* Return address of proper part of the buffer */
  return ptr->mem_buffer + (start_row - ptr->cur_start_row);
}

static JBLOCKARRAY alloc_barray ( sJpeg_common_struct *cinfo, int pool_id, unsigned int blocksperrow, unsigned int numrows )
{
  sMy_memory_mgr *mem = (sMy_memory_mgr *) cinfo->mem;
  JBLOCKARRAY result;
  JBLOCKROW workspace;
  unsigned int rowsperchunk, currow, i;
  long ltemp;

  /* Calculate max # of rows allowed in one allocation chunk */
  ltemp = ( MAX_ALLOC_CHUNK - (size_t)sizeof(uLarge_pool_struct) ) / ( (long) blocksperrow * (size_t)sizeof(short[DCTSIZE2]) );
  if (ltemp <= 0)
    ERREXIT(cinfo, JERR_WIDTH_OVERFLOW);
  if (ltemp < (long) numrows)
    rowsperchunk = (unsigned int) ltemp;
  else
    rowsperchunk = numrows;
  mem->last_rowsperchunk = rowsperchunk;

  /* Get space for row pointers (small object) */
	result = ( JBLOCKARRAY ) alloc_small( cinfo, pool_id, (size_t) (numrows * (size_t)sizeof(short **)));

  /* Get the rows themselves (large objects) */
  currow = 0;
  while (currow < numrows)
	{
    rowsperchunk = MIN(rowsperchunk, numrows - currow);
    workspace = ( JBLOCKROW ) alloc_large( cinfo, pool_id,(size_t) ( (size_t) rowsperchunk * (size_t) blocksperrow * (size_t)sizeof(short[DCTSIZE2]) ) );
    for (i = rowsperchunk; i > 0; i--) 
		{
      result[currow++] = workspace;
      workspace += blocksperrow;
    }
  }

  return result;
}

static void out_of_memory ( sJpeg_common_struct *cinfo, int which )
/* Report an out-of-memory error and stop execution */
/* If we compiled MEM_STATS support, report alloc requests before dying */
{
	#ifdef MEM_STATS
  cinfo->err->trace_level = 2;	/* force self_destruct to report stats */
	#endif
  ERREXIT1(cinfo, JERR_OUT_OF_MEMORY, which);
}

static void* alloc_large( sJpeg_common_struct *cinfo, int pool_id, size_t sizeofobject )
/* Allocate a "large" object */
{
  sMy_memory_mgr *mem = ( sMy_memory_mgr * ) cinfo->mem;
  uLarge_pool_struct *hdr_ptr;
  size_t odd_bytes;

  /* Check for unsatisfiable request (do now to ensure no overflow below) */
	if ( sizeofobject > (size_t) ( MAX_ALLOC_CHUNK - ( size_t )sizeof( uLarge_pool_struct * ) ) )
    out_of_memory(cinfo, 3);	/* request exceeds malloc's ability */

  /* Round up the requested size to a multiple of SIZEOF(ALIGN_TYPE) */
	odd_bytes = sizeofobject % (size_t)sizeof(ALIGN_TYPE);
  if (odd_bytes > 0)
    sizeofobject += (size_t)sizeof(ALIGN_TYPE) - odd_bytes;

  /* Always make a new pool */
  if (pool_id < 0 || pool_id >= JPOOL_NUMPOOLS)
    ERREXIT1(cinfo, JERR_BAD_POOL_ID, pool_id);	/* safety check */

	hdr_ptr = ( uLarge_pool_struct *) jpeg_get_large( cinfo, sizeofobject + (size_t)sizeof( uLarge_pool_struct ) );
  if (hdr_ptr == NULL)
    out_of_memory(cinfo, 4);	/* jpeg_get_large failed */
	mem->total_space_allocated += sizeofobject + (size_t)sizeof( uLarge_pool_struct );

  /* Success, initialize the new pool header and add to list */
  hdr_ptr->hdr.next = mem->large_list[pool_id];
  /* We maintain space counts in each pool header for statistical purposes,
   * even though they are not needed for allocation.
   */
  hdr_ptr->hdr.bytes_used = sizeofobject;
  hdr_ptr->hdr.bytes_left = 0;
  mem->large_list[pool_id] = hdr_ptr;

  return (void *) (hdr_ptr + 1); /* point to first data byte in pool */
}


static void* alloc_small ( sJpeg_common_struct *cinfo, int pool_id, size_t sizeofobject )
/* Allocate a "small" object */
{
  sMy_memory_mgr *mem = (sMy_memory_mgr *) cinfo->mem;
  uSmall_pool_struct *hdr_ptr;
	uSmall_pool_struct *prev_hdr_ptr;
  char * data_ptr;
  size_t odd_bytes, min_request, slop;

  /* Check for unsatisfiable request (do now to ensure no overflow below) */
	if (sizeofobject > (size_t) ( MAX_ALLOC_CHUNK - (size_t)sizeof( uSmall_pool_struct ) ) )
    out_of_memory(cinfo, 1);	/* request exceeds malloc's ability */

  /* Round up the requested size to a multiple of SIZEOF(ALIGN_TYPE) */
	odd_bytes = sizeofobject % (size_t)sizeof(ALIGN_TYPE);
  if (odd_bytes > 0)
		sizeofobject += (size_t)sizeof(ALIGN_TYPE) - odd_bytes;

  /* See if space is available in any existing pool */
  if (pool_id < 0 || pool_id >= JPOOL_NUMPOOLS)
    ERREXIT1(cinfo, JERR_BAD_POOL_ID, pool_id);	/* safety check */
  prev_hdr_ptr = NULL;
  hdr_ptr = mem->small_list[pool_id];
  while (hdr_ptr != NULL) {
    if (hdr_ptr->hdr.bytes_left >= sizeofobject)
      break;			/* found pool with enough space */
    prev_hdr_ptr = hdr_ptr;
    hdr_ptr = hdr_ptr->hdr.next;
  }

  /* Time to make a new pool? */
  if (hdr_ptr == NULL) 
	{
    /* min_request is what we need now, slop is what will be leftover */
		min_request = sizeofobject + (size_t)sizeof( uSmall_pool_struct );
    if (prev_hdr_ptr == NULL)	/* first pool in class? */
      slop = first_pool_slop[pool_id];
    else
      slop = extra_pool_slop[pool_id];
    /* Don't ask for more than MAX_ALLOC_CHUNK */
    if (slop > (size_t) (MAX_ALLOC_CHUNK-min_request))
      slop = (size_t) (MAX_ALLOC_CHUNK-min_request);
    /* Try to get space, if fail reduce slop and try again */
    for (;;) 
		{
      hdr_ptr = ( uSmall_pool_struct *) jpeg_get_small( cinfo, min_request + slop );
      if (hdr_ptr != NULL)
				break;
      slop /= 2;
      if (slop < MIN_SLOP)	    /* give up when it gets real small */
				out_of_memory(cinfo, 2);     /* jpeg_get_small failed */
    }
    mem->total_space_allocated += min_request + slop;
    /* Success, initialize the new pool header and add to end of list */
    hdr_ptr->hdr.next = NULL;
    hdr_ptr->hdr.bytes_used = 0;
    hdr_ptr->hdr.bytes_left = sizeofobject + slop;
    if (prev_hdr_ptr == NULL)	/* first pool in class? */
      mem->small_list[pool_id] = hdr_ptr;
    else
      prev_hdr_ptr->hdr.next = hdr_ptr;
  }

  /* OK, allocate the object from the current pool */
  data_ptr = (char *) (hdr_ptr + 1); /* point to first data byte in pool */
  data_ptr += hdr_ptr->hdr.bytes_used; /* point to place for object */
  hdr_ptr->hdr.bytes_used += sizeofobject;
  hdr_ptr->hdr.bytes_left -= sizeofobject;

  return (void *) data_ptr;
}


static unsigned char** alloc_sarray ( sJpeg_common_struct *cinfo, int pool_id, unsigned int samplesperrow, unsigned int numrows)
/* Allocate a 2-D sample array */
{
  sMy_memory_mgr *mem = (sMy_memory_mgr *) cinfo->mem;
  unsigned char **result;
  unsigned char *workspace;
  unsigned int rowsperchunk, currow, i;
  long ltemp;

  /* Calculate max # of rows allowed in one allocation chunk */
	ltemp = ( MAX_ALLOC_CHUNK - (size_t)sizeof( uLarge_pool_struct ) ) / ( ( long ) samplesperrow * (size_t)sizeof(unsigned char) );
  if (ltemp <= 0)
    ERREXIT(cinfo, JERR_WIDTH_OVERFLOW);
  if ( ltemp < (long) numrows )
    rowsperchunk = (unsigned int) ltemp;
  else
    rowsperchunk = numrows;
  mem->last_rowsperchunk = rowsperchunk;

  /* Get space for row pointers (small object) */
	result = ( unsigned char **)alloc_small( cinfo, pool_id, (size_t) ( numrows * (size_t)sizeof(unsigned char *) ) );

  /* Get the rows themselves (large objects) */
  currow = 0;
  while (currow < numrows) 
	{
    rowsperchunk = MIN(rowsperchunk, numrows - currow);
		workspace = ( unsigned char *) alloc_large( cinfo, pool_id, (size_t) ( (size_t) rowsperchunk * (size_t) samplesperrow * (size_t)sizeof(unsigned char)));
    for (i = rowsperchunk; i > 0; i--) 
		{
      result[currow++] = workspace;
      workspace += samplesperrow;
    }
  }

  return result;
}


/*
 * Release all objects belonging to a specified pool.
 */

static void free_pool ( sJpeg_common_struct *cinfo, int pool_id )
{
  sMy_memory_mgr *mem = (sMy_memory_mgr *) cinfo->mem;
  uSmall_pool_struct *shdr_ptr;
  uLarge_pool_struct *lhdr_ptr;
  size_t space_freed;

  if (pool_id < 0 || pool_id >= JPOOL_NUMPOOLS)
    ERREXIT1(cinfo, JERR_BAD_POOL_ID, pool_id);	/* safety check */

	#ifdef MEM_STATS
  if (cinfo->err->trace_level > 1)
    print_mem_stats(cinfo, pool_id); /* print pool's memory usage statistics */
	#endif

  /* If freeing IMAGE pool, close any virtual arrays first */
  if (pool_id == JPOOL_IMAGE) 
	{
    sJvirt_sarray_control *sptr;
    sJvirt_barray_control *bptr;

    for (sptr = mem->virt_sarray_list; sptr != NULL; sptr = sptr->next) 
		{
      if (sptr->b_s_open) 
			{	/* there may be no backing store */
				sptr->b_s_open = FALSE;	/* prevent recursive close if error */
				( *sptr->b_s_info.close_backing_store ) ( cinfo, & sptr->b_s_info );
      }
    }
    mem->virt_sarray_list = NULL;
    for (bptr = mem->virt_barray_list; bptr != NULL; bptr = bptr->next) 
		{
      if (bptr->b_s_open) 
			{	/* there may be no backing store */
				bptr->b_s_open = FALSE;	/* prevent recursive close if error */
				( *bptr->b_s_info.close_backing_store ) ( cinfo, & bptr->b_s_info );
      }
    }
    mem->virt_barray_list = NULL;
  }

  /* Release large objects */
  lhdr_ptr = mem->large_list[pool_id];
  mem->large_list[pool_id] = NULL;

  while (lhdr_ptr != NULL) 
	{
    uLarge_pool_struct *next_lhdr_ptr = lhdr_ptr->hdr.next;
		space_freed = lhdr_ptr->hdr.bytes_used + lhdr_ptr->hdr.bytes_left + (size_t)sizeof( uLarge_pool_struct );
    jpeg_free_large(cinfo, (void *) lhdr_ptr, space_freed);
    mem->total_space_allocated -= space_freed;
    lhdr_ptr = next_lhdr_ptr;
  }

  /* Release small objects */
  shdr_ptr = mem->small_list[pool_id];
  mem->small_list[pool_id] = NULL;

  while (shdr_ptr != NULL) 
	{
    uSmall_pool_struct *next_shdr_ptr = shdr_ptr->hdr.next;
		space_freed = shdr_ptr->hdr.bytes_used + shdr_ptr->hdr.bytes_left + (size_t)sizeof( uSmall_pool_struct );
    jpeg_free_small(cinfo, (void *) shdr_ptr, space_freed);
    mem->total_space_allocated -= space_freed;
    shdr_ptr = next_shdr_ptr;
  }
}


static sJvirt_sarray_control* request_virt_sarray ( sJpeg_common_struct *cinfo, int pool_id, boolean pre_zero, unsigned int samplesperrow, unsigned int numrows, unsigned int maxaccess )
/* Request a virtual 2-D sample array */
{
  sMy_memory_mgr *mem = (sMy_memory_mgr *) cinfo->mem;
  sJvirt_sarray_control *result;

  /* Only IMAGE-lifetime virtual arrays are currently supported */
  if (pool_id != JPOOL_IMAGE)
    ERREXIT1(cinfo, JERR_BAD_POOL_ID, pool_id);	/* safety check */

  /* get control block */
	result = ( sJvirt_sarray_control * ) alloc_small( cinfo, pool_id, (size_t)sizeof( sJvirt_sarray_control ) );

  result->mem_buffer = NULL;	/* marks array not yet realized */
  result->rows_in_array = numrows;
  result->samplesperrow = samplesperrow;
  result->maxaccess = maxaccess;
  result->pre_zero = pre_zero;
  result->b_s_open = FALSE;	/* no associated backing-store object */
  result->next = mem->virt_sarray_list; /* add to list of virtual arrays */
  mem->virt_sarray_list = result;

  return result;
}


static sJvirt_barray_control* request_virt_barray ( sJpeg_common_struct *cinfo, int pool_id, boolean pre_zero, unsigned int blocksperrow, unsigned int numrows, unsigned int maxaccess)
/* Request a virtual 2-D coefficient-block array */
{
  sMy_memory_mgr *mem = (sMy_memory_mgr *) cinfo->mem;
  sJvirt_barray_control *result;

  /* Only IMAGE-lifetime virtual arrays are currently supported */
  if (pool_id != JPOOL_IMAGE)
    ERREXIT1(cinfo, JERR_BAD_POOL_ID, pool_id);	/* safety check */

  /* get control block */
	result = ( sJvirt_barray_control *) alloc_small( cinfo, pool_id, (size_t)sizeof( sJvirt_barray_control ) );

  result->mem_buffer = NULL;	/* marks array not yet realized */
  result->rows_in_array = numrows;
  result->blocksperrow = blocksperrow;
  result->maxaccess = maxaccess;
  result->pre_zero = pre_zero;
  result->b_s_open = FALSE;	/* no associated backing-store object */
  result->next = mem->virt_barray_list; /* add to list of virtual arrays */
  mem->virt_barray_list = result;

  return result;
}

static void realize_virt_arrays ( sJpeg_common_struct *cinfo )
/* Allocate the in-memory buffers for any unrealized virtual arrays */
{
  sMy_memory_mgr *mem = (sMy_memory_mgr *) cinfo->mem;
  long space_per_minheight, maximum_space, avail_mem;
  long minheights, max_minheights;
  sJvirt_sarray_control *sptr;
  sJvirt_barray_control *bptr;

  /* Compute the minimum space needed (maxaccess rows in each buffer)
   * and the maximum space needed (full image height in each buffer).
   * These may be of use to the system-dependent jpeg_mem_available routine.
   */
  space_per_minheight = 0;
  maximum_space = 0;
  for (sptr = mem->virt_sarray_list; sptr != NULL; sptr = sptr->next) 
	{
    if (sptr->mem_buffer == NULL) 
		{ /* if not realized yet */
      space_per_minheight += (long) sptr->maxaccess *
			     (long) sptr->samplesperrow * (size_t)sizeof(unsigned char);
      maximum_space += (long) sptr->rows_in_array *
		       (long) sptr->samplesperrow * (size_t)sizeof(unsigned char);
    }
  }
  for (bptr = mem->virt_barray_list; bptr != NULL; bptr = bptr->next) {
    if (bptr->mem_buffer == NULL) { /* if not realized yet */
      space_per_minheight += (long) bptr->maxaccess *
			     (long) bptr->blocksperrow * (size_t)sizeof(short[DCTSIZE2]);
      maximum_space += (long) bptr->rows_in_array *
		       (long) bptr->blocksperrow * (size_t)sizeof(short[DCTSIZE2]);
    }
  }

  if (space_per_minheight <= 0)
    return;			/* no unrealized arrays, no work */

  /* Determine amount of memory to actually use; this is system-dependent. */
  avail_mem = jpeg_mem_available(cinfo, space_per_minheight, maximum_space,
				 mem->total_space_allocated);

  /* If the maximum space needed is available, make all the buffers full
   * height; otherwise parcel it out with the same number of minheights
   * in each buffer.
   */
  if (avail_mem >= maximum_space)
    max_minheights = 1000000000L;
  else {
    max_minheights = avail_mem / space_per_minheight;
    /* If there doesn't seem to be enough space, try to get the minimum
     * anyway.  This allows a "stub" implementation of jpeg_mem_available().
     */
    if (max_minheights <= 0)
      max_minheights = 1;
  }

  /* Allocate the in-memory buffers and initialize backing store as needed. */

  for (sptr = mem->virt_sarray_list; sptr != NULL; sptr = sptr->next) 
	{
    if (sptr->mem_buffer == NULL) 
		{ /* if not realized yet */
      minheights = ((long) sptr->rows_in_array - 1L) / sptr->maxaccess + 1L;
      if (minheights <= max_minheights) 
			{
				/* This buffer fits in memory */
				sptr->rows_in_mem = sptr->rows_in_array;
      } 
			else 
			{
				/* It doesn't fit in memory, create backing store. */
				sptr->rows_in_mem = (unsigned int) (max_minheights * sptr->maxaccess);
				jpeg_open_backing_store( cinfo, & sptr->b_s_info,(long) sptr->rows_in_array * (long) sptr->samplesperrow *	(long) (size_t)sizeof(unsigned char) );
				sptr->b_s_open = TRUE;
      }
      sptr->mem_buffer = alloc_sarray(cinfo, JPOOL_IMAGE,
				      sptr->samplesperrow, sptr->rows_in_mem);
      sptr->rowsperchunk = mem->last_rowsperchunk;
      sptr->cur_start_row = 0;
      sptr->first_undef_row = 0;
      sptr->dirty = FALSE;
    }
  }

  for (bptr = mem->virt_barray_list; bptr != NULL; bptr = bptr->next) 
	{
    if (bptr->mem_buffer == NULL) 
		{ /* if not realized yet */
      minheights = ((long) bptr->rows_in_array - 1L) / bptr->maxaccess + 1L;
      if (minheights <= max_minheights) 
			{
				/* This buffer fits in memory */
				bptr->rows_in_mem = bptr->rows_in_array;
      } 
			else 
			{
				/* It doesn't fit in memory, create backing store. */
				bptr->rows_in_mem = (unsigned int) (max_minheights * bptr->maxaccess);
				jpeg_open_backing_store( cinfo, & bptr->b_s_info, (long) bptr->rows_in_array * (long) bptr->blocksperrow * (long) (size_t)sizeof(short[DCTSIZE2]) );
				bptr->b_s_open = TRUE;
      }
      bptr->mem_buffer = alloc_barray( cinfo, JPOOL_IMAGE, bptr->blocksperrow, bptr->rows_in_mem );
      bptr->rowsperchunk = mem->last_rowsperchunk;
      bptr->cur_start_row = 0;
      bptr->first_undef_row = 0;
      bptr->dirty = FALSE;
    }
  }
}


static void self_destruct ( sJpeg_common_struct *cinfo)
{
  int pool;

  /* Close all backing store, release all memory.
   * Releasing pools in reverse order might help avoid fragmentation
   * with some (brain-damaged) malloc libraries.
   */
  for (pool = JPOOL_NUMPOOLS-1; pool >= JPOOL_PERMANENT; pool--) 
	{
    free_pool(cinfo, pool);
  }

  /* Release the memory manager control block too. */
	jpeg_free_small( cinfo, (void *) cinfo->mem, (size_t)sizeof( sMy_memory_mgr ) );
  cinfo->mem = NULL;		/* ensures I will be called only once */

  jpeg_mem_term(cinfo);		/* system-dependent cleanup */
}


void jinit_memory_mgr ( sJpeg_common_struct * cinfo)
{
  sMy_memory_mgr *mem;
  long max_to_use;
  int pool;
  size_t test_mac;

  cinfo->mem = NULL;		// for safety if init fails

  // Check for configuration errors.
  // (size_t)sizeof(ALIGN_TYPE) should be a power of 2; otherwise, it probably
  // doesn't reflect any real hardware alignment requirement.
  // The test is a little tricky: for X>0, X and X-1 have no one-bits
  // in common if and only if X is a power of 2, ie has only one one-bit.
  // Some compilers may give an "unreachable code" warning here; ignore it.
  ///
  if ( ( (size_t)sizeof(ALIGN_TYPE) & ( (size_t)sizeof(ALIGN_TYPE) - 1 ) ) != 0 )
    ERREXIT( cinfo, JERR_BAD_ALIGN_TYPE );
  // MAX_ALLOC_CHUNK must be representable as type size_t, and must be
  // a multiple of (size_t)sizeof(ALIGN_TYPE).
  // Again, an "unreachable code" warning may be ignored here.
  // But a "constant too large" warning means you need to fix MAX_ALLOC_CHUNK.
  ///
  test_mac = (size_t) MAX_ALLOC_CHUNK;
  if ( (long) test_mac != MAX_ALLOC_CHUNK || ( MAX_ALLOC_CHUNK % (size_t)sizeof( ALIGN_TYPE ) ) != 0 )
    ERREXIT( cinfo, JERR_BAD_ALLOC_CHUNK );

  max_to_use = jpeg_mem_init(cinfo); // system-dependent initialization 

  // Attempt to allocate memory manager's control block 
  mem = ( sMy_memory_mgr *) jpeg_get_small(cinfo, (size_t)sizeof(sMy_memory_mgr));

  if (mem == NULL) 
	{
    jpeg_mem_term(cinfo);	// system-dependent cleanup 
    ERREXIT1(cinfo, JERR_OUT_OF_MEMORY, 0);
  }

  // OK, fill in the method pointers 
  mem->pub.alloc_small = alloc_small;
  mem->pub.alloc_large = alloc_large;
  mem->pub.alloc_sarray = alloc_sarray;
  mem->pub.alloc_barray = alloc_barray;
  mem->pub.request_virt_sarray = request_virt_sarray;
  mem->pub.request_virt_barray = request_virt_barray;
  mem->pub.realize_virt_arrays = realize_virt_arrays;
  mem->pub.access_virt_sarray = access_virt_sarray;
  mem->pub.access_virt_barray = access_virt_barray;
  mem->pub.free_pool = free_pool;
  mem->pub.self_destruct = self_destruct;

  // Make MAX_ALLOC_CHUNK accessible to other modules 
  mem->pub.max_alloc_chunk = MAX_ALLOC_CHUNK;

  // Initialize working state 
  mem->pub.max_memory_to_use = max_to_use;

  for (pool = JPOOL_NUMPOOLS-1; pool >= JPOOL_PERMANENT; pool--) {
    mem->small_list[pool] = NULL;
    mem->large_list[pool] = NULL;
  }
  mem->virt_sarray_list = NULL;
  mem->virt_barray_list = NULL;

  mem->total_space_allocated = (size_t)sizeof( sMy_memory_mgr );

  // Declare ourselves open for business
  cinfo->mem = & mem->pub;

  // Check for an environment variable JPEGMEM; if found, override the
  // default max_memory setting from jpeg_mem_init.  Note that the
  // surrounding application may again override this value.
  // If your system doesn't support getenv(), define NO_GETENV to disable
  // this feature.
  ///
#ifndef NO_GETENV
  { 
		char * memenv;
    if ((memenv = getenv("JPEGMEM")) != NULL) 
		{
      char ch = 'x';

      if (sscanf(memenv, "%ld%c", &max_to_use, &ch) > 0) 
			{
				if (ch == 'm' || ch == 'M')
					max_to_use *= 1000L;
					mem->pub.max_memory_to_use = max_to_use * 1000L;
      }
    }
  }
#endif
}
