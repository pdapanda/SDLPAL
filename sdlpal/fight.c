//
// Copyright (c) 2009, Wei Mingzhi <whistler@openoffice.org>.
// All rights reserved.
//
// This file is part of SDLPAL.
//
// SDLPAL is free software: you can redistribute it and/or modify
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
#include <math.h>

// Change this to FALSE to disable Active-Time Battle
BOOL g_fActiveTime = TRUE;

static BOOL
PAL_IsPlayerDying(
   WORD        wPlayerRole
)
/*++
  Purpose:

    Check if the player is dying.

  Parameters:

    [IN]  wPlayerRole - the player role ID.

  Return value:

    TRUE if the player is dying, FALSE if not.

--*/
{
   return gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] < gpGlobals->g.PlayerRoles.rgwMaxHP[wPlayerRole] / 5;
}

INT
PAL_BattleSelectAutoTarget(
   VOID
)
/*++
  Purpose:

    Pick an enemy target automatically.

  Parameters:

    None.

  Return value:

    The index of enemy. -1 if failed.

--*/
{
   int          i;

   for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
   {
      if (g_Battle.rgEnemy[i].wObjectID != 0 &&
         g_Battle.rgEnemy[i].e.wHealth > 0)
      {
         return i;
      }
   }

   return -1;
}

static SHORT
PAL_CalcBaseDamage(
   WORD        wAttackStrength,
   WORD        wDefense
)
/*++
  Purpose:

    Calculate the base damage value of attacking.

  Parameters:

    [IN]  wAttackStrength - attack strength of attacker.

    [IN]  wDefense - defense value of inflictor.

  Return value:

    The base damage value of the attacking.

--*/
{
   SHORT            sDamage;

   //
   // Formula courtesy of palxex and shenyanduxing
   //
   if (wAttackStrength > wDefense)
   {
      sDamage = (SHORT)(wAttackStrength * 2 - wDefense * 1.6 + 0.5);
   }
   else if (wAttackStrength > wDefense * 0.6)
   {
      sDamage = (SHORT)(wAttackStrength - wDefense * 0.6 + 0.5);
   }
   else
   {
      sDamage = 0;
   }

   return sDamage;
}

static SHORT
PAL_CalcMagicDamage(
   WORD             wMagicStrength,
   WORD             wDefense,
   const WORD       rgwElementalResistance[NUM_MAGIC_ELEMENTAL],
   WORD             wPoisonResistance,
   WORD             wMagicID
)
/*++
   Purpose:

     Calculate the damage of magic.

   Parameters:

     [IN]  wMagicStrength - magic strength of attacker.

     [IN]  wDefense - defense value of inflictor.

     [IN]  rgwElementalResistance - inflictor's resistance to the elemental magics.

     [IN]  wPoisonResistance - inflictor's resistance to poison.

     [IN]  wMagicID - object ID of the magic.

   Return value:

     The damage value of the magic attack.

--*/
{
   SHORT           sDamage;
   WORD            wElem;

   wMagicID = gpGlobals->g.rgObject[wMagicID].magic.wMagicNumber;

   //
   // Formula courtesy of palxex and shenyanduxing
   //
   wMagicStrength *= RandomFloat(10, 11);
   wMagicStrength /= 10;

   sDamage = PAL_CalcBaseDamage(wMagicStrength, wDefense);
   sDamage /= 4;

   sDamage += gpGlobals->g.lprgMagic[wMagicID].wBaseDamage;

   if (gpGlobals->g.lprgMagic[wMagicID].wElemental != 0)
   {
      wElem = gpGlobals->g.lprgMagic[wMagicID].wElemental;

      if (wElem > NUM_MAGIC_ELEMENTAL)
      {
         sDamage *= 10 - wPoisonResistance;
      }
      else if (wElem == 0)
      {
         sDamage *= 5;
      }
      else
      {
         sDamage *= 10 - rgwElementalResistance[wElem - 1];
      }

      sDamage /= 5;

      if (wElem <= NUM_MAGIC_ELEMENTAL)
      {
         sDamage *= 10 + gpGlobals->g.lprgBattleField[gpGlobals->wNumBattleField].rgsMagicEffect[wElem - 1];
         sDamage /= 10;
      }
   }

   return sDamage;
}

SHORT
PAL_CalcPhysicalAttackDamage(
   WORD           wAttackStrength,
   WORD           wDefense,
   WORD           wAttackResistance
)
/*++
  Purpose:

    Calculate the damage value of physical attacking.

  Parameters:

    [IN]  wAttackStrength - attack strength of attacker.

    [IN]  wDefense - defense value of inflictor.

    [IN]  wAttackResistance - inflictor's resistance to physical attack.

  Return value:

    The damage value of the physical attacking.

--*/
{
   SHORT             sDamage;

   sDamage = PAL_CalcBaseDamage(wAttackStrength, wDefense);
   sDamage /= wAttackResistance;

   return sDamage;
}

static SHORT
PAL_GetEnemyDexterity(
   WORD          wEnemyIndex
)
/*++
  Purpose:

    Get the dexterity value of the enemy.

  Parameters:

    [IN]  wEnemyIndex - the index of the enemy.

  Return value:

    The dexterity value of the enemy.

--*/
{
   SHORT      s;

   assert(g_Battle.rgEnemy[wEnemyIndex].wObjectID != 0);

   s = (g_Battle.rgEnemy[wEnemyIndex].e.wLevel + 6) * 3;
   s += (SHORT)g_Battle.rgEnemy[wEnemyIndex].e.wDexterity;

   if (g_Battle.rgEnemy[wEnemyIndex].rgStatus[kStatusHaste] != 0)
   {
      s *= 6;
      s /= 5;
   }
   else if (g_Battle.rgEnemy[wEnemyIndex].rgStatus[kStatusSlow] != 0)
   {
      s *= 2;
      s /= 3;
   }

   return s;
}

VOID
PAL_UpdateTimeChargingUnit(
   VOID
)
/*++
  Purpose:

    Update the base time unit of time-charging.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   g_Battle.flTimeChargingUnit = (FLOAT)(pow(PAL_GetPlayerDexterity(0) + 5, 0.3) / 3);
   g_Battle.flTimeChargingUnit /= PAL_GetPlayerDexterity(0);
}

FLOAT
PAL_GetTimeChargingSpeed(
   WORD           wDexterity
)
/*++
  Purpose:

    Calculate the time charging speed.

  Parameters:

    [IN]  wDexterity - the dexterity value of player or enemy.

  Return value:

    The time-charging speed of the player or enemy.

--*/
{
   if (g_Battle.UI.state == kBattleUISelectMove &&
      g_Battle.UI.MenuState != kBattleMenuMain)
   {
      //
      // Pause the time when there are submenus
      //
      return 0;
   }

   if (!g_fActiveTime && g_Battle.UI.state != kBattleUIWait)
   {
      return 0;
   }

   //
   // The battle should be faster when using Auto-Battle
   //
   if (gpGlobals->fAutoBattle)
   {
   	  wDexterity *= 3;
   }

   return g_Battle.flTimeChargingUnit * wDexterity;
}

static WORD
PAL_GetPlayerActualDexterity(
   WORD            wPlayerRole
)
/*++
  Purpose:

    Get player's actual dexterity value in battle.

  Parameters:

    [IN]  wPlayerRole - the player role ID.

  Return value:

    The player's actual dexterity value.

--*/
{
   WORD wDexterity = PAL_GetPlayerDexterity(wPlayerRole);

   if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusHaste] != 0)
   {
      wDexterity *= 6;
      wDexterity /= 5;
   }
   else if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSlow] != 0)
   {
      wDexterity *= 2;
      wDexterity /= 3;
   }

   if (PAL_IsPlayerDying(wPlayerRole))
   {
      //
      // player who is low of HP should be slower
      //
      wDexterity *= 2;
      wDexterity /= 3;
   }

   return wDexterity;
}

VOID
PAL_BattleDelay(
   WORD       wDuration,
   WORD       wObjectID,
   BOOL       fUpdateGesture
)
/*++
  Purpose:

    Delay a while during battle.

  Parameters:

    [IN]  wDuration - Number of frames of the delay.

    [IN]  wObjectID - The object ID to be displayed during the delay.

    [IN]  fUpdateGesture - TRUE if update the gesture for enemies, FALSE if not.

  Return value:

    None.

--*/
{
   int    i, j;
   DWORD  dwTime = SDL_GetTicks() + BATTLE_FRAME_TIME;

   for (i = 0; i < wDuration; i++)
   {
      if (fUpdateGesture)
      {
         //
         // Update the gesture of enemies.
         //
         for (j = 0; j <= g_Battle.wMaxEnemyIndex; j++)
         {
            if (g_Battle.rgEnemy[j].wObjectID == 0)
            {
               continue;
            }

            if (--g_Battle.rgEnemy[j].e.wIdleAnimSpeed == 0)
            {
               g_Battle.rgEnemy[j].wCurrentFrame++;
               g_Battle.rgEnemy[j].e.wIdleAnimSpeed =
                  gpGlobals->g.lprgEnemy[gpGlobals->g.rgObject[g_Battle.rgEnemy[j].wObjectID].enemy.wEnemyID].wIdleAnimSpeed;
            }

            if (g_Battle.rgEnemy[j].wCurrentFrame >= g_Battle.rgEnemy[j].e.wIdleFrames)
            {
               g_Battle.rgEnemy[j].wCurrentFrame = 0;
            }
         }
      }

      //
      // Clear the input state of previous frame.
      //
      PAL_ClearKeyState();

      //
      // Wait for the time of one frame. Accept input here.
      //
      PAL_ProcessEvent();
      while (SDL_GetTicks() <= dwTime)
      {
         PAL_ProcessEvent();
         SDL_Delay(1);
      }

      //
      // Set the time of the next frame.
      //
      dwTime = SDL_GetTicks() + BATTLE_FRAME_TIME;

      PAL_BattleMakeScene();
      SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);
      PAL_BattleUIUpdate();

      if (wObjectID != 0)
      {
         PAL_DrawText(PAL_GetWord(wObjectID), PAL_XY(210, 50),
            15, TRUE, FALSE);
      }

      VIDEO_UpdateScreen(NULL);
   }
}

static VOID
PAL_BattleBackupStat(
   VOID
)
/*++
  Purpose:

    Backup HP and MP values of all players and enemies.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   int i;
   WORD wPlayerRole;

   for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
   {
      if (g_Battle.rgEnemy[i].wObjectID == 0)
      {
         continue;
      }
      g_Battle.rgEnemy[i].wPrevHP = g_Battle.rgEnemy[i].e.wHealth;
   }

   for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
   {
      wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

      g_Battle.rgPlayer[i].wPrevHP =
         gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole];
      g_Battle.rgPlayer[i].wPrevMP =
         gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole];
   }
}

static BOOL
PAL_BattleDisplayStatChange(
   VOID
)
/*++
  Purpose:

    Display the HP and MP changes of all players and enemies.

  Parameters:

    None.

  Return value:

    TRUE if there are any number displayed, FALSE if not.

--*/
{
   int i, x, y;
   SHORT sDamage;
   WORD wPlayerRole;
   BOOL f = FALSE;

   for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
   {
      if (g_Battle.rgEnemy[i].wObjectID == 0)
      {
         continue;
      }

      if (g_Battle.rgEnemy[i].wPrevHP != g_Battle.rgEnemy[i].e.wHealth)
      {
         //
         // Show the number of damage
         //
         sDamage = g_Battle.rgEnemy[i].e.wHealth - g_Battle.rgEnemy[i].wPrevHP;

         x = PAL_X(g_Battle.rgEnemy[i].pos) - 9;
         y = PAL_Y(g_Battle.rgEnemy[i].pos) - 115;

         if (y < 10)
         {
            y = 10;
         }

         if (sDamage < 0)
         {
            PAL_BattleUIShowNum((WORD)(-sDamage), PAL_XY(x, y), kNumColorBlue);
         }
         else
         {
            PAL_BattleUIShowNum((WORD)(sDamage), PAL_XY(x, y), kNumColorYellow);
         }

         f = TRUE;
      }
   }

   for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
   {
      wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

      if (g_Battle.rgPlayer[i].wPrevHP != gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole])
      {
         sDamage =
            gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] - g_Battle.rgPlayer[i].wPrevHP;

         x = PAL_X(g_Battle.rgPlayer[i].pos) - 9;
         y = PAL_Y(g_Battle.rgPlayer[i].pos) - 75;

         if (y < 10)
         {
            y = 10;
         }

         if (sDamage < 0)
         {
            PAL_BattleUIShowNum((WORD)(-sDamage), PAL_XY(x, y), kNumColorBlue);
         }
         else
         {
            PAL_BattleUIShowNum((WORD)(sDamage), PAL_XY(x, y), kNumColorYellow);
         }

         f = TRUE;
      }

      if (g_Battle.rgPlayer[i].wPrevMP != gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole])
      {
         sDamage =
            gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole] - g_Battle.rgPlayer[i].wPrevMP;

         x = PAL_X(g_Battle.rgPlayer[i].pos) - 9;
         y = PAL_Y(g_Battle.rgPlayer[i].pos) - 67;

         if (y < 10)
         {
            y = 10;
         }

         //
         // Only show MP increasing
         //
         if (sDamage > 0)
         {
            PAL_BattleUIShowNum((WORD)(sDamage), PAL_XY(x, y), kNumColorCyan);
         }

         f = TRUE;
      }
   }

   return f;
}

