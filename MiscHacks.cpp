#define _CRT_SECURE_NO_WARNINGS

#include "MiscHacks.h"
#include "Interfaces.h"
#include "RenderManager.h"
#include "Menu.h"

#include <time.h>

template<class T, class U>
inline T clamp(T in, U low, U high)
{
	if (in <= low)
		return low;
	else if (in >= high)
		return high;
	else
		return in;
}

inline float bitsToFloat(unsigned long i)
{
	return *reinterpret_cast<float*>(&i);
}

inline float FloatNegate(float f)
{
	return bitsToFloat(FloatBits(f) ^ 0x80000000);
}

Vector AutoStrafeView;

void CMiscHacks::Init()
{
	// Any init
}

void CMiscHacks::Draw()
{
	// Any drawing	
	// Spams
	switch (Menu::Window.MiscTab.OtherChatSpam.GetIndex())
	{
	case 0:
		// No Chat Spam
		break;
	case 1:
		// Namestealer
		ChatSpamName();
		break;
	case 2:
		// Regular
		ChatSpamRegular();
		break;
	case 3:
		// Interwebz
		ChatSpamInterwebz();
		break;
	case 4:
		// Report Spam
		ChatSpamDisperseName();
		break;
	}
}

void CMiscHacks::Move(CUserCmd *pCmd, bool &bSendPacket)
{
	// Any Move Stuff
	Vector vecOriginalView;
	IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	
	// Bhop
	if (Menu::Window.MiscTab.OtherAutoJump.GetState())
		AutoJump(pCmd);	

	// AutoStrafe
	Interfaces::Engine->GetViewAngles(AutoStrafeView);
	switch (Menu::Window.MiscTab.OtherAutoStrafe.GetIndex())
	{
	case 0:
		// Off
		break;
	case 1:
		LegitStrafe(pCmd);
		break;
	case 2:
		RageStrafe(pCmd);
		break;
	}

	/*if (Menu::Window.MiscTab.OtherWalkbot.GetState())
	{
		walkbot(pCmd, pLocal);
	}*/
	if (Menu::Window.MiscTab.OtherCircle.GetState())
	{
		CircleStrafer(pCmd, vecOriginalView);
	}
	if (Menu::Window.MiscTab.OtherSlowmo.GetState())
	{
		doFakeWalk(pCmd, bSendPacket);
	}
}

static __declspec(naked) void __cdecl Invoke_NET_SetConVar(void* pfn, const char* cvar, const char* value)
{
	__asm 
	{
		push    ebp
			mov     ebp, esp
			and     esp, 0FFFFFFF8h
			sub     esp, 44h
			push    ebx
			push    esi
			push    edi
			mov     edi, cvar
			mov     esi, value
			jmp     pfn
	}
}

void DECLSPEC_NOINLINE NET_SetConVar(const char* value, const char* cvar)
{
	static DWORD setaddr = Utilities::Memory::FindPattern("engine.dll", (PBYTE)"\x8D\x4C\x24\x1C\xE8\x00\x00\x00\x00\x56", "xxxxx????x");
	if (setaddr != 0) 
	{
		void* pvSetConVar = (char*)setaddr;
		Invoke_NET_SetConVar(pvSetConVar, cvar, value);
	}
}

void change_name(const char* name)
{
	if (Interfaces::Engine->IsInGame() && Interfaces::Engine->IsConnected())
		NET_SetConVar(name, "name");
}

void CMiscHacks::AutoJump(CUserCmd *pCmd)
{
	if (pCmd->buttons & IN_JUMP && GUI.GetKeyState(VK_SPACE))
	{
		int iFlags = hackManager.pLocal()->GetFlags();
		if (!(iFlags & FL_ONGROUND))
			pCmd->buttons &= ~IN_JUMP;

		if (hackManager.pLocal()->GetVelocity().Length() <= 50)
		{
			pCmd->forwardmove = 450.f;
		}
	}
}

void CMiscHacks::LegitStrafe(CUserCmd *pCmd)
{
	IClientEntity* pLocal = hackManager.pLocal();
	if (!(pLocal->GetFlags() & FL_ONGROUND))
	{
		pCmd->forwardmove = 0.0f;

		if (pCmd->mousedx < 0)
		{
			pCmd->sidemove = -450.0f;
		}
		else if (pCmd->mousedx > 0)
		{
			pCmd->sidemove = 450.0f;
		}
	}
}

