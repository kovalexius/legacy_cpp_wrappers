#include "jcarith.h"
#include "jaricom.h"
#include "jerror.h"

void jinit_arith_encoder ( sJpeg_compress_struct *cinfo)
{
	arith_entropy_encoder *entropy;
  int i;

  entropy = ( arith_entropy_encoder * )  ( *cinfo->mem->alloc_small) ( (sJpeg_common_struct*) cinfo, JPOOL_IMAGE, (size_t)sizeof( arith_entropy_encoder ) );
  cinfo->entropy = &entropy->pub;
  entropy->pub.start_pass = start_pass;
  entropy->pub.finish_pass = finish_pass;

  /* Mark tables unallocated */
  for (i = 0; i < NUM_ARITH_TBLS; i++) 
	{
    entropy->dc_stats[i] = NULL;
    entropy->ac_stats[i] = NULL;
  }

  /* Initialize index for fixed probability estimation */
  entropy->fixed_bin[0] = 113;
}


static void emit_byte ( int val, sJpeg_compress_struct *cinfo )
{
	sJpeg_destination_mgr * dest = cinfo->dest;

  *dest->next_output_byte++ = (JOCTET) val;
  if (--dest->free_in_buffer == 0)
    if (! (*dest->empty_output_buffer) (cinfo))
      ERREXIT(cinfo, JERR_CANT_SUSPEND);
}


static void start_pass ( sJpeg_compress_struct *cinfo, boolean gather_statistics )
{
	arith_entropy_encoder *entropy = ( arith_entropy_encoder *) cinfo->entropy;
  int ci, tbl;
  sJpeg_component_info * compptr;

  if (gather_statistics)
    /* Make sure to avoid that in the master control logic!
     * We are fully adaptive here and need no extra
     * statistics gathering pass!
     */
    ERREXIT(cinfo, JERR_NOT_COMPILED);

  /* We assume jcmaster.c already validated the progressive scan parameters. */

  /* Select execution routines */
  if (cinfo->progressive_mode) {
    if (cinfo->Ah == 0) {
      if (cinfo->Ss == 0)
	entropy->pub.encode_mcu = encode_mcu_DC_first;
      else
	entropy->pub.encode_mcu = encode_mcu_AC_first;
    } else {
      if (cinfo->Ss == 0)
	entropy->pub.encode_mcu = encode_mcu_DC_refine;
      else
	entropy->pub.encode_mcu = encode_mcu_AC_refine;
    }
  } else
    entropy->pub.encode_mcu = encode_mcu;

  /* Allocate & initialize requested statistics areas */
  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    /* DC needs no table for refinement scan */
    if (cinfo->Ss == 0 && cinfo->Ah == 0) {
      tbl = compptr->dc_tbl_no;
      if (tbl < 0 || tbl >= NUM_ARITH_TBLS)
	ERREXIT1(cinfo, JERR_NO_ARITH_TABLE, tbl);
      if (entropy->dc_stats[tbl] == NULL)
	entropy->dc_stats[tbl] = (unsigned char *) (*cinfo->mem->alloc_small)( (sJpeg_common_struct *) cinfo, JPOOL_IMAGE, DC_STAT_BINS);
      MEMZERO(entropy->dc_stats[tbl], DC_STAT_BINS);
      /* Initialize DC predictions to 0 */
      entropy->last_dc_val[ci] = 0;
      entropy->dc_context[ci] = 0;
    }
    /* AC needs no table when not present */
    if (cinfo->Se) {
      tbl = compptr->ac_tbl_no;
      if (tbl < 0 || tbl >= NUM_ARITH_TBLS)
	ERREXIT1(cinfo, JERR_NO_ARITH_TABLE, tbl);
      if (entropy->ac_stats[tbl] == NULL)
	entropy->ac_stats[tbl] = (unsigned char *) (*cinfo->mem->alloc_small) ( ( sJpeg_common_struct *) cinfo, JPOOL_IMAGE, AC_STAT_BINS );
      MEMZERO(entropy->ac_stats[tbl], AC_STAT_BINS);
	#ifdef CALCULATE_SPECTRAL_CONDITIONING
      if (cinfo->progressive_mode)
	/* Section G.1.3.2: Set appropriate arithmetic conditioning value Kx */
	cinfo->arith_ac_K[tbl] = cinfo->Ss + ((8 + cinfo->Se - cinfo->Ss) >> 4);
	#endif
    }
  }

  /* Initialize arithmetic encoding variables */
  entropy->c = 0;
  entropy->a = 0x10000L;
  entropy->sc = 0;
  entropy->zc = 0;
  entropy->ct = 11;
  entropy->buffer = -1;  /* empty */

  /* Initialize restart stuff */
  entropy->restarts_to_go = cinfo->restart_interval;
  entropy->next_restart_num = 0;
}


