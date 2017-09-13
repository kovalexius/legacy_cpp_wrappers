#include "jcomapi.h"


void jpeg_abort ( sJpeg_common_struct *cinfo)
{
	int pool;

  /* Do nothing if called on a not-initialized or destroyed JPEG object. */
  if (cinfo->mem == NULL)
    return;

  /* Releasing pools in reverse order might help avoid fragmentation
   * with some (brain-damaged) malloc libraries.
   */
  for (pool = JPOOL_NUMPOOLS-1; pool > JPOOL_PERMANENT; pool--) {
    (*cinfo->mem->free_pool) (cinfo, pool);
  }

  /* Reset overall state for possible reuse of object */
  if (cinfo->is_decompressor) {
    cinfo->global_state = DSTATE_START;
    /* Try to keep application from accessing now-deleted marker list.
     * A bit kludgy to do it here, but this is the most central place.
     */
    (( sJpeg_decompress_struct *) cinfo)->marker_list = NULL;
  } else {
    cinfo->global_state = CSTATE_START;
  }
}

void jpeg_destroy ( sJpeg_common_struct *cinfo)
{
	/* We need only tell the memory manager to release everything. */
  /* NB: mem pointer is NULL if memory mgr failed to initialize. */
  if (cinfo->mem != NULL)
    (*cinfo->mem->self_destruct) (cinfo);
  cinfo->mem = NULL;		/* be safe if jpeg_destroy is called twice */
  cinfo->global_state = 0;	/* mark it destroyed */
}

JQUANT_TBL * jpeg_alloc_quant_table ( sJpeg_common_struct *cinfo)
{
  JQUANT_TBL *tbl;

  tbl = (JQUANT_TBL *)
    (*cinfo->mem->alloc_small) (cinfo, JPOOL_PERMANENT, (size_t)sizeof(JQUANT_TBL));
  tbl->sent_table = FALSE;	/* make sure this is false in any new table */
  return tbl;
}


JHUFF_TBL * jpeg_alloc_huff_table ( sJpeg_common_struct *cinfo)
{
  JHUFF_TBL *tbl;

  tbl = (JHUFF_TBL *)
    (*cinfo->mem->alloc_small) (cinfo, JPOOL_PERMANENT, (size_t)sizeof(JHUFF_TBL));
  tbl->sent_table = FALSE;	/* make sure this is false in any new table */
  return tbl;
}