void CMiscHacks::RageStrafe(CUserCmd *pCmd)
{
	float strafespeed = 10;

	IClientEntity* pLocal = hackManager.pLocal();

	bool bKeysPressed = true;
	if (GUI.GetKeyState(0x41) || GUI.GetKeyState(0x57) || GUI.GetKeyState(0x53) || GUI.GetKeyState(0x44)) bKeysPressed = false;

	if ((GetAsyncKeyState(VK_SPACE) && !(pLocal->GetFlags() & FL_ONGROUND)) && bKeysPressed)
	{
		pCmd->forwardmove = (1550.f * strafespeed) / pLocal->GetVelocity().Length2D();

		pCmd->sidemove = (pCmd->command_number % 2) == 0 ? -450.f : 450.f;

			if (pCmd->forwardmove > 450.f)
				pCmd->forwardmove = 450.f;
	}
}

Vector GetAutostrafeView()
{
	return AutoStrafeView;
}

void CMiscHacks::ChatSpamInterwebz()
{
	static clock_t start_t = clock();
	double timeSoFar = (double)(clock() - start_t) / CLOCKS_PER_SEC;
	if (timeSoFar < 0.001)
		return;

	static bool wasSpamming = true;
	//static std::string nameBackup = "INTERWEBZ";

	if (wasSpamming)
	{
		static bool useSpace = true;
		if (useSpace)
		{
			change_name ("Oof");
			useSpace = !useSpace;
		}
		else
		{
			change_name("oOf");
			useSpace = !useSpace;
		}
	}

	start_t = clock();
}

void CMiscHacks::ChatSpamDisperseName()
{
	static clock_t start_t = clock();
	double timeSoFar = (double)(clock() - start_t) / CLOCKS_PER_SEC;
	if (timeSoFar < 0.001)
		return;

	static bool wasSpamming = true;

	if (wasSpamming)
	{
		change_name("\n…e…e…e\n");
	}

	start_t = clock();
}

void CMiscHacks::ChatSpamName()
{
	static clock_t start_t = clock();
	double timeSoFar = (double)(clock() - start_t) / CLOCKS_PER_SEC;
	if (timeSoFar < 0.001)
		return;

	std::vector < std::string > Names;

	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		// Get the entity
		IClientEntity *entity = Interfaces::EntList->GetClientEntity(i);

		player_info_t pInfo;
		// If it's a valid entity and isn't the player
		if (entity && hackManager.pLocal()->GetTeamNum() == entity->GetTeamNum() && entity != hackManager.pLocal())
		{
			ClientClass* cClass = (ClientClass*)entity->GetClientClass();

			// If entity is a player
			if (cClass->m_ClassID == (int)CSGOClassID::CCSPlayer)
			{
				if (Interfaces::Engine->GetPlayerInfo(i, &pInfo))
				{
					if (!strstr(pInfo.name, "GOTV"))
						Names.push_back(pInfo.name);
				}
			}
		}
	}

	static bool wasSpamming = true;
	//static std::string nameBackup = "INTERWEBZ.CC";

	int randomIndex = rand() % Names.size();
	char buffer[128];
	sprintf_s(buffer, "%s ", Names[randomIndex].c_str());

	if (wasSpamming)
	{
		change_name(buffer);
	}
	else
	{
		change_name ("p$i 1337");
	}

	start_t = clock();
}

void CMiscHacks::ChatSpamRegular()
{
	// Don't spam it too fast so you can still do stuff
	static clock_t start_t = clock();
	int spamtime = Menu::Window.MiscTab.OtherChatDelay.GetValue();
	double timeSoFar = (double)(clock() - start_t) / CLOCKS_PER_SEC;
	if (timeSoFar < spamtime)
		return;

	static bool holzed = true;

	if (Menu::Window.MiscTab.OtherTeamChat.GetState())
	{
		SayInTeamChat("COMPLEX OWNS ME AND ALL");
	}
	else
	{
		SayInChat("COMPLEX OWNS ME AND ALL");
	}

	start_t = clock();
}



