/*
 * get-tw wide-character cell handling for WIN_TW / WVT_TW
 * Copyright 2026 Wen Sheng
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file LICENSE.txt.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA (or visit https://www.gnu.org/licenses/).
 *
 * As a special exception, Wen Sheng gives permission for
 * additional uses of the text contained in this release of get-tw.
 *
 * The exception is that, if you link the Harbour libraries or the get-tw
 * code (twwide.c, twwide.h, gtwin_tw.c, gtwvt_tw.c, gtwvt_tw.h, tget.prg)
 * with other files to produce an executable, this does not by itself cause
 * the resulting executable to be covered by the GNU General Public License.
 * Your use of that executable is in no way restricted on account of
 * linking that code into it.
 *
 * This exception does not however invalidate any other reasons why
 * the executable file might be covered by the GNU General Public License.
 *
 * This exception applies only to the code released by Wen Sheng
 * under the name get-tw.  If you copy code from other
 * Harbour Project or Free Software Foundation releases into a copy of
 * get-tw, as the General Public License permits, the exception does
 * not apply to the code that you add in this way.  To avoid misleading
 * anyone as to the status of such modified files, you must delete
 * this exception notice from them.
 *
 * If you write modifications of your own for get-tw, it is your choice
 * whether to permit this exception to apply to your modifications.
 * If you do not wish that, delete this exception notice.
 *
 */

// twwide.c: wide-character cell handling, included by gtwin_tw.c / gtwvt_tw.c via #include
// After inclusion each GT has its own copy of the static functions and state (the super function table is each file's HB_GTSUPER)
// Contents: tw_CharCells, cell kinds, PutChar (automatic trail fill, clearing orphan halves), PutText/PutTextW,
//       WriteAtW, WriteCon/WriteConW (wrapping by cell width), Scroll/Rest (clearing orphan halves at the edges)
// Must not be compiled on its own (do not list this file in an .hbp)

#ifndef HB_GTSUPER
#error "twwide.c can only be included from gtwin_tw.c / gtwvt_tw.c"   // prevent standalone compilation
#endif

#include "twwide.h"

// RestScreen in progress: flags are restored as-is, no automatic trail fill (one per GT)
static HB_BOOL s_twInRest = HB_FALSE;

// Cells per character: UTF8 by Unicode range, CP950 by whether it encodes to 2 bytes
static int tw_CharCells( PHB_GT pGT, HB_USHORT usChar )
{
   // ASCII is always 1 cell
   if( usChar < 0x80 )
      return 1;
   else
   {
      PHB_CODEPAGE cdp = HB_GTSELF_HOSTCP( pGT );   // host codepage
      // UTF8: same wide ranges as tget TwCells
      if( hb_cdpIsUTF8( cdp ) )
      {
         if( ( usChar >= 0x1100 && usChar <= 0x115F ) ||
             ( usChar >= 0x2E80 && usChar <= 0xA4CF ) ||
             ( usChar >= 0xAC00 && usChar <= 0xD7A3 ) ||
             ( usChar >= 0xF900 && usChar <= 0xFAFF ) ||
             ( usChar >= 0xFE30 && usChar <= 0xFE4F ) ||
             ( usChar >= 0xFF00 && usChar <= 0xFF60 ) ||
             ( usChar >= 0xFFE0 && usChar <= 0xFFE6 ) )
            return 2;
         return 1;
      }
      // CP950 and other multibyte codepages: 2 cells when it encodes to 2 bytes
      return HB_CDPCHAR_LEN( cdp, ( HB_WCHAR ) usChar ) == 2 ? 2 : 1;
   }
}

// Wide-lead candidate: not a trail, not a box character, width 2
static HB_BOOL tw_IsWideHead( PHB_GT pGT, HB_BYTE bAttr, HB_USHORT usChar )
{
   // Trail cells and box characters are not wide leads
   if( bAttr & ( TW_ATTR_TRAIL | HB_GT_ATTR_BOX ) )
      return HB_FALSE;
   return tw_CharCells( pGT, usChar ) == 2;
}

// Write a cell as blank (keep the color, clear the flags) directly via the super PutChar
static void tw_Blank( PHB_GT pGT, int iRow, int iCol, int iColor )
{
   HB_GTSUPER->PutChar( pGT, iRow, iCol, iColor, 0, ' ' );   // clear only the character and the flags
}

// Check the pair iCol and iCol+1 (via GetChar, window coordinates): a lead without a trail or a trail without a lead is cleared to blank
static void tw_FixPair( PHB_GT pGT, int iRow, int iCol )
{
   int iColorA, iColorB;
   HB_BYTE bAttrA, bAttrB;
   HB_USHORT usA, usB;
   HB_BOOL fA, fB;

   // An out-of-range cell cannot be read (fA/fB false), which means "no other half"
   fA = HB_GTSELF_GETCHAR( pGT, iRow, iCol, &iColorA, &bAttrA, &usA );
   fB = HB_GTSELF_GETCHAR( pGT, iRow, iCol + 1, &iColorB, &bAttrB, &usB );
   // Right is a trail but left is not the lead of the same character -> clear right
   if( fB && ( bAttrB & TW_ATTR_TRAIL ) )
   {
      if( ! fA || usA != usB || ! tw_IsWideHead( pGT, bAttrA, usA ) )
      {
         tw_Blank( pGT, iRow, iCol + 1, iColorB );
         fB = HB_FALSE;
      }
   }
   // Left is a wide lead but right is not its trail -> clear left
   if( fA && tw_IsWideHead( pGT, bAttrA, usA ) )
   {
      if( ! fB || ! ( bAttrB & TW_ATTR_TRAIL ) || usB != usA )
         tw_Blank( pGT, iRow, iCol, iColorA );
   }
}

