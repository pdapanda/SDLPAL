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

#include "palcommon.h"

INT
PAL_RLEDecode(
   LPCBITMAPRLE    lpBitmapRLE,
   LPBYTE          lpBuffer,
   UINT            uiBufferSize
)
/*++
  Purpose:

    Decode RLE-compressed bitmap.

  Parameters:

    [IN]  lpBitmapRLE - pointer to the RLE-compressed bitmap to be decoded.

    [OUT] lpBuffer - buffer of the decoded bitmap.

    [IN]  uiBufferSize - size of the buffer.

  Return value:

    0 = success, -1 = error.

--*/
{
   UINT      i;
   UINT      uiLen        = 0;
   UINT      uiWidth      = 0;
   UINT      uiHeight     = 0;
   BYTE      T;

   //
   // Check for NULL pointer.
   //
   if (lpBitmapRLE == NULL || lpBuffer == NULL)
   {
      return -1;
   }

   //
   // Skip the 0x00000002 in the file header.
   //
   if (SWAP32(((LPDWORD)lpBitmapRLE)[0]) == 0x00000002)
   {
      lpBitmapRLE += 4;
   }

   //
   // Get the width and height of the bitmap.
   //
   uiWidth = SWAP16(((LPWORD)lpBitmapRLE)[0]);
   uiHeight = SWAP16(((LPWORD)lpBitmapRLE)[1]);

   //
   // Calculate the total length of the bitmap.
   // The bitmap is 8-bpp, each pixel will use 1 byte.
   //
   uiLen = uiWidth * uiHeight;

   //
   // Check if we have enough space for the decoded bitmap.
   //
   if (uiLen > uiBufferSize)
   {
      return -1;
   }

   //
   // Fill the buffer with color key.
   //
   memset(lpBuffer, 0xFF, uiLen);

   //
   // Start decoding the bitmap.
   //
   lpBitmapRLE += 4;
   for (i = 0; i < uiLen;)
   {
      T = *lpBitmapRLE++;
      if ((T & 0x80) && T <= 0x80 + uiWidth)
      {
         i += T - 0x80;
      }
      else
      {
         memcpy(lpBuffer + i, lpBitmapRLE, T);
         lpBitmapRLE += T;
         i += T;
      }
   }

   //
   // Success
   //
   return 0;
}

INT
PAL_RLEBlitToSurface(
   LPCBITMAPRLE      lpBitmapRLE,
   SDL_Surface      *lpDstSurface,
   PAL_POS           pos
)
/*++
  Purpose:

    Blit an RLE-compressed bitmap to an SDL surface.
    NOTE: Assume the surface is already locked, and the surface is a 8-bit one.

  Parameters:

    [IN]  lpBitmapRLE - pointer to the RLE-compressed bitmap to be decoded.

    [OUT] lpDstSurface - pointer to the destination SDL surface.

    [IN]  pos - position of the destination area.

  Return value:

    0 = success, -1 = error.

--*/
{
   UINT          i, j;
   INT           x, y;
   UINT          uiLen       = 0;
   UINT          uiWidth     = 0;
   UINT          uiHeight    = 0;
   BYTE          T;
   INT           dx          = PAL_X(pos);
   INT           dy          = PAL_Y(pos);

   //
   // Check for NULL pointer.
   //
   if (lpBitmapRLE == NULL || lpDstSurface == NULL)
   {
      return -1;
   }

   //
   // Skip the 0x00000002 in the file header.
   //
   if (SWAP32(((LPDWORD)lpBitmapRLE)[0]) == 0x00000002)
   {
      lpBitmapRLE += 4;
   }

   //
   // Get the width and height of the bitmap.
   //
   uiWidth = SWAP16(((LPWORD)lpBitmapRLE)[0]);
   uiHeight = SWAP16(((LPWORD)lpBitmapRLE)[1]);

   //
   // Calculate the total length of the bitmap.
   // The bitmap is 8-bpp, each pixel will use 1 byte.
   //
   uiLen = uiWidth * uiHeight;

   //
   // Start decoding and blitting the bitmap.
   //
   lpBitmapRLE += 4;
   for (i = 0; i < uiLen;)
   {
      T = *lpBitmapRLE++;
      if ((T & 0x80) && T <= 0x80 + uiWidth)
      {
         i += T - 0x80;
      }
      else
      {
         for (j = 0; j < T; j++)
         {
            //
            // Calculate the destination coordination.
            // FIXME: This could be optimized
            //
            y = (i + j) / uiWidth + dy;
            x = (i + j) % uiWidth + dx;

            //
            // Skip the points which are out of the surface.
            //
            if (x < 0)
            {
               j += -x - 1;
               continue;
            }
            else if (x >= lpDstSurface->w)
            {
               j += x - lpDstSurface->w;
               continue;
            }

            if (y < 0)
            {
               j += -y * uiWidth - 1;
               continue;
            }
            else if (y >= lpDstSurface->h)
            {
               goto end; // No more pixels needed, break out
            }

            //
            // Put the pixel onto the surface.
            //
            ((LPBYTE)lpDstSurface->pixels)[y * lpDstSurface->pitch + x] = lpBitmapRLE[j];
         }
         lpBitmapRLE += T;
         i += T;
      }
   }

end:
   //
   // Success
   //
   return 0;
}