void CMiscHacks::walkbot(CUserCmd* pCmd, IClientEntity *pLocal)
{
/*
	bool walkbotBefore = false;

	Vector viewangles;
	Interfaces::Engine->GetViewAngles(viewangles);

	auto fDistanceToWall = [&](Vector diff = Vector(0, 0, 0))->float {
		auto tmpviewangles = viewangles + diff;
		trace_t tr;
		Ray_t ray;
		CTraceFilter filter;
		filter.pSkip = pLocal;
		Vector begin = pLocal->GetEyePosition(), t, end;
		AngleVectors(tmpviewangles, &t);
		Normalize(t, end);
		end *= 8192.0f;
		end += begin;
		ray.Init(begin, end);
		Interfaces::Trace->TraceRay(ray, 0x4600400B, &filter, &tr);
		return (begin - tr.endpos).Size();
	};

	static float old1, old2, old3;
	if (pLocal->GetVelocity().Length() < 3)
	{
		viewangles.y += 2.0f;
	}
	int Distances = Menu::Window.MiscTab.Distance.GetValue();
	if (fDistanceToWall() < Distances) // we are near to some wall
	{
		int turn = Menu::Window.MiscTab.Edgeam.GetValue();
		float negativeDist = fDistanceToWall(Vector(0, -1, 0)), positiveDist = fDistanceToWall(Vector(0, 1, 0));
		if (abs(negativeDist - positiveDist) < 1.0f)
		{
			viewangles.y += turn;
		}
		else
		{
			viewangles.y += positiveDist < negativeDist ? -1 : 1;
		}
	}
	//    //if ( ( pLocalEntity->GetFlags( ) & FL_ONGROUND ) )
	//    //    pVerified->m_cmd.buttons |= IN_JUMP;
	//    //else
	//    //{
	//    //    pVerified->m_cmd.buttons |= IN_DUCK;
	//    //    pVerified->m_cmd.buttons &= ~IN_FORWARD;
	//    //}
	//}

	if (abs(viewangles.x) > 0)
		viewangles.x = 1;

	while (viewangles.y > 180.0f)
		viewangles.y -= 360.0f;
	while (viewangles.y < -180.0f)
		viewangles.y += 360.0f;

	Interfaces::Engine->SetViewAngles(viewangles);

	if (!walkbotBefore)
	{
		Interfaces::Engine->ClientCmd_Unrestricted("+forward");
		walkbotBefore = true;
	}

	else if (walkbotBefore)
	{
		walkbotBefore = false;
		Interfaces::Engine->ClientCmd_Unrestricted("-forward");
	}*/
}


void CMiscHacks::CircleStrafer(CUserCmd* pCmd, Vector& vecOriginalView) {
	IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	static int iCircleFact = 0;
	iCircleFact++;
	if (iCircleFact > 361)
		iCircleFact = 0;

	float flRotation = 3.f * iCircleFact - Interfaces::Globals->interval_per_tick;

	Vector StoredViewAngles = pCmd->viewangles;
	int CircleKey = Menu::Window.MiscTab.OtherCircleKey.GetKey();
	if (CircleKey > 0 && GUI.GetKeyState(CircleKey)) {
		pCmd->forwardmove = 450.f;
		pCmd->viewangles = vecOriginalView;
		flRotation = DEG2RAD(flRotation);

		float cos_rot = cos(flRotation);
		float sin_rot = sin(flRotation);

		float new_forwardmove = (cos_rot * pCmd->forwardmove) - (sin_rot * pCmd->sidemove);
		float new_sidemove = (sin_rot * pCmd->forwardmove) + (cos_rot * pCmd->sidemove);

		pCmd->forwardmove = new_forwardmove;
		pCmd->sidemove = new_sidemove;
	}

	return;
}

void CMiscHacks::doFakeWalk(CUserCmd * pCmd, bool & bSendPacket)
{
	IClientEntity* pLocal = hackManager.pLocal();
	if (GetAsyncKeyState(VK_SHIFT))
	{
		static int iChoked = -1;
		iChoked++;

		if (iChoked < 1)
		{
			bSendPacket = false;

			pCmd->tick_count += 10;
			pCmd->command_number += 7 + pCmd->tick_count % 2 ? 0 : 1;

			pCmd->buttons |= pLocal->GetMoveType() == IN_BACK;
			pCmd->forwardmove = pCmd->sidemove = 0.f;
		}
		else
		{
			bSendPacket = true;
			iChoked = -1;

			Interfaces::Globals->frametime *= (pLocal->GetVelocity().Length2D()) / 1.f;
			pCmd->buttons |= pLocal->GetMoveType() == IN_FORWARD;
		}
	}
}