// Read the cell kind (screen output paths Redraw/PaintText/cursor/copy always use the GetScrChar version;
// write paths PutChar/FixPair/Scroll/Rest always use GetChar, consistent with the hbct window buffer)
static int tw_ScrCellKind( PHB_GT pGT, int iRow, int iCol )
{
   int iColor, iColor2;
   HB_BYTE bAttr, bAttr2;
   HB_USHORT usChar, usChar2;

   // Treat an unreadable cell as normal
   if( ! HB_GTSELF_GETSCRCHAR( pGT, iRow, iCol, &iColor, &bAttr, &usChar ) )
      return TW_CELL_NORMAL;
   // Trail cell: the left cell must be the lead of the same character
   if( bAttr & TW_ATTR_TRAIL )
   {
      if( HB_GTSELF_GETSCRCHAR( pGT, iRow, iCol - 1, &iColor2, &bAttr2, &usChar2 ) &&
          usChar2 == usChar && tw_IsWideHead( pGT, bAttr2, usChar2 ) )
         return TW_CELL_TRAIL;
      return TW_CELL_ORPHAN;
   }
   // Wide lead: the right cell must be the trail of the same character
   if( tw_IsWideHead( pGT, bAttr, usChar ) )
   {
      if( HB_GTSELF_GETSCRCHAR( pGT, iRow, iCol + 1, &iColor2, &bAttr2, &usChar2 ) &&
          ( bAttr2 & TW_ATTR_TRAIL ) && usChar2 == usChar )
         return TW_CELL_LEAD;
      return TW_CELL_ORPHAN;
   }
   return TW_CELL_NORMAL;
}

// PutChar override: writing a wide lead fills the trail automatically; when it does not fit in the last column a blank is written; both neighbor pairs are checked afterwards
static HB_BOOL tw_PutChar( PHB_GT pGT, int iRow, int iCol, int iColor, HB_BYTE bAttr, HB_USHORT usChar )
{
   HB_BOOL fHead = tw_IsWideHead( pGT, bAttr, usChar );   // wide lead?

   // During RestScreen write as-is (flags come from the saved data); only check the left pair
   if( s_twInRest )
   {
      if( ! HB_GTSUPER->PutChar( pGT, iRow, iCol, iColor, bAttr, usChar ) )
         return HB_FALSE;
      tw_FixPair( pGT, iRow, iCol - 1 );                  // left pair
      if( ! fHead )
         tw_FixPair( pGT, iRow, iCol );                   // right pair (a lead waits for the trail written next)
      return HB_TRUE;
   }
   // Lead in the last column: does not fit, write a blank instead
   if( fHead && iCol == HB_GTSELF_MAXCOL( pGT ) )
   {
      usChar = ' ';                                       // pad with blank
      fHead = HB_FALSE;
   }
   if( ! HB_GTSUPER->PutChar( pGT, iRow, iCol, iColor, bAttr, usChar ) )
      return HB_FALSE;
   if( fHead )
   {
      // Automatic trail fill: same character with the trail flag
      HB_GTSUPER->PutChar( pGT, iRow, iCol + 1, iColor, TW_ATTR_TRAIL, usChar );
      tw_FixPair( pGT, iRow, iCol - 1 );                  // left pair
      tw_FixPair( pGT, iRow, iCol + 1 );                  // pair right of the trail
   }
   else
   {
      tw_FixPair( pGT, iRow, iCol - 1 );                  // left pair
      tw_FixPair( pGT, iRow, iCol );                      // right pair
   }
   return HB_TRUE;
}

// Write one character (shared): wide characters advance 2 cells; returns false when it cannot be written on screen
static HB_BOOL tw_PutOne( PHB_GT pGT, int iRow, int * piCol, int iColor, HB_WCHAR wc, HB_BOOL * pfStop )
{
   int n = tw_CharCells( pGT, ( HB_USHORT ) wc );         // cells for this character

   // Wide lead in the last column: pad with a blank and stop, the cursor becomes MaxCol+1
   if( n == 2 && *piCol == HB_GTSELF_MAXCOL( pGT ) )
   {
      HB_GTSELF_PUTCHAR( pGT, iRow, ( *piCol )++, iColor, 0, ' ' );
      *pfStop = HB_TRUE;
      return HB_TRUE;
   }
   if( ! HB_GTSELF_PUTCHAR( pGT, iRow, *piCol, iColor, 0, ( HB_USHORT ) wc ) )
   {
      *piCol += n;                                        // off screen: only accumulate columns
      return HB_FALSE;
   }
   // Wide characters write their own trail in the right cell (the hbct window PutChar replaces tw_PutChar and does not fill automatically)
   if( n == 2 )
      HB_GTSELF_PUTCHAR( pGT, iRow, *piCol + 1, iColor, TW_ATTR_TRAIL, ( HB_USHORT ) wc );   // trail flag
   *piCol += n;                                           // advance by cells
   return HB_TRUE;
}