static VOID
PAL_BattlePostActionCheck(
   BOOL      fCheckPlayers
)
/*++
  Purpose:

    Essential checks after an action is executed.

  Parameters:

    [IN]  fCheckPlayers - TRUE if check for players, FALSE if not.

  Return value:

    None.

--*/
{
   int      i, j;
   BOOL     fFade = FALSE;

   for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
   {
      if (g_Battle.rgEnemy[i].wObjectID == 0)
      {
         continue;
      }

      if ((SHORT)(g_Battle.rgEnemy[i].e.wHealth) <= 0)
      {
         //
         // This enemy is KO'ed
         //
         SOUND_Play(g_Battle.rgEnemy[i].e.wDeathSound);
         g_Battle.rgEnemy[i].wObjectID = 0;
         fFade = TRUE;
      }
   }

   if (fCheckPlayers && !gpGlobals->fAutoBattle)
   {
      for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
      {
         WORD w = gpGlobals->rgParty[i].wPlayerRole, wName;

         if (gpGlobals->g.PlayerRoles.rgwHP[w] < g_Battle.rgPlayer[i].wPrevHP &&
            gpGlobals->g.PlayerRoles.rgwHP[w] == 0)
         {
            w = gpGlobals->g.PlayerRoles.rgwCoveredBy[w];

            for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
            {
               if (gpGlobals->rgParty[j].wPlayerRole == w)
               {
                  break;
               }
            }

            if (j > gpGlobals->wMaxPartyMemberIndex)
            {
               continue;
            }

            if (gpGlobals->g.PlayerRoles.rgwHP[w] > 0)
            {
               wName = gpGlobals->g.PlayerRoles.rgwName[w];

               if (gpGlobals->g.rgObject[wName].player.wScriptOnFriendDeath != 0)
               {
                  PAL_BattleMakeScene();
                  SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);
                  VIDEO_UpdateScreen(NULL);

                  g_Battle.BattleResult = kBattleResultPause;

                  gpGlobals->g.rgObject[wName].player.wScriptOnFriendDeath =
                     PAL_RunTriggerScript(gpGlobals->g.rgObject[wName].player.wScriptOnFriendDeath, w);

                  g_Battle.BattleResult = kBattleResultOnGoing;

                  goto end;
               }
            }
         }
      }

      for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
      {
         WORD w = gpGlobals->rgParty[i].wPlayerRole, wName;

         if (gpGlobals->g.PlayerRoles.rgwHP[w] < g_Battle.rgPlayer[i].wPrevHP)
         {
            if (gpGlobals->g.PlayerRoles.rgwHP[w] > 0 && PAL_IsPlayerDying(w) &&
               g_Battle.rgPlayer[i].wPrevHP >= gpGlobals->g.PlayerRoles.rgwMaxHP[w] / 5)
            {
               WORD wCover = gpGlobals->g.PlayerRoles.rgwCoveredBy[w];

               for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
               {
                  if (gpGlobals->rgParty[j].wPlayerRole == wCover)
                  {
                     break;
                  }
               }

               if (j > gpGlobals->wMaxPartyMemberIndex || gpGlobals->g.PlayerRoles.rgwHP[wCover] == 0)
               {
                  continue;
               }

               wName = gpGlobals->g.PlayerRoles.rgwName[w];

               SOUND_Play(gpGlobals->g.PlayerRoles.rgwDyingSound[w]);
               PAL_BattleDelay(10, 0, TRUE);

               if (gpGlobals->g.rgObject[wName].player.wScriptOnDying != 0)
               {
                  PAL_BattleMakeScene();
                  SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);
                  VIDEO_UpdateScreen(NULL);

                  g_Battle.BattleResult = kBattleResultPause;

                  gpGlobals->g.rgObject[wName].player.wScriptOnDying =
                     PAL_RunTriggerScript(gpGlobals->g.rgObject[wName].player.wScriptOnDying, w);

                  g_Battle.BattleResult = kBattleResultOnGoing;
               }

               goto end;
            }
         }
      }
   }

end:
   if (fFade)
   {
      PAL_BattleBackupScene();
      PAL_BattleMakeScene();
      PAL_BattleFadeScene();
   }

   PAL_ClearKeyState();
}

