#ifndef JCINIT_H
#define JCINIT_H

#include "jinclude.h"
#include "LibJpeg.h"


/*
 * Master selection of compression modules.
 * This is done once at the start of processing an image.  We determine
 * which modules will be used and give them appropriate initialization calls.
 */
void jinit_compress_master ( sJpeg_compress_struct *cinfo );

#endif