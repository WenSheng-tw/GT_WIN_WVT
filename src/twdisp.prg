/* twdisp.prg
 * get-tw display-width helpers (written for get-tw, not derived from Harbour source)
 * Copyright 2026 Wen Sheng
 *
 * GNU GPL v2 or later, with the Harbour / get-tw linking exception
 * as given in twwide.c. This file is part of get-tw.
 *
 * Written by Wen Sheng, 2026-11-13: display-cell functions shared by
 * AChoice / TBrowse / MemoEdit / MemoLine (same rules as tget.prg TwCells).
 * Modified by Wen Sheng, 2026-11-13: D9-C TwDispW now calls the shared
 * TwCodePointCells.
 * This file contains no original Harbour copyright notice.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * The Harbour Project linking exception (as given in twwide.c) also
 * covers this file.
 */

// Same as tget.prg METHOD TwCells: UTF-8 now calls TwCodePointCells; non-UTF-8 (CP950) uses the byte count of each character
FUNCTION TwDispW( cText )

   LOCAL nCells := 0
   LOCAL lUTF8  := hb_cdpIsUTF8( hb_cdpSelect() )
   LOCAL nFor
   LOCAL nCode

   IF ! HB_ISSTRING( cText ) .OR. Len( cText ) == 0
      RETURN 0
   ENDIF

   FOR nFor := 1 TO Len( cText )
      IF lUTF8
         nCode := hb_UTF8Asc( SubStr( cText, nFor, 1 ) )
         // Modified by Wen Sheng, 2026-11-13: now calls the shared C function TwCodePointCells (D9-C merges the four tables; includes the ambiguous width, PUA fixed at 2, adds 0x20000)
         nCells += TwCodePointCells( nCode )
      ELSE
         nCells += hb_BLen( SubStr( cText, nFor, 1 ) )
      ENDIF
   NEXT

   RETURN nCells

// Truncate by display cells and pad with blanks to nWidth cells (same as PadR for English)
FUNCTION TwPadR( cText, nWidth )

   LOCAL cOut   := ""
   LOCAL nCells := 0
   LOCAL nI
   LOCAL nOne
   LOCAL cCh

   IF nWidth == NIL .OR. nWidth <= 0
      RETURN ""
   ENDIF
   IF ! HB_ISSTRING( cText )
      RETURN Space( nWidth )
   ENDIF

   FOR nI := 1 TO Len( cText )
      cCh  := SubStr( cText, nI, 1 )
      nOne := TwDispW( cCh )
      IF nCells + nOne > nWidth
         EXIT
      ENDIF
      cOut   += cCh
      nCells += nOne
   NEXT
   IF nCells < nWidth
      cOut += Space( nWidth - nCells )
   ENDIF

   RETURN cOut

// Take enough characters from character nFrom to fill nWidth display cells, then pad with blanks (same as hb_UPadR(hb_USubStr(...), nWidth) for English)
FUNCTION TwSubStrPad( cText, nFrom, nWidth )

   IF ! HB_ISSTRING( cText )
      cText := ""
   ENDIF
   IF nFrom == NIL .OR. nFrom < 1
      nFrom := 1
   ENDIF

   RETURN TwPadR( hb_USubStr( cText, nFrom ), nWidth )

