#ifndef JDATADST_H
#define JDATADST_H

/*
 * jdatadst.c
 *
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * Modified 2009-2012 by Guido Vollbeding.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains compression data destination routines for the case of
 * emitting JPEG data to memory or to a file (or any stdio stream).
 * While these routines are sufficient for most applications,
 * some will want to use a different destination manager.
 * IMPORTANT: we assume that fwrite() will correctly transcribe an array of
 * JOCTETs into 8-bit-wide elements on external storage.  If char is wider
 * than 8 bits on your machine, you may need to do some tweaking.
 */

/* this is not a core library module, so it doesn't define JPEG_INTERNALS */
#include "jinclude.h"
#include "LibJpeg.h"
#include "jerror.h"

#ifndef HAVE_STDLIB_H		/* <stdlib.h> should declare malloc(),free() */
extern void * malloc JPP((size_t size));
extern void free JPP((void *ptr));
#endif


/* Expanded data destination object for stdio output */
struct sMy_destination_mgr
{
  sJpeg_destination_mgr pub; /* public fields */

  FILE * outfile;		/* target stream */
	JOCTET * buffer;		/* start of buffer */
};

#define OUTPUT_BUF_SIZE  4096	/* choose an efficiently fwrite'able size */

/* Expanded data destination object for memory output */
struct sMy_mem_destination_mgr
{
  sJpeg_destination_mgr pub; /* public fields */

  unsigned char ** outbuffer;	/* target buffer */
  unsigned long * outsize;
  unsigned char * newbuffer;	/* newly allocated buffer */
	JOCTET * buffer;		/* start of buffer */
  size_t bufsize;
};

/*
 * Initialize destination --- called by jpeg_start_compress
 * before any data is actually written.
 */
void init_destination ( sJpeg_compress_struct * cinfo);

void init_mem_destination ( sJpeg_compress_struct *cinfo );

/*
 * Empty the output buffer --- called whenever buffer fills up.
 *
 * In typical applications, this should write the entire output buffer
 * (ignoring the current state of next_output_byte & free_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been dumped.
 *
 * In applications that need to be able to suspend compression due to output
 * overrun, a FALSE return indicates that the buffer cannot be emptied now.
 * In this situation, the compressor will return to its caller (possibly with
 * an indication that it has not accepted all the supplied scanlines).  The
 * application should resume compression after it has made more room in the
 * output buffer.  Note that there are substantial restrictions on the use of
 * suspension --- see the documentation.
 *
 * When suspending, the compressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_output_byte & free_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point will be regenerated after resumption, so do not
 * write it out when emptying the buffer externally.
 */
boolean empty_output_buffer ( sJpeg_compress_struct * cinfo);

boolean empty_mem_output_buffer ( sJpeg_compress_struct *cinfo );

/*
 * Terminate destination --- called by jpeg_finish_compress
 * after all data has been written.  Usually needs to flush buffer.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */
void term_destination ( sJpeg_compress_struct * cinfo);

void term_mem_destination ( sJpeg_compress_struct *cinfo );

#endif