INT
PAL_FBPBlitToSurface(
   LPBYTE            lpBitmapFBP,
   SDL_Surface      *lpDstSurface
)
/*++
  Purpose:

    Blit an uncompressed bitmap in FBP.MKF to an SDL surface.
    NOTE: Assume the surface is already locked, and the surface is a 8-bit 320x200 one.

  Parameters:

    [IN]  lpBitmapFBP - pointer to the RLE-compressed bitmap to be decoded.

    [OUT] lpDstSurface - pointer to the destination SDL surface.

  Return value:

    0 = success, -1 = error.

--*/
{
   int       x, y;
   LPBYTE    p;

   if (lpBitmapFBP == NULL || lpDstSurface == NULL ||
      lpDstSurface->w != 320 || lpDstSurface->h != 200)
   {
      return -1;
   }

   //
   // simply copy everything to the surface
   //
   for (y = 0; y < 200; y++)
   {
      p = (LPBYTE)(lpDstSurface->pixels) + y * lpDstSurface->pitch;
      for (x = 0; x < 320; x++)
      {
         *(p++) = *(lpBitmapFBP++);
      }
   }

   return 0;
}

UINT
PAL_RLEGetWidth(
   LPCBITMAPRLE    lpBitmapRLE
)
/*++
  Purpose:

    Get the width of an RLE-compressed bitmap.

  Parameters:

    [IN]  lpBitmapRLE - pointer to an RLE-compressed bitmap.

  Return value:

    Integer value which indicates the height of the bitmap.

--*/
{
   if (lpBitmapRLE == NULL)
   {
      return 0;
   }

   //
   // Skip the 0x0000002 in the header.
   //
   if (SWAP32(((LPDWORD)lpBitmapRLE)[0]) == 0x00000002)
   {
      lpBitmapRLE += 4;
   }

   return SWAP16(((LPWORD)lpBitmapRLE)[0]);
}

UINT
PAL_RLEGetHeight(
   LPCBITMAPRLE       lpBitmapRLE
)
/*++
  Purpose:

    Get the height of an RLE-compressed bitmap.

  Parameters:

    [IN]  lpBitmapRLE - pointer of an RLE-compressed bitmap.

  Return value:

    Integer value which indicates the height of the bitmap.

--*/
{
   if (lpBitmapRLE == NULL)
   {
      return 0;
   }

   //
   // Skip the 0x0000002 in the header.
   //
   if (SWAP32(((LPDWORD)lpBitmapRLE)[0]) == 0x00000002)
   {
      lpBitmapRLE += 4;
   }

   return SWAP16(((LPWORD)lpBitmapRLE)[1]);
}

LPCBITMAPRLE
PAL_SpriteGetFrame(
   LPCSPRITE       lpSprite,
   INT             iFrameNum
)
/*++
  Purpose:

    Get the pointer to the specified frame from a sprite.

  Parameters:

    [IN]  lpSprite - pointer to the sprite.

    [IN]  iFrameNum - number of the frame.

  Return value:

    Pointer to the specified frame. NULL if the frame does not exist.

--*/
{
   int imagecount = SWAP16(*((LPWORD)lpSprite)) - 1, offset;
   if (iFrameNum < 0 || iFrameNum >= imagecount)
   {
      //
      // The frame does not exist
      //
      return NULL;
   }

   //
   // Get the offset of the frame
   //
   offset = SWAP16(((LPWORD)lpSprite)[iFrameNum]) * 2;
   return lpSprite + offset;
}

