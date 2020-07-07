#ifndef CLIBJPEG_H
#define CLIBJPEG_H

#include <stdio.h>
#include "jconfig.h"
#include "jmorecfg.h"
#include "jerror.h"
#include "jpegint.h"
#include "jcmaster.h"

#define JPEG_LIB_VERSION        90	/* Compatibility version 9.0 */

#define DCTSIZE		    8	/* The basic DCT block is 8x8 coefficients */
#define DCTSIZE2	    64	/* DCTSIZE squared; # of elements in a block */
#define NUM_QUANT_TBLS      4	/* Quantization tables are numbered 0..3 */
#define NUM_HUFF_TBLS       4	/* Huffman tables are numbered 0..3 */
#define NUM_ARITH_TBLS      16	/* Arith-coding tables are numbered 0..15 */
#define MAX_COMPS_IN_SCAN   4	/* JPEG limit on # of components in one scan */
#define MAX_SAMP_FACTOR     4	/* JPEG limit on sampling factors */
/* Unfortunately, some bozo at Adobe saw no reason to be bound by the standard;
 * the PostScript DCT filter can emit files with many more than 10 blocks/MCU.
 * If you happen to run across such a file, you can up D_MAX_BLOCKS_IN_MCU
 * to handle it.  We even let you do this from the jconfig.h file.  However,
 * we strongly discourage changing C_MAX_BLOCKS_IN_MCU; just because Adobe
 * sometimes emits noncompliant files doesn't mean you should too.
 */
#define C_MAX_BLOCKS_IN_MCU   10 /* compressor's limit on blocks per MCU */
#ifndef D_MAX_BLOCKS_IN_MCU
#define D_MAX_BLOCKS_IN_MCU   10 /* decompressor's limit on blocks per MCU */
#endif


/* Data structures for images (arrays of samples and of DCT coefficients).
 * On 80x86 machines, the image arrays are too big for near pointers,
 * but the pointer arrays can fit in near memory.
 */
typedef JSAMPLE FAR *JSAMPROW;	/* ptr to one image row of pixel samples. */
typedef JSAMPROW *JSAMPARRAY;	/* ptr to some rows (a 2-D sample array) */
typedef JSAMPARRAY *JSAMPIMAGE;	/* a 3-D sample array: top index is color */

typedef JCOEF JBLOCK[DCTSIZE2];	/* one block of coefficients */
typedef JBLOCK FAR *JBLOCKROW;	/* pointer to one row of coefficient blocks */
typedef JBLOCKROW *JBLOCKARRAY;		/* a 2-D array of coefficient blocks */
typedef JBLOCKARRAY *JBLOCKIMAGE;	/* a 3-D array of coefficient blocks */

typedef JCOEF FAR *JCOEFPTR;	/* useful in a couple of places */



/* Types for JPEG compression parameters and working tables. */
/* DCT coefficient quantization tables. */
struct JQUANT_TBL
{
  /* This array gives the coefficient quantizers in natural array order
   * (not the zigzag order in which they are stored in a JPEG DQT marker).
   * CAUTION: IJG versions prior to v6a kept this array in zigzag order.
   */
  UINT16 quantval[DCTSIZE2];	/* quantization step for each coefficient */
  /* This field is used only during compression.  It's initialized FALSE when
   * the table is created, and set TRUE when it's been output to the file.
   * You could suppress output of a table by setting this to TRUE.
   * (See jpeg_suppress_tables for an example.)
   */
  boolean sent_table;		/* TRUE when table has been output */
};


/* Huffman coding tables. */
struct JHUFF_TBL
{
  /* These two fields directly represent the contents of a JPEG DHT marker */
  UINT8 bits[17];		/* bits[k] = # of symbols with codes of */
				/* length k bits; bits[0] is unused */
  UINT8 huffval[256];		/* The symbols, in order of incr code length */
  /* This field is used only during compression.  It's initialized FALSE when
   * the table is created, and set TRUE when it's been output to the file.
   * You could suppress output of a table by setting this to TRUE.
   * (See jpeg_suppress_tables for an example.)
   */
  boolean sent_table;		/* TRUE when table has been output */
};