// PutText override: write characters by cell width
static int tw_PutText( PHB_GT pGT, int iRow, int iCol, int iColor, const char * szText, HB_SIZE nLen )
{
   PHB_CODEPAGE cdp = HB_GTSELF_HOSTCP( pGT );
   HB_SIZE nIndex = 0;
   HB_WCHAR wc;
   HB_BOOL fStop = HB_FALSE;                              // right-edge stop flag

   // Decode each character and write it by cell width, stop at the right edge
   while( ! fStop && HB_CDPCHAR_GET( cdp, szText, nLen, &nIndex, &wc ) )
   {
      if( ! tw_PutOne( pGT, iRow, &iCol, iColor, wc, &fStop ) )
      {
         // Off screen: only accumulate columns, as the core does
         while( HB_CDPCHAR_GET( cdp, szText, nLen, &nIndex, &wc ) )
            iCol += tw_CharCells( pGT, ( HB_USHORT ) wc );
         break;
      }
   }
   return iCol;
}

// PutTextW override: wide-string version, writes characters by cell width
static int tw_PutTextW( PHB_GT pGT, int iRow, int iCol, int iColor, const HB_WCHAR * szText, HB_SIZE nLen )
{
   HB_SIZE n;
   HB_BOOL fStop = HB_FALSE;                              // right-edge stop flag

   // Write each character by cell width, stop at the right edge
   for( n = 0; n < nLen && ! fStop; ++n )
   {
      if( ! tw_PutOne( pGT, iRow, &iCol, iColor, szText[ n ], &fStop ) )
      {
         // Off screen: only accumulate columns
         while( ++n < nLen )
            iCol += tw_CharCells( pGT, ( HB_USHORT ) szText[ n ] );
         break;
      }
   }
   return iCol;
}

// WriteAtW override: no truncation by character count; PutTextW handles the right edge by cell width; the cursor goes at most to MaxCol+1
static void tw_WriteAtW( PHB_GT pGT, int iRow, int iCol, const HB_WCHAR * szText, HB_SIZE nLength )
{
   int iMaxCol = HB_GTSELF_MAXCOL( pGT );

   iCol = HB_GTSELF_PUTTEXTW( pGT, iRow, iCol, HB_GTSELF_GETCOLOR( pGT ), szText, nLength );
   // Same as WriteAt: the cursor does not go beyond MaxCol+1
   if( iCol > iMaxCol + 1 )
      iCol = iMaxCol + 1;
   HB_GTSELF_SETPOS( pGT, iRow, iCol );
}

#define TW_WRITECON_BUF  1024                             // WriteConW buffer size in characters

// WriteConW output step: flush the buffer, handle scrolling and the cursor (following the core flow)
static void tw_ConFlush( PHB_GT pGT, HB_WCHAR * szString, int * piLen, int * piRow, int * piCol,
                         int iMaxRow, int iMaxCol, HB_BOOL fNewLine, HB_BOOL * pfBell )
{
   if( *piLen )
      HB_GTSELF_WRITEW( pGT, szString, *piLen );          // flush the buffer
   *piLen = 0;
   if( *piRow > iMaxRow )
   {
      // Normal scroll
      HB_GTSELF_SCROLL( pGT, 0, 0, iMaxRow, iMaxCol, HB_GTSELF_GETCOLOR( pGT ),
                        HB_GTSELF_GETCLEARCHAR( pGT ), *piRow - iMaxRow, 0 );
      *piRow = iMaxRow;
      *piCol = 0;
   }
   else if( *piRow < 0 && fNewLine )
   {
      // Cursor above the top edge and another line feed
      HB_GTSELF_SCROLL( pGT, 0, 0, iMaxRow, iMaxCol, HB_GTSELF_GETCOLOR( pGT ),
                        HB_GTSELF_GETCLEARCHAR( pGT ), 1, 0 );
   }
   HB_GTSELF_SETPOS( pGT, *piRow, *piCol );
   HB_GTSELF_FLUSH( pGT );                                // emulate scrolling
   if( *pfBell )
   {
      HB_GTSELF_BELL( pGT );
      *pfBell = HB_FALSE;
   }
}