static boolean encode_mcu_DC_refine ( sJpeg_compress_struct *cinfo, JBLOCKROW *MCU_data )
{
	arith_entropy_encoder *entropy = ( arith_entropy_encoder * ) cinfo->entropy;
  unsigned char *st;
  int Al, blkn;

  /* Emit restart marker if needed */
  if (cinfo->restart_interval) 
	{
    if (entropy->restarts_to_go == 0) 
		{
      emit_restart(cinfo, entropy->next_restart_num);
      entropy->restarts_to_go = cinfo->restart_interval;
      entropy->next_restart_num++;
      entropy->next_restart_num &= 7;
    }
    entropy->restarts_to_go--;
  }

  st = entropy->fixed_bin;	/* use fixed probability estimation */
  Al = cinfo->Al;

  /* Encode the MCU data blocks */
  for (blkn = 0; blkn < cinfo->blocks_in_MCU; blkn++) 
	{
    /* We simply emit the Al'th bit of the DC coefficient value. */
    arith_encode(cinfo, st, (MCU_data[blkn][0][0] >> Al) & 1);
  }

  return TRUE;
}


static boolean encode_mcu_DC_first ( sJpeg_compress_struct *cinfo, JBLOCKROW *MCU_data )
{
	arith_entropy_encoder *entropy = (arith_entropy_encoder *) cinfo->entropy;
  JBLOCKROW block;
  unsigned char *st;
  int blkn, ci, tbl;
  int v, v2, m;
  //ISHIFT_TEMPS

  /* Emit restart marker if needed */
  if (cinfo->restart_interval) 
	{
    if (entropy->restarts_to_go == 0) 
		{
      emit_restart(cinfo, entropy->next_restart_num);
      entropy->restarts_to_go = cinfo->restart_interval;
      entropy->next_restart_num++;
      entropy->next_restart_num &= 7;
    }
    entropy->restarts_to_go--;
  }

  /* Encode the MCU data blocks */
  for (blkn = 0; blkn < cinfo->blocks_in_MCU; blkn++) 
	{
    block = MCU_data[blkn];
    ci = cinfo->MCU_membership[blkn];
    tbl = cinfo->cur_comp_info[ci]->dc_tbl_no;

    /* Compute the DC value after the required point transform by Al.
     * This is simply an arithmetic right shift.
     */
    m = IRIGHT_SHIFT((int) ((*block)[0]), cinfo->Al);

    /* Sections F.1.4.1 & F.1.4.4.1: Encoding of DC coefficients */

    /* Table F.4: Point to statistics bin S0 for DC coefficient coding */
    st = entropy->dc_stats[tbl] + entropy->dc_context[ci];

    /* Figure F.4: Encode_DC_DIFF */
    if ((v = m - entropy->last_dc_val[ci]) == 0) {
      arith_encode(cinfo, st, 0);
      entropy->dc_context[ci] = 0;	/* zero diff category */
    } else {
      entropy->last_dc_val[ci] = m;
      arith_encode(cinfo, st, 1);
      /* Figure F.6: Encoding nonzero value v */
      /* Figure F.7: Encoding the sign of v */
      if (v > 0) {
	arith_encode(cinfo, st + 1, 0);	/* Table F.4: SS = S0 + 1 */
	st += 2;			/* Table F.4: SP = S0 + 2 */
	entropy->dc_context[ci] = 4;	/* small positive diff category */
      } else {
	v = -v;
	arith_encode(cinfo, st + 1, 1);	/* Table F.4: SS = S0 + 1 */
	st += 3;			/* Table F.4: SN = S0 + 3 */
	entropy->dc_context[ci] = 8;	/* small negative diff category */
      }
      /* Figure F.8: Encoding the magnitude category of v */
      m = 0;
      if (v -= 1) {
	arith_encode(cinfo, st, 1);
	m = 1;
	v2 = v;
	st = entropy->dc_stats[tbl] + 20; /* Table F.4: X1 = 20 */
	while (v2 >>= 1) 
	{
	  arith_encode(cinfo, st, 1);
	  m <<= 1;
	  st += 1;
	}
      }
      arith_encode(cinfo, st, 0);
      /* Section F.1.4.4.1.2: Establish dc_context conditioning category */
      if (m < (int) ((1L << cinfo->arith_dc_L[tbl]) >> 1))
	entropy->dc_context[ci] = 0;	/* zero diff category */
      else if (m > (int) ((1L << cinfo->arith_dc_U[tbl]) >> 1))
	entropy->dc_context[ci] += 8;	/* large diff category */
      /* Figure F.9: Encoding the magnitude bit pattern of v */
      st += 14;
      while (m >>= 1)
	arith_encode(cinfo, st, (m & v) ? 1 : 0);
    }
  }
  return TRUE;
}


