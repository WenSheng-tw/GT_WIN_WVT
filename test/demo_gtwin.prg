// Manual demo: CONFIRM, nMaxChars, date, numeric and CT window GETs
// Usage: demo_gtwin|demo_gtwvt <codepage CP950|UTF8> <CONFIRM ON|OFF> [AUTO]
// Defaults: UTF8 / OFF; 3rd argument AUTO = send keys automatically (off by default)
// The form uses standard @ SAY GET + READ (pre-painted); character indexing on; WVT_TW uses the MingLiU font

#include "inkey.ch"
#include "set.ch"
#include "hbgtinfo.ch"

REQUEST HB_CODEPAGE_CP950                       // UTF8 is built in and needs no REQUEST
REQUEST HB_GT_WIN_TW_DEFAULT

// Entry point
PROCEDURE Main( cCP, cConfirm, cAuto )

   LOCAL cGt, lConfirm, lAuto, cScr
   LOCAL cName := Space( 10 )                   // 1 Name: width 10, no nMaxChars
   LOCAL cMax  := Space( 10 )                   // 2 nMaxChars=6, width 10
   LOCAL cAsc  := Space( 10 )                   // 3 letters and digits only
   LOCAL dBirth := CToD( "" )                   // 4 date
   LOCAL nAmt  := 0                             // 5 numeric
   LOCAL cWin  := Space( 10 )                   // GET inside a CT window
   LOCAL cLabelName, cHint

   hb_default( @cCP, "UTF8" )
   hb_default( @cConfirm, "OFF" )
   hb_default( @cAuto, "" )
   cCP := Upper( AllTrim( cCP ) )
   cConfirm := Upper( AllTrim( cConfirm ) )
   cAuto := Upper( AllTrim( cAuto ) )
   IF ! ( cCP == "CP950" .OR. cCP == "UTF8" )
      cCP := "UTF8"
   ENDIF
   IF ! ( cConfirm == "ON" .OR. cConfirm == "OFF" )
      cConfirm := "OFF"
   ENDIF
   lAuto := ( cAuto == "AUTO" )
   IF ! hb_cdpExists( cCP )
      QOut( "ERROR unknown codepage " + cCP )
      ErrorLevel( 99 )
      RETURN
   ENDIF

   hb_cdpSelect( cCP )
   // Character indexing on (for both CP950 and UTF8)
   hb_cdpIsCharIdx( cCP, .T. )

   lConfirm := ( cConfirm == "ON" )
   Set( _SET_CONFIRM, lConfirm )
   Set( _SET_SCOREBOARD, .F. )
   Set( _SET_ESCAPE, .T. )
   Set( _SET_DATEFORMAT, "yyyy.mm.dd" )
   SET CENTURY ON                               // Century on (set.ch has no _SET_CENTURY)

   cGt := Upper( hb_gtVersion() )
   // Accept only WVT_TW / WIN_TW; do not match on the "WVT" substring
   IF cGt == "WVT_TW"
      hb_gtInfo( HB_GTI_FONTNAME, "MingLiU" )   // MingLiU (Traditional Chinese font)
   ENDIF

   cLabelName := hb_UChar( 0x59D3 ) + hb_UChar( 0x540D )   // "Name" in Chinese
   cHint := "idx1  Enter=next  Esc=exit READ  AUTO=" + iif( lAuto, "ON", "OFF" )

   // AUTO: queue the form keys first; later sections are queued separately so leftover keys do not end READ early
   IF lAuto
      DemoQueueFormKeys()
   ENDIF

   // Unfocused blank string fields look near-black with these colors; compare with dBirth / nAmt when checking visually
   SetColor( "W/N,N/W,,,W/N" )
   CLS
   @ 0, 1 SAY "GT=" + cGt + "  CP=" + cCP + "  CONFIRM=" + cConfirm + "  idx=1"
   @ 1, 1 SAY cHint
   @ 2, 1 SAY Replicate( "-", 62 )

   // Standard @ SAY GET: READ pre-paints all five fields
   // Field 1 Name (Chinese), no nMaxChars; the GET starts at column 12
   @ 4, 2 SAY cLabelName GET cName PICTURE Replicate( "X", 10 )

   // Field 2: the label "nMaxChars=6" is 11 characters from column 2, so "6" falls on column 12
   // so the GET starts at column 14 to avoid the label
   @ 6, 2 SAY "nMaxChars=6" GET cMax PICTURE Replicate( "X", 10 )
   // The GET just added is the last one in GetList; set nMaxChars (tget DATA)
   ATail( GetList ):nMaxChars := 6

   // Field 3 letters and digits only
   @ 8, 2 SAY "ALNUM" GET cAsc PICTURE "@!" ;
      VALID Empty( RTrim( cAsc ) ) .OR. DemoIsAlnum( RTrim( cAsc ) )

   // Field 4 date
   @ 10, 2 SAY "dBirth" GET dBirth

   // Field 5 numeric
   @ 12, 2 SAY "nAmt" GET nAmt PICTURE "999,999.99"

   IF lAuto
      DemoAutoPause( 2.5 )                      // AUTO: pause after the form is pre-painted (for a screenshot)
   ENDIF
   READ

   QOut( "" )
   QOut( "---- after READ ----" )
   DemoDumpStr( "cName", cName )
   DemoDumpStr( "cMax ", cMax )
   DemoDumpStr( "cAsc ", cAsc )
   QOut( "dBirth=[" + DToC( dBirth ) + "]" )
   QOut( "nAmt  =[" + Transform( nAmt, "999,999.99" ) + "]" )
   QOut( "press a key: SaveScreen / CLS ..." )
   IF lAuto
      DemoClearKeys()
      DemoQueueNavKeys()
   ENDIF
   Inkey( 0 )

   cScr := SaveScreen( 0, 0, MaxRow(), MaxCol() )
   CLS
   @ 0, 1 SAY "CLS done; press a key to RestScreen"
   Inkey( 0 )
   RestScreen( 0, 0, MaxRow(), MaxCol(), cScr )
   @ MaxRow(), 1 SAY "RestScreen done; press a key for CT window"
   Inkey( 0 )

   // CT window: also @ SAY GET + READ
   CLS
   @ 0, 1 SAY "CT window  GT=" + cGt + "  CP=" + cCP + "  CONFIRM=" + cConfirm
   WOpen( 3, 5, 12, 54, .T. )
   WBox()
   IF lAuto
      DemoClearKeys()
      DemoQueueCtKeys()
   ENDIF
   @ 0, 1 SAY cLabelName + " GET (Esc=leave)" GET cWin PICTURE Replicate( "X", 10 )
   READ
   @ 4, 1 SAY "CT page ready"
   IF lAuto
      DemoClearKeys()
      DemoAutoPause( 1.0 )                      // AUTO: let the CT page settle first
   ENDIF
   Inkey( 0 )                                   // AUTO stops here (for a screenshot)
   WClose()

   QOut( "CT cWin dump:" )
   DemoDumpStr( "cWin ", cWin )
   QOut( "done; press a key to exit" )
   Inkey( 0 )

   RETURN

