# get-tw: Traditional Chinese (wide character) support for Harbour GET

[中文](README.md)

get-tw makes Harbour `@ … SAY … GET` and `READ` display and edit Traditional Chinese text correctly, in both CP950 (Big5) and UTF-8:

- Each Chinese character occupies two screen cells and is never split in half.
- The cursor moves by whole characters; Backspace and Del remove a whole character; insert and overwrite work on whole characters.
- A GET can limit how many characters may be entered with `nMaxChars`.
- Alert dialogs are laid out by Chinese character width.

The package has two parts: a modified GET class (`tget.prg`) and two modified Harbour terminal drivers (GTs):

| Terminal | Kind | Based on |
|---|---|---|
| `WIN_TW` | Console | Harbour GTWIN |
| `WVT_TW` | GUI window | Harbour GTWVT |

> **Important:** the modified GET relies on the wide-character handling inside `WIN_TW` / `WVT_TW`. Do **not** use it with the original Harbour GTWIN or GTWVT; Chinese text will not be displayed or edited correctly.

## Download

The source repository does not contain prebuilt exes. The demo exes and `MANIFEST.txt` are in the zip package attached to the GitHub Release: <https://github.com/WenSheng-tw/GT_WIN_WVT/releases>

## Files

| File | Description |
|---|---|
| `src\tget.prg` | Modified GET class, replaces Harbour's built-in Get |
| `src\gtwin_tw.c` | `WIN_TW` console terminal |
| `src\gtwvt_tw.c` | `WVT_TW` GUI terminal |
| `src\gtwvt_tw.h` | Header for `WVT_TW` |
| `src\twwide.c` | Common wide-character cell handling, `#include`d by `gtwin_tw.c` / `gtwvt_tw.c`; do **not** list it in an .hbp or compile it on its own |
| `src\twwide.h` | Constants for `twwide.c` |
| `test\` | Demo programs (.prg), build files (.hbp) and launch scripts (.bat); the prebuilt demo exes are only in the Release zip |
| `bin\win32-bcc\`, `bin\win32-msvc\`, `bin\win64-msvc\` | Prebuilt demo exes (one set each for BCC 32-bit, MSVC 32-bit and MSVC 64-bit) with the same four batch files; the exes are only in the Release zip |
| [`get-tw_user_guide_en.html`](get-tw_user_guide_en.html) | User guide (English) |
| [`get-tw_user_guide_zh.html`](get-tw_user_guide_zh.html) | User guide (Traditional Chinese) |
| `LICENSE.txt` | Harbour license (unchanged copy) |
| `MANIFEST.txt` | File list with checksums for the zip package (only in the Release zip) |

## Building and linking

Requirements: hbmk2 from Harbour 3.2 (harbour-core) and a Windows C compiler. The included demos were built with 32-bit Embarcadero BCC 7.4; building with MSVC from Visual Studio 2019 (32-bit and 64-bit) has also been tried; other compilers have not been tried.

Add these to your own .hbp (console version):

```text
-winuni
-workdir=work\myprog
-omyprog
hbct.hbc
myprog.prg
..\get-tw\src\tget.prg
..\get-tw\src\gtwin_tw.c
```

`hbct.hbc` is only needed if you use CT windows (`WOpen` etc.). For the GUI version list `gtwvt_tw.c` instead and add `-gui`:

```text
-winuni
-gui
-workdir=work\myprog
-omyprog
myprog.prg
..\get-tw\src\tget.prg
..\get-tw\src\gtwvt_tw.c
```

Build: run hbmk2 in the folder of the .hbp, with one of the three compilers below.

### BCC 32-bit

Embarcadero BCC 7.4 (32-bit) must be on the PATH; use hbmk2 from the BCC build of Harbour (Harbour's `bin\win\bcc\`):

```bat
hbmk2 -comp=bcc -plat=win myprog.hbp
```

### MSVC 32-bit

First run Visual Studio's `vcvars32.bat` in the same command prompt (or open the "x86 Native Tools Command Prompt"), then use hbmk2 from the MSVC build of Harbour (Harbour's `bin\win\msvc\`):

```bat
call "<Visual Studio install folder>\VC\Auxiliary\Build\vcvars32.bat"
hbmk2 -comp=msvc -plat=win myprog.hbp
```

### MSVC 64-bit

First run `vcvars64.bat` (or open the "x64 Native Tools Command Prompt"), then use hbmk2 from the 64-bit MSVC build of Harbour (Harbour's `bin\win\msvc64\`):

```bat
call "<Visual Studio install folder>\VC\Auxiliary\Build\vcvars64.bat"
hbmk2 -comp=msvc64 -plat=win myprog.hbp
```

Executables built with MSVC need no extra DLL to be installed or shipped: the C runtime is linked statically, and only the system DLLs that come with Windows are used.

Notes:

- Put `-comp=… -plat=win` on the command line, and use the Harbour build that matches the compiler (`bin\win\bcc\`, `bin\win\msvc\`, `bin\win\msvc64\`).
- The get-tw sources are UTF-8. With MSVC, add `-cflag=-utf-8{msvc|msvc64}` to the .hbp (both demo .hbp files do); `{msvc}` alone does not apply to `-comp=msvc64`. Do not pass it to BCC.
- If one .hbp is used with several compilers, `-workdir` can be written as `work\${hb_comp}\myprog` so that each compiler keeps its intermediate files apart (the demo .hbp files do this).
- Create the `-workdir` folder yourself first; some hbmk2 versions do not create it and stop with an error.
- Do not list `twwide.c`.
- Always use `-winuni` (both demo .hbp files do); GUI programs also need `-gui`.
- Do **not** use hbmk2's `-gt` option for these terminals (e.g. `-gtwin_tw`): hbmk2 would also try to link a library of that name, which this package does not provide, so linking fails. Use the REQUEST line below instead.

## Selecting the terminal in your program

Put one line at module level (outside any FUNCTION / PROCEDURE) in one of your .prg files:

```clipper
REQUEST HB_GT_WIN_TW_DEFAULT     // console
// or
REQUEST HB_GT_WVT_TW_DEFAULT     // GUI window
```

For CP950 also add `REQUEST HB_CODEPAGE_CP950` (UTF-8 is built in) and select the codepage at startup. The demo does:

```clipper
hb_cdpSelect( "CP950" )            // or "UTF8"
hb_cdpIsCharIdx( "CP950", .T. )    // character indexing on (the demo uses it for both codepages)
```

The demo has only been operated with character indexing on; doing the same is recommended.

Other notes:

- `hb_gtVersion()` returns the active terminal name, `"WIN_TW"` or `"WVT_TW"`, so a program can check which one is in use; the `GT=` text on the demo's top line shows it.
- To pick the terminal on the command line use `//GTWIN_TW` or `//GTWVT_TW` (or `//GT:WIN_TW`), or set the environment variable `HB_GT=WIN_TW`.
- The old names `TWWIN` / `TWWVT` and `REQUEST HB_GT_TWWIN_DEFAULT` / `REQUEST HB_GT_TWWVT_DEFAULT` no longer exist. Programs that used them must be changed to `WIN_TW` / `WVT_TW` and the REQUEST lines above.
- The GUI version needs a font that can display Traditional Chinese; the demo uses MingLiU:

```clipper
#include "hbgtinfo.ch"
hb_gtInfo( HB_GTI_FONTNAME, "MingLiU" )
```

## nMaxChars: limiting the number of characters

`nMaxChars` is a property of the Get object, `NIL` (no limit) by default. There is no `@…GET` clause for it; set it on the Get object in `GetList` after the GET is created.

- UTF-8 counts characters (one Chinese character = 1); CP950 counts bytes (one Chinese character = 2).
- `NIL`, 0, a negative value or a non-number: no limit. Fractions are truncated.
- The effective limit is the smaller of `nMaxChars` and the field width.
- Only character fields are affected, and only when the PICTURE mask consists of `! A N X 9 #`.
- When the content was edited, it is cut to the limit by whole characters before leaving the field, then padded with spaces to the field width.

Single field:

```clipper
LOCAL cMax := Space( 10 )
@ 6, 2 SAY "Note" GET cMax PICTURE Replicate( "X", 10 )
ATail( GetList ):nMaxChars := 6
READ
```

Several fields with a loop (`GetList` is in the order the `@…GET` lines are written):

```clipper
LOCAL cName := Space( 10 )
LOCAL cAddr := Space( 20 )
LOCAL cCode := Space( 8 )
LOCAL aMax  := { 4, 10, NIL }       // in GET order; NIL = no limit
LOCAL i
@ 4, 2 SAY "Name"    GET cName PICTURE Replicate( "X", 10 )
@ 6, 2 SAY "Address" GET cAddr PICTURE Replicate( "X", 20 )
@ 8, 2 SAY "Code"    GET cCode PICTURE "@!"
FOR i := 1 TO Len( GetList )
   GetList[ i ]:nMaxChars := aMax[ i ]
NEXT
READ
```

## Running the demos

