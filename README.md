# get-tw：Harbour GET 繁體中文（寬字）支援

[English](README-en.md)

get-tw 讓 Harbour 程式的 `@ … SAY … GET`、`READ` 能正確顯示和編輯繁體中文，CP950（Big5）與 UTF-8 都適用：

- 一個中文字在畫面上佔 2 格，不會被切成半個字。
- 游標一次跳過一整個中文字；倒退鍵與 Del 一次刪掉整個字；插入、覆寫都以整字為單位。
- GET 可以用 `nMaxChars` 限制最多輸入幾個字。
- Alert 對話框依中文寬度排版。

套件由兩部分組成：修改過的 GET 類別（`tget.prg`），以及兩個修改過的畫面層（Harbour 稱為 GT，負責把字畫到螢幕上）：

| 畫面層 | 種類 | 來源 |
|---|---|---|
| `WIN_TW` | 主控台（文字視窗） | 由 Harbour 原版 GTWIN 修改 |
| `WVT_TW` | 獨立視窗（GUI） | 由 Harbour 原版 GTWVT 修改 |

> **重要：** 修改過的 GET 依賴 `WIN_TW`／`WVT_TW` 裡的寬字處理，**不可**搭配 Harbour 原版 GTWIN／GTWVT 使用，否則中文會顯示或編輯錯誤。

## 下載

原始碼庫不含編好的 exe。示範 exe 與 `MANIFEST.txt` 放在 GitHub Release 的 zip 套件裡：<https://github.com/WenSheng-tw/GT_WIN_WVT/releases>

## 檔案

