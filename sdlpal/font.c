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

#include "font.h"
#include "ascii.h"

typedef struct tagFont
{
   LPWORD           buf_chars;
   LPBYTE           buf_fonts;
   INT              num_char;
} font_t;

static font_t *gpFont = NULL;

INT
PAL_InitFont(
   VOID
)
/*++
  Purpose:

    Load the font files.

  Parameters:

    None.

  Return value:

    0 if succeed, -1 if cannot allocate memory, -2 if cannot load files.

--*/
{
   FILE *fp;

   if (gpFont != NULL)
   {
      //
      // Already initialized
      //
      return 0;
   }

   gpFont = (font_t *)calloc(1, sizeof(font_t));
   if (gpFont == NULL)
   {
      return -1;
   }

   //
   // Load the wor16.asc file.
   //
   fp = fopen("wor16.asc", "rb");
   if (fp == NULL)
   {
      free(gpFont);
      return -2;
   }

   //
   // Get the size of wor16.asc file.
   //
   fseek(fp, 0, SEEK_END);
   gpFont->num_char = ftell(fp);
   gpFont->num_char /= 2;

   //
   // Read all the character codes.
   //
   gpFont->buf_chars = (LPWORD)calloc(gpFont->num_char, sizeof(WORD));
   if (gpFont->buf_chars == NULL)
   {
      free(gpFont);
      gpFont = NULL;
      return -1;
   }

   fseek(fp, 0, SEEK_SET);
   fread(gpFont->buf_chars, sizeof(WORD), gpFont->num_char, fp);

   //
   // Close wor16.asc file.
   //
   fclose(fp);

   //
   // Read all bitmaps from wor16.fon file.
   //
   fp = fopen("wor16.fon", "rb");
   if (fp == NULL)
   {
      free(gpFont->buf_chars);
      free(gpFont);
      gpFont = NULL;
      return -3;
   }

   gpFont->buf_fonts = (LPBYTE)calloc(gpFont->num_char, 30);
   if (gpFont->buf_fonts == NULL)
   {
      free(gpFont->buf_chars);
      free(gpFont);
      gpFont = NULL;
      return -1;
   }

   //
   // The font glyph data begins at offset 0x682 in wor16.fon.
   //
   fseek(fp, 0x682, SEEK_SET);
   fread(gpFont->buf_fonts, 30, gpFont->num_char, fp);
   fclose(fp);

   return 0;
}

VOID
PAL_FreeFont(
   VOID
)
/*++
  Purpose:

    Free the memory used for fonts.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   if (gpFont != NULL)
   {
      free(gpFont->buf_chars);
      free(gpFont->buf_fonts);
      free(gpFont);
   }
   gpFont = NULL;
}

VOID
PAL_DrawCharOnSurface(
   WORD                     wChar,
   SDL_Surface             *lpSurface,
   PAL_POS                  pos,
   BYTE                     bColor
)
/*++
  Purpose:

    Draw a BIG-5 Chinese character on a surface.

  Parameters:

    [IN]  wChar - the character to be drawn (in BIG-5).

    [OUT] lpSurface - the destination surface.

    [IN]  pos - the destination location of the surface.

    [IN]  bColor - the color of the character.

  Return value:

    None.

--*/
{
   int i, j, dx, dy;
   int x = PAL_X(pos), y = PAL_Y(pos);
   LPBYTE pChar;

   //
   // Check for NULL pointer.
   //
   if (lpSurface == NULL || gpFont == NULL)
   {
      return;
   }

   //
   // Locate for this character in the font lib.
   //
   for (i = 0; i < gpFont->num_char; i++)
   {
      if (gpFont->buf_chars[i] == wChar)
      {
         break;
      }
   }

   if (i >= gpFont->num_char)
   {
      //
      // This character does not exist in the font lib.
      //
      return;
   }

   pChar = gpFont->buf_fonts + i * 30;

   //
   // Draw the character to the surface.
   //
   for (i = 0; i < 30; i++)
   {
      for (j = 0; j < 8; j++)
      {
         if (pChar[i] & (1 << (7 - j)))
         {
            dx = x + j + 8 * (i & 1);
            dy = y + i / 2;
            ((LPBYTE)(lpSurface->pixels))[dy * lpSurface->pitch + dx] = bColor;
         }
      }
   }
}

VOID
PAL_DrawASCIICharOnSurface(
   BYTE                     bChar,
   SDL_Surface             *lpSurface,
   PAL_POS                  pos,
   BYTE                     bColor
)
/*++
  Purpose:

    Draw a ASCII character on a surface.

  Parameters:

    [IN]  bChar - the character to be drawn.

    [OUT] lpSurface - the destination surface.

    [IN]  pos - the destination location of the surface.

    [IN]  bColor - the color of the character.

  Return value:

    None.

--*/
{
   int i, j, dx;
   int x = PAL_X(pos), y = PAL_Y(pos);
   LPBYTE pChar = &iso_font[(int)(bChar & ~128) * 15];

   //
   // Check for NULL pointer.
   //
   if (lpSurface == NULL)
   {
      return;
   }

   //
   // Draw the character to the surface.
   //
   for (i = 0; i < 15; i++)
   {
      dx = x;
      for (j = 0; j < 8; j++)
      {
         if (pChar[i] & (1 << j))
         {
            ((LPBYTE)(lpSurface->pixels))[y * lpSurface->pitch + dx] = bColor;
         }
         dx++;
      }
      y++;
   }
}