typedef enum 
{
	JCS_UNKNOWN,		/* error/unspecified */
	JCS_GRAYSCALE,		/* monochrome */
	JCS_RGB,		/* red/green/blue */
	JCS_YCbCr,		/* Y/Cb/Cr (also known as YUV) */
	JCS_CMYK,		/* C/M/Y/K */
	JCS_YCCK		/* Y/Cb/Cr/K */
} J_COLOR_SPACE;

typedef enum 
{
	JDCT_ISLOW,		/* slow but accurate integer algorithm */
	JDCT_IFAST,		/* faster, less accurate integer method */
	JDCT_FLOAT		/* floating-point: accurate, fast on fast HW */
} J_DCT_METHOD;

typedef enum 
{
	JCT_NONE           = 0,
	JCT_SUBTRACT_GREEN = 1
} J_COLOR_TRANSFORM;


#define JPOOL_PERMANENT	0	/* lasts until master record is destroyed */
#define JPOOL_IMAGE	1	/* lasts until done with image/datastream */
#define JPOOL_NUMPOOLS	2

struct sJpeg_common_struct;
struct sJvirt_sarray_control;
struct sJvirt_barray_control;
struct sJpeg_memory_mgr 
{
	void* ( *alloc_small )( sJpeg_common_struct *cinfo, int pool_id, size_t sizeofobject );
	void* ( *alloc_large )( sJpeg_common_struct *cinfo, int pool_id, size_t sizeofobject );
	unsigned char **( *alloc_sarray )( sJpeg_common_struct *cinfo, int pool_id, unsigned int samplesperrow, unsigned int numrows );
  short ***( *alloc_barray )( sJpeg_common_struct *cinfo, int pool_id, unsigned int blocksperrow, unsigned int numrows );
  sJvirt_sarray_control* ( *request_virt_sarray)( sJpeg_common_struct * cinfo, int pool_id, boolean pre_zero, unsigned int samplesperrow, unsigned int numrows, unsigned int maxaccess );
  sJvirt_barray_control* ( *request_virt_barray)( sJpeg_common_struct * cinfo, int pool_id, boolean pre_zero, unsigned int blocksperrow, unsigned int numrows, unsigned int maxaccess );
  void ( *realize_virt_arrays ) ( sJpeg_common_struct * cinfo );
  unsigned char ** ( *access_virt_sarray ) ( sJpeg_common_struct * cinfo, sJvirt_sarray_control *ptr, unsigned int start_row, unsigned int num_rows, boolean writable );
  short*** ( *access_virt_barray ) ( sJpeg_common_struct * cinfo, sJvirt_barray_control *ptr, unsigned int start_row, unsigned int num_rows, boolean writable );
  void ( *free_pool ) ( sJpeg_common_struct * cinfo, int pool_id );
  void ( *self_destruct) (sJpeg_common_struct * cinfo );
  /* Limit on memory allocation for this JPEG object.  (Note that this is
   * merely advisory, not a guaranteed maximum; it only affects the space
   * used for virtual-array buffers.)  May be changed by outer application
   * after creating the JPEG object.
   */
  long max_memory_to_use;
  /* Maximum allocation request accepted by alloc_large. */
  long max_alloc_chunk;
};


struct sJpeg_component_info
{
  /* These values are fixed over the whole image. */
  /* For compression, they must be supplied by parameter setup; */
  /* for decompression, they are read from the SOF marker. */
  int component_id;		/* identifier for this component (0..255) */
  int component_index;		/* its index in SOF or cinfo->comp_info[] */
  int h_samp_factor;		/* horizontal sampling factor (1..4) */
  int v_samp_factor;		/* vertical sampling factor (1..4) */
  int quant_tbl_no;		/* quantization table selector (0..3) */
  /* These values may vary between scans. */
  /* For compression, they must be supplied by parameter setup; */
  /* for decompression, they are read from the SOS marker. */
  /* The decompressor output side may not use these variables. */
  int dc_tbl_no;		/* DC entropy table selector (0..3) */
  int ac_tbl_no;		/* AC entropy table selector (0..3) */
  
  /* Remaining fields should be treated as private by applications. */
  