static boolean encode_mcu_AC_first ( sJpeg_compress_struct *cinfo, JBLOCKROW *MCU_data)
{
	arith_entropy_encoder *entropy = ( arith_entropy_encoder * ) cinfo->entropy;
  JBLOCKROW block;
  unsigned char *st;
  int tbl, k, ke;
  int v, v2, m;
  const int * natural_order;

  /* Emit restart marker if needed */
  if (cinfo->restart_interval) {
    if (entropy->restarts_to_go == 0) {
      emit_restart(cinfo, entropy->next_restart_num);
      entropy->restarts_to_go = cinfo->restart_interval;
      entropy->next_restart_num++;
      entropy->next_restart_num &= 7;
    }
    entropy->restarts_to_go--;
  }

  natural_order = cinfo->natural_order;

  /* Encode the MCU data block */
  block = MCU_data[0];
  tbl = cinfo->cur_comp_info[0]->ac_tbl_no;

  /* Sections F.1.4.2 & F.1.4.4.2: Encoding of AC coefficients */

  /* Establish EOB (end-of-block) index */
  ke = cinfo->Se;
  do {
    /* We must apply the point transform by Al.  For AC coefficients this
     * is an integer division with rounding towards 0.  To do this portably
     * in C, we shift after obtaining the absolute value.
     */
    if ((v = (*block)[natural_order[ke]]) >= 0) {
      if (v >>= cinfo->Al) break;
    } else {
      v = -v;
      if (v >>= cinfo->Al) break;
    }
  } while (--ke);

  /* Figure F.5: Encode_AC_Coefficients */
  for (k = cinfo->Ss - 1; k < ke;) {
    st = entropy->ac_stats[tbl] + 3 * k;
    arith_encode(cinfo, st, 0);		/* EOB decision */
    for (;;) {
      if ((v = (*block)[natural_order[++k]]) >= 0) {
	if (v >>= cinfo->Al) {
	  arith_encode(cinfo, st + 1, 1);
	  arith_encode(cinfo, entropy->fixed_bin, 0);
	  break;
	}
      } else {
	v = -v;
	if (v >>= cinfo->Al) {
	  arith_encode(cinfo, st + 1, 1);
	  arith_encode(cinfo, entropy->fixed_bin, 1);
	  break;
	}
      }
      arith_encode(cinfo, st + 1, 0);
      st += 3;
    }
    st += 2;
    /* Figure F.8: Encoding the magnitude category of v */
    m = 0;
    if (v -= 1) {
      arith_encode(cinfo, st, 1);
      m = 1;
      v2 = v;
      if (v2 >>= 1) {
	arith_encode(cinfo, st, 1);
	m <<= 1;
	st = entropy->ac_stats[tbl] +
	     (k <= cinfo->arith_ac_K[tbl] ? 189 : 217);
	while (v2 >>= 1) {
	  arith_encode(cinfo, st, 1);
	  m <<= 1;
	  st += 1;
	}
      }
    }
    arith_encode(cinfo, st, 0);
    /* Figure F.9: Encoding the magnitude bit pattern of v */
    st += 14;
    while (m >>= 1)
      arith_encode(cinfo, st, (m & v) ? 1 : 0);
  }
  /* Encode EOB decision only if k < cinfo->Se */
  if (k < cinfo->Se) {
    st = entropy->ac_stats[tbl] + 3 * k;
    arith_encode(cinfo, st, 1);
  }

  return TRUE;
}


