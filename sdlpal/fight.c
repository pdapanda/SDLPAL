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
   return (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] < 100 &&
      gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] < gpGlobals->g.PlayerRoles.rgwMaxHP[wPlayerRole] / 5);
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

SHORT
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

SHORT
PAL_CalcMagicDamage(
   WORD             wMagicStrength,
   WORD             wDefense,
   const WORD       rgwElementalResistance[NUM_MAGIC_ELEMENTAL],
   WORD             wMagicID
)
/*++
   Purpose:

     Calculate the damage of magic.

   Parameters:

     [IN]  wMagicStrength - magic strength of attacker.

     [IN]  wDefense - defense value of inflictor.

     [IN]  rgwAttribResistance - inflictor's resistance to the attributed magics.

     [IN]  wMagicID - object ID of the magic.

   Return value:

     The damage value of the magic attack.

--*/
{
   SHORT           sDamage;
   WORD            wAttrib;

   wMagicID = gpGlobals->g.rgObject[wMagicID].magic.wMagicNumber;

   //
   // Formula courtesy of palxex and shenyanduxing
   //
   wMagicStrength *= RandomLong(9, 11);
   wMagicStrength /= 10;

   sDamage = PAL_CalcBaseDamage(wMagicStrength, wDefense) / 2;
   sDamage += gpGlobals->g.lprgMagic[wMagicID].wBaseDamage;

   if (gpGlobals->g.lprgMagic[wMagicID].wElemental != 0)
   {
      wAttrib = gpGlobals->g.lprgMagic[wMagicID].wElemental - 1;

      sDamage *= 10 - rgwElementalResistance[wAttrib];
      sDamage /= 5;

      if (wAttrib < NUM_MAGIC_ELEMENTAL)
      {
         sDamage *= 10 + gpGlobals->g.lprgBattleField[gpGlobals->wNumBattleField].rgsMagicEffect[wAttrib];
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

    [IN]  wAttackResistance - inflictor's resistance to phycal attack.

  Return value:

    The damage value of the physical attacking.

--*/
{
   SHORT             sDamage;

   sDamage = PAL_CalcBaseDamage(wAttackStrength, wDefense);
   sDamage /= wAttackResistance;

   sDamage = (SHORT)(sDamage * RandomFloat(1, 1.125));

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
   if (g_Battle.UI.state == kBattleUISelectMove && g_Battle.UI.MenuState != kBattleMenuMain)
   {
      //
      // Pause the time when there are submenus
      //
      return 0;
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
      wDexterity /= 2;
   }

   return wDexterity;
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

      if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusConfused] != 0)
      {
         //
         // Player is confused
         //
         g_Battle.rgPlayer[i].pos =
            PAL_XY(PAL_X(g_Battle.rgPlayer[i].pos) + RandomLong(-3, 3),
                   PAL_Y(g_Battle.rgPlayer[i].pos) + RandomLong(-3, 3));
      }

      if (g_Battle.rgPlayer[i].state == kFighterAct &&
         g_Battle.rgPlayer[i].action.ActionType == kBattleActionMagic)
      {
         //
         // Player is using a magic
         //
         g_Battle.rgPlayer[i].wCurrentFrame = 5;
         continue;
      }

      if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0)
      {
         if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusPuppet] == 0)
         {
            g_Battle.rgPlayer[i].wCurrentFrame = 2;
         }
      }
      else if (PAL_IsPlayerDying(wPlayerRole) ||
         gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSleep] != 0)
      {
         g_Battle.rgPlayer[i].wCurrentFrame = 1;
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

      if (g_Battle.rgEnemy[i].rgStatus[kStatusSleep] > 0)
      {
         g_Battle.rgEnemy[i].wCurrentFrame = 0;
         continue;
      }
      else if (g_Battle.rgEnemy[i].rgStatus[kStatusConfused] > 0)
      {
         g_Battle.rgEnemy[i].pos =
            PAL_XY(PAL_X(g_Battle.rgEnemy[i].pos) + RandomLong(-3, 3),
                   PAL_Y(g_Battle.rgEnemy[i].pos) + RandomLong(-3, 3));
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
      return;
   }
   else
   {
      fEnded = TRUE;

      for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
      {
         wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;
         if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] != 0 ||
            gpGlobals->rgPlayerStatus[wPlayerRole][kStatusPuppet] != 0)
         {
            fEnded = FALSE;
            break;
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
         break;

      case kFighterAct:
////TEST///////////////////////////////////////////////////////////
SOUND_Play(g_Battle.rgEnemy[i].e.wAttackSound);
if (g_Battle.rgEnemy[i].fFirstMoveDone)
PAL_BattleUIShowText(va("enemy %d attack (2nd)",i), 500);
else PAL_BattleUIShowText(va("enemy %d attack",i), 500);
g_Battle.rgEnemy[i].flTimeMeter =0;
g_Battle.rgEnemy[i].state = kFighterWait;

PAL_BattleUIShowNum(RandomLong(0, 29999), PAL_XY(RandomLong(5, 100), RandomLong(5, 100)), kNumColorCyan);
////////////////////////////////////////////////////////////////////
         if (!g_Battle.rgEnemy[i].fFirstMoveDone)
         {
            if (g_Battle.rgEnemy[i].e.wDualMove >= 2 ||
               g_Battle.rgEnemy[i].e.wDualMove != 0 && RandomLong(0, 1))
            {
               g_Battle.rgEnemy[i].flTimeMeter = 100;
               g_Battle.rgEnemy[i].state = kFighterWait;
               g_Battle.rgEnemy[i].fFirstMoveDone = TRUE;
               break;
            }
         }

         g_Battle.rgEnemy[i].fFirstMoveDone = FALSE;
         g_Battle.rgEnemy[i].fTurnStart = TRUE;
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

      //
      // Skip slept players if player should still be sleeping
      //
      if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSleep] != 0)
      {
         // TODO: check if the player should be wake up

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
         wDexterity = PAL_GetPlayerActualDexterity(wPlayerRole);
         g_Battle.rgPlayer[i].action.flRemainingTime -= PAL_GetTimeChargingSpeed(wDexterity);

         if (g_Battle.rgPlayer[i].action.flRemainingTime < 0)
         {
            //
            // Perform the action for this player.
            //
            PAL_BattlePlayerPerformAction(i);

            //
            // Reduce the time for other players when uses coopmagic
            //
            if (g_Battle.rgPlayer[i].action.ActionType == kBattleActionCoopMagic)
            {
               for (iMax = 0; iMax <= gpGlobals->wMaxPartyMemberIndex; iMax++)
               {
                  if (iMax == g_Battle.rgPlayer[i].action.wActionID)
                  {
                     g_Battle.rgPlayer[iMax].flTimeMeter = 0;
                     continue;
                  }

                  if (g_Battle.rgPlayer[iMax].flTimeMeter > 100)
                  {
                     g_Battle.rgPlayer[iMax].flTimeMeter = 100;
                  }

                  g_Battle.rgPlayer[iMax].flTimeMeter -= 40;
               }
            }
            else
            {
               g_Battle.rgPlayer[i].flTimeMeter = 0;
            }

            //
            // Revert this player back to waiting state.
            //
            g_Battle.rgPlayer[i].state = kFighterWait;
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

////TEST/////////////////////////////////////////////////////////-START
if (g_Battle.iExpGained==0)
{
for(i=g_Battle.wMaxEnemyIndex;i>=0;i--)
{
   if(g_Battle.rgEnemy[i].wObjectID)
   {
      g_Battle.iExpGained += g_Battle.rgEnemy[i].e.wExp;
      g_Battle.iCashGained += g_Battle.rgEnemy[i].e.wCash;
   }
}}
if (g_InputState.dwKeyPress & kKeyRepeat){
   for (i = g_Battle.wMaxEnemyIndex; i>=0;i--)
   {
      if(g_Battle.rgEnemy[i].wObjectID)
      {
         g_Battle.rgEnemy[i].wObjectID=0;
         break;
      }
   }
   PAL_BattleBackupScene();
   PAL_BattleMakeScene();
   PAL_BattleFadeScene();
}else if (g_InputState.dwKeyPress & kKeyFlee){
    for (i = 0; i < 5; i++){
        gpGlobals->g.PlayerRoles.rgwHP[i] = 0;
    }
   return;
}
////TEST/////////////////////////////////////////////////////////-END
}

VOID
PAL_BattleCommitAction(
   VOID
)
/*++
  Purpose:

    Commit the action which the player decided.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   WORD      w;

   g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.ActionType =
      g_Battle.UI.wActionType;
   g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.sTarget =
      (SHORT)g_Battle.UI.wSelectedIndex;
   g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.wActionID =
      g_Battle.UI.wObjectID;

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
            w == kMagicTypeTransform)
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
   case kBattleActionAttack:
   case kBattleActionThrowItem:
   case kBattleActionUseItem:
      //
      // Attacking and item-using should take very short
      //
      g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.flRemainingTime = RandomFloat(0, 5);
      break;

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
            p->wType == kMagicTypeTransform)
         {
            //
            // Healing magics should take shorter
            //
            wCostMP /= 3;
         }

         g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.flRemainingTime = wCostMP;
      }
      break;

   case kBattleActionFlee:
      //
      // Fleeing should take a fairly long time
      //
      g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.flRemainingTime = RandomFloat(25, 75);
      break;

   default:
      g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.flRemainingTime = 0;
      break;
   }

   g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].state = kFighterAct;
   g_Battle.UI.state = kBattleUIWait;
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
   // TODO
   switch (g_Battle.rgPlayer[wPlayerIndex].action.ActionType)
   {
   case kBattleActionAttack:
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
      break;

   case kBattleActionThrowItem:
      break;

   case kBattleActionUseItem:
      break;
   }
}