  /* These values are computed during compression or decompression startup: */
  /* Component's size in DCT blocks.
   * Any dummy blocks added to complete an MCU are not counted; therefore
   * these values do not depend on whether a scan is interleaved or not.
   */
  unsigned int width_in_blocks;
  unsigned int height_in_blocks;
  /* Size of a DCT block in samples,
   * reflecting any scaling we choose to apply during the DCT step.
   * Values from 1 to 16 are supported.
   * Note that different components may receive different DCT scalings.
   */
  int DCT_h_scaled_size;
  int DCT_v_scaled_size;
  /* The downsampled dimensions are the component's actual, unpadded number
   * of samples at the main buffer (preprocessing/compression interface);
   * DCT scaling is included, so
   * downsampled_width = ceil(image_width * Hi/Hmax * DCT_h_scaled_size/DCTSIZE)
   * and similarly for height.
   */
  unsigned int downsampled_width;	 /* actual width in samples */
  unsigned int downsampled_height; /* actual height in samples */
  // This flag is used only for decompression.  In cases where some of the
  // components will be ignored (eg grayscale output from YCbCr image),
  // we can skip most computations for the unused components.
  boolean component_needed;	/* do we need the value of this component? */

  // These values are computed before starting a scan of the component. */
  // The decompressor output side may not use these variables. */
  int MCU_width;		/* number of blocks per MCU, horizontally */
  int MCU_height;		/* number of blocks per MCU, vertically */
  int MCU_blocks;		/* MCU_width * MCU_height */
  int MCU_sample_width;	/* MCU width in samples: MCU_width * DCT_h_scaled_size */
  int last_col_width;		/* # of non-dummy blocks across in last MCU */
  int last_row_height;		/* # of non-dummy blocks down in last MCU */

  // Saved quantization table for component; NULL if none yet saved.
  // See jdinput.c comments about the need for this information.
  // This field is currently used only for decompression.
  JQUANT_TBL * quant_table;

  // Private per-component storage for DCT or IDCT subsystem.
  void * dct_table;
};

struct sJpeg_scan_info
{
  int comps_in_scan;		/* number of components encoded in this scan */
  int component_index[MAX_COMPS_IN_SCAN]; /* their SOF/comp_info[] indexes */
  int Ss, Se;			/* progressive JPEG spectral selection parms */
  int Ah, Al;			/* progressive JPEG successive approx. parms */
};


/* Progress monitor object */
struct sJpeg_common_struct;
struct sJpeg_progress_mgr 
{
  void ( *progress_monitor ) ( sJpeg_common_struct * cinfo );

  long pass_counter;		/* work units completed in this pass */
  long pass_limit;		/* total number of work units in this pass */
  int completed_passes;		/* passes completed so far */
  int total_passes;		/* total number of passes expected */
};

/* Routines that are to be used by both halves of the library are declared
 * to receive a pointer to this structure.  There are no actual instances of
 * jpeg_common_struct, only of jpeg_compress_struct and jpeg_decompress_struct.
 */

struct sJpeg_error_mgr;
struct sJpeg_memory_mgr;
struct sJpeg_common_struct 
{
  sJpeg_error_mgr * err;	/* Error handler module */\
  sJpeg_memory_mgr * mem;	/* Memory manager module */\
  sJpeg_progress_mgr * progress; /* Progress monitor, or NULL if none */\
  void * client_data;		/* Available for use by application */\
  boolean is_decompressor;	/* So common code can tell which is which */\
  int global_state;		/* For checking call sequence validity */
  /* Additional fields follow in an actual jpeg_compress_struct or
   * jpeg_decompress_struct.  All three structs must agree on these
   * initial fields!  (This would be a lot cleaner in C++.)
   */
};

/* Error handler object */
struct sJpeg_error_mgr
{
  /* Error exit handler: does not return to caller */
  void ( *error_exit ) ( sJpeg_common_struct *cinfo);
  /* Conditionally emit a trace or warning message */
  void ( *emit_message ) ( sJpeg_common_struct *cinfo, int msg_level);
  /* Routine that actually outputs a trace or error message */
  void ( *output_message )( sJpeg_common_struct *cinfo );
  /* Format a message string for the most recent JPEG error or message */
  void ( *format_message )( sJpeg_common_struct *cinfo, char * buffer );
	#define JMSG_LENGTH_MAX  200	/* recommended size of format_message buffer */
  /* Reset error state variables at start of a new image */
  void ( *reset_error_mgr ) ( sJpeg_common_struct *cinfo );
  