// WriteConW override: ? / QOut advance and wrap by cell width; a wide character that does not fit moves whole to the next row
static void tw_WriteConW( PHB_GT pGT, const HB_WCHAR * szText, HB_SIZE nLength )
{
   int iLen = 0, iRow, iCol, iMaxRow, iMaxCol, n;
   HB_BOOL fDisp = HB_FALSE, fBell = HB_FALSE, fNewLine = HB_FALSE;
   HB_WCHAR szString[ TW_WRITECON_BUF ];
   HB_SIZE nIndex = 0;

   iMaxRow = HB_GTSELF_MAXROW( pGT );
   iMaxCol = HB_GTSELF_MAXCOL( pGT );
   HB_GTSELF_GETPOS( pGT, &iRow, &iCol );
   // Clamp the start position to MaxRow, MaxCol (as the core does)
   if( iRow > iMaxRow || iCol > iMaxCol )
   {
      if( iRow > iMaxRow )
         iRow = iMaxRow;
      if( iCol > iMaxCol )
         iCol = iMaxCol;
      HB_GTSELF_SETPOS( pGT, iRow, iCol );
   }
   // Per character: control characters follow the core flow, other characters accumulate into the buffer by cell width
   while( nIndex < nLength )
   {
      HB_WCHAR wc = szText[ nIndex++ ];
      switch( wc )
      {
         // Bell: handled on output
         case HB_CHAR_BEL:
            fDisp = fBell = HB_TRUE;
            break;
         case HB_CHAR_BS:
            // Back one cell (backspace over a wide character clears only 1 cell; known limitation)
            if( iCol > 0 )
            {
               --iCol;
               fDisp = HB_TRUE;
            }
            else if( iCol == 0 && iRow > 0 )
            {
               iCol = iMaxCol;
               --iRow;
               fDisp = HB_TRUE;
            }
            // Overwrite the cell moved back over with a blank
            if( fDisp )
            {
               if( iLen )
                  szString[ iLen - 1 ] = ' ';
               else
               {
                  HB_GTSELF_SETPOS( pGT, iRow, iCol );
                  szString[ iLen++ ] = ' ';
               }
            }
            break;
         // Line feed: back to the left edge, next row
         case HB_CHAR_LF:
            iCol = 0;
            if( iRow >= 0 )
               ++iRow;
            fDisp = fNewLine = HB_TRUE;
            break;
         // Carriage return; CR followed by LF is treated as a line feed
         case HB_CHAR_CR:
            iCol = 0;
            if( nIndex < nLength && szText[ nIndex ] == HB_CHAR_LF )
            {
               if( iRow >= 0 )
                  ++iRow;
               fNewLine = HB_TRUE;
               ++nIndex;
            }
            fDisp = HB_TRUE;
            break;
         default:
            n = tw_CharCells( pGT, ( HB_USHORT ) wc );     // cells for this character
            // Wide character with only 1 cell left: pad the last column with a blank and wrap
            if( n == 2 && iCol == iMaxCol )
            {
               szString[ iLen++ ] = ' ';                   // pad with blank
               iCol = 0;
               if( iRow >= 0 )
                  ++iRow;
               tw_ConFlush( pGT, szString, &iLen, &iRow, &iCol, iMaxRow, iMaxCol, HB_TRUE, &fBell );
            }
            iCol += n;                                     // advance by cell width
            if( iCol > iMaxCol || iCol <= 0 )
            {
               // Exactly reached the last column (or the start was left of the edge): wrap after writing
               if( iCol > 0 )
                  szString[ iLen++ ] = wc;
               iCol = 0;
               if( iRow >= 0 )
                  ++iRow;
               fDisp = fNewLine = HB_TRUE;
            }
            else
               szString[ iLen++ ] = wc;
            if( iLen >= TW_WRITECON_BUF )
               fDisp = HB_TRUE;                            // buffer full
      }
      // Output needed or end of string: flush the buffer and handle scrolling
      if( fDisp || nIndex == nLength )
      {
         tw_ConFlush( pGT, szString, &iLen, &iRow, &iCol, iMaxRow, iMaxCol, fNewLine, &fBell );
         fDisp = fNewLine = HB_FALSE;
      }
   }
}

// WriteCon override: convert to a wide string with the host codepage first, then pass it to WriteConW
static void tw_WriteCon( PHB_GT pGT, const char * szText, HB_SIZE nLength )
{
   PHB_CODEPAGE cdp = HB_GTSELF_HOSTCP( pGT );
   HB_SIZE nIndex = 0, nW = 0;
   HB_WCHAR wc;
   HB_WCHAR * pW = ( HB_WCHAR * ) hb_xgrab( ( nLength + 1 ) * sizeof( HB_WCHAR ) );   // the character count never exceeds the byte count

   // Decode each character into a wide character
   while( HB_CDPCHAR_GET( cdp, szText, nLength, &nIndex, &wc ) )
      pW[ nW++ ] = wc;
   if( nW )
      tw_WriteConW( pGT, pW, nW );
   hb_xfree( pW );
}

// Replicate override: one wide character every 2 cells, a remaining single cell is padded with a blank (DispBox characters carry the box flag and never reach the wide branch)
static void tw_Replicate( PHB_GT pGT, int iRow, int iCol, int iColor, HB_BYTE bAttr, HB_USHORT usChar, HB_SIZE nLength )
{
   // Normal characters as before
   if( ! tw_IsWideHead( pGT, bAttr, usChar ) )
   {
      HB_GTSUPER->Replicate( pGT, iRow, iCol, iColor, bAttr, usChar, nLength );
      tw_FixPair( pGT, iRow, iCol - 1 );                  // left edge pair
      tw_FixPair( pGT, iRow, iCol + ( int ) nLength - 1 ); // right edge pair
      return;
   }
   // Wide characters: one lead every 2 cells, the trail is written explicitly (the hbct window PutChar does not go through tw_PutChar)
   while( nLength >= 2 )
   {
      // Lead in the last column: does not fit, pad with a blank and stop (same as tw_PutOne)
      if( iCol == HB_GTSELF_MAXCOL( pGT ) )
      {
         HB_GTSELF_PUTCHAR( pGT, iRow, iCol, iColor, 0, ' ' );                  // pad with blank
         return;
      }
      HB_GTSELF_PUTCHAR( pGT, iRow, iCol, iColor, bAttr, usChar );              // lead
      HB_GTSELF_PUTCHAR( pGT, iRow, iCol + 1, iColor, TW_ATTR_TRAIL, usChar );  // trail flag
      iCol += 2;
      nLength -= 2;
   }
   if( nLength )
      HB_GTSELF_PUTCHAR( pGT, iRow, iCol, iColor, 0, ' ' );          // pad the remaining 1 cell with a blank
}

