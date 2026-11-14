/* twambig.c
 * get-tw shared East-Asian / ambiguous cell-width (D9-C)
 * Copyright 2026 Wen Sheng
 *
 * GNU GPL v2 or later, with the Harbour / get-tw linking exception
 * as given in twwide.c. This file is part of get-tw.
 *
 * Modified by Wen Sheng, 2026-11-13: new shared width table. s_iAmbigW defaults to 2; the optional environment variable GETTW_AMBIGW accepts only 1 or 2;
 * range (1): EAW=A, 191 ranges, 902 code points in total (combining marks, variation selectors, PUA and U+FFFD excluded);
 * PUA is fixed at 2 cells (option 2, does not follow TwAmbigW); the 0x20000 block is added.
 * This .c is compiled only once (twwide.c is #included into both GTs, so the static table cannot live in twwide.c).
 */

#include "hbapi.h"
#include "hbapicdp.h"
#include "twambig.h"

/* Modified by Wen Sheng, 2026-11-13: D9-C ambiguous width defaults to 2 cells. */
static int s_iAmbigW  = 2;
static int s_fEnvDone = 0;

/* Modified by Wen Sheng, 2026-11-13: the environment variable is read once on first use; only "1" or "2" is accepted; a later setting by the program overrides the environment value. */
static void tw_AmbigW_ReadEnv( void )
{
   char * psz;

   if( s_fEnvDone )
      return;
   s_fEnvDone = 1;

   psz = hb_getenv( "GETTW_AMBIGW" );
   if( psz )
   {
      if( psz[ 0 ] == '1' && psz[ 1 ] == '\0' )
         s_iAmbigW = 1;
      else if( psz[ 0 ] == '2' && psz[ 1 ] == '\0' )
         s_iAmbigW = 2;
      hb_xfree( psz );
   }
}

int tw_AmbigW( int iNew )
{
   int iOld;

   tw_AmbigW_ReadEnv();
   iOld = s_iAmbigW;
   if( iNew == 1 || iNew == 2 )
      s_iAmbigW = iNew;
   return iOld;
}

HB_FUNC( TWAMBIGW )
{
   int iNew = 0;

   if( hb_pcount() >= 1 && HB_ISNUM( 1 ) )
      iNew = hb_parni( 1 );
   hb_retni( tw_AmbigW( iNew ) );
}

typedef struct
{
   HB_U32 lo;
   HB_U32 hi;
} TW_AMBIG_RANGE;