The `bin\` folder of the Release zip contains three prebuilt sets: `bin\win32-bcc\` (BCC 32-bit), `bin\win32-msvc\` (MSVC 32-bit) and `bin\win64-msvc\` (MSVC 64-bit, needs 64-bit Windows). Each folder has the two exes and the same four batch files; double-click a batch file:

| Batch file | Terminal | Codepage |
|---|---|---|
| `demo_gtwin_utf8.bat` | console `WIN_TW` | UTF-8 |
| `demo_gtwin_cp950.bat` | console `WIN_TW` | CP950 |
| `demo_gtwvt_utf8.bat` | GUI `WVT_TW` | UTF-8 |
| `demo_gtwvt_cp950.bat` | GUI `WVT_TW` | CP950 |

The names demo_gtwin / demo_gtwvt only say which kind of window is used; both use `WIN_TW` / `WVT_TW`. The top line shows `GT=` with the terminal name, the codepage and the CONFIRM setting. The form has a field with a Chinese label, a field limited to 6 characters, an alphanumeric field, a date and a number, followed by SaveScreen / RestScreen and a GET inside a CT window. CONFIRM is off by default; change the last argument in the batch file from `OFF` to `ON` to try it.

In a quick trial with these four batch files in each of the three sets, the start screen showed `GT=WIN_TW` and `GT=WVT_TW` on the top line and the Chinese label was displayed normally; editing inside the form is listed under "Not yet verified" below.

Rebuilding the demos (the exes are written to `test\` and can be started with the batch files there). The example uses BCC; for MSVC, call vcvars first and replace `bcc` with `msvc` or `msvc64` (see "Building and linking"):

```bat
cd test
mkdir work\bcc\demo_gtwin work\bcc\demo_gtwvt
hbmk2 -comp=bcc -plat=win demo_gtwin.hbp
hbmk2 -comp=bcc -plat=win demo_gtwvt.hbp
```

All paths in the .hbp files are relative to `test\`; intermediate files go to `test\work\bcc\`, `test\work\msvc\` or `test\work\msvc64\` by compiler.

## Known limitations

- Do not use with the original GTWIN / GTWVT (see "Important" above).
- Using a Chinese character as the DispBox fill character is not supported.
- TBrowse truncates column text by bytes, so Chinese text can be cut short or in the middle of a character; AChoice, MemoEdit and similar functions may behave alike (not tested one by one).
- UTF-8 characters at 0x20000 and above (mostly rare characters) cannot be displayed.
- After editing a field, pressing a hot key (SetKey) cuts the content to the limit immediately, not when leaving the field.
- Alert inside a CT window uses hbct's own code and is not laid out by Chinese width.

## Not yet verified

The following have not been checked on the current version:

- How a GET field looks after it is covered by other output and redrawn.
- Cutting and redrawing with CP950 and character indexing off.
- Chinese output with `QOut`, `?`, `QQOut` (including automatic wrapping) and drawing Chinese with `Replicate`.
- SaveScreen / RestScreen of screens containing Chinese (when a region edge cuts through a character).
- Whether box lines stay continuous in the GUI version with MingLiU.
- Linking both terminals into one program and switching between them.
- Lead/trail detection when a CT window is on top.
- Clipboard content after selecting a Chinese region with the mouse in the GUI version.
- Alert when not full screen, and Alert inside a CT window.
- READ inside DispBegin, colour-only redraws, flicker.
- The actual effect of `//GTWIN_TW` and the `HB_GT` environment variable.
- Compilers other than BCC 7.4 (MSVC 32-bit and 64-bit have only been tried for building and the start screen).

Chinese input method entry: tried by the author with a normal input method; input worked normally.

## License

`tget.prg`, `gtwin_tw.c`, `gtwvt_tw.c` and `gtwvt_tw.h` are modified versions of Harbour source files; `twwide.c` / `twwide.h` are compiled into `gtwin_tw.c` / `gtwvt_tw.c`. They are distributed under the original Harbour license (GNU GPL version 2 or later, with the Harbour exception); see [LICENSE.txt](LICENSE.txt) and the notices at the top of each source file.

The get-tw modifications and `twwide.c` / `twwide.h` are Copyright 2026 Wen Sheng and are licensed under the same terms (GNU GPL version 2 or later, with the Harbour exception). Each of the four modified source files starts with a modification notice (who, when, what), followed by the unchanged Harbour copyright and license notice. The linking exception has been widened to also cover get-tw's own code: you may link get-tw into your own executable without publishing your own source code. get-tw itself, including modified versions, remains under the GNU GPL, and its source must be made available when distributed.