VOID
PAL_BattleUpdateFighters(
   VOID
)
/*++
  Purpose:

    Update players' and enemies' gestures and locations in battle.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   int        i;
   WORD       wPlayerRole;

   //
   // Update the gesture for all players
   //
   for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
   {
      wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

      g_Battle.rgPlayer[i].pos = g_Battle.rgPlayer[i].posOriginal;
      g_Battle.rgPlayer[i].iColorShift = 0;

      if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0)
      {
         if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusPuppet] == 0)
         {
            g_Battle.rgPlayer[i].wCurrentFrame = 2; // dead
         }
         else
         {
            g_Battle.rgPlayer[i].wCurrentFrame = 0; // puppet
         }
      }
      else if (PAL_IsPlayerDying(wPlayerRole) ||
         gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSleep] != 0)
      {
         g_Battle.rgPlayer[i].wCurrentFrame = 1;
      }
      else if (g_Battle.rgPlayer[i].state == kFighterAct &&
         g_Battle.rgPlayer[i].action.ActionType == kBattleActionMagic)
      {
         //
         // Player is using a magic
         //
         g_Battle.rgPlayer[i].wCurrentFrame = 5;
      }
      else if (g_Battle.rgPlayer[i].fDefending)
      {
         g_Battle.rgPlayer[i].wCurrentFrame = 3;
      }
      else
      {
         g_Battle.rgPlayer[i].wCurrentFrame = 0;
      }
   }

   //
   // Update the gesture for all enemies
   //
   for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
   {
      if (g_Battle.rgEnemy[i].wObjectID == 0)
      {
         continue;
      }

      g_Battle.rgEnemy[i].pos = g_Battle.rgEnemy[i].posOriginal;
      g_Battle.rgEnemy[i].iColorShift = 0;

      if (g_Battle.rgEnemy[i].rgStatus[kStatusSleep] > 0)
      {
         g_Battle.rgEnemy[i].wCurrentFrame = 0;
         continue;
      }

      if (--g_Battle.rgEnemy[i].e.wIdleAnimSpeed == 0)
      {
         g_Battle.rgEnemy[i].wCurrentFrame++;
         g_Battle.rgEnemy[i].e.wIdleAnimSpeed =
            gpGlobals->g.lprgEnemy[gpGlobals->g.rgObject[g_Battle.rgEnemy[i].wObjectID].enemy.wEnemyID].wIdleAnimSpeed;
      }

      if (g_Battle.rgEnemy[i].wCurrentFrame >= g_Battle.rgEnemy[i].e.wIdleFrames)
      {
         g_Battle.rgEnemy[i].wCurrentFrame = 0;
      }
   }
}

VOID
PAL_BattleStartFrame(
   VOID
)
/*++
  Purpose:

    Called once per video frame in battle.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   int                      i;
   int                      iMax;
   BOOL                     fEnded;
   WORD                     wPlayerRole;
   WORD                     wDexterity;
   FLOAT                    flMax;
   BOOL                     fMoved = FALSE, fOnlyPuppet = TRUE;

   PAL_BattleUpdateFighters();

   //
   // Update the scene
   //
   PAL_BattleMakeScene();
   SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);

   //
   // Check if the battle is over
   //
   fEnded = TRUE;

   for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
   {
      if (g_Battle.rgEnemy[i].wObjectID != 0)
      {
         fEnded = FALSE;
         break;
      }
   }

   if (fEnded)
   {
      //
      // All enemies are cleared. Won the battle.
      //
      g_Battle.BattleResult = kBattleResultWon;
      SOUND_Play(-1);
      return;
   }
   else
   {
      fEnded = TRUE;

      for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
      {
         wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

         if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] != 0)
         {
            fOnlyPuppet = FALSE;
            fEnded = FALSE;
            break;
         }
         else if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusPuppet] != 0)
         {
            fEnded = FALSE;
         }
      }

      if (fEnded)
      {
         //
         // All players are dead. Lost the battle.
         //
         g_Battle.BattleResult = kBattleResultLost;
         return;
      }
   }

   //
   // Check for hiding status
   //
   if (g_Battle.iHidingTime > 0)
   {
      if (PAL_GetTimeChargingSpeed(1000) > 0)
      {
         g_Battle.iHidingTime--;
      }

      if (g_Battle.iHidingTime == 0)
      {
         PAL_BattleBackupScene();
         PAL_BattleMakeScene();
         PAL_BattleFadeScene();
      }
   }

   //
   // Run the logic for all enemies
   //
   for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
   {
      if (g_Battle.rgEnemy[i].wObjectID == 0)
      {
         continue;
      }

      if (g_Battle.rgEnemy[i].fTurnStart)
      {
         g_Battle.rgEnemy[i].wScriptOnTurnStart =
            PAL_RunTriggerScript(g_Battle.rgEnemy[i].wScriptOnTurnStart, i);

         g_Battle.rgEnemy[i].fTurnStart = FALSE;
      }
   }

   for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
   {
      if (g_Battle.rgEnemy[i].wObjectID == 0)
      {
         continue;
      }

      switch (g_Battle.rgEnemy[i].state)
      {
      case kFighterWait:
         flMax = PAL_GetTimeChargingSpeed(PAL_GetEnemyDexterity(i));
         flMax /= (gpGlobals->fAutoBattle ? 2 : 1);

         if (flMax != 0)
         {
            g_Battle.rgEnemy[i].flTimeMeter += flMax;

            if (g_Battle.rgEnemy[i].flTimeMeter > 100 && flMax > 0)
            {
               g_Battle.rgEnemy[i].state = kFighterCom;
            }
         }
         break;

      case kFighterCom:
         g_Battle.rgEnemy[i].wScriptOnReady =
            PAL_RunTriggerScript(g_Battle.rgEnemy[i].wScriptOnReady, i);
         g_Battle.rgEnemy[i].state = kFighterAct;
         fMoved = TRUE;
         break;

      case kFighterAct:
         if (!fMoved && PAL_GetTimeChargingSpeed(9999) > 0 && !fOnlyPuppet)
         {
            fMoved = TRUE;

            g_Battle.fEnemyMoving = TRUE;

            if (g_Battle.iHidingTime == 0)
            {
               PAL_BattleEnemyPerformAction(i);
            }

            g_Battle.rgEnemy[i].flTimeMeter = 0;
            g_Battle.rgEnemy[i].state = kFighterWait;
            g_Battle.fEnemyMoving = FALSE;

            if (!g_Battle.rgEnemy[i].fFirstMoveDone)
            {
               if (g_Battle.rgEnemy[i].e.wDualMove >= 2 ||
                  (g_Battle.rgEnemy[i].e.wDualMove != 0 && RandomLong(0, 1)))
               {
                  g_Battle.rgEnemy[i].flTimeMeter = 100;
                  g_Battle.rgEnemy[i].state = kFighterCom;
                  g_Battle.rgEnemy[i].fFirstMoveDone = TRUE;
                  break;
               }
            }

            g_Battle.rgEnemy[i].fFirstMoveDone = FALSE;
            g_Battle.rgEnemy[i].fTurnStart = TRUE;
         }
         break;
      }
   }

   //
   // Update the battle UI
   //
   PAL_BattleUIUpdate();

   //
   // Run the logic for all players
   //
   for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
   {
      wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

      //
      // Skip dead players
      //
      if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0 &&
         gpGlobals->rgPlayerStatus[wPlayerRole][kStatusPuppet] == 0)
      {
         g_Battle.rgPlayer[i].state = kFighterWait;
         g_Battle.rgPlayer[i].flTimeMeter = 0;
         continue;
      }

      switch (g_Battle.rgPlayer[i].state)
      {
      case kFighterWait:
         wDexterity = PAL_GetPlayerActualDexterity(wPlayerRole);
         g_Battle.rgPlayer[i].flTimeMeter +=
            PAL_GetTimeChargingSpeed(wDexterity) * g_Battle.rgPlayer[i].flTimeSpeedModifier;
         break;

      case kFighterCom:
         if (g_Battle.UI.state == kBattleUIWait)
         {
            PAL_BattleUIPlayerReady(i);
         }
         break;

      case kFighterAct:
         if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSleep] > 0 ||
            gpGlobals->rgPlayerStatus[wPlayerRole][kStatusConfused] > 0)
         {
            g_Battle.rgPlayer[i].action.ActionType = kBattleActionPass;
            g_Battle.rgPlayer[i].action.flRemainingTime = 0;
         }

         wDexterity = PAL_GetPlayerActualDexterity(wPlayerRole);
         g_Battle.rgPlayer[i].action.flRemainingTime -= PAL_GetTimeChargingSpeed(wDexterity);

         if (g_Battle.rgPlayer[i].action.flRemainingTime < 0 && !fMoved)
         {
            //
            // Perform the action for this player.
            //
            PAL_BattlePlayerPerformAction(i);

            fMoved = TRUE;

            //
            // Reduce the time for other players when uses coopmagic
            //
            if (g_Battle.rgPlayer[i].action.ActionType == kBattleActionCoopMagic)
            {
               for (iMax = 0; iMax <= gpGlobals->wMaxPartyMemberIndex; iMax++)
               {
                  g_Battle.rgPlayer[iMax].flTimeMeter = 0;
                  g_Battle.rgPlayer[iMax].flTimeSpeedModifier = 2.0f;
               }
            }
            else
            {
               g_Battle.rgPlayer[i].flTimeMeter = 0;
            }

            g_Battle.rgPlayer[i].flTimeSpeedModifier = 1.0f;
         }
         break;
      }
   }

   //
   // Start the UI for the fastest and ready player
   //
   flMax = 0;
   iMax = 0;

   for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
   {
      if (g_Battle.rgPlayer[i].state == kFighterCom ||
         (g_Battle.rgPlayer[i].state == kFighterAct && g_Battle.rgPlayer[i].action.ActionType == kBattleActionCoopMagic))
      {
         flMax = 0;
         break;
      }
      else if (g_Battle.rgPlayer[i].state == kFighterWait)
      {
         if (g_Battle.rgPlayer[i].flTimeMeter > flMax)
         {
            iMax = i;
            flMax = g_Battle.rgPlayer[i].flTimeMeter;
         }
      }
   }

   if (flMax > 100.0f)
   {
      g_Battle.rgPlayer[iMax].state = kFighterCom;
      g_Battle.rgPlayer[iMax].fDefending = FALSE;
   }
}

VOID
PAL_BattleCommitAction(
   BOOL           fRepeat
)
/*++
  Purpose:

    Commit the action which the player decided.

  Parameters:

    [IN]  fRepeat - TRUE if repeat the last action.

  Return value:

    None.

--*/
{
   WORD      w;

   if (!fRepeat)
   {
      g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.ActionType =
         g_Battle.UI.wActionType;
      g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.sTarget =
         (SHORT)g_Battle.UI.wSelectedIndex;
      g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.wActionID =
         g_Battle.UI.wObjectID;
   }

   //
   // Check if the action is valid
   //
   switch (g_Battle.UI.wActionType)
   {
   case kBattleActionMagic:
      w = gpGlobals->g.lprgMagic[gpGlobals->g.rgObject[g_Battle.UI.wObjectID].magic.wMagicNumber].wCostMP;
      if (gpGlobals->g.PlayerRoles.rgwMP[gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole] < w)
      {
         w = gpGlobals->g.lprgMagic[gpGlobals->g.rgObject[g_Battle.UI.wObjectID].magic.wMagicNumber].wType;
         if (w == kMagicTypeApplyToPlayer || w == kMagicTypeApplyToParty ||
            w == kMagicTypeTrance)
         {
            g_Battle.UI.wActionType = kBattleActionDefend;
         }
         else
         {
            g_Battle.UI.wActionType = kBattleActionAttack;
            if (g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.sTarget == -1)
            {
               g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.sTarget = 0;
            }
         }
      }
      break;

   case kBattleActionThrowItem:
   case kBattleActionUseItem:
      break;
   }

   //
   // Calculate the waiting time for the action
   //
   switch (g_Battle.UI.wActionType)
   {
   case kBattleActionMagic:
      {
         LPMAGIC      p;
         WORD         wCostMP;

         //
         // The base casting time of magic is set to the MP costed
         //
         p = &(gpGlobals->g.lprgMagic[gpGlobals->g.rgObject[g_Battle.UI.wObjectID].magic.wMagicNumber]);
         wCostMP = p->wCostMP;

         if (wCostMP == 1)
         {
            if (p->wType == kMagicTypeSummon)
            {
               //
               // The Wine God is an ultimate move which should take long
               //
               wCostMP = 200;
            }
         }
         else if (p->wType == kMagicTypeApplyToPlayer || p->wType == kMagicTypeApplyToParty ||
            p->wType == kMagicTypeTrance)
         {
            //
            // Healing magics should take shorter
            //
            wCostMP /= 3;
         }

         g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.flRemainingTime = wCostMP + 5;
      }
      break;

   case kBattleActionFlee:
      //
      // Fleeing should take a fairly long time
      //
      g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.flRemainingTime = RandomFloat(25, 75);
      break;

   case kBattleActionDefend:
   case kBattleActionCoopMagic:
      //
      // Defend takes no time
      //
      g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.flRemainingTime = 0;
      break;

   default:
      g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.flRemainingTime = 5;
      break;
   }

   g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].state = kFighterAct;
   g_Battle.UI.state = kBattleUIWait;
}

static VOID
PAL_BattleShowPlayerAttackAnim(
   WORD        wPlayerIndex,
   BOOL        fCritical
)
/*++
  Purpose:

    Show the physical attack effect for player.

  Parameters:

    [IN]  wPlayerIndex - the index of the player.

    [IN]  fCritical - TRUE if this is a critical hit.

  Return value:

    None.

--*/
{
   WORD wPlayerRole = gpGlobals->rgParty[wPlayerIndex].wPlayerRole;
   SHORT sTarget = g_Battle.rgPlayer[wPlayerIndex].action.sTarget;

   int index, i, j;
   int enemy_x = 0, enemy_y = 0, enemy_h = 0, x, y, dist = 0;

   DWORD dwTime;

   if (sTarget != -1)
   {
      enemy_x = PAL_X(g_Battle.rgEnemy[sTarget].pos);
      enemy_y = PAL_Y(g_Battle.rgEnemy[sTarget].pos);

      enemy_h = PAL_RLEGetHeight(PAL_SpriteGetFrame(g_Battle.rgEnemy[sTarget].lpSprite, g_Battle.rgEnemy[sTarget].wCurrentFrame));

      if (sTarget >= 3)
      {
         dist = (sTarget - wPlayerIndex) * 8;
      }
   }
   else
   {
      enemy_x = 150;
      enemy_y = 100;
   }

   index = gpGlobals->g.rgwBattleEffectIndex[PAL_GetPlayerBattleSprite(wPlayerRole)][1];
   index *= 3;

   //
   // Play the attack voice
   //
   if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] > 0)
   {
      if (!fCritical)
      {
         SOUND_Play(gpGlobals->g.PlayerRoles.rgwAttackSound[wPlayerRole]);
      }
      else
      {
         SOUND_Play(gpGlobals->g.PlayerRoles.rgwCriticalSound[wPlayerRole]);
      }
   }

   //
   // Show the animation
   //
   x = enemy_x - dist + 64;
   y = enemy_y + dist + 20;

   g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 8;
   g_Battle.rgPlayer[wPlayerIndex].pos = PAL_XY(x, y);

   PAL_BattleDelay(2, 0, TRUE);

   x -= 10;
   y -= 2;
   g_Battle.rgPlayer[wPlayerIndex].pos = PAL_XY(x, y);

   PAL_BattleDelay(1, 0, TRUE);

   g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 9;
   x -= 16;
   y -= 4;

   SOUND_Play(gpGlobals->g.PlayerRoles.rgwWeaponSound[wPlayerRole]);

   x = enemy_x;
   y = enemy_y - enemy_h / 3 + 10;

   dwTime = SDL_GetTicks() + BATTLE_FRAME_TIME;

   for (i = 0; i < 3; i++)
   {
      LPCBITMAPRLE b = PAL_SpriteGetFrame(g_Battle.lpEffectSprite, index++);

      //
      // Clear the input state of previous frame.
      //
      PAL_ClearKeyState();

      //
      // Wait for the time of one frame. Accept input here.
      //
      PAL_ProcessEvent();
      while (SDL_GetTicks() <= dwTime)
      {
         PAL_ProcessEvent();
         SDL_Delay(1);
      }

      //
      // Set the time of the next frame.
      //
      dwTime = SDL_GetTicks() + BATTLE_FRAME_TIME;

      //
      // Update the gesture of enemies.
      //
      for (j = 0; j <= g_Battle.wMaxEnemyIndex; j++)
      {
         if (g_Battle.rgEnemy[j].wObjectID == 0)
         {
            continue;
         }

         if (--g_Battle.rgEnemy[j].e.wIdleAnimSpeed == 0)
         {
            g_Battle.rgEnemy[j].wCurrentFrame++;
            g_Battle.rgEnemy[j].e.wIdleAnimSpeed =
               gpGlobals->g.lprgEnemy[gpGlobals->g.rgObject[g_Battle.rgEnemy[j].wObjectID].enemy.wEnemyID].wIdleAnimSpeed;
         }

         if (g_Battle.rgEnemy[j].wCurrentFrame >= g_Battle.rgEnemy[j].e.wIdleFrames)
         {
            g_Battle.rgEnemy[j].wCurrentFrame = 0;
         }
      }

      PAL_BattleMakeScene();
      SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);

      PAL_RLEBlitToSurface(b, gpScreen, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
      x -= 16;
      y += 16;

      PAL_BattleUIUpdate();

      if (i == 0)
      {
         if (sTarget == -1)
         {
            for (j = 0; j <= g_Battle.wMaxEnemyIndex; j++)
            {
               g_Battle.rgEnemy[j].iColorShift = 6;
            }
         }
         else
         {
            g_Battle.rgEnemy[sTarget].iColorShift = 6;
         }

         //
         // Flash the screen if it's a critical hit
         //
         if (fCritical)
         {
            SDL_FillRect(gpScreen, NULL, 15);
         }
      }

      VIDEO_UpdateScreen(NULL);

      if (i == 1)
      {
         g_Battle.rgPlayer[wPlayerIndex].pos =
            PAL_XY(PAL_X(g_Battle.rgPlayer[wPlayerIndex].pos) + 2,
                   PAL_Y(g_Battle.rgPlayer[wPlayerIndex].pos) + 1);
      }
   }

   dist = 8;

   for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
   {
      g_Battle.rgEnemy[i].iColorShift = 0;
   }

   if (sTarget == -1)
   {
      for (i = 0; i < 3; i++)
      {
         for (j = 0; j <= g_Battle.wMaxEnemyIndex; j++)
         {
            x = PAL_X(g_Battle.rgEnemy[j].pos);
            y = PAL_Y(g_Battle.rgEnemy[j].pos);

            x -= dist;
            y -= dist / 2;
            g_Battle.rgEnemy[j].pos = PAL_XY(x, y);
         }

         PAL_BattleDelay(1, 0, TRUE);
         dist /= -2;
      }
   }
   else
   {
      x = PAL_X(g_Battle.rgEnemy[sTarget].pos);
      y = PAL_Y(g_Battle.rgEnemy[sTarget].pos);

      for (i = 0; i < 3; i++)
      {
         x -= dist;
         dist /= -2;
         y += dist;
         g_Battle.rgEnemy[sTarget].pos = PAL_XY(x, y);

         PAL_BattleDelay(1, 0, TRUE);
      }
   }
}