/* Modified by Wen Sheng, 2026-11-13: range (1) Unicode 16.0 EAW=A, excluding combining marks, variation selectors, PUA and U+FFFD. Returns 1 or 2 cells according to s_iAmbigW. */
static const TW_AMBIG_RANGE s_aAmbigA[] =
{
   { 0x00A1, 0x00A1 },
   { 0x00A4, 0x00A4 },
   { 0x00A7, 0x00A7 },
   { 0x00A8, 0x00A8 },
   { 0x00AA, 0x00AA },
   { 0x00AD, 0x00AD },
   { 0x00AE, 0x00AE },
   { 0x00B0, 0x00B0 },
   { 0x00B1, 0x00B1 },
   { 0x00B2, 0x00B3 },
   { 0x00B4, 0x00B4 },
   { 0x00B6, 0x00B7 },
   { 0x00B8, 0x00B8 },
   { 0x00B9, 0x00B9 },
   { 0x00BA, 0x00BA },
   { 0x00BC, 0x00BE },
   { 0x00BF, 0x00BF },
   { 0x00C6, 0x00C6 },
   { 0x00D0, 0x00D0 },
   { 0x00D7, 0x00D7 },
   { 0x00D8, 0x00D8 },
   { 0x00DE, 0x00E1 },
   { 0x00E6, 0x00E6 },
   { 0x00E8, 0x00EA },
   { 0x00EC, 0x00ED },
   { 0x00F0, 0x00F0 },
   { 0x00F2, 0x00F3 },
   { 0x00F7, 0x00F7 },
   { 0x00F8, 0x00FA },
   { 0x00FC, 0x00FC },
   { 0x00FE, 0x00FE },
   { 0x0101, 0x0101 },
   { 0x0111, 0x0111 },
   { 0x0113, 0x0113 },
   { 0x011B, 0x011B },
   { 0x0126, 0x0127 },
   { 0x012B, 0x012B },
   { 0x0131, 0x0133 },
   { 0x0138, 0x0138 },
   { 0x013F, 0x0142 },
   { 0x0144, 0x0144 },
   { 0x0148, 0x014B },
   { 0x014D, 0x014D },
   { 0x0152, 0x0153 },
   { 0x0166, 0x0167 },
   { 0x016B, 0x016B },
   { 0x01CE, 0x01CE },
   { 0x01D0, 0x01D0 },
   { 0x01D2, 0x01D2 },
   { 0x01D4, 0x01D4 },
   { 0x01D6, 0x01D6 },
   { 0x01D8, 0x01D8 },
   { 0x01DA, 0x01DA },
   { 0x01DC, 0x01DC },
   { 0x0251, 0x0251 },
   { 0x0261, 0x0261 },
   { 0x02C4, 0x02C4 },
   { 0x02C7, 0x02C7 },
   { 0x02C9, 0x02CB },
   { 0x02CD, 0x02CD },
   { 0x02D0, 0x02D0 },
   { 0x02D8, 0x02DB },
   { 0x02DD, 0x02DD },
   { 0x02DF, 0x02DF },
   { 0x0391, 0x03A1 },
   { 0x03A3, 0x03A9 },
   { 0x03B1, 0x03C1 },
   { 0x03C3, 0x03C9 },
   { 0x0401, 0x0401 },
   { 0x0410, 0x044F },
   { 0x0451, 0x0451 },
   { 0x2010, 0x2010 },
   { 0x2013, 0x2015 },
   { 0x2016, 0x2016 },
   { 0x2018, 0x2018 },
   { 0x2019, 0x2019 },
   { 0x201C, 0x201C },
   { 0x201D, 0x201D },
   { 0x2020, 0x2022 },
   { 0x2024, 0x2027 },
   { 0x2030, 0x2030 },
   { 0x2032, 0x2033 },
   { 0x2035, 0x2035 },
   { 0x203B, 0x203B },
   { 0x203E, 0x203E },
   { 0x2074, 0x2074 },
   { 0x207F, 0x207F },
   { 0x2081, 0x2084 },
   { 0x20AC, 0x20AC },
   { 0x2103, 0x2103 },
   { 0x2105, 0x2105 },
   { 0x2109, 0x2109 },
   { 0x2113, 0x2113 },
   { 0x2116, 0x2116 },
   { 0x2121, 0x2122 },
   { 0x2126, 0x2126 },
   { 0x212B, 0x212B },
   { 0x2153, 0x2154 },
   { 0x215B, 0x215E },
   { 0x2160, 0x216B },
   { 0x2170, 0x2179 },
   { 0x2189, 0x2189 },
   { 0x2190, 0x2194 },
   { 0x2195, 0x2199 },
   { 0x21B8, 0x21B9 },
   { 0x21D2, 0x21D2 },
   { 0x21D4, 0x21D4 },
   { 0x21E7, 0x21E7 },
   { 0x2200, 0x2200 },
   { 0x2202, 0x2203 },
   { 0x2207, 0x2208 },
   { 0x220B, 0x220B },
   { 0x220F, 0x220F },
   { 0x2211, 0x2211 },
   { 0x2215, 0x2215 },
   { 0x221A, 0x221A },
   { 0x221D, 0x2220 },
   { 0x2223, 0x2223 },
   { 0x2225, 0x2225 },
   { 0x2227, 0x222C },
   { 0x222E, 0x222E },
   { 0x2234, 0x2237 },
   { 0x223C, 0x223D },
   { 0x2248, 0x2248 },
   { 0x224C, 0x224C },
   { 0x2252, 0x2252 },
   { 0x2260, 0x2261 },
   { 0x2264, 0x2267 },
   { 0x226A, 0x226B },
   { 0x226E, 0x226F },
   { 0x2282, 0x2283 },
   { 0x2286, 0x2287 },
   { 0x2295, 0x2295 },
   { 0x2299, 0x2299 },
   { 0x22A5, 0x22A5 },
   { 0x22BF, 0x22BF },
   { 0x2312, 0x2312 },
   { 0x2460, 0x249B },
   { 0x249C, 0x24E9 },
   { 0x24EB, 0x24FF },
   { 0x2500, 0x254B },
   { 0x2550, 0x2573 },
   { 0x2580, 0x258F },
   { 0x2592, 0x2595 },
   { 0x25A0, 0x25A1 },
   { 0x25A3, 0x25A9 },
   { 0x25B2, 0x25B3 },
   { 0x25B6, 0x25B6 },
   { 0x25B7, 0x25B7 },
   { 0x25BC, 0x25BD },
   { 0x25C0, 0x25C0 },
   { 0x25C1, 0x25C1 },
   { 0x25C6, 0x25C8 },
   { 0x25CB, 0x25CB },
   { 0x25CE, 0x25D1 },
   { 0x25E2, 0x25E5 },
   { 0x25EF, 0x25EF },
   { 0x2605, 0x2606 },
   { 0x2609, 0x2609 },
   { 0x260E, 0x260F },
   { 0x261C, 0x261C },
   { 0x261E, 0x261E },
   { 0x2640, 0x2640 },
   { 0x2642, 0x2642 },
   { 0x2660, 0x2661 },
   { 0x2663, 0x2665 },
   { 0x2667, 0x266A },
   { 0x266C, 0x266D },
   { 0x266F, 0x266F },
   { 0x269E, 0x269F },
   { 0x26BF, 0x26BF },
   { 0x26C6, 0x26CD },
   { 0x26CF, 0x26D3 },
   { 0x26D5, 0x26E1 },
   { 0x26E3, 0x26E3 },
   { 0x26E8, 0x26E9 },
   { 0x26EB, 0x26F1 },
   { 0x26F4, 0x26F4 },
   { 0x26F6, 0x26F9 },
   { 0x26FB, 0x26FC },
   { 0x26FE, 0x26FF },
   { 0x273D, 0x273D },
   { 0x2776, 0x277F },
   { 0x2B56, 0x2B59 },
   { 0x3248, 0x324F },
   { 0x1F100, 0x1F10A },
   { 0x1F110, 0x1F12D },
   { 0x1F130, 0x1F169 },
   { 0x1F170, 0x1F18D },
   { 0x1F18F, 0x1F190 },
   { 0x1F19B, 0x1F1AC }
};

