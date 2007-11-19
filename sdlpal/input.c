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

PALINPUTSTATE g_InputState;

VOID PAL_Shutdown(VOID);

static int
PAL_EventFilter(
   const SDL_Event       *lpEvent
)
/*++
  Purpose:

    SDL event filter function. A filter to process all events.

  Parameters:

    [IN]  lpEvent - pointer to the event.

  Return value:

    1 = the event will be added to the internal queue.
    0 = the event will be dropped from the queue.

--*/
{
   switch (lpEvent->type)
   {
   case SDL_VIDEORESIZE:
      //
      // resized the window
      //
      VIDEO_Resize(lpEvent->resize.w, lpEvent->resize.h);
      break;

   case SDL_QUIT:
      //
      // clicked on the close button of the window. Quit immediately.
      //
      PAL_Shutdown();
      exit(0);

   case SDL_KEYDOWN:
      //
      // Pressed a key
      //
      if (lpEvent->key.keysym.mod & KMOD_ALT)
      {
         if (lpEvent->key.keysym.sym == SDLK_RETURN)
         {
            //
            // Pressed Alt+Enter (toggle fullscreen)...
            //
            VIDEO_ToggleFullscreen();
            return 0;
         }
         else if (lpEvent->key.keysym.sym == SDLK_F4)
         {
            //
            // Pressed Alt+F4 (Exit program)...
            //
            PAL_Shutdown();
            exit(0);
         }
      }

      switch (lpEvent->key.keysym.sym)
      {
      case SDLK_UP:
      case SDLK_KP8:
         g_InputState.prevdir = g_InputState.dir;
         g_InputState.dir = kDirNorth;
         g_InputState.dwKeyPress |= kKeyUp;
         break;

      case SDLK_DOWN:
      case SDLK_KP2:
         g_InputState.prevdir = g_InputState.dir;
         g_InputState.dir = kDirSouth;
         g_InputState.dwKeyPress |= kKeyDown;
         break;

      case SDLK_LEFT:
      case SDLK_KP4:
         g_InputState.prevdir = g_InputState.dir;
         g_InputState.dir = kDirWest;
         g_InputState.dwKeyPress |= kKeyLeft;
         break;

      case SDLK_RIGHT:
      case SDLK_KP6:
         g_InputState.prevdir = g_InputState.dir;
         g_InputState.dir = kDirEast;
         g_InputState.dwKeyPress |= kKeyRight;
         break;

      case SDLK_ESCAPE:
      case SDLK_INSERT:
      case SDLK_LALT:
      case SDLK_RALT:
      case SDLK_KP0:
         g_InputState.dwKeyPress |= kKeyMenu;
         break;

      case SDLK_RETURN:
      case SDLK_SPACE:
      case SDLK_KP_ENTER:
         g_InputState.dwKeyPress |= kKeyExplore;
         break;

      case SDLK_PAGEUP:
      case SDLK_KP9:
         g_InputState.dwKeyPress |= kKeyPgUp;
         break;

      case SDLK_PAGEDOWN:
      case SDLK_KP3:
         g_InputState.dwKeyPress |= kKeyPgDn;
         break;

      case SDLK_r:
         g_InputState.dwKeyPress |= kKeyRepeat;
         break;

      case SDLK_a:
         g_InputState.dwKeyPress |= kKeyAuto;
         break;

      case SDLK_d:
         g_InputState.dwKeyPress |= kKeyDefend;
         break;

      case SDLK_e:
         g_InputState.dwKeyPress |= kKeyUseItem;
         break;

      case SDLK_w:
         g_InputState.dwKeyPress |= kKeyThrowItem;
         break;

      case SDLK_q:
         g_InputState.dwKeyPress |= kKeyEscape;
         break;

      case SDLK_s:
         g_InputState.dwKeyPress |= kKeyStatus;
         break;

      case SDLK_f:
         g_InputState.dwKeyPress |= kKeyForce;
         break;

      case SDLK_p:
         VIDEO_SaveScreenshot();
         break;
      }
      break;

   case SDL_KEYUP:
      //
      // Released a key
      //
      switch (lpEvent->key.keysym.sym)
      {
      case SDLK_UP:
      case SDLK_DOWN:
      case SDLK_LEFT:
      case SDLK_RIGHT:
      case SDLK_KP2:
      case SDLK_KP4:
      case SDLK_KP6:
      case SDLK_KP8:
         //
         // Released an arrow key
         //
         g_InputState.dir = g_InputState.prevdir;
         g_InputState.prevdir = kDirUnknown;
         break;
      }
      break;
   }

   //
   // All events are handled here; don't put anything to the internal queue
   //
   return 0;
}

VOID
PAL_ClearKeyState(
   VOID
)
/*++
  Purpose:

    Clear the record of pressed keys.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   g_InputState.dwKeyPress = 0;
}

VOID
PAL_InitInput(
   VOID
)
/*++
  Purpose:

    Initialize the input subsystem.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   memset(&g_InputState, 0, sizeof(g_InputState));
   g_InputState.dir = kDirUnknown;
   g_InputState.prevdir = kDirUnknown;
   SDL_SetEventFilter(PAL_EventFilter);
}

VOID
PAL_ProcessEvent(
   VOID
)
/*++
  Purpose:

    Process all events.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   while (SDL_PollEvent(NULL));
}