  /* The message ID code and any parameters are saved here.
   * A message can have one string parameter or up to 8 int parameters.
   */
  int msg_code;
#define JMSG_STR_PARM_MAX  80
  union 
  {
    int i[8];
    char s[JMSG_STR_PARM_MAX];
  } msg_parm;
  
  /* Standard state variables for error facility */
  
  int trace_level;		/* max msg_level that will be displayed */
  
  /* For recoverable corrupt-data errors, we emit a warning message,
   * but keep going unless emit_message chooses to abort.  emit_message
   * should count warnings in num_warnings.  The surrounding application
   * can check for bad data by seeing if num_warnings is nonzero at the
   * end of processing.
   */
  long num_warnings;		/* number of corrupt-data warnings */

  /* These fields point to the table(s) of error message strings.
   * An application can change the table pointer to switch to a different
   * message list (typically, to change the language in which errors are
   * reported).  Some applications may wish to add additional error codes
   * that will be handled by the JPEG library error mechanism; the second
   * table pointer is used for this purpose.
   *
   * First table includes all errors generated by JPEG library itself.
   * Error code 0 is reserved for a "no such error string" message.
   */
  const char * const * jpeg_message_table; /* Library errors */
  int last_jpeg_message;    /* Table contains strings 0..last_jpeg_message */
  /* Second table can be added by application (see cjpeg/djpeg for example).
   * It contains strings numbered first_addon_message..last_addon_message.
   */
  const char * const * addon_message_table; /* Non-library errors */
  int first_addon_message;	/* code for first string in addon table */
  int last_addon_message;	/* code for last string in addon table */
};


struct sJpeg_destination_mgr;
/* Master record for a compression instance */
struct sJpeg_compress_struct 
{
  sJpeg_error_mgr * err;	/* Error handler module */\
  sJpeg_memory_mgr * mem;	/* Memory manager module */\
  sJpeg_progress_mgr * progress; /* Progress monitor, or NULL if none */\
  void * client_data;		/* Available for use by application */\
  boolean is_decompressor;	/* So common code can tell which is which */\
  int global_state	;	/* For checking call sequence validity */

  /* Destination for compressed data */
  sJpeg_destination_mgr * dest;

  /* Description of source image --- these fields must be filled in by
   * outer application before starting compression.  in_color_space must
   * be correct before you can even call jpeg_set_defaults().
   */

  unsigned int image_width;			/* input image width */
  unsigned int image_height;		/* input image height */
  int input_components;		/* # of color components in input image */
  J_COLOR_SPACE in_color_space;	/* colorspace of input image */

  double input_gamma;		/* image gamma of input image */

  /* Compression parameters --- these fields must be set before calling
   * jpeg_start_compress().  We recommend calling jpeg_set_defaults() to
   * initialize everything to reasonable defaults, then changing anything
   * the application specifically wants to change.  That way you won't get
   * burnt when new parameters are added.  Also note that there are several
   * helper routines to simplify changing parameters.
   */

  unsigned int scale_num, scale_denom; /* fraction by which to scale image */

  unsigned int jpeg_width;	/* scaled JPEG image width */
  unsigned int jpeg_height;	/* scaled JPEG image height */
  /* Dimensions of actual JPEG image that will be written to file,
   * derived from input dimensions by scaling factors above.
   * These fields are computed by jpeg_start_compress().
   * You can also use jpeg_calc_jpeg_dimensions() to determine these values
   * in advance of calling jpeg_start_compress().
   */

  int data_precision;		/* bits of precision in image data */

  int num_components;		/* # of color components in JPEG image */
  J_COLOR_SPACE jpeg_color_space; /* colorspace of JPEG image */

  sJpeg_component_info * comp_info;
  /* comp_info[i] describes component that appears i'th in SOF */