static int tw_InAmbigA( HB_U32 nCode )
{
   HB_SIZE nLo = 0;
   HB_SIZE nHi = HB_SIZEOFARRAY( s_aAmbigA );
   while( nLo < nHi )
   {
      HB_SIZE nMid = nLo + ( nHi - nLo ) / 2;
      if( nCode < s_aAmbigA[ nMid ].lo )
         nHi = nMid;
      else if( nCode > s_aAmbigA[ nMid ].hi )
         nLo = nMid + 1;
      else
         return 1;
   }
   return 0;
}

/* Modified by Wen Sheng, 2026-11-13: fixed 2 cells: the existing East Asian wide ranges plus the range from 0x20000 (previously missing); PUA U+E000-F8FF is always 2 cells under option 2 and does not follow the ambiguous-width switch. */
static int tw_AlwaysWide( HB_U32 nCode )
{
   if( ( nCode >= 0x1100 && nCode <= 0x115F ) ||
       ( nCode >= 0x2E80 && nCode <= 0xA4CF ) ||
       ( nCode >= 0xAC00 && nCode <= 0xD7A3 ) ||
       ( nCode >= 0xF900 && nCode <= 0xFAFF ) ||
       ( nCode >= 0xFE30 && nCode <= 0xFE4F ) ||
       ( nCode >= 0xFF00 && nCode <= 0xFF60 ) ||
       ( nCode >= 0xFFE0 && nCode <= 0xFFE6 ) ||
       ( nCode >= 0xE000 && nCode <= 0xF8FF ) ||
       nCode >= 0x20000 )
      return 1;
   return 0;
}

int tw_CodePointCells( HB_U32 nCode )
{
   tw_AmbigW_ReadEnv();

   if( nCode < 0x80 )
      return 1;
   if( tw_AlwaysWide( nCode ) )
      return 2;
   if( tw_InAmbigA( nCode ) )
      return s_iAmbigW;
   return 1;
}

HB_FUNC( TWCODEPOINTCELLS )
{
   hb_retni( tw_CodePointCells( ( HB_U32 ) hb_parnl( 1 ) ) );
}