INT
PAL_MKFGetChunkCount(
   FILE *fp
)
/*++
  Purpose:

    Get the number of chunks in an MKF archive.

  Parameters:

    [IN]  fp - pointer to an fopen'ed MKF file.

  Return value:

    Integer value which indicates the number of chunks in the specified MKF file.

--*/
{
   INT iNumChunk;
   if (fp == NULL)
   {
      return 0;
   }

   fseek(fp, 0, SEEK_SET);
   fread(&iNumChunk, sizeof(INT), 1, fp);

   iNumChunk = (SWAP32(iNumChunk) - 4) / 4;
   return iNumChunk;
}

INT
PAL_MKFGetChunkSize(
   UINT    uiChunkNum,
   FILE   *fp
)
/*++
  Purpose:

    Get the size of a chunk in an MKF archive.

  Parameters:

    [IN]  uiChunkNum - the number of the chunk in the MKF archive.

    [IN]  fp - pointer to the fopen'ed MKF file.

  Return value:

    Integer value which indicates the size of the chunk.
    -1 if the chunk does not exist.

--*/
{
   UINT    uiOffset       = 0;
   UINT    uiNextOffset   = 0;
   UINT    uiChunkCount   = 0;

   //
   // Get the total number of chunks.
   //
   uiChunkCount = PAL_MKFGetChunkCount(fp);
   if (uiChunkNum >= uiChunkCount)
   {
      return -1;
   }

   //
   // Get the offset of the specified chunk and the next chunk.
   //
   fseek(fp, 4 * uiChunkNum, SEEK_SET);
   fread(&uiOffset, sizeof(UINT), 1, fp);
   fread(&uiNextOffset, sizeof(UINT), 1, fp);
   uiOffset = SWAP32(uiOffset);
   uiNextOffset = SWAP32(uiNextOffset);

   //
   // Return the length of the chunk.
   //
   return uiNextOffset - uiOffset;
}

INT
PAL_MKFReadChunk(
   LPBYTE          lpBuffer,
   UINT            uiBufferSize,
   UINT            uiChunkNum,
   FILE           *fp
)
/*++
  Purpose:

    Read a chunk from an MKF archive into lpBuffer.

  Parameters:

    [OUT] lpBuffer - pointer to the destination buffer.

    [IN]  uiBufferSize - size of the destination buffer.

    [IN]  uiChunkNum - the number of the chunk in the MKF archive to read.

    [IN]  fp - pointer to the fopen'ed MKF file.

  Return value:

    Integer value which indicates the size of the chunk.
    -1 if there are error in parameters.
    -2 if buffer size is not enough.

--*/
{
   UINT     uiOffset       = 0;
   UINT     uiNextOffset   = 0;
   UINT     uiChunkCount   = 0;
   UINT     uiChunkLen     = 0;

   if (lpBuffer == NULL || fp == NULL || uiBufferSize == 0)
   {
      return -1;
   }

   //
   // Get the total number of chunks.
   //
   uiChunkCount = PAL_MKFGetChunkCount(fp);
   if (uiChunkNum >= uiChunkCount)
   {
      return -1;
   }

   //
   // Get the offset of the chunk.
   //
   fseek(fp, 4 * uiChunkNum, SEEK_SET);
   fread(&uiOffset, 4, 1, fp);
   fread(&uiNextOffset, 4, 1, fp);
   uiOffset = SWAP32(uiOffset);
   uiNextOffset = SWAP32(uiNextOffset);

   //
   // Get the length of the chunk.
   //
   uiChunkLen = uiNextOffset - uiOffset;

   if (uiChunkLen > uiBufferSize)
   {
      return -2;
   }

   if (uiChunkLen != 0)
   {
      fseek(fp, uiOffset, SEEK_SET);
      fread(lpBuffer, uiChunkLen, 1, fp);
   }
   else
   {
      return -1;
   }

   return (INT)uiChunkLen;
}