  JQUANT_TBL * quant_tbl_ptrs[NUM_QUANT_TBLS];
  int q_scale_factor[NUM_QUANT_TBLS];
  /* ptrs to coefficient quantization tables, or NULL if not defined,
   * and corresponding scale factors (percentage, initialized 100).
   */

  JHUFF_TBL * dc_huff_tbl_ptrs[NUM_HUFF_TBLS];
  JHUFF_TBL * ac_huff_tbl_ptrs[NUM_HUFF_TBLS];
  /* ptrs to Huffman coding tables, or NULL if not defined */

  UINT8 arith_dc_L[NUM_ARITH_TBLS]; /* L values for DC arith-coding tables */
  UINT8 arith_dc_U[NUM_ARITH_TBLS]; /* U values for DC arith-coding tables */
  UINT8 arith_ac_K[NUM_ARITH_TBLS]; /* Kx values for AC arith-coding tables */

  int num_scans;		/* # of entries in scan_info array */
  const sJpeg_scan_info * scan_info; /* script for multi-scan file, or NULL */
  /* The default value of scan_info is NULL, which causes a single-scan
   * sequential JPEG file to be emitted.  To create a multi-scan file,
   * set num_scans and scan_info to point to an array of scan definitions.
   */

  boolean raw_data_in;		/* TRUE=caller supplies downsampled data */
  boolean arith_code;		/* TRUE=arithmetic coding, FALSE=Huffman */
  boolean optimize_coding;	/* TRUE=optimize entropy encoding parms */
  boolean CCIR601_sampling;	/* TRUE=first samples are cosited */
  boolean do_fancy_downsampling; /* TRUE=apply fancy downsampling */
  int smoothing_factor;		/* 1..100, or 0 for no input smoothing */
  J_DCT_METHOD dct_method;	/* DCT algorithm selector */

  /* The restart interval can be specified in absolute MCUs by setting
   * restart_interval, or in MCU rows by setting restart_in_rows
   * (in which case the correct restart_interval will be figured
   * for each scan).
   */
  unsigned int restart_interval; /* MCUs per restart, or 0 for no restart */
  int restart_in_rows;		/* if > 0, MCU rows per restart interval */

  /* Parameters controlling emission of special markers. */

  boolean write_JFIF_header;	/* should a JFIF marker be written? */
  UINT8 JFIF_major_version;	/* What to write for the JFIF version number */
  UINT8 JFIF_minor_version;
  /* These three values are not used by the JPEG code, merely copied */
  /* into the JFIF APP0 marker.  density_unit can be 0 for unknown, */
  /* 1 for dots/inch, or 2 for dots/cm.  Note that the pixel aspect */
  /* ratio is defined by X_density/Y_density even when density_unit=0. */
  UINT8 density_unit;		/* JFIF code for pixel size units */
  UINT16 X_density;		/* Horizontal pixel density */
  UINT16 Y_density;		/* Vertical pixel density */
  boolean write_Adobe_marker;	/* should an Adobe marker be written? */

  J_COLOR_TRANSFORM color_transform;
  /* Color transform identifier, writes LSE marker if nonzero */

  /* State variable: index of next scanline to be written to
   * jpeg_write_scanlines().  Application may use this to control its
   * processing loop, e.g., "while (next_scanline < image_height)".
   */

  unsigned int next_scanline;	/* 0 .. image_height-1  */

  /* Remaining fields are known throughout compressor, but generally
   * should not be touched by a surrounding application.
   */

  /*
   * These fields are computed during compression startup
   */
  boolean progressive_mode;	/* TRUE if scan script uses progressive mode */
  int max_h_samp_factor;	/* largest h_samp_factor */
  int max_v_samp_factor;	/* largest v_samp_factor */

  int min_DCT_h_scaled_size;	/* smallest DCT_h_scaled_size of any component */
  int min_DCT_v_scaled_size;	/* smallest DCT_v_scaled_size of any component */

  unsigned int total_iMCU_rows;	/* # of iMCU rows to be input to coef ctlr */
  /* The coefficient controller receives data in units of MCU rows as defined
   * for fully interleaved scans (whether the JPEG file is interleaved or not).
   * There are v_samp_factor * DCTSIZE sample rows of each component in an
   * "iMCU" (interleaved MCU) row.
   */
  