static void arith_encode ( sJpeg_compress_struct *cinfo, unsigned char *st, int val )
{
	register arith_entropy_encoder *e = (arith_entropy_encoder *) cinfo->entropy;
  register unsigned char nl, nm;
  register INT32 qe, temp;
  register int sv;

  /* Fetch values from our compact representation of Table D.3(D.2):
   * Qe values and probability estimation state machine
   */
  sv = *st;
  qe = jpeg_aritab[sv & 0x7F];	/* => Qe_Value */
  nl = qe & 0xFF; qe >>= 8;	/* Next_Index_LPS + Switch_MPS */
  nm = qe & 0xFF; qe >>= 8;	/* Next_Index_MPS */

  /* Encode & estimation procedures per sections D.1.4 & D.1.5 */
  e->a -= qe;
  if (val != (sv >> 7)) {
    /* Encode the less probable symbol */
    if (e->a >= qe) {
      /* If the interval size (qe) for the less probable symbol (LPS)
       * is larger than the interval size for the MPS, then exchange
       * the two symbols for coding efficiency, otherwise code the LPS
       * as usual: */
      e->c += e->a;
      e->a = qe;
    }
    *st = (sv & 0x80) ^ nl;	/* Estimate_after_LPS */
  } else {
    /* Encode the more probable symbol */
    if (e->a >= 0x8000L)
      return;  /* A >= 0x8000 -> ready, no renormalization required */
    if (e->a < qe) {
      /* If the interval size (qe) for the less probable symbol (LPS)
       * is larger than the interval size for the MPS, then exchange
       * the two symbols for coding efficiency: */
      e->c += e->a;
      e->a = qe;
    }
    *st = (sv & 0x80) ^ nm;	/* Estimate_after_MPS */
  }

  /* Renormalization & data output per section D.1.6 */
  do {
    e->a <<= 1;
    e->c <<= 1;
    if (--e->ct == 0) {
      /* Another byte is ready for output */
      temp = e->c >> 19;
      if (temp > 0xFF) {
	/* Handle overflow over all stacked 0xFF bytes */
	if (e->buffer >= 0) {
	  if (e->zc)
	    do emit_byte(0x00, cinfo);
	    while (--e->zc);
	  emit_byte(e->buffer + 1, cinfo);
	  if (e->buffer + 1 == 0xFF)
	    emit_byte(0x00, cinfo);
	}
	e->zc += e->sc;  /* carry-over converts stacked 0xFF bytes to 0x00 */
	e->sc = 0;
	/* Note: The 3 spacer bits in the C register guarantee
	 * that the new buffer byte can't be 0xFF here
	 * (see page 160 in the P&M JPEG book). */
	e->buffer = temp & 0xFF;  /* new output byte, might overflow later */
      } else if (temp == 0xFF) {
	++e->sc;  /* stack 0xFF byte (which might overflow later) */
      } else {
	/* Output all stacked 0xFF bytes, they will not overflow any more */
	if (e->buffer == 0)
	  ++e->zc;
	else if (e->buffer >= 0) {
	  if (e->zc)
	    do emit_byte(0x00, cinfo);
	    while (--e->zc);
	  emit_byte(e->buffer, cinfo);
	}
	if (e->sc) {
	  if (e->zc)
	    do emit_byte(0x00, cinfo);
	    while (--e->zc);
	  do {
	    emit_byte(0xFF, cinfo);
	    emit_byte(0x00, cinfo);
	  } while (--e->sc);
	}
	e->buffer = temp & 0xFF;  /* new output byte (can still overflow) */
      }
      e->c &= 0x7FFFFL;
      e->ct += 8;
    }
  } while (e->a < 0x8000L);
}