// Clear the typeahead so leftover keys do not run through everything
STATIC PROCEDURE DemoClearKeys()

   DO WHILE NextKey() != 0
      Inkey()
   ENDDO

   RETURN

// AUTO short pause (consumes no keys)
STATIC PROCEDURE DemoAutoPause( nSec )

   hb_idleSleep( nSec )

   RETURN

// AUTO: keys for the five form fields
STATIC PROCEDURE DemoQueueFormKeys()

   LOCAL cZh := hb_UChar( 0x4E2D ) + hb_UChar( 0x6587 )   // "Chinese" in Chinese
   LOCAL aKeys := {}

   AAdd( aKeys, cZh )
   AAdd( aKeys, K_ENTER )
   AAdd( aKeys, "ab" )
   AAdd( aKeys, K_ENTER )
   AAdd( aKeys, "ABC12" )
   AAdd( aKeys, K_ENTER )
   AAdd( aKeys, "20261109" )
   AAdd( aKeys, K_ENTER )
   AAdd( aKeys, "1234.56" )
   AAdd( aKeys, K_ENTER )
   hb_keyPut( aKeys )

   RETURN

// AUTO: one space key each for SaveScreen / CLS / Rest
STATIC PROCEDURE DemoQueueNavKeys()

   hb_keyPut( { Asc( " " ), Asc( " " ), Asc( " " ) } )

   RETURN

// AUTO: Chinese text + Enter in the CT window
STATIC PROCEDURE DemoQueueCtKeys()

   LOCAL cZh := hb_UChar( 0x4E2D ) + hb_UChar( 0x6587 )

   hb_keyPut( { cZh, K_ENTER } )

   RETURN

// String field: Len(), hb_BLen(), hb_ULen()
STATIC PROCEDURE DemoDumpStr( cTitle, cVal )

   LOCAL cTrim := RTrim( cVal )

   QOut( cTitle + "=[" + cTrim + "]" + ;
         " Len=" + hb_ntos( Len( cTrim ) ) + ;
         " BLen=" + hb_ntos( hb_BLen( cTrim ) ) + ;
         " ULen=" + hb_ntos( hb_ULen( cTrim ) ) )

   RETURN

// Letters and digits only
STATIC FUNCTION DemoIsAlnum( c )

   LOCAL i, ch

   FOR i := 1 TO Len( c )
      ch := SubStr( c, i, 1 )
      IF ! ( ( ch >= "A" .AND. ch <= "Z" ) .OR. ;
             ( ch >= "a" .AND. ch <= "z" ) .OR. ;
             ( ch >= "0" .AND. ch <= "9" ) )
         RETURN .F.
      ENDIF
   NEXT

   RETURN .T.