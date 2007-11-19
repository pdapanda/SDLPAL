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

LPGLOBALVARS gpGlobals = NULL;

VOID PAL_Shutdown(VOID);

static FILE *
PAL_OpenRequiredFile(
   LPCSTR       lpszFileName
)
/*++
  Purpose:

    Open a required file. If fails, quit the program.

  Parameters:

    [IN]  lpszFileName - file name to open.

  Return value:

    Pointer to the file.

--*/
{
   FILE         *fp;

   fp = fopen(lpszFileName, "rb");

   if (fp == NULL)
   {
      TerminateOnError("File not found: %s!\n", lpszFileName);
   }

   return fp;
}

INT
PAL_InitGlobals(
   VOID
)
/*++
  Purpose:

    Initialize global data.

  Parameters:

    None.

  Return value:

    0 = success, -1 = error.

--*/
{
   if (gpGlobals == NULL)
   {
      gpGlobals = (LPGLOBALVARS)calloc(1, sizeof(GLOBALVARS));
      if (gpGlobals == NULL)
      {
         return -1;
      }
   }

   //
   // Open files
   //
   gpGlobals->fpFBP = PAL_OpenRequiredFile("fbp.mkf");
   gpGlobals->fpMAP = PAL_OpenRequiredFile("map.mkf");
   gpGlobals->fpGOP = PAL_OpenRequiredFile("gop.mkf");
   gpGlobals->fpRNG = PAL_OpenRequiredFile("rng.mkf");
   gpGlobals->fpPAT = PAL_OpenRequiredFile("pat.mkf");
   gpGlobals->fpMGO = PAL_OpenRequiredFile("mgo.mkf");
   gpGlobals->fpABC = PAL_OpenRequiredFile("abc.mkf");
   gpGlobals->fpBALL = PAL_OpenRequiredFile("ball.mkf");
   gpGlobals->fpDATA = PAL_OpenRequiredFile("data.mkf");
   gpGlobals->fpF = PAL_OpenRequiredFile("f.mkf");
   gpGlobals->fpFIRE = PAL_OpenRequiredFile("fire.mkf");
   gpGlobals->fpRGM = PAL_OpenRequiredFile("rgm.mkf");
   gpGlobals->fpSSS = PAL_OpenRequiredFile("sss.mkf");
   gpGlobals->fpVOC = PAL_OpenRequiredFile("voc.mkf");

   return 0;
}

VOID
PAL_FreeGlobals(
   VOID
)
/*++
  Purpose:

    Free global data.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   if (gpGlobals != NULL)
   {
      //
      // Close all opened files
      //
      fclose(gpGlobals->fpFBP);
      fclose(gpGlobals->fpGOP);
      fclose(gpGlobals->fpMAP);
      fclose(gpGlobals->fpRNG);
      fclose(gpGlobals->fpPAT);
      fclose(gpGlobals->fpMGO);
      fclose(gpGlobals->fpABC);
      fclose(gpGlobals->fpBALL);
      fclose(gpGlobals->fpDATA);
      fclose(gpGlobals->fpF);
      fclose(gpGlobals->fpFIRE);
      fclose(gpGlobals->fpRGM);
      fclose(gpGlobals->fpSSS);
      fclose(gpGlobals->fpVOC);

      //
      // Delete the instance
      //
      free(gpGlobals);
   }

   gpGlobals = NULL;
}