static boolean encode_mcu_AC_refine ( sJpeg_compress_struct *cinfo, JBLOCKROW *MCU_data )
{
	arith_entropy_encoder *entropy = ( arith_entropy_encoder * ) cinfo->entropy;
  JBLOCKROW block;
  unsigned char *st;
  int tbl, k, ke, kex;
  int v;
  const int * natural_order;

  /* Emit restart marker if needed */
  if (cinfo->restart_interval) 
	{
    if (entropy->restarts_to_go == 0) 
		{
      emit_restart(cinfo, entropy->next_restart_num);
      entropy->restarts_to_go = cinfo->restart_interval;
      entropy->next_restart_num++;
      entropy->next_restart_num &= 7;
    }
    entropy->restarts_to_go--;
  }

  natural_order = cinfo->natural_order;

  /* Encode the MCU data block */
  block = MCU_data[0];
  tbl = cinfo->cur_comp_info[0]->ac_tbl_no;

  /* Section G.1.3.3: Encoding of AC coefficients */

  /* Establish EOB (end-of-block) index */
  ke = cinfo->Se;
  do 
	{
    /* We must apply the point transform by Al.  For AC coefficients this
     * is an integer division with rounding towards 0.  To do this portably
     * in C, we shift after obtaining the absolute value.
     */
    if ((v = (*block)[natural_order[ke]]) >= 0) 
		{
      if (v >>= cinfo->Al) break;
    } 
		else 
		{
      v = -v;
      if (v >>= cinfo->Al) break;
    }
  } while (--ke);

  /* Establish EOBx (previous stage end-of-block) index */
  for (kex = ke; kex > 0; kex--)
    if ((v = (*block)[natural_order[kex]]) >= 0) 
		{
      if (v >>= cinfo->Ah) break;
    } 
		else 
		{
      v = -v;
      if (v >>= cinfo->Ah) break;
    }

  /* Figure G.10: Encode_AC_Coefficients_SA */
  for ( k = cinfo->Ss - 1; k < ke; ) 
	{
    st = entropy->ac_stats[tbl] + 3 * k;
    if ( k >= kex )
      arith_encode(cinfo, st, 0);	/* EOB decision */
    for (;;) 
		{
      if ((v = (*block)[natural_order[++k]]) >= 0) 
			{
				if (v >>= cinfo->Al) 
				{
				  if (v >> 1)			/* previously nonzero coef */
				    arith_encode(cinfo, st + 2, (v & 1));
				  else 
					{			/* newly nonzero coef */
				    arith_encode(cinfo, st + 1, 1);
				    arith_encode(cinfo, entropy->fixed_bin, 0);
				  }
				  break;
				}
      } 
			else 
			{
	v = -v;
	if (v >>= cinfo->Al) 
	{
	  if (v >> 1)			/* previously nonzero coef */
	    arith_encode(cinfo, st + 2, (v & 1));
	  else {			/* newly nonzero coef */
	    arith_encode(cinfo, st + 1, 1);
	    arith_encode(cinfo, entropy->fixed_bin, 1);
	  }
	  break;
	}
      }
      arith_encode(cinfo, st + 1, 0);
      st += 3;
    }
  }
  /* Encode EOB decision only if k < cinfo->Se */
  if (k < cinfo->Se) {
    st = entropy->ac_stats[tbl] + 3 * k;
    arith_encode(cinfo, st, 1);
  }

  return TRUE;
}


