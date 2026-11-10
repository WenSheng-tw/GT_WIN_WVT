/*
 * get-tw wide-character cell constants for WIN_TW / WVT_TW
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

// twwide.h: shared constants for wide-character cell handling
// The lead cell stores the character; the trail cell stores the same character with the TW_ATTR_TRAIL flag set in bAttr
#ifndef TW_WIDE_H_
#define TW_WIDE_H_

// Trail-cell flag (0x10 in bAttr, a bit not used by the core)
#define TW_ATTR_TRAIL   0x10

// Cell kinds: normal, lead, trail, orphan half cell
#define TW_CELL_NORMAL  0
#define TW_CELL_LEAD    1
#define TW_CELL_TRAIL   2
#define TW_CELL_ORPHAN  3

#endif // TW_WIDE_H_