static VOID
PAL_BattleShowPlayerUseItemAnim(
   WORD         wPlayerIndex,
   WORD         wObjectID,
   SHORT        sTarget
)
/*++
  Purpose:

    Show the "use item" effect for player.

  Parameters:

    [IN]  wPlayerIndex - the index of the player.

    [IN]  wObjectID - the object ID of the item to be used.

    [IN]  sTarget - the target player of the action.

  Return value:

    None.

--*/
{
   int i, j;

   PAL_BattleDelay(4, 0, TRUE);

   g_Battle.rgPlayer[wPlayerIndex].pos =
      PAL_XY(PAL_X(g_Battle.rgPlayer[wPlayerIndex].pos) - 15,
             PAL_Y(g_Battle.rgPlayer[wPlayerIndex].pos) - 7);

   g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 5;

   SOUND_Play(28);

   for (i = 0; i <= 6; i++)
   {
      if (sTarget == -1)
      {
         for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
         {
            g_Battle.rgPlayer[j].iColorShift = i;
         }
      }
      else
      {
         g_Battle.rgPlayer[sTarget].iColorShift = i;
      }

      PAL_BattleDelay(1, wObjectID, TRUE);
   }

   for (i = 5; i >= 0; i--)
   {
      if (sTarget == -1)
      {
         for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
         {
            g_Battle.rgPlayer[j].iColorShift = i;
         }
      }
      else
      {
         g_Battle.rgPlayer[sTarget].iColorShift = i;
      }

      PAL_BattleDelay(1, wObjectID, TRUE);
   }
}

VOID
PAL_BattleShowPlayerPreMagicAnim(
   WORD         wPlayerIndex,
   BOOL         fSummon
)
/*++
  Purpose:

    Show the effect for player before using a magic.

  Parameters:

    [IN]  wPlayerIndex - the index of the player.

    [IN]  fSummon - TRUE if player is using a summon magic.

  Return value:

    None.

--*/
{
   int   i, j;
   DWORD dwTime = SDL_GetTicks();
   WORD  wPlayerRole = gpGlobals->rgParty[wPlayerIndex].wPlayerRole;

   for (i = 0; i < 4; i++)
   {
      g_Battle.rgPlayer[wPlayerIndex].pos =
         PAL_XY(PAL_X(g_Battle.rgPlayer[wPlayerIndex].pos) - (4 - i),
                PAL_Y(g_Battle.rgPlayer[wPlayerIndex].pos) - (4 - i) / 2);

      PAL_BattleDelay(1, 0, TRUE);
   }

   PAL_BattleDelay(2, 0, TRUE);

   g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 5;
   SOUND_Play(gpGlobals->g.PlayerRoles.rgwMagicSound[wPlayerRole]);

   if (!fSummon)
   {
      int x, y, index;

      x = PAL_X(g_Battle.rgPlayer[wPlayerIndex].pos);
      y = PAL_Y(g_Battle.rgPlayer[wPlayerIndex].pos);

      index = gpGlobals->g.rgwBattleEffectIndex[PAL_GetPlayerBattleSprite(wPlayerRole)][0];
      index *= 10;
      index += 15;

      for (i = 0; i < 10; i++)
      {
         LPCBITMAPRLE b = PAL_SpriteGetFrame(g_Battle.lpEffectSprite, index++);

         //
         // Clear the input state of previous frame.
         //
         PAL_ClearKeyState();

         //
         // Wait for the time of one frame. Accept input here.
         //
         PAL_ProcessEvent();
         while (SDL_GetTicks() <= dwTime)
         {
            PAL_ProcessEvent();
            SDL_Delay(1);
         }

         //
         // Set the time of the next frame.
         //
         dwTime = SDL_GetTicks() + BATTLE_FRAME_TIME;

         //
         // Update the gesture of enemies.
         //
         for (j = 0; j <= g_Battle.wMaxEnemyIndex; j++)
         {
            if (g_Battle.rgEnemy[j].wObjectID == 0)
            {
               continue;
            }

            if (--g_Battle.rgEnemy[j].e.wIdleAnimSpeed == 0)
            {
               g_Battle.rgEnemy[j].wCurrentFrame++;
               g_Battle.rgEnemy[j].e.wIdleAnimSpeed =
                  gpGlobals->g.lprgEnemy[gpGlobals->g.rgObject[g_Battle.rgEnemy[j].wObjectID].enemy.wEnemyID].wIdleAnimSpeed;
            }

            if (g_Battle.rgEnemy[j].wCurrentFrame >= g_Battle.rgEnemy[j].e.wIdleFrames)
            {
               g_Battle.rgEnemy[j].wCurrentFrame = 0;
            }
         }

         PAL_BattleMakeScene();
         SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);

         PAL_RLEBlitToSurface(b, gpScreen, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

         PAL_BattleUIUpdate();

         VIDEO_UpdateScreen(NULL);
      }
   }

   PAL_BattleDelay(1, 0, TRUE);
}

static VOID
PAL_BattleShowPlayerDefMagicAnim(
   WORD         wPlayerIndex,
   WORD         wObjectID,
   SHORT        sTarget
)
/*++
  Purpose:

    Show the defensive magic effect for player.

  Parameters:

    [IN]  wPlayerIndex - the index of the player.

    [IN]  wObjectID - the object ID of the magic to be used.

    [IN]  sTarget - the target player of the action.

  Return value:

    None.

--*/
{
   LPSPRITE   lpSpriteEffect;
   int        l, iMagicNum, iEffectNum, n, i, j, x, y;
   DWORD      dwTime = SDL_GetTicks();

   iMagicNum = gpGlobals->g.rgObject[wObjectID].magic.wMagicNumber;
   iEffectNum = gpGlobals->g.lprgMagic[iMagicNum].wEffect;

   l = PAL_MKFGetDecompressedSize(iEffectNum, gpGlobals->f.fpFIRE);
   if (l <= 0)
   {
      return;
   }

   lpSpriteEffect = (LPSPRITE)UTIL_malloc(l);

   PAL_MKFDecompressChunk((LPBYTE)lpSpriteEffect, l, iEffectNum, gpGlobals->f.fpFIRE);

   n = PAL_SpriteGetNumFrames(lpSpriteEffect);

   g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 6;
   PAL_BattleDelay(1, 0, TRUE);

   for (i = 0; i < n; i++)
   {
      LPCBITMAPRLE b = PAL_SpriteGetFrame(lpSpriteEffect, i);

      if (i == gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay)
      {
         SOUND_Play(gpGlobals->g.lprgMagic[iMagicNum].wSound);
      }

      //
      // Clear the input state of previous frame.
      //
      PAL_ClearKeyState();

      //
      // Wait for the time of one frame. Accept input here.
      //
      PAL_ProcessEvent();
      while (SDL_GetTicks() <= dwTime)
      {
         PAL_ProcessEvent();
         SDL_Delay(1);
      }

      //
      // Set the time of the next frame.
      //
      dwTime = SDL_GetTicks() +
         ((SHORT)(gpGlobals->g.lprgMagic[iMagicNum].wSpeed) + 5) * 10;

      PAL_BattleMakeScene();
      SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);

      if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeApplyToParty)
      {
         assert(sTarget == -1);

         for (l = 0; l <= gpGlobals->wMaxPartyMemberIndex; l++)
         {
            x = PAL_X(g_Battle.rgPlayer[l].pos);
            y = PAL_Y(g_Battle.rgPlayer[l].pos);

            x += (SHORT) gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
            y += (SHORT) gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

            PAL_RLEBlitToSurface(b, gpScreen,
               PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
         }
      }
      else if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeApplyToPlayer)
      {
         assert(sTarget != -1);

         x = PAL_X(g_Battle.rgPlayer[sTarget].pos);
         y = PAL_Y(g_Battle.rgPlayer[sTarget].pos);

         x += (SHORT) gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
         y += (SHORT) gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

         PAL_RLEBlitToSurface(b, gpScreen,
            PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

         //
         // Repaint the previous player
         //
         if (sTarget > 0 && g_Battle.iHidingTime == 0)
         {
            LPCBITMAPRLE p = PAL_SpriteGetFrame(g_Battle.rgPlayer[sTarget - 1].lpSprite, g_Battle.rgPlayer[sTarget - 1].wCurrentFrame);

            if (gpGlobals->rgPlayerStatus[gpGlobals->rgParty[sTarget - 1].wPlayerRole][kStatusConfused] == 0)
            {
               PAL_RLEBlitToSurface(p, gpScreen, g_Battle.rgPlayer[sTarget - 1].pos);
            }
         }
      }
      else
      {
         assert(FALSE);
      }

      PAL_BattleUIUpdate();

      VIDEO_UpdateScreen(NULL);
   }

   free(lpSpriteEffect);

   for (i = 0; i < 6; i++)
   {
      if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeApplyToParty)
      {
         for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
         {
            g_Battle.rgPlayer[j].iColorShift = i;
         }
      }
      else
      {
         g_Battle.rgPlayer[sTarget].iColorShift = i;
      }

      PAL_BattleDelay(1, 0, TRUE);
   }

   for (i = 6; i >= 0; i--)
   {
      if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeApplyToParty)
      {
         for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
         {
            g_Battle.rgPlayer[j].iColorShift = i;
         }
      }
      else
      {
         g_Battle.rgPlayer[sTarget].iColorShift = i;
      }

      PAL_BattleDelay(1, 0, TRUE);
   }
}

