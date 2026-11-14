# get-tw v1.0.1 — release notes

## Fixes (D1–D9)

- **D1.** AChoice list items that contain wide (Chinese) characters now line up by display width.
- **D2.** TBrowse cells and headers that contain wide characters now line up by display width.
- **D3.** MemoEdit line wrapping now fits the window by display width, so a wide character is not split across a line break.
- **D4.** MemoEdit and MemoLine no longer drop characters when wrapping. A character that does not fit on the current line is moved to the next line instead of being lost. Wrapping uses display width.
- **D5.** In the editor, Left and Right move by a whole Chinese character, and Col() reports the display column correctly.
- **D6.** When character indexing is switched on for the code page (for example with hb_cdpIsCharIdx( "CP950", .T. ), as the demos do), MemoLine() and hb_MLEval() now copy the original bytes of each character into the line they return. Before, each character was converted to Unicode and back, and in CP950 that round trip is not exact: a few codes come back as a different code (for example some box-drawing symbols), and bytes that have no Unicode mapping in CP950, such as many UAO extension or end-user-defined characters, were read one byte at a time and could come back as different characters. The code page is now used only to find where each character starts and ends. MemoEdit uses hb_MLEval() when it loads text and when it re-wraps a paragraph, so the text it edits keeps its original bytes.
- **D7.** The default font of the GUI terminal is now MingLiU_HKSCS.
- **D9.** Ambiguous-width characters are counted as two cells by default, which suits Chinese layout. Programs that consist mainly of Western-European text should call TwAmbigW(1) or set the GETTW_AMBIGW environment variable so those characters are counted as one cell. Characters in the Unicode Private Use Area (PUA) are always counted as two cells.

## Code pages

Both BIG5 and CP950 are supported.

## Known limitation (D8)

If your data contains characters that exist only in BIG5, use hb_cdpSelect("BIG5") or UTF-8 instead of CP950.

Characters that the UAO extension maps to the Unicode Private Use Area (PUA) are drawn from whatever the active font and the PC's end-user-defined character (EUDC) file provide, so they may look different or appear blank on different PCs. This is a known limitation.

Setting CODEPAGE=255 at runtime makes the window fall back to the Terminal font.

In Windows Terminal, highlight bars that end early or land in the wrong place are a further case of the same character-width issue; see "Known limitation: Windows Terminal" below.

## Known limitation: Windows Terminal

When the console build (WIN_TW) runs in Windows Terminal 1.24, Windows Terminal draws symbols whose width is ambiguous (for example € ○ ※ × ° and box-drawing characters) one cell wide, while get-tw counts them as two cells for Chinese layout. On a line that contains such symbols, the text after them and the highlight bar are out of place. Ordinary Chinese characters are not affected. In that case, use the classic console window (conhost) or the window build (WVT_TW). The ambiguousWidth setting in Windows Terminal 1.25 may help, but it has not been tested.

## Known limitation: resizing the console window

Resizing the window: In the window build (WVT_TW), dragging the window to a new size keeps the number of rows and columns, and the text scales with the window. In the console build (WIN_TW), resizing the console or Windows Terminal window while a program is running makes Windows rewrap the text on screen and may scroll it, so the program's screen becomes misaligned and the cursor no longer lines up with the text. After that the program only updates the parts that change, so ordinary keys such as the arrow keys do not restore the screen. It can only recover when the program redraws the whole screen; the safest way is to exit the program and run it again. A program that redraws its whole screen when it receives HB_K_RESIZE (the notice that the window size has changed) gets its screen and cursor back in line. For this the event mask must include HB_INKEY_GTEVENT, for example Set( _SET_EVENTMASK, hb_bitOr( INKEY_ALL, HB_INKEY_GTEVENT ) ) (needs #include "inkey.ch"). MemoEdit, AChoice and TBrowse do not redraw this way by themselves. Do not resize the console window while a program is running.

## Windows Terminal support

Tested with Windows Terminal 1.24. It works, with the limitation described above.

