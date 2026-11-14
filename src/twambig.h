/* twambig.h
 * get-tw shared East-Asian / ambiguous cell-width (D9-C)
 * Copyright 2026 Wen Sheng
 *
 * GNU GPL v2 or later, with the Harbour / get-tw linking exception
 * as given in twwide.c. This file is part of get-tw.
 *
 * Modified by Wen Sheng, 2026-11-13: new shared width table and TwAmbigW / tw_AmbigW / tw_CodePointCells;
 * the four old tables are merged into this one (twwide.c tw_CharCells, tget.prg TwCells, twdisp.prg TwDispW, mlcfunc.c tw_disp_cell);
 * the missing 0x20000 block is added; PUA U+E000-F8FF is fixed at 2 cells (option 2).
 */

#ifndef TWAMBIG_H_
#define TWAMBIG_H_

#include "hbapi.h"

/* Modified by Wen Sheng, 2026-11-13: query or set the ambiguous width; 0 = query, 1 or 2 = set, returns the old value; other values are ignored. */
int tw_AmbigW( int iNew );

/* Modified by Wen Sheng, 2026-11-13: how many cells one code point takes (UTF-8 path); the parameter is HB_U32 so HB_USHORT does not cut off Ext-B. */
int tw_CodePointCells( HB_U32 nCode );

#endif /* TWAMBIG_H_ */