| 檔案 | 說明 |
|---|---|
| `src\tget.prg` | 修改過的 GET 類別，取代 Harbour 內建的 Get |
| `src\gtwin_tw.c` | `WIN_TW` 主控台畫面層 |
| `src\gtwvt_tw.c` | `WVT_TW` 視窗畫面層 |
| `src\gtwvt_tw.h` | `WVT_TW` 用的標頭檔 |
| `src\twwide.c` | 中文寬字格子的共用處理；已由 `gtwin_tw.c`／`gtwvt_tw.c` 以 `#include` 併入，**不要**另外列進 .hbp，也不要單獨編譯 |
| `src\twwide.h` | `twwide.c` 用的常數 |
| `test\` | 示範程式（.prg）、建置設定（.hbp）、啟動批次檔（.bat）；編好的示範 exe 只在 Release zip 裡 |
| `bin\win32-bcc\`、`bin\win32-msvc\`、`bin\win64-msvc\` | 編好的示範 exe（BCC 32 位元、MSVC 32 位元、MSVC 64 位元各一組）與同樣的 4 個批次檔；exe 只在 Release zip 裡 |
| [`get-tw_user_guide_zh.html`](get-tw_user_guide_zh.html) | 中文使用說明 |
| [`get-tw_user_guide_en.html`](get-tw_user_guide_en.html) | 英文使用說明 |
| `LICENSE.txt` | Harbour 授權（原樣） |
| `MANIFEST.txt` | zip 套件的檔案清單與檢查碼（只在 Release zip 裡） |

## 編譯與連結

需要：Harbour 3.2（harbour-core）的 hbmk2，以及 Windows C 編譯器。附的示範程式是用 32 位元 Embarcadero BCC 7.4 編的；也試過用 Visual Studio 2019 的 MSVC 編 32 位元與 64 位元版；其他編譯器沒有試過。

在自己的 .hbp 裡列出（主控台版）：

```text
-winuni
-workdir=work\myprog
-omyprog
hbct.hbc
myprog.prg
..\get-tw\src\tget.prg
..\get-tw\src\gtwin_tw.c
```

`hbct.hbc` 只有用到 CT 視窗（`WOpen` 等）才需要。視窗版改列 `gtwvt_tw.c`，並加上 `-gui`：

```text
-winuni
-gui
-workdir=work\myprog
-omyprog
myprog.prg
..\get-tw\src\tget.prg
..\get-tw\src\gtwvt_tw.c
```

編譯：在 .hbp 所在的資料夾執行，下面三種編譯器擇一。

### BCC 32 位元

Embarcadero BCC 7.4（32 位元）要在 PATH 上，並使用 BCC 版 Harbour 的 hbmk2（Harbour 的 `bin\win\bcc\`）：

```bat
hbmk2 -comp=bcc -plat=win myprog.hbp
```

### MSVC 32 位元

先在同一個命令提示字元裡執行 Visual Studio 的 `vcvars32.bat`（或直接開「x86 Native Tools Command Prompt」），再使用 MSVC 版 Harbour 的 hbmk2（Harbour 的 `bin\win\msvc\`）：

```bat
call "<Visual Studio 安裝資料夾>\VC\Auxiliary\Build\vcvars32.bat"
hbmk2 -comp=msvc -plat=win myprog.hbp
```

### MSVC 64 位元

先執行 `vcvars64.bat`（或直接開「x64 Native Tools Command Prompt」），再使用 MSVC 64 位元版 Harbour 的 hbmk2（Harbour 的 `bin\win\msvc64\`）：

```bat
call "<Visual Studio 安裝資料夾>\VC\Auxiliary\Build\vcvars64.bat"
hbmk2 -comp=msvc64 -plat=win myprog.hbp
```

用 MSVC 編出的 exe 不需要另外安裝或附帶 DLL：C 執行階段是靜態連結的，只用到 Windows 內建的系統 DLL。

注意事項：

- `-comp=… -plat=win` 建議寫在命令列；hbmk2 要用和編譯器對應的那一份 Harbour（`bin\win\bcc\`、`bin\win\msvc\`、`bin\win\msvc64\`）。
- get-tw 的原始碼是 UTF-8。用 MSVC 時請在 .hbp 加 `-cflag=-utf-8{msvc|msvc64}`（示範的 .hbp 都有）；只寫 `{msvc}` 不會套用到 `-comp=msvc64`。BCC 不要加。
- 同一份 .hbp 給多種編譯器用時，`-workdir` 可以寫成 `work\${hb_comp}\myprog`，各編譯器的中間檔就會分開放（示範的 .hbp 就是這樣寫）。
- 先自己建好 `-workdir` 指定的資料夾；有些版本的 hbmk2 不會自動建立，會報錯停止。
- `twwide.c` 不要列。
- 一定要加 `-winuni`（示範的 .hbp 都有）；視窗版另外加 `-gui`。
- **不要**用 hbmk2 的 `-gt` 參數指定這兩個畫面層（例如 `-gtwin_tw`）：hbmk2 會另外去連結一個同名程式庫，本套件沒有這個程式庫，連結會失敗。請在程式裡寫 REQUEST（見下一節）。

## 在程式裡選用畫面層

在任一個 .prg 的最外層（不在任何 FUNCTION／PROCEDURE 裡）加一行：

```clipper
REQUEST HB_GT_WIN_TW_DEFAULT     // 主控台版
// 或
REQUEST HB_GT_WVT_TW_DEFAULT     // 視窗版
```

用 CP950 時另外加 `REQUEST HB_CODEPAGE_CP950`（UTF-8 內建，不用加），並在程式開頭選好碼頁。示範程式的寫法：

```clipper
hb_cdpSelect( "CP950" )            // 或 "UTF8"
hb_cdpIsCharIdx( "CP950", .T. )    // 開字元索引（示範程式兩種碼頁都開）
```

示範程式只在「字元索引開」的設定下操作過，建議照做。

其他：

- `hb_gtVersion()` 回傳目前畫面層名稱 `"WIN_TW"` 或 `"WVT_TW"`，可以拿來確認程式實際用的是哪一個；示範程式畫面第一列的 `GT=` 就是顯示它。
- 從命令列切換畫面層：`//GTWIN_TW`、`//GTWVT_TW`（也可寫 `//GT:WIN_TW`）；或設環境變數 `HB_GT=WIN_TW`。
- 舊名稱 `TWWIN`／`TWWVT`，以及 `REQUEST HB_GT_TWWIN_DEFAULT`／`REQUEST HB_GT_TWWVT_DEFAULT`，已經不存在。用到舊名稱的程式要改成 `WIN_TW`／`WVT_TW` 與上面的 REQUEST 寫法。
- 視窗版需要能顯示繁體中文的字型，示範程式設定細明體：