static VOID
PAL_BattleShowPlayerOffMagicAnim(
   WORD         wPlayerIndex,
   WORD         wObjectID,
   SHORT        sTarget
)
/*++
  Purpose:

    Show the offensive magic animation for player.

  Parameters:

    [IN]  wPlayerIndex - the index of the player.

    [IN]  wObjectID - the object ID of the magic to be used.

    [IN]  sTarget - the target enemy of the action.

  Return value:

    None.

--*/
{
   LPSPRITE   lpSpriteEffect;
   int        l, iMagicNum, iEffectNum, n, i, k, x, y, wave, blow;
   DWORD      dwTime = SDL_GetTicks();

   iMagicNum = gpGlobals->g.rgObject[wObjectID].magic.wMagicNumber;
   iEffectNum = gpGlobals->g.lprgMagic[iMagicNum].wEffect;

   l = PAL_MKFGetDecompressedSize(iEffectNum, gpGlobals->f.fpFIRE);
   if (l <= 0)
   {
      return;
   }

   lpSpriteEffect = (LPSPRITE)UTIL_malloc(l);

   PAL_MKFDecompressChunk((LPBYTE)lpSpriteEffect, l, iEffectNum, gpGlobals->f.fpFIRE);

   n = PAL_SpriteGetNumFrames(lpSpriteEffect);

   g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 6;
   PAL_BattleDelay(1, 0, TRUE);

   l = n - gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay;
   l *= (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wEffectTimes;
   l += n;
   l += gpGlobals->g.lprgMagic[iMagicNum].wShake;

   wave = gpGlobals->wScreenWave;
   gpGlobals->wScreenWave += gpGlobals->g.lprgMagic[iMagicNum].wWave;

   for (i = 0; i < l; i++)
   {
      LPCBITMAPRLE b;

      blow = ((g_Battle.iBlow > 0) ? RandomLong(0, g_Battle.iBlow) : RandomLong(g_Battle.iBlow, 0));

      for (k = 0; k <= g_Battle.wMaxEnemyIndex; k++)
      {
         if (g_Battle.rgEnemy[k].wObjectID == 0)
         {
            continue;
         }

         x = PAL_X(g_Battle.rgEnemy[k].pos) + blow;
         y = PAL_Y(g_Battle.rgEnemy[k].pos) + blow / 2;

         g_Battle.rgEnemy[k].pos = PAL_XY(x, y);
      }

      if (l - i > gpGlobals->g.lprgMagic[iMagicNum].wShake)
      {
         b = PAL_SpriteGetFrame(lpSpriteEffect, i % n);
      }
      else
      {
         VIDEO_ShakeScreen(i, 3);
         b = PAL_SpriteGetFrame(lpSpriteEffect, (l - gpGlobals->g.lprgMagic[iMagicNum].wShake - 1) % n);
      }

      if (i == gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay)
      {
         SOUND_Play(gpGlobals->g.lprgMagic[iMagicNum].wSound);
      }

      //
      // Clear the input state of previous frame.
      //
      PAL_ClearKeyState();

      //
      // Wait for the time of one frame. Accept input here.
      //
      PAL_ProcessEvent();
      while (SDL_GetTicks() <= dwTime)
      {
         PAL_ProcessEvent();
         SDL_Delay(1);
      }

      //
      // Set the time of the next frame.
      //
      dwTime = SDL_GetTicks() +
         ((SHORT)(gpGlobals->g.lprgMagic[iMagicNum].wSpeed) + 5) * 10;

      PAL_BattleMakeScene();
      SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);

      if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeNormal)
      {
         assert(sTarget != -1);

         x = PAL_X(g_Battle.rgEnemy[sTarget].pos);
         y = PAL_Y(g_Battle.rgEnemy[sTarget].pos);

         x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
         y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

         PAL_RLEBlitToSurface(b, gpScreen, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

         if (i == l - 1 && gpGlobals->wScreenWave < 9 &&
            gpGlobals->g.lprgMagic[iMagicNum].wKeepEffect == 0xFFFF)
         {
            PAL_RLEBlitToSurface(b, g_Battle.lpBackground,
               PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
         }
      }
      else if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackAll)
      {
         const int effectpos[3][2] = {{70, 140}, {100, 110}, {160, 100}};

         assert(sTarget == -1);

         for (k = 0; k < 3; k++)
         {
            x = effectpos[k][0];
            y = effectpos[k][1];

            x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
            y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

            PAL_RLEBlitToSurface(b, gpScreen, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

            if (i == l - 1 && gpGlobals->wScreenWave < 9 &&
               gpGlobals->g.lprgMagic[iMagicNum].wKeepEffect == 0xFFFF)
            {
               PAL_RLEBlitToSurface(b, g_Battle.lpBackground,
                  PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
            }
         }
      }
      else if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackWhole ||
         gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackField)
      {
         assert(sTarget == -1);

         if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackWhole)
         {
            x = 120;
            y = 100;
         }
         else
         {
            x = 160;
            y = 200;
         }

         x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
         y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

         PAL_RLEBlitToSurface(b, gpScreen, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

         if (i == l - 1 && gpGlobals->wScreenWave < 9 &&
            gpGlobals->g.lprgMagic[iMagicNum].wKeepEffect == 0xFFFF)
         {
            PAL_RLEBlitToSurface(b, g_Battle.lpBackground,
               PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
         }
      }
      else
      {
         assert(FALSE);
      }

      PAL_BattleUIUpdate();

      VIDEO_UpdateScreen(NULL);
   }

   gpGlobals->wScreenWave = wave;
   VIDEO_ShakeScreen(0, 0);

   free(lpSpriteEffect);

   for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
   {
      g_Battle.rgEnemy[i].pos = g_Battle.rgEnemy[i].posOriginal;
   }
}

static VOID
PAL_BattleShowEnemyMagicAnim(
   WORD         wObjectID,
   SHORT        sTarget
)
/*++
  Purpose:

    Show the offensive magic animation for enemy.

  Parameters:

    [IN]  wObjectID - the object ID of the magic to be used.

    [IN]  sTarget - the target player index of the action.

  Return value:

    None.

--*/
{
   LPSPRITE   lpSpriteEffect;
   int        l, iMagicNum, iEffectNum, n, i, k, x, y, wave, blow;
   DWORD      dwTime = SDL_GetTicks();

   iMagicNum = gpGlobals->g.rgObject[wObjectID].magic.wMagicNumber;
   iEffectNum = gpGlobals->g.lprgMagic[iMagicNum].wEffect;

   l = PAL_MKFGetDecompressedSize(iEffectNum, gpGlobals->f.fpFIRE);
   if (l <= 0)
   {
      return;
   }

   lpSpriteEffect = (LPSPRITE)UTIL_malloc(l);

   PAL_MKFDecompressChunk((LPBYTE)lpSpriteEffect, l, iEffectNum, gpGlobals->f.fpFIRE);

   n = PAL_SpriteGetNumFrames(lpSpriteEffect);

   l = n - gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay;
   l *= (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wEffectTimes;
   l += n;
   l += gpGlobals->g.lprgMagic[iMagicNum].wShake;

   wave = gpGlobals->wScreenWave;
   gpGlobals->wScreenWave += gpGlobals->g.lprgMagic[iMagicNum].wWave;

   for (i = 0; i < l; i++)
   {
      LPCBITMAPRLE b;

      blow = ((g_Battle.iBlow > 0) ? RandomLong(0, g_Battle.iBlow) : RandomLong(g_Battle.iBlow, 0));

      for (k = 0; k <= gpGlobals->wMaxPartyMemberIndex; k++)
      {
         x = PAL_X(g_Battle.rgPlayer[k].pos) - blow;
         y = PAL_Y(g_Battle.rgPlayer[k].pos) - blow / 2;

         g_Battle.rgPlayer[k].pos = PAL_XY(x, y);
      }

      if (l - i > gpGlobals->g.lprgMagic[iMagicNum].wShake)
      {
         b = PAL_SpriteGetFrame(lpSpriteEffect, i % n);
      }
      else
      {
         VIDEO_ShakeScreen(i, 3);
         b = PAL_SpriteGetFrame(lpSpriteEffect, (l - gpGlobals->g.lprgMagic[iMagicNum].wShake - 1) % n);
      }

      if (i == gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay)
      {
         SOUND_Play(gpGlobals->g.lprgMagic[iMagicNum].wSound);
      }

      //
      // Clear the input state of previous frame.
      //
      PAL_ClearKeyState();

      //
      // Wait for the time of one frame. Accept input here.
      //
      PAL_ProcessEvent();
      while (SDL_GetTicks() <= dwTime)
      {
         PAL_ProcessEvent();
         SDL_Delay(1);
      }

      //
      // Set the time of the next frame.
      //
      dwTime = SDL_GetTicks() +
         ((SHORT)(gpGlobals->g.lprgMagic[iMagicNum].wSpeed) + 5) * 10;

      PAL_BattleMakeScene();
      SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);

      if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeNormal)
      {
         assert(sTarget != -1);

         x = PAL_X(g_Battle.rgPlayer[sTarget].pos);
         y = PAL_Y(g_Battle.rgPlayer[sTarget].pos);

         x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
         y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

         PAL_RLEBlitToSurface(b, gpScreen, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

         if (i == l - 1 && gpGlobals->wScreenWave < 9 &&
            gpGlobals->g.lprgMagic[iMagicNum].wKeepEffect == 0xFFFF)
         {
            PAL_RLEBlitToSurface(b, g_Battle.lpBackground,
               PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
         }
      }
      else if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackAll)
      {
         const int effectpos[3][2] = {{180, 180}, {234, 170}, {270, 146}};

         assert(sTarget == -1);

         for (k = 0; k < 3; k++)
         {
            x = effectpos[k][0];
            y = effectpos[k][1];

            x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
            y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

            PAL_RLEBlitToSurface(b, gpScreen, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

            if (i == l - 1 && gpGlobals->wScreenWave < 9 &&
               gpGlobals->g.lprgMagic[iMagicNum].wKeepEffect == 0xFFFF)
            {
               PAL_RLEBlitToSurface(b, g_Battle.lpBackground,
                  PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
            }
         }
      }
      else if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackWhole ||
         gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackField)
      {
         assert(sTarget == -1);

         if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackWhole)
         {
            x = 240;
            y = 150;
         }
         else
         {
            x = 160;
            y = 200;
         }

         x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
         y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

         PAL_RLEBlitToSurface(b, gpScreen, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

         if (i == l - 1 && gpGlobals->wScreenWave < 9 &&
            gpGlobals->g.lprgMagic[iMagicNum].wKeepEffect == 0xFFFF)
         {
            PAL_RLEBlitToSurface(b, g_Battle.lpBackground,
               PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
         }
      }
      else
      {
         assert(FALSE);
      }

      PAL_BattleUIUpdate();

      VIDEO_UpdateScreen(NULL);
   }

   gpGlobals->wScreenWave = wave;
   VIDEO_ShakeScreen(0, 0);

   free(lpSpriteEffect);

   for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
   {
      g_Battle.rgPlayer[i].pos = g_Battle.rgPlayer[i].posOriginal;
   }
}

static VOID
PAL_BattleShowPlayerSummonMagicAnim(
   WORD         wPlayerIndex,
   WORD         wObjectID,
   SHORT        sTarget
)
/*++
  Purpose:

    Show the summon magic animation for player.

  Parameters:

    [IN]  wPlayerIndex - the index of the player.

    [IN]  wObjectID - the object ID of the magic to be used.

    [IN]  sTarget - the target enemy of the action.

  Return value:

    None.

--*/
{
}

static VOID
PAL_BattleShowPostMagicAnim(
   VOID
)
/*++
  Purpose:

    Show the post-magic animation.

  Parameters:

    None

  Return value:

    None.

--*/
{
   int i, j, x, y, dist = 8;
   PAL_POS rgEnemyPosBak[MAX_ENEMIES_IN_TEAM];

   for (i = 0; i < MAX_ENEMIES_IN_TEAM; i++)
   {
      rgEnemyPosBak[i] = g_Battle.rgEnemy[i].pos;
   }

   for (i = 0; i < 3; i++)
   {
      for (j = 0; j <= g_Battle.wMaxEnemyIndex; j++)
      {
         if (g_Battle.rgEnemy[j].e.wHealth == g_Battle.rgEnemy[j].wPrevHP)
         {
            continue;
         }

         x = PAL_X(g_Battle.rgEnemy[j].pos);
         y = PAL_Y(g_Battle.rgEnemy[j].pos);

         x -= dist;
         y -= dist / 2;

         g_Battle.rgEnemy[j].pos = PAL_XY(x, y);

         g_Battle.rgEnemy[j].iColorShift = ((i == 1) ? 6 : 0);
      }

      PAL_BattleDelay(1, 0, TRUE);
      dist /= -2;
   }

   for (i = 0; i < MAX_ENEMIES_IN_TEAM; i++)
   {
      g_Battle.rgEnemy[i].pos = rgEnemyPosBak[i];
   }
   PAL_BattleDelay(1, 0, TRUE);
}

static VOID
PAL_BattlePlayerValidateAction(
   WORD         wPlayerIndex
)
/*++
  Purpose:

    Validate player's action, fallback to other action when needed.

  Parameters:

    [IN]  wPlayerIndex - the index of the player.

  Return value:

    None.

--*/
{
   WORD   wPlayerRole = gpGlobals->rgParty[wPlayerIndex].wPlayerRole;
   WORD   wObjectID = g_Battle.rgPlayer[wPlayerIndex].action.wActionID;
   SHORT  sTarget = g_Battle.rgPlayer[wPlayerIndex].action.sTarget;
   BOOL   fValid = TRUE, fToEnemy = FALSE;
   WORD   w;
   int    i;

   switch (g_Battle.rgPlayer[wPlayerIndex].action.ActionType)
   {
   case kBattleActionAttack:
      fToEnemy = TRUE;
      break;

   case kBattleActionPass:
      break;

   case kBattleActionDefend:
      break;

   case kBattleActionMagic:
      w = gpGlobals->g.rgObject[wObjectID].magic.wMagicNumber;

      if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSilence] > 0)
      {
         //
         // Player is silenced
         //
         fValid = FALSE;
      }

      if (gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole] <
         gpGlobals->g.lprgMagic[w].wCostMP)
      {
         //
         // No enough MP
         //
         fValid = FALSE;
      }

      //
      // Fallback to physical attack if player is using an offensive magic,
      // defend if player is using a defensive or healing magic
      //
      if (gpGlobals->g.rgObject[wObjectID].magic.wFlags & kMagicFlagUsableToEnemy)
      {
         if (!fValid)
         {
            g_Battle.rgPlayer[wPlayerIndex].action.ActionType = kBattleActionAttack;
         }
         fToEnemy = TRUE;
      }
      else if (!fValid)
      {
         g_Battle.rgPlayer[wPlayerIndex].action.ActionType = kBattleActionDefend;
      }
      break;

   case kBattleActionCoopMagic:
      fToEnemy = TRUE;

      for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
      {
         wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

         if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0 ||
            gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSilence] > 0)
         {
            g_Battle.rgPlayer[wPlayerIndex].action.ActionType = kBattleActionAttack;
            break;
         }
      }
      break;

   case kBattleActionFlee:
      break;

   case kBattleActionThrowItem:
      fToEnemy = TRUE;

      if (PAL_GetItemAmount(wObjectID) == 0)
      {
         g_Battle.rgPlayer[wPlayerIndex].action.ActionType = kBattleActionAttack;
      }
      break;

   case kBattleActionUseItem:
      if (PAL_GetItemAmount(wObjectID) == 0)
      {
         g_Battle.rgPlayer[wPlayerIndex].action.ActionType = kBattleActionDefend;
      }
      break;

   case kBattleActionAttackMate:
      break;
   }

   //
   // Check if player can attack all enemies at once, or attack one enemy
   //
   if (g_Battle.rgPlayer[wPlayerIndex].action.ActionType == kBattleActionAttack)
   {
      if (sTarget == -1)
      {
         if (!PAL_PlayerCanAttackAll(wPlayerRole))
         {
            g_Battle.rgPlayer[wPlayerIndex].action.sTarget = PAL_BattleSelectAutoTarget();
         }
      }
      else if (PAL_PlayerCanAttackAll(wPlayerRole))
      {
         g_Battle.rgPlayer[wPlayerIndex].action.sTarget = -1;
      }
   }

   if (fToEnemy && g_Battle.rgPlayer[wPlayerIndex].action.sTarget >= 0)
   {
      if (g_Battle.rgEnemy[g_Battle.rgPlayer[wPlayerIndex].action.sTarget].wObjectID == 0)
      {
         g_Battle.rgPlayer[wPlayerIndex].action.sTarget = PAL_BattleSelectAutoTarget();
         assert(g_Battle.rgPlayer[wPlayerIndex].action.sTarget >= 0);
      }
   }
}

