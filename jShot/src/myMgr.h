#ifndef MYMGR_H
#define MYMGR_H

#include "LibJpeg.h"

namespace jMyMgr
{
	struct jmemdst_s
	{
	    struct sJpeg_destination_mgr dst;
	    unsigned char *data;
	    size_t asize;        // allocated size
	    size_t isize;        // image size
	};
	
	void init_destination( sJpeg_compress_struct * cinfo);
	
	boolean empty_output_buffer( sJpeg_compress_struct *cinfo );

	void term_destination( sJpeg_compress_struct * cinfo );

	void jpeg_mem_dst( sJpeg_compress_struct *cinfo );
}

#endif