```clipper
#include "hbgtinfo.ch"
hb_gtInfo( HB_GTI_FONTNAME, "MingLiU" )
```

## nMaxChars：限制最多輸入幾個字

`nMaxChars` 是 Get 物件的屬性，預設 `NIL`（不限制）。沒有對應的 `@…GET` 子句，要在 GET 建立後設定 `GetList` 裡的 Get 物件。

- UTF-8 算字數（一個中文字算 1）；CP950 算位元組（一個中文字算 2）。
- `NIL`、0、負數或不是數字：不限制。有小數時取整數。
- 實際上限 ＝ `nMaxChars` 與欄位寬度取小的。
- 只對字元型欄位有效，且 PICTURE 遮罩只能由 `! A N X 9 #` 組成。
- 內容改過時，離開欄位前會整字截到上限（不切半個字），再以空白補回欄寬。

單一欄位：

```clipper
LOCAL cMax := Space( 10 )
@ 6, 2 SAY "備註" GET cMax PICTURE Replicate( "X", 10 )
ATail( GetList ):nMaxChars := 6
READ
```

多個欄位用迴圈設定（`GetList` 的順序就是 `@…GET` 寫的順序）：

```clipper
LOCAL cName := Space( 10 )
LOCAL cAddr := Space( 20 )
LOCAL cCode := Space( 8 )
LOCAL aMax  := { 4, 10, NIL }       // 依 GET 的順序；NIL＝不限制
LOCAL i
@ 4, 2 SAY "姓名" GET cName PICTURE Replicate( "X", 10 )
@ 6, 2 SAY "地址" GET cAddr PICTURE Replicate( "X", 20 )
@ 8, 2 SAY "代號" GET cCode PICTURE "@!"
FOR i := 1 TO Len( GetList )
   GetList[ i ]:nMaxChars := aMax[ i ]
NEXT
READ
```

## 執行示範程式

