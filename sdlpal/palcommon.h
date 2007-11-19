//
// Copyright (c) 2007, Wei Mingzhi <whistler@openoffice.org>.
// All rights reserved.
//
// Based on PAL MapEditor by Baldur.
//
// Based on PalLibrary by Lou Yihua <louyihua@21cn.com>.
// Copyright (c) 2006-2007, Lou Yihua.
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

#ifndef _PALUTILS_H
#define _PALUTILS_H

#include "common.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef LPBYTE      LPSPRITE, LPBITMAPRLE;
typedef LPCBYTE     LPCSPRITE, LPCBITMAPRLE;

typedef INT             PAL_POS;

#define PAL_XY(x, y)    (PAL_POS)((((WORD)y) << 16) | ((WORD)x))
#define PAL_X(xy)       (SHORT)((xy) & 0xFFFF)
#define PAL_Y(xy)       (SHORT)((xy) >> 16)

typedef enum tagPALDIRECTION
{
   kDirWest = 0,
   kDirNorth,
   kDirEast,
   kDirSouth,
   kDirUnknown
} PALDIRECTION, *LPPALDIRECTION;

INT
PAL_RLEDecode(
   LPCBITMAPRLE    lpBitmapRLE,
   LPBYTE          lpBuffer,
   UINT            uiBufferSize
);

INT
PAL_RLEBlitToSurface(
   LPCBITMAPRLE      lpBitmapRLE,
   SDL_Surface      *lpDstSurface,
   PAL_POS           pos
);

INT
PAL_FBPBlitToSurface(
   LPBYTE            lpBitmapFBP,
   SDL_Surface      *lpDstSurface
);

UINT
PAL_RLEGetWidth(
   LPCBITMAPRLE      lpBitmapRLE
);

UINT
PAL_RLEGetHeight(
   LPCBITMAPRLE      lpBitmapRLE
);

LPCBITMAPRLE
PAL_SpriteGetFrame(
   LPCSPRITE       lpSprite,
   INT             iFrameNum
);

INT
PAL_MKFGetChunkCount(
   FILE *fp
);

INT
PAL_MKFGetChunkSize(
   UINT    uiChunkNum,
   FILE   *fp
);

INT
PAL_MKFReadChunk(
   LPBYTE          lpBuffer,
   UINT            uiBufferSize,
   UINT            uiChunkNum,
   FILE           *fp
);

INT
PAL_RNGBlitToSurface(
   INT                      iNumRNG,
   INT                      iNumFrame,
   SDL_Surface             *lpDstSurface,
   FILE                    *fpRngMKF
);

// From yj1.c:
INT
DecodeYJ1(
   LPCVOID      Source,
   LPVOID       Destination,
   INT          DestSize
);

#ifdef __cplusplus
}
#endif

#endif // _PALUTILS_H