// Rest override: restore as before (flags as-is), then clear orphan half cells at the left/right edges of each row
static void tw_Rest( PHB_GT pGT, int iTop, int iLeft, int iBottom, int iRight, const void * pBuffer )
{
   int iRow;
   HB_BOOL fOld = s_twInRest;                            // nesting guard

   s_twInRest = HB_TRUE;                                  // no automatic trail fill while restoring
   HB_GTSUPER->Rest( pGT, iTop, iLeft, iBottom, iRight, pBuffer );
   s_twInRest = fOld;
   // Check orphan half cells at the left/right edges of each row
   for( iRow = iTop; iRow <= iBottom; ++iRow )
   {
      tw_FixPair( pGT, iRow, iLeft - 1 );                 // left edge pair
      tw_FixPair( pGT, iRow, iRight );                    // right edge pair
   }
}

// Scroll override: scroll as before (internally via tw_Rest), then check the left/right edges of each row
static void tw_Scroll( PHB_GT pGT, int iTop, int iLeft, int iBottom, int iRight,
                       int iColor, HB_USHORT usChar, int iRows, int iCols )
{
   int iRow;

   HB_GTSUPER->Scroll( pGT, iTop, iLeft, iBottom, iRight, iColor, usChar, iRows, iCols );
   // Check orphan half cells at the left/right edges of each row
   for( iRow = iTop; iRow <= iBottom; ++iRow )
   {
      tw_FixPair( pGT, iRow, iLeft - 1 );                 // left edge pair
      tw_FixPair( pGT, iRow, iRight );                    // right edge pair
   }
}

// Wide-character blank used by Alert (same as s_szSpaceW in hbgtcore.c, renamed to avoid clashing with symbols in the GT file)
static const HB_WCHAR s_twSpaceW[] = { ' ', 0 };

// Number of cells taken by the first nLen characters of a wide string
static HB_SIZE tw_StrCells( PHB_GT pGT, const HB_WCHAR * szText, HB_SIZE nLen )
{
   HB_SIZE n, nCells = 0;

   // Sum cells per character
   for( n = 0; n < nLen; ++n )
      nCells += ( HB_SIZE ) tw_CharCells( pGT, ( HB_USHORT ) szText[ n ] );
   return nCells;
}

// Among the first nLen characters, how many fit within nMax cells; *pnCells returns the cells of those characters
static HB_SIZE tw_FitChars( PHB_GT pGT, const HB_WCHAR * szText, HB_SIZE nLen, HB_SIZE nMax, HB_SIZE * pnCells )
{
   HB_SIZE n, nCells = 0;

   // Sum per character, stop when it does not fit (never split a Chinese character)
   for( n = 0; n < nLen; ++n )
   {
      int iC = tw_CharCells( pGT, ( HB_USHORT ) szText[ n ] );   // cells for this character
      if( nCells + iC > nMax )
         break;
      nCells += iC;
   }
   *pnCells = nCells;
   return n;
}

// Cells taken by the text of button i
static HB_SIZE tw_OptCells( PHB_GT pGT, PHB_ITEM pOptions, int i )
{
   void * hOpt;
   HB_SIZE nLen, nCells;
   const HB_WCHAR * szOptW = hb_arrayGetStrU16( pOptions, i, HB_CDP_ENDIAN_NATIVE, &hOpt, &nLen );

   nCells = szOptW ? tw_StrCells( pGT, szOptW, nLen ) : 0;   // non-strings count as 0 cells
   hb_strfree( hOpt );
   return nCells;
}

