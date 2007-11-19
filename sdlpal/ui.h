//
// Copyright (c) 2007, Wei Mingzhi <whistler@openoffice.org>.
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef UI_H
#define UI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "common.h"

typedef struct tagBOX
{
   PAL_POS        pos;
   WORD           wWidth, wHeight;
   SDL_Surface   *lpSavedArea;
} BOX, *LPBOX;

typedef struct tagMENUITEM
{
   WORD          wValue;
   BYTE          szLabel[64];
   BOOL          fEnabled;
   PAL_POS       pos;
} MENUITEM, *LPMENUITEM;

typedef VOID (*LPITEMCHANGED_CALLBACK)(WORD);

#define MENUITEM_VALUE_CANCELLED      0xFFFF
#define MENUITEM_COLOR                0x4F

typedef enum tagNUMCOLOR
{
   kNumColorYellow,
   kNumColorBlue,
   kNumColorCyan
} NUMCOLOR;

typedef enum tagNUMALIGN
{
   kNumAlignLeft,
   kNumAlignMid,
   kNumAlignRight
} NUMALIGN;

INT
PAL_InitUI(
   VOID
);

VOID
PAL_FreeUI(
   VOID
);

LPBOX
PAL_CreateBox(
   PAL_POS        pos,
   INT            nRows,
   INT            nColumns,
   INT            iStyle,
   BOOL           fSaveScreen
);

LPBOX
PAL_CreateSingleLineBox(
   PAL_POS        pos,
   INT            nLen,
   BOOL           fSaveScreen
);

VOID
PAL_DeleteBox(
   LPBOX          lpBox
);

WORD
PAL_ReadMenu(
   LPITEMCHANGED_CALLBACK    lpfnMenuItemChanged,
   LPMENUITEM                rgMenuItem,
   INT                       nMenuItem,
   BYTE                      bLabelColor
);

VOID
PAL_DrawNumber(
   UINT            iNum,
   UINT            nLength,
   PAL_POS         pos,
   NUMCOLOR        color,
   NUMALIGN        align
);

#ifdef __cplusplus
}
#endif

#endif