static boolean encode_mcu ( sJpeg_compress_struct *cinfo, JBLOCKROW *MCU_data )
{
	arith_entropy_encoder *entropy = (arith_entropy_encoder *) cinfo->entropy;
  sJpeg_component_info * compptr;
  JBLOCKROW block;
  unsigned char *st;
  int blkn, ci, tbl, k, ke;
  int v, v2, m;
  const int * natural_order;

  /* Emit restart marker if needed */
  if (cinfo->restart_interval) {
    if (entropy->restarts_to_go == 0) {
      emit_restart(cinfo, entropy->next_restart_num);
      entropy->restarts_to_go = cinfo->restart_interval;
      entropy->next_restart_num++;
      entropy->next_restart_num &= 7;
    }
    entropy->restarts_to_go--;
  }

  natural_order = cinfo->natural_order;

  /* Encode the MCU data blocks */
  for (blkn = 0; blkn < cinfo->blocks_in_MCU; blkn++) {
    block = MCU_data[blkn];
    ci = cinfo->MCU_membership[blkn];
    compptr = cinfo->cur_comp_info[ci];

    /* Sections F.1.4.1 & F.1.4.4.1: Encoding of DC coefficients */

    tbl = compptr->dc_tbl_no;

    /* Table F.4: Point to statistics bin S0 for DC coefficient coding */
    st = entropy->dc_stats[tbl] + entropy->dc_context[ci];

    /* Figure F.4: Encode_DC_DIFF */
    if ((v = (*block)[0] - entropy->last_dc_val[ci]) == 0) {
      arith_encode(cinfo, st, 0);
      entropy->dc_context[ci] = 0;	/* zero diff category */
    } else {
      entropy->last_dc_val[ci] = (*block)[0];
      arith_encode(cinfo, st, 1);
      /* Figure F.6: Encoding nonzero value v */
      /* Figure F.7: Encoding the sign of v */
      if (v > 0) {
	arith_encode(cinfo, st + 1, 0);	/* Table F.4: SS = S0 + 1 */
	st += 2;			/* Table F.4: SP = S0 + 2 */
	entropy->dc_context[ci] = 4;	/* small positive diff category */
      } else {
	v = -v;
	arith_encode(cinfo, st + 1, 1);	/* Table F.4: SS = S0 + 1 */
	st += 3;			/* Table F.4: SN = S0 + 3 */
	entropy->dc_context[ci] = 8;	/* small negative diff category */
      }
      /* Figure F.8: Encoding the magnitude category of v */
      m = 0;
      if (v -= 1) {
	arith_encode(cinfo, st, 1);
	m = 1;
	v2 = v;
	st = entropy->dc_stats[tbl] + 20; /* Table F.4: X1 = 20 */
	while (v2 >>= 1) {
	  arith_encode(cinfo, st, 1);
	  m <<= 1;
	  st += 1;
	}
      }
      arith_encode(cinfo, st, 0);
      /* Section F.1.4.4.1.2: Establish dc_context conditioning category */
      if (m < (int) ((1L << cinfo->arith_dc_L[tbl]) >> 1))
	entropy->dc_context[ci] = 0;	/* zero diff category */
      else if (m > (int) ((1L << cinfo->arith_dc_U[tbl]) >> 1))
	entropy->dc_context[ci] += 8;	/* large diff category */
      /* Figure F.9: Encoding the magnitude bit pattern of v */
      st += 14;
      while (m >>= 1)
	arith_encode(cinfo, st, (m & v) ? 1 : 0);
    }

    /* Sections F.1.4.2 & F.1.4.4.2: Encoding of AC coefficients */

    if ((ke = cinfo->lim_Se) == 0) continue;
    tbl = compptr->ac_tbl_no;

    /* Establish EOB (end-of-block) index */
    do {
      if ((*block)[natural_order[ke]]) break;
    } while (--ke);

    /* Figure F.5: Encode_AC_Coefficients */
    for (k = 0; k < ke;) {
      st = entropy->ac_stats[tbl] + 3 * k;
      arith_encode(cinfo, st, 0);	/* EOB decision */
      while ((v = (*block)[natural_order[++k]]) == 0) {
	arith_encode(cinfo, st + 1, 0);
	st += 3;
      }
      arith_encode(cinfo, st + 1, 1);
      /* Figure F.6: Encoding nonzero value v */
      /* Figure F.7: Encoding the sign of v */
      if (v > 0) {
	arith_encode(cinfo, entropy->fixed_bin, 0);
      } else {
	v = -v;
	arith_encode(cinfo, entropy->fixed_bin, 1);
      }
      st += 2;
      /* Figure F.8: Encoding the magnitude category of v */
      m = 0;
      if (v -= 1) {
	arith_encode(cinfo, st, 1);
	m = 1;
	v2 = v;
	if (v2 >>= 1) {
	  arith_encode(cinfo, st, 1);
	  m <<= 1;
	  st = entropy->ac_stats[tbl] +
	       (k <= cinfo->arith_ac_K[tbl] ? 189 : 217);
	  while (v2 >>= 1) {
	    arith_encode(cinfo, st, 1);
	    m <<= 1;
	    st += 1;
	  }
	}
      }
      arith_encode(cinfo, st, 0);
      /* Figure F.9: Encoding the magnitude bit pattern of v */
      st += 14;
      while (m >>= 1)
	arith_encode(cinfo, st, (m & v) ? 1 : 0);
    }
    /* Encode EOB decision only if k < cinfo->lim_Se */
    if (k < cinfo->lim_Se) {
      st = entropy->ac_stats[tbl] + 3 * k;
      arith_encode(cinfo, st, 1);
    }
  }

  return TRUE;
}