// Alert override: copied from hb_gt_def_Alert in harbour-core src/rtl/hbgtcore.c (lines 2016-2377),
// changing only "one character = 1 cell" to tw_CharCells cells (wrapping, button width, centering, button positions, mouse areas); non-fullscreen mode is passed to the super
static int tw_Alert( PHB_GT pGT, PHB_ITEM pMessage, PHB_ITEM pOptions,
                     int iClrNorm, int iClrHigh, double dDelay )
{
   int iRet = 0, iOptions;

   if( pMessage && HB_IS_HASH( pMessage ) )
   {
      if( ! pOptions || hb_arrayLen( pOptions ) == 0 )
         pOptions = hb_hashGetCItemPtr( pMessage, "BTN" );
      if( dDelay <= 0 )
      {
         PHB_ITEM pVal = hb_hashGetCItemPtr( pMessage, "TIM" );
         if( pVal && HB_IS_NUMERIC( pVal ) )
           dDelay = hb_itemGetND( pVal );
      }
      pMessage = hb_hashGetCItemPtr( pMessage, "TXT" );
   }

   if( pMessage && HB_IS_STRING( pMessage ) &&
       pOptions && ( iOptions = ( int ) hb_arrayLen( pOptions ) ) > 0 )
   {
      HB_SIZE nLen;
      void * hMessage;
      const HB_WCHAR * szMessageW = hb_itemGetStrU16( pMessage, HB_CDP_ENDIAN_NATIVE, &hMessage, &nLen );
      HB_BOOL fScreen = HB_FALSE, fKeyBoard = HB_FALSE;
      PHB_CODEPAGE cdp = HB_GTSELF_HOSTCP( pGT );
      char szKey[ HB_MAX_CHAR_LEN ];
      HB_SIZE nChar;
      int iKey, i, iRows, iCols;
      HB_GT_INFO gtInfo;

      memset( &gtInfo, 0, sizeof( gtInfo ) );

      HB_GTSELF_INFO( pGT, HB_GTI_ISSCREENPOS, &gtInfo );
      if( gtInfo.pResult )
      {
         fScreen = hb_itemGetL( gtInfo.pResult );
      }
      HB_GTSELF_INFO( pGT, HB_GTI_KBDSUPPORT, &gtInfo );
      if( gtInfo.pResult )
      {
         fKeyBoard = hb_itemGetL( gtInfo.pResult );
         hb_itemRelease( gtInfo.pResult );
      }
      HB_GTSELF_GETSIZE( pGT, &iRows, &iCols );
      if( iCols <= 4 || iRows <= 4 )
         fScreen = HB_FALSE;

      if( fScreen )
      {
         void * pBuffer = NULL;
         int iDspCount, iStyle, iRow, iCol, iTop, iLeft, iBottom, iRight, iPos;
         HB_UINT ulLines = 0, ulWidth = 0, ulCurrWidth = 0, ulMsg = 0, ulDst = 0,
                 ulLast = 0, ulSpace1 = 0, ulSpace2 = 0, ulDefWidth, ulMaxWidth;
         HB_WCHAR * szMsgDsp;
         int iFlag;

         ulMaxWidth = iCols - 4;
         ulDefWidth = ( ulMaxWidth * 3 ) >> 2;
         if( ulDefWidth == 0 )
            ulDefWidth = 1;
         // Wrapping by cells can give fewer characters per line; there are at most nLen line breaks, so the buffer is doubled
         szMsgDsp = ( HB_WCHAR * ) hb_xgrab( ( nLen * 2 + 1 ) * sizeof( HB_WCHAR ) );

         while( ulMsg < nLen )
         {
            if( szMessageW[ ulMsg ] == '\n' )
            {
               if( ulCurrWidth > ulMaxWidth )
               {
                  ulDst = ulLast;
               }
               else
               {
                  ++ulLines;
                  if( ulCurrWidth > ulWidth )
                     ulWidth = ulCurrWidth;
                  ulCurrWidth = ulSpace1 = ulSpace2 = 0;
                  szMsgDsp[ ulDst++ ] = '\n';
                  ulLast = ulDst;
               }
            }
            else
            {
               if( szMessageW[ ulMsg ] == ' ' )
               {
                  if( ulCurrWidth <= ulDefWidth )
                     ulSpace1 = ulMsg;
                  else if( ulCurrWidth <= ulMaxWidth && ! ulSpace2 )
                     ulSpace2 = ulMsg;
               }
               szMsgDsp[ ulDst++ ] = szMessageW[ ulMsg ];
               // Sum by cells (Chinese = 2 cells)
               ulCurrWidth += ( HB_UINT ) tw_CharCells( pGT, ( HB_USHORT ) szMessageW[ ulMsg ] );
               if( ulCurrWidth > ulDefWidth && ulSpace1 )
               {
                  // Back up to a blank: subtract the cells of the characters backed over
                  ulCurrWidth -= ( HB_UINT ) tw_StrCells( pGT, szMessageW + ulSpace1, ulMsg - ulSpace1 + 1 ); /* explicit cast: avoids MSVC x64 C4244 */
                  ulDst -= ulMsg - ulSpace1 + 1;
                  ulMsg = ulSpace1;
                  ++ulLines;
                  if( ulCurrWidth > ulWidth )
                     ulWidth = ulCurrWidth;
                  ulCurrWidth = ulSpace1 = ulSpace2 = 0;
                  szMsgDsp[ ulDst++ ] = '\n';
                  ulLast = ulDst;
               }
               else if( ulCurrWidth > ulMaxWidth )
               {
                  if( ulSpace2 )
                  {
                     // Back up to a blank: subtract the cells of the characters backed over
                     ulCurrWidth -= ( HB_UINT ) tw_StrCells( pGT, szMessageW + ulSpace2, ulMsg - ulSpace2 + 1 ); /* explicit cast: avoids MSVC x64 C4244 */
                     ulDst -= ulMsg - ulSpace2 + 1;
                     ulMsg = ulSpace2;
                     ++ulLines;
                     if( ulCurrWidth > ulWidth )
                        ulWidth = ulCurrWidth;
                     ulCurrWidth = ulSpace1 = ulSpace2 = 0;
                     szMsgDsp[ ulDst++ ] = '\n';
                     ulLast = ulDst;
                  }
#ifndef HB_CLP_STRICT
                  // Hard break: move the whole character to the next line (never split a Chinese character); do not break when it is the only character on the line, to avoid an endless loop
                  else if( ulCurrWidth > ( HB_UINT ) tw_CharCells( pGT, ( HB_USHORT ) szMessageW[ ulMsg ] ) )
                  {
                     // Subtract the cells of this character
                     ulCurrWidth -= ( HB_UINT ) tw_CharCells( pGT, ( HB_USHORT ) szMessageW[ ulMsg ] );
                     ulDst--;
                     ulMsg--;
                     szMsgDsp[ ulDst++ ] = '\n';
                     ulLast = ulDst;
                     ++ulLines;
                     if( ulCurrWidth > ulWidth )
                        ulWidth = ulCurrWidth;
                     ulCurrWidth = ulSpace1 = ulSpace2 = 0;
                  }
#endif
               }
            }
            ++ulMsg;
         }
         ulLines++;
         if( ulCurrWidth > ulWidth )
            ulWidth = ulCurrWidth;
         if( ulLines == 1 && ulWidth < ulDefWidth )
            ulWidth += HB_MIN( 4, ulDefWidth - ulWidth );

         ulCurrWidth = 0;
         for( i = 1; i <= iOptions; ++i )
         {
            // Button width = cells of the button text + 4
            ulCurrWidth += ( HB_UINT ) tw_OptCells( pGT, pOptions, i ) + 4;
         }
         if( ulCurrWidth > ulMaxWidth )
            ulCurrWidth = ulMaxWidth;
         if( ulCurrWidth > ulWidth )
            ulWidth = ulCurrWidth;
         if( ( HB_SIZE ) iRows < ulLines + 4 )
            ulLines = iRows - 4;
         iTop = ( iRows - ulLines - 4 ) >> 1;
         iLeft = ( iCols - ulWidth - 4 ) >> 1;
         iBottom = iTop + ulLines + 3;
         iRight = iLeft + ulWidth + 3;

         if( iClrNorm == 0 )
            iClrNorm = 0x4F;
         if( iClrHigh == 0 )
            iClrHigh = 0x1F;
         iDspCount = HB_GTSELF_DISPCOUNT( pGT );
         if( iDspCount == 0 )
            HB_GTSELF_DISPBEGIN( pGT );
         HB_GTSELF_GETPOS( pGT, &iRow, &iCol );
         iStyle = HB_GTSELF_GETCURSORSTYLE( pGT );
         HB_GTSELF_SETCURSORSTYLE( pGT, SC_NONE );
         iFlag = HB_GTSELF_SETFLAG( pGT, HB_GTI_COMPATBUFFER, 0 );
         nLen = HB_GTSELF_RECTSIZE( pGT, iTop, iLeft, iBottom, iRight );
         if( nLen )
         {
            pBuffer = hb_xgrab( nLen );
            HB_GTSELF_SAVE( pGT, iTop, iLeft, iBottom, iRight, pBuffer );
         }
         HB_GTSELF_BOXS( pGT, iTop, iLeft, iBottom, iRight, NULL, iClrNorm );
         HB_GTSELF_BOX( pGT, iTop + 1, iLeft + 1, iBottom - 1, iRight - 1, NULL, iClrNorm );
         ulLast = 0;
         i = iTop + 1;
         for( ulMsg = 0; ulMsg < ulDst; ++ulMsg )
         {
            if( szMsgDsp[ ulMsg ] == '\n' )
            {
               if( ulMsg > ulLast )
               {
                  HB_SIZE nCells;
                  nLen = ulMsg - ulLast;
                  // Center by cells; when wider than the box take only the whole characters that fit
                  nLen = tw_FitChars( pGT, szMsgDsp + ulLast, nLen, ulWidth, &nCells );
                  HB_GTSELF_PUTTEXTW( pGT, i, iLeft + ( int ) ( ( ulWidth - nCells + 1 ) >> 1 ) + 2,
                                      iClrNorm, szMsgDsp + ulLast, nLen );
               }
               ulLast = ulMsg + 1;
               if( ++i >= iBottom - 1 )
                  break;
            }
         }
         if( ulMsg > ulLast && i < iBottom - 1 )
         {
            HB_SIZE nCells;
            nLen = ulMsg - ulLast;
            // Center by cells; when wider than the box take only the whole characters that fit
            nLen = tw_FitChars( pGT, szMsgDsp + ulLast, nLen, ulWidth, &nCells );
            HB_GTSELF_PUTTEXTW( pGT, i, iLeft + ( int ) ( ( ulWidth - nCells + 1 ) >> 1 ) + 2,
                                iClrNorm, szMsgDsp + ulLast, nLen );
         }
         hb_xfree( szMsgDsp );

         iPos = 1;
         while( iRet == 0 )
         {
            int iMnuCol;
            HB_GTSELF_DISPBEGIN( pGT );
            iMnuCol = iLeft + ( ( ulWidth - ulCurrWidth ) >> 1 ) + 3;
            for( i = 1; i <= iOptions; ++i )
            {
               void * hOpt;
               const HB_WCHAR * szOptW;
               int iClr = i == iPos ? iClrHigh : iClrNorm;
               szOptW = hb_arrayGetStrU16( pOptions, i, HB_CDP_ENDIAN_NATIVE, &hOpt, &nLen );
               HB_GTSELF_PUTTEXTW( pGT, iBottom - 1, iMnuCol, iClr, s_twSpaceW, 1 );
               HB_GTSELF_PUTTEXTW( pGT, iBottom - 1, iMnuCol + 1, iClr, szOptW, nLen );
               // The right blank and the next button start use cells
               HB_GTSELF_PUTTEXTW( pGT, iBottom - 1, iMnuCol + 1 + ( int ) tw_StrCells( pGT, szOptW, nLen ), iClr, s_twSpaceW, 1 );
               iMnuCol += ( int ) tw_StrCells( pGT, szOptW, nLen ) + 4;
               hb_strfree( hOpt );
            }
            while( HB_GTSELF_DISPCOUNT( pGT ) )
               HB_GTSELF_DISPEND( pGT );
            HB_GTSELF_REFRESH( pGT );

            iKey = fKeyBoard ? HB_GTSELF_INKEYGET( pGT, HB_TRUE, dDelay, INKEY_ALL ) : 0;

            if( iKey == K_ESC )
               break;
            else if( iKey == K_ENTER || iKey == K_SPACE || iKey == 0 )
            {
               iRet = iPos;
            }
            else if( iKey == K_LEFT || iKey == K_SH_TAB )
            {
               if( --iPos == 0 )
                  iPos = iOptions;
               dDelay = 0.0;
            }
            else if( iKey == K_RIGHT || iKey == K_TAB )
            {
               if( ++iPos > iOptions )
                  iPos = 1;
               dDelay = 0.0;
            }
#ifdef HB_COMPAT_C53
            else if( iKey == K_LBUTTONDOWN )
            {
               int iMRow, iMCol;
               HB_GTSELF_MOUSEGETPOS( pGT, &iMRow, &iMCol );
               if( iMRow == iBottom - 1 )
               {
                  iMnuCol = iLeft + ( ( ulWidth - ulCurrWidth ) >> 1 ) + 4;
                  for( i = 1; i <= iOptions; ++i )
                  {
                     // The mouse area uses the cells of the button text
                     nLen = tw_OptCells( pGT, pOptions, i );
                     if( iMCol >= iMnuCol && iMCol < iMnuCol + ( int ) nLen )
                     {
                        iRet = i;
                        break;
                     }
                     iMnuCol += ( int ) nLen + 4;
                  }
               }
            }
#endif
            else if( ( nChar = hb_inkeyKeyString( iKey, szKey, sizeof( szKey ) ) ) > 0 )
            {
               for( i = 1; i <= iOptions; ++i )
               {
                  nLen = hb_arrayGetCLen( pOptions, i );
                  if( nLen > 0 )
                  {
                     HB_SIZE nIdx1 = 0, nIdx2 = 0;
                     if( hb_cdpCharCaseEq( cdp, szKey, nChar, &nIdx1,
                           hb_arrayGetCPtr( pOptions, i ), nLen, &nIdx2 ) )
                     {
                        iRet = i;
                        break;
                     }
                  }
               }
            }
         }

         if( pBuffer )
         {
            HB_GTSELF_REST( pGT, iTop, iLeft, iBottom, iRight, pBuffer );
            hb_xfree( pBuffer );
         }
         if( iFlag != 0 )
            HB_GTSELF_SETFLAG( pGT, HB_GTI_COMPATBUFFER, iFlag );
         HB_GTSELF_SETPOS( pGT, iRow, iCol );
         HB_GTSELF_SETCURSORSTYLE( pGT, iStyle );
         HB_GTSELF_REFRESH( pGT );
         while( HB_GTSELF_DISPCOUNT( pGT ) < iDspCount )
            HB_GTSELF_DISPBEGIN( pGT );
      }
      // Non-fullscreen mode: not copied from the original; pass to the super's original Alert
      else
         iRet = HB_GTSUPER->Alert( pGT, pMessage, pOptions, iClrNorm, iClrHigh, dDelay );
      hb_strfree( hMessage );
   }

   return iRet;
}

// Install the overrides (called from each GT's FuncInit)
static void tw_WideInit( PHB_GT_FUNCS pFuncTable )
{
   pFuncTable->PutChar   = tw_PutChar;                    // automatic trail fill, clear orphans
   pFuncTable->PutText   = tw_PutText;                    // by cell width
   pFuncTable->PutTextW  = tw_PutTextW;                   // wide-string version
   pFuncTable->WriteAtW  = tw_WriteAtW;                   // no truncation by character count
   pFuncTable->WriteCon  = tw_WriteCon;                   // ? / QOut
   pFuncTable->WriteConW = tw_WriteConW;                  // wide-string version
   pFuncTable->Scroll    = tw_Scroll;                     // clear orphan halves at the edges
   pFuncTable->Replicate = tw_Replicate;                  // wide-character fill
   pFuncTable->Rest      = tw_Rest;                       // flags as-is, clear orphan halves at the edges
   pFuncTable->Alert     = tw_Alert;                      // Alert laid out by cells
}
