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

// Screen buffer
SDL_Surface *gpScreen = NULL;

// The real screen surface
static SDL_Surface *gpScreenReal = NULL;

// Size ratio, real screen size / buffer size
static float g_flWidthRatio;
static float g_flHeightRatio;

// Initial screen size
static WORD g_wInitialWidth = 640;
static WORD g_wInitialHeight = 480;

INT
VIDEO_Init(
   WORD             wScreenWidth,
   WORD             wScreenHeight,
   BOOL             fFullScreen
)
/*++
  Purpose:

    Initialze the video subsystem.

  Parameters:

    [IN]  wScreenWidth - width of the screen.

    [IN]  wScreenHeight - height of the screen.

    [IN]  fFullScreen - TRUE to use full screen mode, FALSE to use windowed mode.

  Return value:

    0 = success, -1 = fail to create the screen surface,
    -2 = fail to create screen buffer.

--*/
{
   g_wInitialWidth = wScreenWidth;
   g_wInitialHeight = wScreenHeight;

   //
   // Create the screen surface.
   //
   gpScreenReal = SDL_SetVideoMode(wScreenWidth, wScreenHeight, 8,
      SDL_HWSURFACE | SDL_RESIZABLE | (fFullScreen ? SDL_FULLSCREEN : 0));

   if (gpScreenReal == NULL)
   {
      //
      // Fall back to 640x480 software mode.
      //
      gpScreenReal = SDL_SetVideoMode(640, 480, 8,
         SDL_SWSURFACE | (fFullScreen ? SDL_FULLSCREEN : 0));
   }

   //
   // Still fail?
   //
   if (gpScreenReal == NULL)
   {
      return -1;
   }

   //
   // Create the screen buffer.
   //
   gpScreen = SDL_CreateRGBSurface(gpScreenReal->flags & ~SDL_HWSURFACE, 320, 200, 8,
      gpScreenReal->format->Rmask, gpScreenReal->format->Gmask,
      gpScreenReal->format->Bmask, gpScreenReal->format->Amask);

   //
   // Failed?
   //
   if (gpScreen == NULL)
   {
      SDL_FreeSurface(gpScreenReal);
      return -2;
   }

   g_flWidthRatio = (float)(gpScreenReal->w) / (float)(gpScreen->w);
   g_flHeightRatio = (float)(gpScreenReal->h) / (float)(gpScreen->h);

   return 0;
}

VOID
VIDEO_Shutdown(
   VOID
)
/*++
  Purpose:

    Shutdown the video subsystem.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   if (gpScreen != NULL)
   {
      SDL_FreeSurface(gpScreen);
   }
   gpScreen = NULL;

   if (gpScreenReal != NULL)
   {
      SDL_FreeSurface(gpScreenReal);
   }
   gpScreenReal = NULL;
}

VOID
VIDEO_UpdateScreen(
   SDL_Rect        *lpRect,
   WORD             wEffect,
   FLOAT            flEffectTime
)
/*++
  Purpose:

    Update the screen area specified by lpRect.

  Parameters:

    [IN]  lpRect - Screen area to update.

    [IN]  wEffect - Effect for updating (fade, etc.).

    [IN]  flEffectDuration - Effect duration time.

  Return value:

    None.

--*/
{
   // TODO: effect

   SDL_Rect        dstrect;

   if (lpRect != NULL)
   {
      dstrect.x = (SHORT)(lpRect->x * g_flWidthRatio);
      dstrect.y = (SHORT)(lpRect->y * g_flHeightRatio);
      dstrect.w = (WORD)(lpRect->w * g_flWidthRatio);
      dstrect.h = (WORD)(lpRect->h * g_flHeightRatio);

      SDL_SoftStretch(gpScreen, lpRect, gpScreenReal, &dstrect);
      SDL_UpdateRect(gpScreenReal, dstrect.x, dstrect.y, dstrect.w, dstrect.h);
   }
   else
   {
      SDL_SoftStretch(gpScreen, NULL, gpScreenReal, NULL);
      SDL_UpdateRect(gpScreenReal, 0, 0, gpScreenReal->w, gpScreenReal->h);
   }
}

