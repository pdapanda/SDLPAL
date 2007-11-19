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

#define WORD_LENGTH      10

typedef struct tagTEXTLIB
{
   LPBYTE          lpWordBuf;
   LPBYTE          lpMsgBuf;
   LPDWORD         lpMsgOffset;

   int             nWords;
   int             nMsgs;
} TEXTLIB, *LPTEXTLIB;

TEXTLIB         g_TextLib;

INT
PAL_InitText(
   VOID
)
/*++
  Purpose:

    Initialize the in-game texts.

  Parameters:

    None.

  Return value:

    0 = success.
    -1 = memory allocation error.
    -2 = cannot open file m.msg or word.dat.

--*/
{
   FILE       *fpMsg, *fpWord;
   int         i;

   //
   // Open the message and word data files.
   //
   fpMsg = fopen("m.msg", "rb");
   if (fpMsg == NULL)
   {
      return -2;
   }
   fpWord = fopen("word.dat", "rb");
   if (fpWord == NULL)
   {
      fclose(fpMsg);
      return -2;
   }

   //
   // See how many words we have
   //
   fseek(fpWord, 0, SEEK_END);
   i = ftell(fpWord);

   //
   // Each word has 10 bytes
   //
   g_TextLib.nWords = (i + (WORD_LENGTH - 1)) / WORD_LENGTH;

   //
   // Read the words
   //
   g_TextLib.lpWordBuf = (LPBYTE)malloc(i);
   if (g_TextLib.lpWordBuf == NULL)
   {
      fclose(fpWord);
      fclose(fpMsg);
      return -1;
   }
   fseek(fpWord, 0, SEEK_SET);
   fread(g_TextLib.lpWordBuf, i, 1, fpWord);

   //
   // Close the words file
   //
   fclose(fpWord);

   //
   // Read the message offsets. The message offsets are in SSS.MKF #3
   //
   i = PAL_MKFGetChunkSize(3, gpGlobals->fpSSS) / sizeof(DWORD);
   g_TextLib.nMsgs = i - 1;

   g_TextLib.lpMsgOffset = (LPDWORD)malloc(i * sizeof(DWORD));
   if (g_TextLib.lpMsgOffset == NULL)
   {
      free(g_TextLib.lpWordBuf);
      fclose(fpMsg);
      return -1;
   }

   PAL_MKFReadChunk((LPBYTE)(g_TextLib.lpMsgOffset), i * sizeof(DWORD), 3,
      gpGlobals->fpSSS);

   //
   // Read the messages.
   //
   fseek(fpMsg, 0, SEEK_END);
   i = ftell(fpMsg);

   g_TextLib.lpMsgBuf = (LPBYTE)malloc(i);
   if (g_TextLib.lpMsgBuf == NULL)
   {
      free(g_TextLib.lpMsgOffset);
      free(g_TextLib.lpWordBuf);
      fclose(fpMsg);
      return -1;
   }

   fseek(fpMsg, 0, SEEK_SET);
   fread(g_TextLib.lpMsgBuf, 1, i, fpMsg);

   fclose(fpMsg);

   return 0;
}

VOID
PAL_FreeText(
   VOID
)
/*++
  Purpose:

    Free the memory used by the texts.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   free(g_TextLib.lpMsgBuf);
   free(g_TextLib.lpMsgOffset);
   free(g_TextLib.lpWordBuf);
}

LPCSTR
PAL_GetWord(
   WORD       wNumWord
)
/*++
  Purpose:

    Get the specified word.

  Parameters:

    [IN]  wNumWord - the number of the requested word.

  Return value:

    Pointer to the requested word. NULL if not found.

--*/
{
   static char buf[WORD_LENGTH + 1];

   if (wNumWord >= g_TextLib.nWords)
   {
      return NULL;
   }

   memcpy(buf, g_TextLib.lpWordBuf + wNumWord * WORD_LENGTH, WORD_LENGTH);
   buf[WORD_LENGTH] = '\0';

   //
   // Remove the trailing spaces
   //
   trim(buf);

   return buf;
}

LPCSTR
PAL_GetMsg(
   WORD       wNumMsg
)
/*++
  Purpose:

    Get the specified message.

  Parameters:

    [IN]  wNumMsg - the number of the requested message.

  Return value:

    Pointer to the requested message. NULL if not found.

--*/
{
   static char    buf[256];
   DWORD          dwOffset, dwSize;

   if (wNumMsg >= g_TextLib.nMsgs)
   {
      return NULL;
   }

   dwOffset = g_TextLib.lpMsgOffset[wNumMsg];
   dwSize = g_TextLib.lpMsgOffset[wNumMsg + 1] - dwOffset;
   assert(dwSize < 255);

   memcpy(buf, g_TextLib.lpMsgBuf + dwOffset, dwSize);
   buf[dwSize] = '\0';

   return buf;
}

VOID
PAL_DrawText(
   LPCSTR     lpszText,
   PAL_POS    pos,
   BYTE       bColor,
   BOOL       fShadow
)
/*++
  Purpose:

    Draw text on the screen.

  Parameters:

    [IN]  lpszText - the text to be drawn.

    [IN]  pos - Position of the text.

    [IN]  bColor - Color of the text.

    [IN]  fShadow - Whether the text is shadowed or not.

  Return value:

    None.

--*/
{
   SDL_Rect   rect;
   int        t;

   rect.x = PAL_X(pos);
   rect.y = PAL_Y(pos);
   rect.w = 16;
   rect.h = 16;

   while (*lpszText)
   {
      //
      // Draw the character
      //
      if (*lpszText & 0x80)
      {
         //
         // BIG-5 Chinese Character
         //
         if (fShadow)
         {
            PAL_DrawCharOnSurface(*((WORD *)lpszText), gpScreen, PAL_XY(rect.x + 1, rect.y + 1), 0);
         }
         PAL_DrawCharOnSurface(*((WORD *)lpszText), gpScreen, PAL_XY(rect.x, rect.y), bColor);
         lpszText += 2;
         t = 16;
      }
      else
      {
         //
         // ASCII character
         //
         if (fShadow)
         {
            PAL_DrawASCIICharOnSurface(*lpszText, gpScreen, PAL_XY(rect.x + 1, rect.y + 1), 0);
         }
         PAL_DrawASCIICharOnSurface(*lpszText, gpScreen, PAL_XY(rect.x, rect.y), bColor);
         lpszText++;
         t = 8;
      }

      //
      // Update the screen area
      //
      VIDEO_UpdateScreen(&rect, 0, 0);
      rect.x += t;
   }
}
