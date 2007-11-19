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

VOID
PAL_RNGPlay(
   INT           iNumRNG,
   INT           iStartFrame,
   INT           iEndFrame,
   INT           iSpeed
)
/*++
  Purpose:

    Play a RNG movie.

  Parameters:

    [IN]  iNumRNG - number of the RNG movie.

    [IN]  iStartFrame - start frame number.

    [IN]  iEndFrame - end frame number.

    [IN]  iSpeed - speed of playing.

  Return value:

    None.

--*/
{
   UINT            iTime;
   int             iDelay = 800 / (iSpeed == 0 ? 16 : iSpeed);

   for (; iStartFrame <= iEndFrame; iStartFrame++)
   {
      iTime = SDL_GetTicks() + iDelay;

      if (PAL_RNGBlitToSurface(iNumRNG, iStartFrame, gpScreen, gpGlobals->fpRNG) == -1)
      {
         //
         // Failed to get the frame, don't go further
         //
         return;
      }

      //
      // Update the screen
      //
      VIDEO_UpdateScreen(NULL, 0, 0);

      //
      // Delay for a while
      //
      while (SDL_GetTicks() <= iTime)
      {
         PAL_ProcessEvent();
         SDL_Delay(1);
      }
   }
}