VOID
VIDEO_SetPalette(
   SDL_Color        rgPalette[256]
)
/*++
  Purpose:

    Set the palette of the screen.

  Parameters:

    [IN]  rgPalette - array of 256 colors.

  Return value:

    None.

--*/
{
   SDL_SetPalette(gpScreenReal, SDL_LOGPAL | SDL_PHYSPAL, rgPalette, 0, 256);
}

VOID
VIDEO_Resize(
   INT             w,
   INT             h
)
/*++
  Purpose:

    This function is called when user resized the window.

  Parameters:

    [IN]  w - width of the window after resizing.

    [IN]  h - height of the window after resizing.

  Return value:

    None.

--*/
{
   DWORD          flags;
   SDL_Color      palette[256];
   int            i;

   //
   // Get the original palette.
   //
   for (i = 0; i < gpScreenReal->format->palette->ncolors; i++)
   {
      palette[i] = gpScreenReal->format->palette->colors[i];
   }

   //
   // Calculate the new size ratios.
   //
   g_flWidthRatio = w / (float)(gpScreen->w);
   g_flHeightRatio = h / (float)(gpScreen->h);

   //
   // Create the screen surface.
   //
   flags = gpScreenReal->flags;

   SDL_FreeSurface(gpScreenReal);
   gpScreenReal = SDL_SetVideoMode(w, h, 8, flags);

   if (gpScreenReal == NULL)
   {
      //
      // Fall back to 640x480 software windowed mode.
      //
      gpScreenReal = SDL_SetVideoMode(640, 480, 8, SDL_SWSURFACE);
   }

   SDL_SetPalette(gpScreenReal, SDL_PHYSPAL | SDL_LOGPAL, palette, 0, i);
   VIDEO_UpdateScreen(NULL, 0, 0);
}

SDL_Color *
VIDEO_GetPalette(
   VOID
)
/*++
  Purpose:
    
    Get the current palette of the screen.

  Parameters:

    None.

  Return value:

    Pointer to the current palette.

--*/
{
   return gpScreenReal->format->palette->colors;
}

VOID
VIDEO_ToggleFullscreen(
   VOID
)
/*++
  Purpose:

    Toggle fullscreen mode.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   DWORD          flags;
   SDL_Color      palette[256];
   int            i;

   //
   // Get the original palette.
   //
   for (i = 0; i < gpScreenReal->format->palette->ncolors; i++)
   {
      palette[i] = gpScreenReal->format->palette->colors[i];
   }

   //
   // Get the flags of the original screen surface
   //
   flags = gpScreenReal->flags;

   if (flags & SDL_FULLSCREEN)
   {
      //
      // Already in fullscreen mode. Remove the fullscreen flag.
      //
      flags &= ~SDL_FULLSCREEN;
      flags |= SDL_RESIZABLE;
   }
   else
   {
      //
      // Not in fullscreen mode. Set the fullscreen flag.
      //
      flags |= SDL_FULLSCREEN;
   }

   //
   // Free the original screen surface
   //
   SDL_FreeSurface(gpScreenReal);

   //
   // ... and create a new one
   //
   gpScreenReal = SDL_SetVideoMode(g_wInitialWidth, g_wInitialHeight, 8, flags);
   VIDEO_SetPalette(palette);

   //
   // Calculate the new size ratios.
   //
   g_flWidthRatio = g_wInitialWidth / (float)(gpScreen->w);
   g_flHeightRatio = g_wInitialHeight / (float)(gpScreen->h);

   //
   // Update the screen
   //
   VIDEO_UpdateScreen(NULL, 0, 0);
}

VOID
VIDEO_SaveScreenshot(
   VOID
)
/*++
  Purpose:

    Save the screenshot of current screen to a BMP file.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   int      iNumBMP = 0;
   FILE    *fp;

   //
   // Find a usable BMP filename.
   //
   for (iNumBMP = 0; iNumBMP <= 9999; iNumBMP++)
   {
      fp = fopen(va("scrn%.4d.bmp", iNumBMP), "rb");
      if (fp == NULL)
      {
         break;
      }
      fclose(fp);
   }

   if (iNumBMP > 9999)
   {
      return;
   }

   //
   // Save the screenshot.
   //
   SDL_SaveBMP(gpScreenReal, va("scrn%.4d.bmp", iNumBMP));
}
