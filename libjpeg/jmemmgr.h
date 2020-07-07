#ifndef JMEMMGR_H
#define JMEMMGR_H

#include "LibJpeg.h"
#include "jmemsys.h"

/*
 * Some important notes:
 *   The allocation routines provided here must never return NULL.
 *   They should exit to error_exit if unsuccessful.
 *
 *   It's not a good idea to try to merge the sarray and barray routines,
 *   even though they are textually almost the same, because samples are
 *   usually stored as bytes while coefficients are shorts or ints.  Thus,
 *   in machines where byte pointers have a different representation from
 *   word pointers, the resulting machine code could not be the same.
 */


/*
 * Many machines require storage alignment: longs must start on 4-byte
 * boundaries, doubles on 8-byte boundaries, etc.  On such machines, malloc()
 * always returns pointers that are multiples of the worst-case alignment
 * requirement, and we had better do so too.
 * There isn't any really portable way to determine the worst-case alignment
 * requirement.  This module assumes that the alignment requirement is
 * multiples of sizeof(ALIGN_TYPE).
 * By default, we define ALIGN_TYPE as double.  This is necessary on some
 * workstations (where doubles really do need 8-byte alignment) and will work
 * fine on nearly everything.  If your machine has lesser alignment needs,
 * you can save a few bytes by making ALIGN_TYPE smaller.
 * The only place I know of where this will NOT work is certain Macintosh
 * 680x0 compilers that define double as a 10-byte IEEE extended float.
 * Doing 10-byte alignment is counterproductive because longwords won't be
 * aligned well.  Put "#define ALIGN_TYPE long" in jconfig.h if you have
 * such a compiler.
 */

#ifndef ALIGN_TYPE		/* so can override from jconfig.h */
#define ALIGN_TYPE  double
#endif


/*
 * Allocation of "small" objects.
 *
 * For these, we use pooled storage.  When a new pool must be created,
 * we try to get enough space for the current request plus a "slop" factor,
 * where the slop will be the amount of leftover space in the new pool.
 * The speed vs. space tradeoff is largely determined by the slop values.
 * A different slop value is provided for each pool class (lifetime),
 * and we also distinguish the first pool of a class from later ones.
 * NOTE: the values given work fairly well on both 16- and 32-bit-int
 * machines, but may be too small if longs are 64 bits or more.
 */

static const size_t first_pool_slop[JPOOL_NUMPOOLS] = 
{
	1600,			/* first PERMANENT pool */
	16000			/* first IMAGE pool */
};

static const size_t extra_pool_slop[JPOOL_NUMPOOLS] = 
{
	0,			/* additional PERMANENT pools */
	5000			/* additional IMAGE pools */
};

#define MIN_SLOP  50		/* greater than 0 to avoid futile looping */



union uSmall_pool_struct 
{
  struct 
  {
    uSmall_pool_struct *next;	// next in list of pools
    size_t bytes_used;				// how many bytes already used within pool
    size_t bytes_left;				// bytes still available in this pool
  } hdr;
  ALIGN_TYPE dummy;						// included in union to ensure alignment
};

union uLarge_pool_struct 
{
  struct
  {
    uLarge_pool_struct *next;	// next in list of pools
    size_t bytes_used;				// how many bytes already used within pool
    size_t bytes_left;				// bytes still available in this pool
  } hdr;
  ALIGN_TYPE dummy;		// included in union to ensure alignment
};

/*
 * The control blocks for virtual arrays.
 * Note that these blocks are allocated in the "small" pool area.
 * System-dependent info for the associated backing store (if any) is hidden
 * inside the backing_store_info struct.
 */
struct sJvirt_sarray_control 
{
  JSAMPARRAY mem_buffer;	/* => the in-memory buffer */
  unsigned int rows_in_array;	/* total virtual array height */
  unsigned int samplesperrow;	/* width of array (and of memory buffer) */
  unsigned int maxaccess;		/* max rows accessed by access_virt_sarray */
  unsigned int rows_in_mem;	/* height of memory buffer */
  unsigned int rowsperchunk;	/* allocation chunk size in mem_buffer */
  unsigned int cur_start_row;	/* first logical row # in the buffer */
  unsigned int first_undef_row;	/* row # of first uninitialized row */
  boolean pre_zero;		/* pre-zero mode requested? */
  boolean dirty;		/* do current buffer contents need written? */
  boolean b_s_open;		/* is backing-store data valid? */
  sJvirt_sarray_control *next;	/* link to next virtual sarray control block */
  sBacking_store_struct b_s_info;	/* System-dependent control info */
};

struct sJvirt_barray_control 
{
  JBLOCKARRAY mem_buffer;	/* => the in-memory buffer */
  unsigned int rows_in_array;	/* total virtual array height */
  unsigned int blocksperrow;	/* width of array (and of memory buffer) */
  unsigned int maxaccess;		/* max rows accessed by access_virt_barray */
  unsigned int rows_in_mem;	/* height of memory buffer */
  unsigned int rowsperchunk;	/* allocation chunk size in mem_buffer */
  unsigned int cur_start_row;	/* first logical row # in the buffer */
  unsigned int first_undef_row;	/* row # of first uninitialized row */
  boolean pre_zero;		/* pre-zero mode requested? */
  boolean dirty;		/* do current buffer contents need written? */
  boolean b_s_open;		/* is backing-store data valid? */
  sJvirt_barray_control *next;	/* link to next virtual barray control block */
  sBacking_store_struct b_s_info;	/* System-dependent control info */
};


struct sMy_memory_mgr 
{
  struct sJpeg_memory_mgr pub;	/* public fields */

  /* Each pool identifier (lifetime class) names a linked list of pools. */
  uSmall_pool_struct *small_list[JPOOL_NUMPOOLS];
  uLarge_pool_struct *large_list[JPOOL_NUMPOOLS];

  /* Since we only have one lifetime class of virtual arrays, only one
   * linked list is necessary (for each datatype).  Note that the virtual
   * array control blocks being linked together are actually stored somewhere
   * in the small-pool list.
   */
  sJvirt_sarray_control *virt_sarray_list;
  sJvirt_barray_control *virt_barray_list;

  /* This counts total space obtained from jpeg_get_small/large */
  long total_space_allocated;

  /* alloc_sarray and alloc_barray set this value for use by virtual
   * array routines.
   */
  unsigned int last_rowsperchunk;	/* from most recent alloc_sarray/barray */
};

static void* alloc_small ( sJpeg_common_struct *cinfo, int pool_id, size_t sizeofobject );
static void* alloc_large( sJpeg_common_struct *cinfo, int pool_id, size_t sizeofobject );
static void do_barray_io ( sJpeg_common_struct *cinfo, sJvirt_barray_control *ptr, boolean writing);
void jinit_memory_mgr ( sJpeg_common_struct * cinfo);

/*
 * Creation of 2-D coefficient-block arrays.
 * This is essentially the same as the code for sample arrays, above.
 */
static JBLOCKARRAY alloc_barray ( sJpeg_common_struct *cinfo, int pool_id, unsigned int blocksperrow, unsigned int numrows );
/* Allocate a 2-D coefficient-block array */

static JBLOCKARRAY access_virt_barray ( sJpeg_common_struct *cinfo, sJvirt_barray_control *ptr, unsigned int start_row, unsigned int num_rows, boolean writable);
/* Access the part of a virtual block array starting at start_row */
/* and extending for num_rows rows.  writable is true if  */
/* caller intends to modify the accessed area. */

#endif