static INT
PAL_MKFReadSubChunk(
   LPBYTE          lpBuffer,
   UINT            uiBufferSize,
   UINT            uiChunkNum,
   UINT            uiSubChunkNum,
   FILE           *fp
)
/*++
  Purpose:

    Read a chunk from a sub-MKF in an MKF archive into lpBuffer. Only used for RNG
    animations.

  Parameters:

    [OUT] lpBuffer - pointer to the destination buffer.

    [IN]  uiBufferSize - size of the destination buffer.

    [IN]  uiChunkNum - the number of the chunk in the main MKF archive to read.

    [IN]  uiSubChunkNum - the number of the chunk in the sub MKF.

    [IN]  fp - pointer to the fopen'ed MKF file.

  Return value:

    Integer value which indicates the size of the chunk.
    -1 if there are error in parameters.
    -2 if buffer size is not enough.

--*/
{
   UINT         uiOffset       = 0;
   UINT         uiSubOffset    = 0;
   UINT         uiNextOffset   = 0;
   UINT         uiChunkCount   = 0;
   INT          iChunkLen      = 0;

   if (lpBuffer == NULL || fp == NULL || uiBufferSize == 0)
   {
      return -1;
   }

   //
   // Get the total number of chunks.
   //
   uiChunkCount = PAL_MKFGetChunkCount(fp);
   if (uiChunkNum >= uiChunkCount)
   {
      return -1;
   }

   //
   // Get the offset of the chunk.
   //
   fseek(fp, 4 * uiChunkNum, SEEK_SET);
   fread(&uiOffset, sizeof(UINT), 1, fp);
   fread(&uiNextOffset, sizeof(UINT), 1, fp);
   uiOffset = SWAP32(uiOffset);
   uiNextOffset = SWAP32(uiNextOffset);

   //
   // Get the length of the chunk.
   //
   iChunkLen = uiNextOffset - uiOffset;
   if (iChunkLen != 0)
   {
      fseek(fp, uiOffset, SEEK_SET);
   }
   else
   {
      return -1;
   }

   //
   // Get the number of sub chunks.
   //
   fread(&uiChunkCount, sizeof(UINT), 1, fp);
   uiChunkCount = (SWAP32(uiChunkCount) - 4) / 4;
   if (uiSubChunkNum >= uiChunkCount)
   {
      return -1;
   }

   //
   // Get the offset of the sub chunk.
   //
   fseek(fp, uiOffset + 4 * uiSubChunkNum, SEEK_SET);
   fread(&uiSubOffset, sizeof(UINT), 1, fp);
   fread(&uiNextOffset, sizeof(UINT), 1, fp);
   uiSubOffset = SWAP32(uiSubOffset);
   uiNextOffset = SWAP32(uiNextOffset);

   //
   // Get the length of the sub chunk.
   //
   iChunkLen = uiNextOffset - uiSubOffset;
   if ((UINT)iChunkLen > uiBufferSize)
   {
      return -2;
   }

   if (iChunkLen != 0)
   {
      fseek(fp, uiOffset + uiSubOffset, SEEK_SET);
      fread(lpBuffer, iChunkLen, 1, fp);
   }
   else
   {
      return -1;
   }

   return iChunkLen;
}