VOID
PAL_BattlePlayerPerformAction(
   WORD         wPlayerIndex
)
/*++
  Purpose:

    Perform the selected action for a player.

  Parameters:

    [IN]  wPlayerIndex - the index of the player.

  Return value:

    None.

--*/
{
   SHORT    sDamage;
   WORD     wPlayerRole = gpGlobals->rgParty[wPlayerIndex].wPlayerRole;
   SHORT    sTarget;
   int      x, y;
   int      i, t;
   WORD     str, def, res, wObject, wMagicNum;
   BOOL     fCritical, fPoisoned;

   g_Battle.wMovingPlayerIndex = wPlayerIndex;
   g_Battle.iBlow = 0;

   PAL_BattlePlayerValidateAction(wPlayerIndex);
   PAL_BattleBackupStat();

   sTarget = g_Battle.rgPlayer[wPlayerIndex].action.sTarget;

   switch (g_Battle.rgPlayer[wPlayerIndex].action.ActionType)
   {
   case kBattleActionAttack:
      if (sTarget != -1)
      {
         //
         // Attack one enemy
         //
         for (t = 0; t < (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusDualAttack] ? 2 : 1); t++)
         {
            str = PAL_GetPlayerAttackStrength(wPlayerRole);
            def = g_Battle.rgEnemy[sTarget].e.wDefense;
            def += (g_Battle.rgEnemy[sTarget].e.wLevel + 6) * 4;
            res = g_Battle.rgEnemy[sTarget].e.wPhysicalResistance;
            fCritical = FALSE;

            sDamage = PAL_CalcPhysicalAttackDamage(str, def, res);

            if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusBravery] > 0)
            {
               sDamage *= 3;
            }

            if (RandomLong(0, 5) == 0)
            {
               //
               // Critical Hit
               //
               sDamage *= 3;
               fCritical = TRUE;
            }

            if (wPlayerRole == 0 && RandomLong(0, 11) == 0)
            {
               //
               // Bonus hit for Li Xiaoyao
               //
               sDamage *= 2;
               fCritical = TRUE;
            }

            sDamage = (SHORT)(sDamage * RandomFloat(1, 1.125));

            if (sDamage <= 0)
            {
               sDamage = 1;
            }

            g_Battle.rgEnemy[sTarget].e.wHealth -= sDamage;

            if (t == 0)
            {
               g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 7;
               PAL_BattleDelay(4, 0, TRUE);
            }

            PAL_BattleShowPlayerAttackAnim(wPlayerIndex, fCritical);

            //
            // Show the number of damage
            //
            x = PAL_X(g_Battle.rgEnemy[sTarget].pos) - 9;
            y = PAL_Y(g_Battle.rgEnemy[sTarget].pos) - 115;

            if (y < 10)
            {
               y = 10;
            }

            PAL_BattleUIShowNum((WORD)sDamage, PAL_XY(x, y), kNumColorBlue);
         }
      }
      else
      {
         //
         // Attack all enemies
         //
         for (t = 0; t < (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusDualAttack] ? 2 : 1); t++)
         {
            int division = 1;
            const int index[MAX_ENEMIES_IN_TEAM] = {2, 1, 0, 4, 3};

            fCritical = (RandomLong(0, 5) == 0);

            if (t == 0)
            {
               g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 7;
               PAL_BattleDelay(4, 0, TRUE);
            }

            PAL_BattleShowPlayerAttackAnim(wPlayerIndex, fCritical);

            for (i = 0; i < MAX_ENEMIES_IN_TEAM; i++)
            {
               if (g_Battle.rgEnemy[index[i]].wObjectID == 0 ||
                  index[i] > g_Battle.wMaxEnemyIndex)
               {
                  continue;
               }

               str = PAL_GetPlayerAttackStrength(wPlayerRole);
               def = g_Battle.rgEnemy[index[i]].e.wDefense;
               def += (g_Battle.rgEnemy[index[i]].e.wLevel + 6) * 4;
               res = g_Battle.rgEnemy[index[i]].e.wPhysicalResistance;

               sDamage = PAL_CalcPhysicalAttackDamage(str, def, res);

               if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusBravery] > 0)
               {
                  sDamage *= 3;
               }

               if (fCritical)
               {
                  //
                  // Critical Hit
                  //
                  sDamage *= 3;
               }

               sDamage /= division;

               sDamage = (SHORT)(sDamage * RandomFloat(1, 1.125));

               if (sDamage <= 0)
               {
                  sDamage = 1;
               }

               g_Battle.rgEnemy[index[i]].e.wHealth -= sDamage;

               //
               // Show the number of damage
               //
               x = PAL_X(g_Battle.rgEnemy[index[i]].pos) - 9;
               y = PAL_Y(g_Battle.rgEnemy[index[i]].pos) - 115;

               if (y < 10)
               {
                  y = 10;
               }

               PAL_BattleUIShowNum((WORD)sDamage, PAL_XY(x, y), kNumColorBlue);

               division++;
               if (division > 3)
               {
                  division = 3;
               }
            }
         }
      }

      PAL_BattleUpdateFighters();
      PAL_BattleMakeScene();
      PAL_BattleDelay(5, 0, TRUE);
      break;

   case kBattleActionAttackMate:
      break;

   case kBattleActionCoopMagic:
      break;

   case kBattleActionDefend:
      g_Battle.rgPlayer[wPlayerIndex].fDefending = TRUE;
      break;

   case kBattleActionFlee:
      break;

   case kBattleActionMagic:
      wObject = g_Battle.rgPlayer[wPlayerIndex].action.wActionID;
      wMagicNum = gpGlobals->g.rgObject[wObject].magic.wMagicNumber;

      PAL_BattleShowPlayerPreMagicAnim(wPlayerIndex,
         (gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeSummon));

      if (!gpGlobals->fAutoBattle)
      {
         gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole] -= gpGlobals->g.lprgMagic[wMagicNum].wCostMP;
         if ((SHORT)(gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole]) < 0)
         {
            gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole] = 0;
         }
      }

      if (gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeApplyToPlayer ||
         gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeApplyToParty ||
         gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeTrance)
      {
         //
         // Using a defensive magic
         //
         WORD w = 0;

         if (g_Battle.rgPlayer[wPlayerIndex].action.sTarget != -1)
         {
            w = gpGlobals->rgParty[g_Battle.rgPlayer[wPlayerIndex].action.sTarget].wPlayerRole;
         }
         else if (gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeTrance)
         {
            w = wPlayerRole;
         }

         gpGlobals->g.rgObject[wObject].magic.wScriptOnUse =
            PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].magic.wScriptOnUse, wPlayerRole);

         if (g_fScriptSuccess)
         {
            PAL_BattleShowPlayerDefMagicAnim(wPlayerIndex, wObject, sTarget);

            gpGlobals->g.rgObject[wObject].magic.wScriptOnSuccess =
               PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].magic.wScriptOnSuccess, w);

            if (g_fScriptSuccess)
            {
               if (gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeTrance)
               {
                  for (i = 0; i < 6; i++)
                  {
                     g_Battle.rgPlayer[wPlayerIndex].iColorShift = i * 2;
                     PAL_BattleDelay(1, 0, TRUE);
                  }

                  PAL_BattleBackupScene();
                  PAL_LoadBattleSprites();

                  g_Battle.rgPlayer[wPlayerIndex].iColorShift = 0;

                  PAL_BattleMakeScene();
                  PAL_BattleFadeScene();
               }
            }
         }
      }
      else
      {
         //
         // Using an offensive magic
         //
         gpGlobals->g.rgObject[wObject].magic.wScriptOnUse =
            PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].magic.wScriptOnUse, wPlayerRole);

         if (g_fScriptSuccess)
         {
            if (gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeSummon)
            {
               PAL_BattleShowPlayerSummonMagicAnim(wPlayerIndex, wObject, sTarget);
            }
            else
            {
               PAL_BattleShowPlayerOffMagicAnim(wPlayerIndex, wObject, sTarget);
            }

            gpGlobals->g.rgObject[wObject].magic.wScriptOnSuccess =
               PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].magic.wScriptOnSuccess, (WORD)sTarget);

            //
            // Inflict damage to enemies
            //
            if ((SHORT)(gpGlobals->g.lprgMagic[wMagicNum].wBaseDamage) > 0)
            {
               if (sTarget == -1)
               {
                  //
                  // Attack all enemies
                  //
                  for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
                  {
                     if (g_Battle.rgEnemy[i].wObjectID == 0)
                     {
                        continue;
                     }

                     str = PAL_GetPlayerMagicStrength(wPlayerRole);
                     def = g_Battle.rgEnemy[i].e.wDefense;
                     def += (g_Battle.rgEnemy[i].e.wLevel + 6) * 4;

                     sDamage = PAL_CalcMagicDamage(str, def,
                        g_Battle.rgEnemy[i].e.wElemResistance, g_Battle.rgEnemy[i].e.wPoisonResistance, wObject);

                     if (sDamage <= 0)
                     {
                        sDamage = 1;
                     }

                     g_Battle.rgEnemy[i].e.wHealth -= sDamage;
                  }
               }
               else
               {
                  //
                  // Attack one enemy
                  //
                  str = PAL_GetPlayerMagicStrength(wPlayerRole);
                  def = g_Battle.rgEnemy[sTarget].e.wDefense;
                  def += (g_Battle.rgEnemy[sTarget].e.wLevel + 6) * 4;

                  sDamage = PAL_CalcMagicDamage(str, def,
                     g_Battle.rgEnemy[sTarget].e.wElemResistance, g_Battle.rgEnemy[sTarget].e.wPoisonResistance, wObject);

                  if (sDamage <= 0)
                  {
                     sDamage = 1;
                  }

                  g_Battle.rgEnemy[sTarget].e.wHealth -= sDamage;
               }
            }
         }
      }

      PAL_BattleDisplayStatChange();
      PAL_BattleShowPostMagicAnim();
      PAL_BattleDelay(5, 0, TRUE);

      break;

   case kBattleActionThrowItem:
      break;

   case kBattleActionUseItem:
      wObject = g_Battle.rgPlayer[wPlayerIndex].action.wActionID;

      PAL_BattleShowPlayerUseItemAnim(wPlayerIndex, wObject, sTarget);

      //
      // Run the script
      //
      gpGlobals->g.rgObject[wObject].item.wScriptOnUse =
         PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].item.wScriptOnUse,
            (sTarget == -1) ? 0xFFFF : gpGlobals->rgParty[sTarget].wPlayerRole);

      //
      // Remove the item if the item is consuming
      //
      if (gpGlobals->g.rgObject[wObject].item.wFlags & kItemFlagConsuming)
      {
         PAL_AddItemToInventory(wObject, -1);
      }

      if (g_Battle.iHidingTime < 0)
      {
         g_Battle.iHidingTime = -g_Battle.iHidingTime * 75;
         PAL_BattleBackupScene();
         PAL_BattleMakeScene();
         PAL_BattleFadeScene();
      }

      PAL_BattleUpdateFighters();
      PAL_BattleDisplayStatChange();
      PAL_BattleDelay(8, 0, TRUE);
      break;

   case kBattleActionPass:
      break;
   }

   //
   // Revert this player back to waiting state.
   //
   g_Battle.rgPlayer[wPlayerIndex].state = kFighterWait;
   g_Battle.rgPlayer[wPlayerIndex].flTimeMeter = 0;

   PAL_BattlePostActionCheck(FALSE);

   //
   // Check for poisons
   //
   fPoisoned = FALSE;
   PAL_BattleBackupStat();

   for (i = 0; i < MAX_POISONS; i++)
   {
      wObject = gpGlobals->rgPoisonStatus[i][wPlayerIndex].wPoisonID;

      if (wObject != 0)
      {
         fPoisoned = TRUE;
         gpGlobals->rgPoisonStatus[i][wPlayerIndex].wPoisonScript =
            PAL_RunTriggerScript(gpGlobals->rgPoisonStatus[i][wPlayerIndex].wPoisonScript, wPlayerRole);
      }
   }

   if (fPoisoned)
   {
      PAL_BattleDelay(3, 0, TRUE);
      PAL_BattleUpdateFighters();
      if (PAL_BattleDisplayStatChange())
      {
         PAL_BattleDelay(12, 0, TRUE);
      }
   }

   //
   // Update statuses
   //
   for (i = 0; i < kStatusAll; i++)
   {
      if (gpGlobals->rgPlayerStatus[wPlayerRole][i] > 0)
      {
         gpGlobals->rgPlayerStatus[wPlayerRole][i]--;
      }
   }
}

