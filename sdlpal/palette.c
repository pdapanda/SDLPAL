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

#include "main.h"

SDL_Color *
PAL_GetPalette(
   INT         iPaletteNum,
   BOOL        fNight
)
/*++
  Purpose:

    Get the specified palette in pat.mkf file.

  Parameters:

    [IN]  iPaletteNum - number of the palette.

    [IN]  fNight - whether use the night palette or not.

  Return value:

    Pointer to the palette. NULL if failed.

--*/
{
   static SDL_Color      palette[256];
   BYTE                  buf[1536];
   INT                   i;

   //
   // Read the palette data from the pat.mkf file
   //
   i = PAL_MKFReadChunk(buf, 1536, iPaletteNum, gpGlobals->fpPAT);

   if (i < 0)
   {
      //
      // Read failed
      //
      return NULL;
   }
   else if (i <= 256 * 3)
   {
      //
      // There is no night colors in the palette
      //
      fNight = FALSE;
   }

   for (i = 0; i < 256; i++)
   {
      palette[i].r = buf[(fNight ? 256 * 3 : 0) + i * 3] << 2;
      palette[i].g = buf[(fNight ? 256 * 3 : 0) + i * 3 + 1] << 2;
      palette[i].b = buf[(fNight ? 256 * 3 : 0) + i * 3 + 2] << 2;
#if 0
      palette[i].r += (255 - palette[i].r) / 5;
      palette[i].g += (255 - palette[i].g) / 5;
      palette[i].b += (255 - palette[i].b) / 5;
#endif
   }

   return palette;
}

VOID
PAL_SetPalette(
   INT         iPaletteNum,
   BOOL        fNight
)
/*++
  Purpose:

    Set the screen palette to the specified one.

  Parameters:

    [IN]  iPaletteNum - number of the palette.

    [IN]  fNight - whether use the night palette or not.

  Return value:

    None.

--*/
{
   SDL_Color *p = PAL_GetPalette(iPaletteNum, fNight);

   if (p != NULL)
   {
      VIDEO_SetPalette(p);
   }
}

VOID
PAL_FadeOut(
   INT         iDelay
)
/*++
  Purpose:

    Fadeout screen to black from the specified palette.

  Parameters:

    [IN]  iPaletteNum - number of the palette.

    [IN]  fNight - whether use the night palette or not.

    [IN]  iDelay - delay time for each step.

  Return value:

    None.

--*/
{
   int            i, j;
   UINT           time;
   SDL_Color      palette[256], newpalette[256];

   //
   // Get the original palette...
   //
   for (i = 0; i < 256; i++)
   {
      palette[i] = VIDEO_GetPalette()[i];
   }

   //
   // Start fading out...
   //
   for (i = 63; i >= 3; i--)
   {
      time = SDL_GetTicks() + iDelay * 10;

      //
      // Set the current palette...
      //
      for (j = 0; j < 256; j++)
      {
         newpalette[j].r = (palette[j].r * i) >> 6;
         newpalette[j].g = (palette[j].g * i) >> 6;
         newpalette[j].b = (palette[j].b * i) >> 6;
      }
      VIDEO_SetPalette(newpalette);

      while (SDL_GetTicks() < time)
      {
         while (SDL_PollEvent(NULL));
         SDL_Delay(1);
      }
   }
}

VOID
PAL_FadeIn(
   INT         iPaletteNum,
   BOOL        fNight,
   INT         iDelay
)
/*++
  Purpose:

    Fade in the screen to the specified palette.

  Parameters:

    [IN]  iPaletteNum - number of the palette.

    [IN]  fNight - whether use the night palette or not.

    [IN]  iDelay - delay time for each step.

  Return value:

    None.

--*/
{
   int            i, j;
   UINT           time;
   SDL_Color     *palette = PAL_GetPalette(iPaletteNum, fNight);
   SDL_Color      newpalette[256];

   if (palette == NULL)
   {
      return;
   }

   //
   // Start fading in...
   //
   for (i = 4; i < 64; i++)
   {
      time = SDL_GetTicks() + iDelay * 10;

      //
      // Calculate the current palette...
      //
      for (j = 0; j < 256; j++)
      {
         newpalette[j].r = (palette[j].r * i) >> 6;
         newpalette[j].g = (palette[j].g * i) >> 6;
         newpalette[j].b = (palette[j].b * i) >> 6;
      }
      VIDEO_SetPalette(newpalette);

      while (SDL_GetTicks() < time)
      {
         while (SDL_PollEvent(NULL));
         SDL_Delay(1);
      }
   }
}