INT
PAL_RNGBlitToSurface(
   INT                      iNumRNG,
   INT                      iNumFrame,
   SDL_Surface             *lpDstSurface,
   FILE                    *fpRngMKF
)
/*++
  Purpose:

    Blit one frame in an RNG animation to an SDL surface.
    The surface should contain the last frame of the RNG, or blank if it's the first
    frame.

    NOTE: Assume the surface is already locked, and the surface is a 320x200 8-bit one.

  Parameters:

    [IN]  iNumRNG - The number of the animation in the MKF archive.

    [IN]  iNumFrame - The number of the frame in the animation.

    [OUT] lpDstSurface - pointer to the destination SDL surface.

    [IN]  fpRngMKF - Pointer to the fopen'ed rng.mkf file.

  Return value:

    0 = success, -1 = error.

--*/
{
   INT                   ptr         = 0;
   INT                   dst_ptr     = 0;
   BYTE                  data        = 0;
   WORD                  wdata       = 0;
   INT                   x, y, i, n;
   LPBYTE                rng         = NULL;
   LPBYTE                buf         = NULL;

   //
   // Check for invalid parameters.
   //
   if (lpDstSurface == NULL || iNumRNG < 0 || iNumFrame < 0)
   {
      return -1;
   }

   buf = (LPBYTE)calloc(1, 320 * 200);
   if (buf == NULL)
   {
      return -1;
   }

   //
   // Read the frame.
   //
   if (PAL_MKFReadSubChunk(buf, 320 * 200, iNumRNG, iNumFrame, fpRngMKF) < 0)
   {
      return -1;
   }

   //
   // Decompress the frame.
   //
   rng = (LPBYTE)calloc(1, 320 * 200);
   if (rng == NULL)
   {
      return -1;
   }
   DecodeYJ1(buf, rng, 320 * 200);
   free(buf);

   //
   // Draw the frame to the surface.
   //
   while (1)
   {
      data = rng[ptr++];
      switch (data)
      {
      case 0x00:
      case 0x13:
         //
         // End
         //
         goto end;

      case 0x02:
         dst_ptr += 2;
         break;

      case 0x03:
         data = rng[ptr++];
         dst_ptr += (data + 1) * 2;
         break;

      case 0x04:
         wdata = SWAP16(*(LPWORD)(rng + ptr));
         ptr += 2;
         dst_ptr += ((unsigned int)wdata + 1) * 2;
         break;

      case 0x0a:
         x = dst_ptr % 320;
         y = dst_ptr / 320;
         ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = rng[ptr++];
         if (++x >= 320)
         {
            x = 0;
            ++y;
         }
         ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = rng[ptr++];
         dst_ptr += 2;

      case 0x09:
         x = dst_ptr % 320;
         y = dst_ptr / 320;
         ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = rng[ptr++];
         if (++x >= 320)
         {
            x = 0;
            ++y;
         }
         ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = rng[ptr++];
         dst_ptr += 2;

      case 0x08:
         x = dst_ptr % 320;
         y = dst_ptr / 320;
         ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = rng[ptr++];
         if (++x >= 320)
         {
            x = 0;
            ++y;
         }
         ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = rng[ptr++];
         dst_ptr += 2;

      case 0x07:
         x = dst_ptr % 320;
         y = dst_ptr / 320;
         ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = rng[ptr++];
         if (++x >= 320)
         {
            x = 0;
            ++y;
         }
         ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = rng[ptr++];
         dst_ptr += 2;

      case 0x06:
         x = dst_ptr % 320;
         y = dst_ptr / 320;
         ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = rng[ptr++];
         if (++x >= 320)
         {
            x = 0;
            ++y;
         }
         ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = rng[ptr++];
         dst_ptr += 2;
         break;

      case 0x0b:
         data = *(rng + ptr++);
         for (i = 0; i <= data; i++)
         {
            x = dst_ptr % 320;
            y = dst_ptr / 320;
            ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = rng[ptr++];
            if (++x >= 320)
            {
               x = 0;
               ++y;
            }
            ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = rng[ptr++];
            dst_ptr += 2;
         }
         break;

      case 0x0c:
         wdata = SWAP16(*(LPWORD)(rng + ptr));
         ptr += 2;
         for (i = 0; i <= wdata; i++)
         {
            x = dst_ptr % 320;
            y = dst_ptr / 320;
            ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = rng[ptr++];
            if (++x >= 320)
            {
               x = 0;
               ++y;
            }
            ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = rng[ptr++];
            dst_ptr += 2;
         }
         break;

      case 0x0d:
      case 0x0e:
      case 0x0f:
      case 0x10:
         for (i = 0; i < data - (0x0d - 2); i++)
         {
            x = dst_ptr % 320;
            y = dst_ptr / 320;
            ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = rng[ptr];
            if (++x >= 320)
            {
               x = 0;
               ++y;
            }
            ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = rng[ptr + 1];
            dst_ptr += 2;
         }
         ptr += 2;
         break;

      case 0x11:
         data = *(rng + ptr++);
         for (i = 0; i <= data; i++)
         {
            x = dst_ptr % 320;
            y = dst_ptr / 320;
            ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = rng[ptr];
            if (++x >= 320)
            {
               x = 0;
               ++y;
            }
            ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = rng[ptr + 1];
            dst_ptr += 2;
         }
         ptr += 2;
         break;

      case 0x12:
         n = SWAP16(*(LPWORD)(rng + ptr)) + 1;
         ptr += 2;
         for (i = 0; i < n; i++)
         {
            x = dst_ptr % 320;
            y = dst_ptr / 320;
            ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = rng[ptr];
            if (++x >= 320)
            {
               x = 0;
               ++y;
            }
            ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = rng[ptr + 1];
            dst_ptr += 2;
         }
         ptr += 2;
         break;
      }
   }

end:
   free(rng);
   return 0;
}