static INT
PAL_BattleEnemySelectTargetIndex(
   VOID
)
/*++
  Purpose:

    Select a attackable player randomly.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   int i;

   i = RandomLong(0, gpGlobals->wMaxPartyMemberIndex);

   while (gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[i].wPlayerRole] == 0)
   {
      i = RandomLong(0, gpGlobals->wMaxPartyMemberIndex);
   }

   return i;
}

VOID
PAL_BattleEnemyPerformAction(
   WORD         wEnemyIndex
)
/*++
  Purpose:

    Perform the selected action for a player.

  Parameters:

    [IN]  wEnemyIndex - the index of the player.

  Return value:

    None.

--*/
{
   int        str, def, iCoverIndex, i, x, y, ex, ey, iSound;
   WORD       rgwElementalResistance[NUM_MAGIC_ELEMENTAL];
   WORD       wPlayerRole, w, wMagic, wMagicNum;
   SHORT      sTarget, sDamage;
   BOOL       fAutoDefend = FALSE, rgfMagAutoDefend[MAX_PLAYERS_IN_PARTY];

   PAL_BattleBackupStat();

   sTarget = PAL_BattleEnemySelectTargetIndex();
   wPlayerRole = gpGlobals->rgParty[sTarget].wPlayerRole;
   wMagic = g_Battle.rgEnemy[wEnemyIndex].e.wMagic;

   if (FALSE)
   {
      // TODO: status
   }
   else if (wMagic != 0 &&
      RandomLong(0, 9) < g_Battle.rgEnemy[wEnemyIndex].e.wMagicRate)
   {
      //
      // Magical attack
      //
      if (wMagic == 0xFFFF)
      {
         //
         // Do nothing
         //
         return;
      }

      wMagicNum = gpGlobals->g.rgObject[wMagic].magic.wMagicNumber;

      str = g_Battle.rgEnemy[wEnemyIndex].e.wMagicStrength;
      str += (g_Battle.rgEnemy[wEnemyIndex].e.wLevel + 6) * 6;

      ex = PAL_X(g_Battle.rgEnemy[wEnemyIndex].pos);
      ey = PAL_Y(g_Battle.rgEnemy[wEnemyIndex].pos);

      ex += 12;
      ey += 6;

      g_Battle.rgEnemy[wEnemyIndex].pos = PAL_XY(ex, ey);
      PAL_BattleDelay(1, 0, FALSE);

      ex += 4;
      ey += 2;

      g_Battle.rgEnemy[wEnemyIndex].pos = PAL_XY(ex, ey);
      PAL_BattleDelay(1, 0, FALSE);

      SOUND_Play(g_Battle.rgEnemy[wEnemyIndex].e.wMagicSound);

      for (i = 0; i < g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames; i++)
      {
         g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame =
            g_Battle.rgEnemy[wEnemyIndex].e.wIdleFrames + i;
         PAL_BattleDelay(g_Battle.rgEnemy[wEnemyIndex].e.wActWaitFrames, 0, FALSE);
      }

      if (g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames == 0)
      {
         PAL_BattleDelay(1, 0, FALSE);
      }

      if (gpGlobals->g.lprgMagic[wMagicNum].wSoundDelay == 0)
      {
         for (i = 0; i <= g_Battle.rgEnemy[wEnemyIndex].e.wAttackFrames; i++)
         {
            g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame =
               i - 1 + g_Battle.rgEnemy[wEnemyIndex].e.wIdleFrames + g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames;
            PAL_BattleDelay(g_Battle.rgEnemy[wEnemyIndex].e.wActWaitFrames, 0, FALSE);
         }
      }

      if (gpGlobals->g.lprgMagic[wMagicNum].wType != kMagicTypeNormal)
      {
         sTarget = -1;

         for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
         {
            w = gpGlobals->rgParty[i].wPlayerRole;

            if (gpGlobals->rgPlayerStatus[w][kStatusSleep] == 0 &&
               gpGlobals->rgPlayerStatus[w][kStatusSlow] == 0 &&
               RandomLong(0, 2) == 0 &&
               gpGlobals->g.PlayerRoles.rgwHP[w] != 0)
            {
               rgfMagAutoDefend[i] = TRUE;
               g_Battle.rgPlayer[i].wCurrentFrame = 3;
            }
            else
            {
               rgfMagAutoDefend[i] = FALSE;
            }
         }
      }
      else if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSleep] == 0 &&
         gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSlow] == 0 &&
         RandomLong(0, 2) == 0)
      {
         fAutoDefend = TRUE;
         g_Battle.rgPlayer[sTarget].wCurrentFrame = 3;
      }

      gpGlobals->g.rgObject[wMagic].magic.wScriptOnUse =
         PAL_RunTriggerScript(gpGlobals->g.rgObject[wMagic].magic.wScriptOnUse, wPlayerRole);

      if (g_fScriptSuccess)
      {
         PAL_BattleShowEnemyMagicAnim(wMagic, sTarget);

         gpGlobals->g.rgObject[wMagic].magic.wScriptOnSuccess =
            PAL_RunTriggerScript(gpGlobals->g.rgObject[wMagic].magic.wScriptOnSuccess, wPlayerRole);
      }

      if ((SHORT)(gpGlobals->g.lprgMagic[wMagicNum].wBaseDamage) > 0)
      {
         if (sTarget == -1)
         {
            //
            // damage all players
            //
            for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
            {
               w = gpGlobals->rgParty[i].wPlayerRole;
               if (gpGlobals->g.PlayerRoles.rgwHP[w] == 0)
               {
                  //
                  // skip dead players
                  //
                  continue;
               }

               def = PAL_GetPlayerDefense(w);

               for (x = 0; x < NUM_MAGIC_ELEMENTAL; x++)
               {
                  rgwElementalResistance[x] =
                     5 + PAL_GetPlayerElementalResistance(w, x) / 20;
               }

               sDamage = PAL_CalcMagicDamage(str, def, rgwElementalResistance,
                  5 + PAL_GetPlayerPoisonResistance(w) / 20, wMagic);

               sDamage /= ((g_Battle.rgPlayer[i].fDefending ? 2 : 1) *
                  ((gpGlobals->rgPlayerStatus[w][kStatusProtect] > 0) ? 2 : 1)) +
                  (rgfMagAutoDefend[i] ? 1 : 0);

               if (sDamage > gpGlobals->g.PlayerRoles.rgwHP[w])
               {
                  sDamage = gpGlobals->g.PlayerRoles.rgwHP[w];
               }

               gpGlobals->g.PlayerRoles.rgwHP[w] -= sDamage;

               if (gpGlobals->g.PlayerRoles.rgwHP[w] == 0)
               {
                  SOUND_Play(gpGlobals->g.PlayerRoles.rgwDeathSound[w]);
               }
            }
         }
         else
         {
            //
            // damage one player
            //
            def = PAL_GetPlayerDefense(wPlayerRole);

            for (x = 0; x < NUM_MAGIC_ELEMENTAL; x++)
            {
               rgwElementalResistance[x] =
                  5 + PAL_GetPlayerElementalResistance(wPlayerRole, x) / 20;
            }

            sDamage = PAL_CalcMagicDamage(str, def, rgwElementalResistance,
               5 + PAL_GetPlayerPoisonResistance(wPlayerRole) / 20, wMagic);

            sDamage /= ((g_Battle.rgPlayer[sTarget].fDefending ? 2 : 1) *
               ((gpGlobals->rgPlayerStatus[wPlayerRole][kStatusProtect] > 0) ? 2 : 1)) +
               (fAutoDefend ? 1 : 0);

            if (sDamage > gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole])
            {
               sDamage = gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole];
            }

            gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] -= sDamage;

            if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0)
            {
               SOUND_Play(gpGlobals->g.PlayerRoles.rgwDeathSound[wPlayerRole]);
            }
         }
      }

      PAL_BattleDisplayStatChange();

      for (i = 0; i < 5; i++)
      {
         if (sTarget == -1)
         {
            for (x = 0; x <= gpGlobals->wMaxPartyMemberIndex; x++)
            {
               if (g_Battle.rgPlayer[x].wPrevHP ==
                  gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[x].wPlayerRole])
               {
                  //
                  // Skip unaffected players
                  //
                  continue;
               }

               g_Battle.rgPlayer[x].wCurrentFrame = 4;
               if (i > 0)
               {
                  g_Battle.rgPlayer[x].pos =
                     PAL_XY(PAL_X(g_Battle.rgPlayer[x].pos) + (8 >> i),
                            PAL_Y(g_Battle.rgPlayer[x].pos) + (4 >> i));
               }
               g_Battle.rgPlayer[x].iColorShift = ((i < 3) ? 6 : 0);
            }
         }
         else
         {
            g_Battle.rgPlayer[sTarget].wCurrentFrame = 4;
            if (i > 0)
            {
               g_Battle.rgPlayer[sTarget].pos =
                  PAL_XY(PAL_X(g_Battle.rgPlayer[sTarget].pos) + (8 >> i),
                         PAL_Y(g_Battle.rgPlayer[sTarget].pos) + (4 >> i));
            }
            g_Battle.rgPlayer[sTarget].iColorShift = ((i < 3) ? 6 : 0);
         }

         PAL_BattleDelay(1, 0, FALSE);
      }

      g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame = 0;
      g_Battle.rgEnemy[wEnemyIndex].pos = g_Battle.rgEnemy[wEnemyIndex].posOriginal;

      PAL_BattleDelay(1, 0, FALSE);
      PAL_BattleUpdateFighters();

      PAL_BattlePostActionCheck(TRUE);
      PAL_BattleDelay(8, 0, TRUE);
   }
   else
   {
      //
      // Physical attack
      //
      WORD wFrameBak = g_Battle.rgPlayer[sTarget].wCurrentFrame;

      str = g_Battle.rgEnemy[wEnemyIndex].e.wAttackStrength;
      str += (g_Battle.rgEnemy[wEnemyIndex].e.wLevel + 6) * 6;
      def = PAL_GetPlayerDefense(wPlayerRole);

      if (g_Battle.rgPlayer[sTarget].fDefending)
      {
         def *= 2;
      }

      SOUND_Play(g_Battle.rgEnemy[wEnemyIndex].e.wAttackSound);

      iCoverIndex = -1;

      fAutoDefend = (RandomLong(0, 16) >= 10);

      //
      // Check if the inflictor should be protected
      //
      if ((PAL_IsPlayerDying(wPlayerRole) ||
         gpGlobals->rgPlayerStatus[wPlayerRole][kStatusConfused] > 0 ||
         gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSleep] > 0) && fAutoDefend)
      {
         w = gpGlobals->g.PlayerRoles.rgwCoveredBy[wPlayerRole];

         for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
         {
            if (gpGlobals->rgParty[i].wPlayerRole == w)
            {
               iCoverIndex = i;
               break;
            }
         }

         if (iCoverIndex != -1)
         {
            if (PAL_IsPlayerDying(gpGlobals->rgParty[iCoverIndex].wPlayerRole) ||
               gpGlobals->rgPlayerStatus[gpGlobals->rgParty[iCoverIndex].wPlayerRole][kStatusConfused] > 0 ||
               gpGlobals->rgPlayerStatus[gpGlobals->rgParty[iCoverIndex].wPlayerRole][kStatusSleep] > 0)
            {
               iCoverIndex = -1;
            }
         }
      }

      for (i = 0; i < g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames; i++)
      {
         g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame =
            g_Battle.rgEnemy[wEnemyIndex].e.wIdleFrames + i;
         PAL_BattleDelay(2, 0, FALSE);
      }

      for (i = 0; i < 3 - g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames; i++)
      {
         x = PAL_X(g_Battle.rgEnemy[wEnemyIndex].pos) - 2;
         y = PAL_Y(g_Battle.rgEnemy[wEnemyIndex].pos) - 1;
         g_Battle.rgEnemy[wEnemyIndex].pos = PAL_XY(x, y);
         PAL_BattleDelay(1, 0, FALSE);
      }

      SOUND_Play(g_Battle.rgEnemy[wEnemyIndex].e.wActionSound);
      PAL_BattleDelay(1, 0, FALSE);

      ex = PAL_X(g_Battle.rgPlayer[sTarget].pos) - 44;
      ey = PAL_Y(g_Battle.rgPlayer[sTarget].pos) - 16;

      iSound = g_Battle.rgEnemy[wEnemyIndex].e.wCallSound;

      if (iCoverIndex != -1)
      {
         iSound = gpGlobals->g.PlayerRoles.rgwCoverSound[gpGlobals->rgParty[iCoverIndex].wPlayerRole];

         g_Battle.rgPlayer[iCoverIndex].wCurrentFrame = 3;

         x = PAL_X(g_Battle.rgPlayer[sTarget].pos) - 24;
         y = PAL_Y(g_Battle.rgPlayer[sTarget].pos) - 12;

         g_Battle.rgPlayer[iCoverIndex].pos = PAL_XY(x, y);
      }
      else if (fAutoDefend)
      {
         g_Battle.rgPlayer[sTarget].wCurrentFrame = 3;
         iSound = gpGlobals->g.PlayerRoles.rgwCoverSound[wPlayerRole];
      }

      if (g_Battle.rgEnemy[wEnemyIndex].e.wAttackFrames == 0)
      {
         g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame =
            g_Battle.rgEnemy[wEnemyIndex].e.wIdleFrames - 1;

         g_Battle.rgEnemy[wEnemyIndex].pos = PAL_XY(ex, ey);

         PAL_BattleDelay(2, 0, FALSE);
      }
      else
      {
         for (i = 0; i <= g_Battle.rgEnemy[wEnemyIndex].e.wAttackFrames; i++)
         {
            g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame =
               g_Battle.rgEnemy[wEnemyIndex].e.wIdleFrames +
               g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames + i - 1;

            g_Battle.rgEnemy[wEnemyIndex].pos = PAL_XY(ex, ey);

            PAL_BattleDelay(g_Battle.rgEnemy[wEnemyIndex].e.wActWaitFrames, 0, FALSE);
         }
      }

      if (!fAutoDefend)
      {
         g_Battle.rgPlayer[sTarget].wCurrentFrame = 4;

         sDamage = PAL_CalcPhysicalAttackDamage(str, def, 2);
         sDamage *= RandomFloat(0.9, 1.1);

         if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusProtect])
         {
            sDamage /= 2;
         }

         if ((SHORT)gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] < sDamage)
         {
            sDamage = gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole];
         }

         if (sDamage <= 0)
         {
            sDamage = 1;
         }

         gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] -= sDamage;
         PAL_BattleDisplayStatChange();

         g_Battle.rgPlayer[sTarget].iColorShift = 6;
      }

      SOUND_Play(iSound);
      PAL_BattleDelay(1, 0, FALSE);

      g_Battle.rgPlayer[sTarget].iColorShift = 0;

      if (iCoverIndex != -1)
      {
         g_Battle.rgEnemy[wEnemyIndex].pos =
            PAL_XY(PAL_X(g_Battle.rgEnemy[wEnemyIndex].pos) - 10,
                   PAL_Y(g_Battle.rgEnemy[wEnemyIndex].pos) - 8);
         g_Battle.rgPlayer[iCoverIndex].pos =
            PAL_XY(PAL_X(g_Battle.rgPlayer[iCoverIndex].pos) + 4,
                   PAL_Y(g_Battle.rgPlayer[iCoverIndex].pos) + 2);
      }
      else
      {
         g_Battle.rgPlayer[sTarget].pos =
            PAL_XY(PAL_X(g_Battle.rgPlayer[sTarget].pos) + 8,
                   PAL_Y(g_Battle.rgPlayer[sTarget].pos) + 4);
      }

      PAL_BattleDelay(1, 0, FALSE);

      if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0)
      {
         SOUND_Play(gpGlobals->g.PlayerRoles.rgwDeathSound[wPlayerRole]);
         wFrameBak = 2;
      }
      else if (PAL_IsPlayerDying(wPlayerRole))
      {
         wFrameBak = 1;
      }

      if (iCoverIndex == -1)
      {
         g_Battle.rgPlayer[sTarget].pos =
            PAL_XY(PAL_X(g_Battle.rgPlayer[sTarget].pos) + 2,
                   PAL_Y(g_Battle.rgPlayer[sTarget].pos) + 1);
      }

      PAL_BattleDelay(3, 0, FALSE);

      g_Battle.rgEnemy[wEnemyIndex].pos = g_Battle.rgEnemy[wEnemyIndex].posOriginal;
      g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame = 0;

      PAL_BattleDelay(1, 0, FALSE);

      g_Battle.rgPlayer[sTarget].wCurrentFrame = wFrameBak;
      PAL_BattleDelay(1, 0, TRUE);

      g_Battle.rgPlayer[sTarget].pos = g_Battle.rgPlayer[sTarget].posOriginal;
      PAL_BattleDelay(7, 0, TRUE);

      PAL_BattleUpdateFighters();

      if (iCoverIndex == -1 &&
         g_Battle.rgEnemy[wEnemyIndex].e.wAttackEquivItemRate >= RandomLong(1, 10))
      {
         i = g_Battle.rgEnemy[wEnemyIndex].e.wAttackEquivItem;
         gpGlobals->g.rgObject[i].item.wScriptOnUse =
            PAL_RunTriggerScript(gpGlobals->g.rgObject[i].item.wScriptOnUse, wPlayerRole);
      }

      PAL_BattlePostActionCheck(TRUE);
   }
}

