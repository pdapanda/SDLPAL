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

#ifndef GLOBAL_H
#define GLOBAL_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct tagGLOBALVARS
{
   FLOAT            flTime;

   FILE            *fpFBP;      // battlefield background images
   FILE            *fpMAP;      // map data
   FILE            *fpGOP;      // map tile data
   FILE            *fpRNG;      // movie data
   FILE            *fpPAT;      // palette data
   FILE            *fpMGO;      // sprites in scenes
   FILE            *fpABC;      // enemy sprites during battle
   FILE            *fpBALL;     // item bitmaps
   FILE            *fpDATA;     // misc data
   FILE            *fpF;        // player sprites during battle
   FILE            *fpFIRE;     // fire effect sprites
   FILE            *fpRGM;      // character face bitmaps
   FILE            *fpSSS;      // script data
   FILE            *fpVOC;      // sound data
} GLOBALVARS, *LPGLOBALVARS;

extern LPGLOBALVARS gpGlobals;

INT
PAL_InitGlobals(
   VOID
);

VOID
PAL_FreeGlobals(
   VOID
);

#ifdef __cplusplus
}
#endif

#endif