  /*
   * These fields are valid during any one scan.
   * They describe the components and MCUs actually appearing in the scan.
   */
  int comps_in_scan;		/* # of JPEG components in this scan */
  sJpeg_component_info * cur_comp_info[MAX_COMPS_IN_SCAN];
  /* *cur_comp_info[i] describes component that appears i'th in SOS */
  
  unsigned int MCUs_per_row;	/* # of MCUs across the image */
  unsigned int MCU_rows_in_scan;	/* # of MCU rows in the image */
  
  int blocks_in_MCU;		/* # of DCT blocks per MCU */
  int MCU_membership[C_MAX_BLOCKS_IN_MCU];
  /* MCU_membership[i] is index in cur_comp_info of component owning */
  /* i'th block in an MCU */

  int Ss, Se, Ah, Al;		/* progressive JPEG parameters for scan */

  int block_size;		/* the basic DCT block size: 1..16 */
  const int * natural_order;	/* natural-order position array */
  int lim_Se;			/* min( Se, DCTSIZE2-1 ) */

  /*
   * Links to compression subobjects (methods and private variables of modules)
   */
  sJpeg_comp_master * master;
  sJpeg_c_main_controller * main;
  sJpeg_c_prep_controller * prep;
  sJpeg_c_coef_controller * coef;
  sJpeg_marker_writer * marker;
  sJpeg_color_converter * cconvert;
  sJpeg_downsampler * downsample;
  sJpeg_forward_dct * fdct;
  sJpeg_entropy_encoder * entropy;
  sJpeg_scan_info * script_space; /* workspace for jpeg_simple_progression */
  int script_space_size;
};


/* Data destination object for compression */
struct sJpeg_destination_mgr 
{
	unsigned char * next_output_byte;	/* => next byte to write in buffer */
  size_t free_in_buffer;	/* # of byte spaces remaining in buffer */

  void ( *init_destination ) ( sJpeg_compress_struct * cinfo );
  boolean  ( * empty_output_buffer ) ( sJpeg_compress_struct * cinfo );
  void ( *term_destination ) ( sJpeg_compress_struct * cinfo );
};


/* Expanded data destination object for stdio output */
struct sMy_destination_mgr
{
  sJpeg_destination_mgr pub; /* public fields */

  FILE * outfile;		/* target stream */
	unsigned char * buffer;		/* start of buffer */
};

#define OUTPUT_BUF_SIZE  4096	/* choose an efficiently fwrite'able size */

/* Expanded data destination object for memory output */
struct sMy_mem_destination_mgr
{
  sJpeg_destination_mgr pub; /* public fields */

  unsigned char ** outbuffer;	/* target buffer */
  unsigned long * outsize;
  unsigned char * newbuffer;	/* newly allocated buffer */
	unsigned char * buffer;		/* start of buffer */
  size_t bufsize;
};

/* These marker codes are exported since applications and data source modules
 * are likely to want to use them.
 */

#define JPEG_RST0	0xD0	/* RST0 marker code */
#define JPEG_EOI	0xD9	/* EOI marker code */
#define JPEG_APP0	0xE0	/* APP0 marker code */
#define JPEG_COM	0xFE	/* COM marker code */



class CLibJpeg
{
public:
	void jpeg_create_compress( sJpeg_compress_struct *cinfo, int version = JPEG_LIB_VERSION, size_t structsize = (size_t) sizeof( sJpeg_compress_struct ) );
	void jpeg_mem_dest ( sJpeg_compress_struct *cinfo, unsigned char **outbuffer, unsigned long *outsize );
	void jpeg_start_compress ( sJpeg_compress_struct *cinfo, boolean write_all_tables );

	void jpeg_set_quality ( sJpeg_compress_struct *cinfo, int quality, boolean force_baseline );
	// Set or change the 'quality' (quantization) setting, using default tables.
	// This is the standard quality-adjusting entry point for typical user
	// interfaces; only those who want detailed control over quantization tables
	// would use the preceding routines directly.



};

#endif