VOID
PAL_BattleStealFromEnemy(
   WORD           wTarget,
   WORD           wStealRate
)
/*++
  Purpose:

    Steal from the enemy.

  Parameters:

    [IN]  wTarget - the target enemy index.

    [IN]  wStealRate - the rate of successful theft.

  Return value:

    None.

--*/
{
   int   iPlayerIndex = g_Battle.wMovingPlayerIndex;
   int   offset, x, y, i;
   char  s[256] = "";

   g_Battle.rgPlayer[iPlayerIndex].wCurrentFrame = 10;
   offset = ((INT)wTarget - iPlayerIndex) * 8;

   x = PAL_X(g_Battle.rgEnemy[wTarget].pos) + 64 - offset;
   y = PAL_Y(g_Battle.rgEnemy[wTarget].pos) + 20 - offset / 2;

   g_Battle.rgPlayer[iPlayerIndex].pos = PAL_XY(x, y);

   PAL_BattleDelay(1, 0, TRUE);

   for (i = 0; i < 5; i++)
   {
      x -= i + 8;
      y -= 4;

      g_Battle.rgPlayer[iPlayerIndex].pos = PAL_XY(x, y);

      if (i == 4)
      {
         g_Battle.rgEnemy[wTarget].iColorShift = 6;
      }

      PAL_BattleDelay(1, 0, TRUE);
   }

   g_Battle.rgEnemy[wTarget].iColorShift = 0;
   x--;
   g_Battle.rgPlayer[iPlayerIndex].pos = PAL_XY(x, y);
   PAL_BattleDelay(3, 0, TRUE);

   if (g_Battle.rgEnemy[wTarget].e.nStealItem > 0 &&
      (RandomLong(0, 10) <= wStealRate || wStealRate == 0))
   {
      if (g_Battle.rgEnemy[wTarget].e.wStealItem == 0)
      {
         //
         // stolen coins
         //
         int c = g_Battle.rgEnemy[wTarget].e.nStealItem / RandomLong(2, 3);
         g_Battle.rgEnemy[wTarget].e.nStealItem -= c;
         gpGlobals->dwCash += c;

         if (c > 0)
         {
            strcpy(s, PAL_GetWord(34));
            strcat(s, " ");
            strcat(s, va("%d", c));
            strcat(s, " ");
            strcat(s, PAL_GetWord(10));
         }
      }
      else
      {
         //
         // stolen item
         //
         g_Battle.rgEnemy[wTarget].e.nStealItem--;
         PAL_AddItemToInventory(g_Battle.rgEnemy[wTarget].e.wStealItem, 1);

         strcpy(s, PAL_GetWord(34));
         strcat(s, PAL_GetWord(g_Battle.rgEnemy[wTarget].e.wStealItem));
      }

      if (s[0] != '\0')
      {
         PAL_BattleUIShowText(s, 1000);
      }
   }

   g_Battle.rgPlayer[iPlayerIndex].state = kFighterWait;
   g_Battle.rgPlayer[iPlayerIndex].flTimeMeter = 0;
   PAL_BattleUpdateFighters();
   PAL_BattleDelay(1, 0, TRUE);
}
