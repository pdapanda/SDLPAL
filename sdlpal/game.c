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

#define MAINMENU_BACKGROUND_FBPNUM     60
#define MAINMENU_RIX                   4
#define MAINMENU_LABEL_NEWGAME         7
#define MAINMENU_LABEL_LOADGAME        8

static VOID
PAL_DrawMainMenuBackground(
   VOID
)
/*++
  Purpose:

    Draw the background of the main menu.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   BYTE        buf[320 * 200], buf2[320 * 200];

   //
   // Read the picture from fbp.mkf.
   //
   PAL_MKFReadChunk(buf, 320 * 200, MAINMENU_BACKGROUND_FBPNUM, gpGlobals->fpFBP);
   DecodeYJ1(buf, buf2, 320 * 200);

   //
   // ...and blit it to the screen buffer.
   //
   PAL_FBPBlitToSurface(buf2, gpScreen);
   VIDEO_UpdateScreen(NULL, 0, 0);
}

INT
PAL_MainMenu(
   VOID
)
/*++
  Purpose:

    Show the main menu

  Parameters:

    None.

  Return value:

    Which saved slot to load from (1-5). 0 to start a new game.

--*/
{
   WORD          wItemSelected;

   MENUITEM      rgMainMenuItem[2] = {
      // value   label          enabled   position
      {  0,      "New Game",    TRUE,     PAL_XY(125, 95)  },
      {  1,      "Load Game",   TRUE,     PAL_XY(125, 112) }
   };

   strcpy(rgMainMenuItem[0].szLabel, PAL_GetWord(MAINMENU_LABEL_NEWGAME));
   strcpy(rgMainMenuItem[1].szLabel, PAL_GetWord(MAINMENU_LABEL_LOADGAME));

   //
   // Play the background music
   //
   RIX_Play(MAINMENU_RIX, TRUE, 1);

   //
   // Draw the background
   //
   PAL_DrawMainMenuBackground();
   PAL_FadeIn(0, FALSE, 1);

   //
   // Activate the menu
   //
   wItemSelected = PAL_ReadMenu(NULL, rgMainMenuItem, 2, MENUITEM_COLOR);

   if (wItemSelected > 0)
   {
      //
      // Load game
      //

      // TODO
   }

   //
   // Fade out the screen and the music
   //
   RIX_Play(0, FALSE, 1);
   PAL_FadeOut(1);

   return (INT)wItemSelected;
}

VOID
PAL_GameMain(
   VOID
)
/*++
  Purpose:

    The main game routine.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   int           iSlot;

   iSlot = PAL_MainMenu();
}