static void emit_restart ( sJpeg_compress_struct *cinfo, int restart_num )
{
	arith_entropy_encoder *entropy = (arith_entropy_encoder *) cinfo->entropy;
  int ci;
  sJpeg_component_info * compptr;

  finish_pass(cinfo);

  emit_byte(0xFF, cinfo);
  emit_byte(JPEG_RST0 + restart_num, cinfo);

  /* Re-initialize statistics areas */
  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    /* DC needs no table for refinement scan */
    if (cinfo->Ss == 0 && cinfo->Ah == 0) {
      MEMZERO(entropy->dc_stats[compptr->dc_tbl_no], DC_STAT_BINS);
      /* Reset DC predictions to 0 */
      entropy->last_dc_val[ci] = 0;
      entropy->dc_context[ci] = 0;
    }
    /* AC needs no table when not present */
    if (cinfo->Se) {
      MEMZERO(entropy->ac_stats[compptr->ac_tbl_no], AC_STAT_BINS);
    }
  }

  /* Reset arithmetic encoding variables */
  entropy->c = 0;
  entropy->a = 0x10000L;
  entropy->sc = 0;
  entropy->zc = 0;
  entropy->ct = 11;
  entropy->buffer = -1;  /* empty */
}

static void finish_pass ( sJpeg_compress_struct *cinfo )
{
	arith_entropy_encoder *e = (arith_entropy_encoder *) cinfo->entropy;
  INT32 temp;

  /* Section D.1.8: Termination of encoding */

  /* Find the e->c in the coding interval with the largest
   * number of trailing zero bits */
  if ((temp = (e->a - 1 + e->c) & 0xFFFF0000L) < e->c)
    e->c = temp + 0x8000L;
  else
    e->c = temp;
  /* Send remaining bytes to output */
  e->c <<= e->ct;
  if (e->c & 0xF8000000L) 
	{
    /* One final overflow has to be handled */
    if (e->buffer >= 0) 
		{
      if (e->zc)
				do emit_byte(0x00, cinfo);
				while (--e->zc);
					emit_byte(e->buffer + 1, cinfo);
      if (e->buffer + 1 == 0xFF)
	emit_byte(0x00, cinfo);
    }
    e->zc += e->sc;  /* carry-over converts stacked 0xFF bytes to 0x00 */
    e->sc = 0;
  } else {
    if (e->buffer == 0)
      ++e->zc;
    else if (e->buffer >= 0) {
      if (e->zc)
	do emit_byte(0x00, cinfo);
	while (--e->zc);
      emit_byte(e->buffer, cinfo);
    }
    if (e->sc) {
      if (e->zc)
	do emit_byte(0x00, cinfo);
	while (--e->zc);
      do {
	emit_byte(0xFF, cinfo);
	emit_byte(0x00, cinfo);
      } while (--e->sc);
    }
  }
  /* Output final bytes only if they are not 0x00 */
  if (e->c & 0x7FFF800L) {
    if (e->zc)  /* output final pending zero bytes */
      do emit_byte(0x00, cinfo);
      while (--e->zc);
    emit_byte((e->c >> 19) & 0xFF, cinfo);
    if (((e->c >> 19) & 0xFF) == 0xFF)
      emit_byte(0x00, cinfo);
    if (e->c & 0x7F800L) {
      emit_byte((e->c >> 11) & 0xFF, cinfo);
      if (((e->c >> 11) & 0xFF) == 0xFF)
	emit_byte(0x00, cinfo);
    }
  }
}
