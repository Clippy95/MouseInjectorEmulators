//==========================================================================
// Mouse Injector for Dolphin
//==========================================================================
// Copyright (C) 2019 Carnivorous
// All rights reserved.
//
// Mouse Injector is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, visit http://www.gnu.org/licenses/gpl-2.0.html
//==========================================================================
#include <stdint.h>
#include "../main.h"
#include "../memory.h"
#include "../mouse.h"
#include "game.h"

#define PI 3.14159265f // 0x40490FDB
#define TAU 6.2831853f // 0x40C90FDB
#define CROSSHAIRX 0.296f // 0x3E978D50
#define CROSSHAIRY 0.39999999f // 0xBECCCCCC
#define SENTRYMINY -0.1616991162f // 0xBE259474
#define SENTRYMAXY 0.1349733025f // 0x3E0A3671
#define SENTRYFOVBASE 41.25f // 0x42250000
// NF ADDRESSES - OFFSET ADDRESSES BELOW (REQUIRES PLAYERBASE TO USE)
#define NF_camx 0x80B96E2C - 0x80B96DEC
#define NF_camy 0x80B976F0 - 0x80B96DEC
#define NF_fov 0x80B97718 - 0x80B96DEC
#define NF_crosshairx 0x80B96F98 - 0x80B96DEC
#define NF_crosshairy 0x80B96F9C - 0x80B96DEC
#define NF_health 0x80B976DC - 0x80B96DEC
#define NF_lookspring 0x80B96FA8 - 0x80B96DEC
#define NF_sentryx 0x81112E94 - 0x81112CE0
#define NF_sentryy 0x81112E90 - 0x81112CE0
// STATIC ADDRESSES BELOW
#define NF_playerbase 0x802BE87C // playable character pointer
#define NF_sentrybase 0x803780DC // sentry interface pointer (heli/jet ski)
#define NF_sentryfov 0x802E441C // sentry fov
#define NF_pauseflag 0x8024C2B4

static uint8_t NF_Status(void);
static void NF_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"007: NightFire",
	NF_Status,
	NF_Inject,
	1, // 1000 Hz tickrate
	1 // crosshair sway supported for driver
};

const GAMEDRIVER *GAME_NF = &GAMEDRIVER_INTERFACE;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t NF_Status(void)
{
	return (MEM_ReadUInt(0x80000000) == 0x474F3745U && MEM_ReadUInt(0x80000004) == 0x36390000U); // check game header to see if it matches NF
}
//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void NF_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;
	const uint32_t playerbase = MEM_ReadUInt(NF_playerbase);
	const float looksensitivity = (float)sensitivity / 40.f;
	const float crosshairsensitivity = ((float)crosshair / 100.f) * looksensitivity;
	if(WITHINMEMRANGE(playerbase)) // if playerbase is valid
	{
		if(MEM_ReadInt(playerbase + NF_lookspring) == 0x03010002) // disable lookspring when spawned
			MEM_WriteInt(playerbase + NF_lookspring, 0x01010002);
		float camx = MEM_ReadFloat(playerbase + NF_camx);
		float camy = MEM_ReadFloat(playerbase + NF_camy);
		const float fov = MEM_ReadFloat(playerbase + NF_fov);
		const float hp = MEM_ReadFloat(playerbase + NF_health);
		const uint32_t pauseflag = MEM_ReadUInt(NF_pauseflag);
		if(camx >= -PI && camx <= PI && camy >= -1.f && camy <= 1.f && fov >= 1.f && hp > 0 && !pauseflag)
		{
			camx -= (float)xmouse / 10.f * looksensitivity / (360.f / TAU) / (fov / 1.f); // normal calculation method for X
			camy += (float)(!invertpitch ? -ymouse : ymouse) / 10.f * looksensitivity / 90.f / (fov / 1.f); // normal calculation method for Y
			if(camx <= -PI)
				camx += TAU;
			else if(camx >= PI)
				camx -= TAU;
			camy = ClampFloat(camy, -1.f, 1.f);
			MEM_WriteFloat(playerbase + NF_camx, camx);
			MEM_WriteFloat(playerbase + NF_camy, camy);
			if(crosshair) // if crosshair sway is enabled
			{
				float crosshairx = MEM_ReadFloat(playerbase + NF_crosshairx); // after camera x and y have been calculated and injected, calculate the crosshair/gun sway
				float crosshairy = MEM_ReadFloat(playerbase + NF_crosshairy);
				crosshairx += (float)xmouse / 80.f * crosshairsensitivity / (fov / 1.f);
				crosshairy += (float)(!invertpitch ? -ymouse : ymouse) / 80.f * crosshairsensitivity / (fov / 1.f);
				MEM_WriteFloat(playerbase + NF_crosshairx, ClampFloat(crosshairx, -CROSSHAIRX, CROSSHAIRX));
				MEM_WriteFloat(playerbase + NF_crosshairy, ClampFloat(crosshairy, -CROSSHAIRY, CROSSHAIRY));
			}
		}
	}
	else // if playerbase is invalid, check for sentry mode
	{
		const uint32_t sentrybase = MEM_ReadUInt(NF_sentrybase);
		if(NOTWITHINMEMRANGE(sentrybase)) // if sentrybase is invalid
			return;
		float sentryx = MEM_ReadFloat(sentrybase + NF_sentryx);
		float sentryy = MEM_ReadFloat(sentrybase + NF_sentryy);
		const float fov = MEM_ReadFloat(NF_sentryfov);
		if(sentryx >= -1.f && sentryx <= 1.f)
		{
			sentryx += (float)xmouse / 10.f * looksensitivity / 360.f / (SENTRYFOVBASE / fov);
			sentryy += (float)(!invertpitch ? ymouse : -ymouse) / 10.f * looksensitivity / (90.f / (SENTRYMAXY - SENTRYMINY)) / (SENTRYFOVBASE / fov);
			if(sentryx <= -1.f)
				sentryx += 1.f;
			else if(sentryx >= 1.f)
				sentryx -= 1.f;
			sentryy = ClampFloat(sentryy, SENTRYMINY, SENTRYMAXY);
			MEM_WriteFloat(sentrybase + NF_sentryx, sentryx);
			MEM_WriteFloat(sentrybase + NF_sentryy, sentryy);
		}
	}
}