Release zip 的 `bin\` 底下有三組編好的示範程式：`bin\win32-bcc\`（BCC 32 位元）、`bin\win32-msvc\`（MSVC 32 位元）、`bin\win64-msvc\`（MSVC 64 位元，需要 64 位元 Windows）。每組資料夾都有 2 個 exe 和同樣的 4 個批次檔，雙擊批次檔即可：

| 批次檔 | 畫面層 | 碼頁 |
|---|---|---|
| `demo_gtwin_utf8.bat` | 主控台 `WIN_TW` | UTF-8 |
| `demo_gtwin_cp950.bat` | 主控台 `WIN_TW` | CP950 |
| `demo_gtwvt_utf8.bat` | 視窗 `WVT_TW` | UTF-8 |
| `demo_gtwvt_cp950.bat` | 視窗 `WVT_TW` | CP950 |

示範程式名稱 demo_gtwin／demo_gtwvt 只表示視窗種類，實際用的是 `WIN_TW`／`WVT_TW`。畫面第一列顯示 `GT=` 畫面層名稱、碼頁與 CONFIRM 設定；表單有中文標籤欄位、限制 6 個字的欄位、英數欄位、日期、數字，之後示範 SaveScreen／RestScreen 與 CT 視窗內的 GET。CONFIRM 預設關，要試開啟就把批次檔最後一個參數 `OFF` 改成 `ON`。

三組各以這 4 個批次檔試跑時，開啟畫面第一列分別顯示 `GT=WIN_TW`、`GT=WVT_TW`，中文標籤顯示正常；表單內的實際編輯操作請見下方「尚未驗證」。

重編示範程式（exe 會寫到 `test\`，可用 `test\` 裡的批次檔執行）。以下以 BCC 為例；用 MSVC 時先呼叫 vcvars，並把 `bcc` 換成 `msvc` 或 `msvc64`（見「編譯與連結」）：

```bat
cd test
mkdir work\bcc\demo_gtwin work\bcc\demo_gtwvt
hbmk2 -comp=bcc -plat=win demo_gtwin.hbp
hbmk2 -comp=bcc -plat=win demo_gtwvt.hbp
```

.hbp 裡的路徑都相對於 `test\`；中間檔依編譯器放在 `test\work\bcc\`、`test\work\msvc\`、`test\work\msvc64\`。

## 已知限制

- 不可搭配原版 GTWIN／GTWVT（見上方「重要」）。
- DispBox 用中文字當填滿字元：不支援，顯示不正確。
- TBrowse 依位元組截斷欄寬，含中文時可能被截短或切在字中間；AChoice、MemoEdit 等也可能有類似情形（未逐一測試）。
- UTF-8 字碼 0x20000 以上的字（多為罕用字）顯示不出來。
- 欄位改過之後按熱鍵（SetKey），內容會先截到上限，不等離開欄位。
- CT 視窗裡的 Alert 走 hbct 自己的程式，沒有依中文寬度排版。

## 尚未驗證

以下項目目前沒有在最新版本上實際驗證：

- GET 欄位被其他內容蓋過、再重畫回來時的樣子。
- CP950 且字元索引關閉時的截字與重畫。
- 用 `QOut`、`?`、`QQOut` 輸出中文（含自動換行），以及用 `Replicate` 畫中文。
- SaveScreen／RestScreen 存回含中文的畫面（區域邊界切過中文字時）。
- 視窗版用細明體時方框框線是否連續。
- 同一支程式同時連結兩個畫面層並互相切換。
- CT 視窗疊在上面時的中文前半／後半判斷。
- 視窗版滑鼠選取含中文區域後複製的剪貼簿內容。
- 非全螢幕的 Alert，以及 CT 視窗裡的 Alert。
- DispBegin 包住的 READ、只改顏色的重畫、畫面閃爍。
- 命令列 `//GTWIN_TW` 與環境變數 `HB_GT` 的實際效果。
- BCC 7.4 以外的編譯器（MSVC 32／64 位元目前只試過編譯與開啟畫面）。

中文輸入法輸入：作者以一般輸入法試過，輸入正常。

## 授權

`tget.prg`、`gtwin_tw.c`、`gtwvt_tw.c`、`gtwvt_tw.h` 是 Harbour 原始檔的修改版；`twwide.c`／`twwide.h` 編進 `gtwin_tw.c`／`gtwvt_tw.c` 裡。依 Harbour 原授權（GNU GPL 第 2 版或更新版，附 Harbour 例外條款）散布，詳見 [LICENSE.txt](LICENSE.txt) 與各原始檔開頭的聲明。

get-tw 的修改部分，以及 `twwide.c`／`twwide.h`：Copyright 2026 Wen Sheng，同樣以 GNU GPL 第 2 版或更新版（附 Harbour 例外條款）授權。四個修改過的原始檔開頭有修改聲明（修改者、日期、修改內容），其後是原樣保留的 Harbour 著作權與授權聲明。上述例外條款的範圍已放寬，也涵蓋 get-tw 本身的程式碼：可以把 get-tw 連結進自己的執行檔，不必公開自己的程式碼；但 get-tw 本身（含修改版）仍依 GNU GPL 授權，散布時必須公開其原始碼。
