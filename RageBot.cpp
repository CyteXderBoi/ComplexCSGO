#include "RageBot.h"
#include "RenderManager.h"
#include "Autowall.h"
#include <iostream>
#include "UTIL Functions.h"
#include "Resolver.h"
#include "Nospread.h"

#include <stdlib.h>
#include <intrin.h>
#include <math.h>

#define TICK_INTERVAL			( Interfaces::Globals->interval_per_tick )
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )

void CRageBot::StartLagCompensation(IClientEntity* pEntity, CUserCmd* pCmd)
{
	float flSimTime = pEntity->GetSimulationTime();
	pCmd->tick_count = TIME_TO_TICKS(flSimTime + 0.031f);
}

void CRageBot::Init()
{
	IsAimStepping = false;
	IsLocked = false;
	TargetID = -1;
}

void CRageBot::Draw()
{

}

bool IsAbleToShoot(IClientEntity* pLocal)
{
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	if (!pLocal)
		return false;

	if (!pWeapon)
		return false;

	float flServerTime = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;

	return (!(pWeapon->GetNextPrimaryAttack() > flServerTime));
}

// The Interp Fix
float InterpolationFix()
{
	static ConVar* cvar_cl_interp = Interfaces::CVar->FindVar("cl_interp");
	static ConVar* cvar_cl_updaterate = Interfaces::CVar->FindVar("cl_updaterate");
	static ConVar* cvar_sv_maxupdaterate = Interfaces::CVar->FindVar("sv_maxupdaterate");
	static ConVar* cvar_sv_minupdaterate = Interfaces::CVar->FindVar("sv_minupdaterate");
	static ConVar* cvar_cl_interp_ratio = Interfaces::CVar->FindVar("cl_interp_ratio");

	IClientEntity* pLocal = hackManager.pLocal();
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	float cl_interp = cvar_cl_interp->GetFloat();
	int cl_updaterate = cvar_cl_updaterate->GetInt();
	int sv_maxupdaterate = cvar_sv_maxupdaterate->GetInt();
	int sv_minupdaterate = cvar_sv_minupdaterate->GetInt();
	int cl_interp_ratio = cvar_cl_interp_ratio->GetInt();

	if (sv_maxupdaterate <= cl_updaterate)
		cl_updaterate = sv_maxupdaterate;

	if (sv_minupdaterate > cl_updaterate)
		cl_updaterate = sv_minupdaterate;

	float new_interp = (float)cl_interp_ratio / (float)cl_updaterate;

	if (new_interp > cl_interp)
		cl_interp = new_interp;

	return max(cl_interp, cl_interp_ratio / cl_updaterate);
}

float RandomFloat(float min, float max)
{
	typedef float(*RandomFloat_t)(float, float);
	static RandomFloat_t m_RandomFloat = (RandomFloat_t)GetProcAddress(GetModuleHandle(TEXT("vstdlib.dll")), "RandomFloat");
	return m_RandomFloat(min, max);
}

float hitchance(IClientEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	//	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	float hitchance = 101;
	if (!pWeapon) return 0;
	if (Menu::Window.RageBotTab.AccuracyHitchance.GetValue() > 1)
	{//Inaccuracy method
		float inaccuracy = pWeapon->GetInaccuracy();
		if (inaccuracy == 0) inaccuracy = 0.0000001;
		inaccuracy = 1 / inaccuracy;
		hitchance = inaccuracy;

	}
	return hitchance;
}

bool CanOpenFire()
{
	IClientEntity* pLocalEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!pLocalEntity)
		return false;

	CBaseCombatWeapon* entwep = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocalEntity->GetActiveWeaponHandle());

	float flServerTime = (float)pLocalEntity->GetTickBase() * Interfaces::Globals->interval_per_tick;
	float flNextPrimaryAttack = entwep->GetNextPrimaryAttack();

	std::cout << flServerTime << " " << flNextPrimaryAttack << std::endl;

	return !(flNextPrimaryAttack > flServerTime);
}

void CRageBot::Move(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pLocalEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!pLocalEntity)
		return;

	// Master switch
	if (!Menu::Window.RageBotTab.Active.GetState())
		return;

	// Anti Aim 
	if (Menu::Window.HvHTab.AntiAimEnable.GetState())
	{
		static int ChokedPackets = -1;

		CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
		if (!pWeapon)
			return;

		if (ChokedPackets < 1 && pLocalEntity->GetLifeState() == LIFE_ALIVE && pCmd->buttons & IN_ATTACK && CanOpenFire() && GameUtils::IsBallisticWeapon(pWeapon))
		{
			bSendPacket = false;
		}
		else
		{
			if (pLocalEntity->GetLifeState() == LIFE_ALIVE)
			{
				DoAntiAim(pCmd, bSendPacket);
			}
			ChokedPackets = -1;
		}
	}

	// Aimbot
	if (Menu::Window.RageBotTab.AimbotEnable.GetState())
		DoAimbot(pCmd, bSendPacket);

	// Position Adjustment
	if (Menu::Window.HvHTab.AccuracyPositionAdjustment.GetState())
		PositionAdjustment(pCmd);

	// Recoil
	if (Menu::Window.RageBotTab.AccuracyRecoil.GetState())
		DoNoRecoil(pCmd);

	// Aimstep
	if (Menu::Window.RageBotTab.AimbotAimStep.GetState())
	{
		Vector AddAngs = pCmd->viewangles - LastAngle;
		if (AddAngs.Length2D() > 25.f)
		{
			Normalize(AddAngs, AddAngs);
			AddAngs *= 25;
			pCmd->viewangles = LastAngle + AddAngs;
			GameUtils::NormaliseViewAngle(pCmd->viewangles);
		}
	}

	LastAngle = pCmd->viewangles;
}

void VectorAngles3(Vector forward, Vector &angles)
{
	float tmp, yaw, pitch;

	if (forward[2] == 0 && forward[0] == 0)
	{
		yaw = 0;

		if (forward[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180 / PI);

		if (yaw < 0)
			yaw += 360;

		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180 / PI);

		if (pitch < 0)
			pitch += 360;
	}

	if (pitch > 180)
		pitch -= 360;
	else if (pitch < -180)
		pitch += 360;

	if (yaw > 180)
		yaw -= 360;
	else if (yaw < -180)
		yaw += 360;

	if (pitch > 89)
		pitch = 89;
	else if (pitch < -89)
		pitch = -89;

	if (yaw > 180)
		yaw = 180;
	else if (yaw < -180)
		yaw = -180;

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}

Vector TickPrediction(Vector AimPoint, IClientEntity* pTarget)
{
	return AimPoint + (pTarget->GetVelocity() * Interfaces::Globals->interval_per_tick);
}

Vector BestPoint(IClientEntity *targetPlayer, Vector &final)
{
	IClientEntity* pLocal = hackManager.pLocal();

	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	filter.pSkip = targetPlayer;
	ray.Init(final + Vector(0, 0, 10), final);
	Interfaces::Trace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	final = tr.endpos;
	return final;

}

void CRageBot::PositionAdjustment(CUserCmd* pCmd)
{
	static ConVar* cvar_cl_interp = Interfaces::CVar->FindVar("cl_interp");
	static ConVar* cvar_cl_updaterate = Interfaces::CVar->FindVar("cl_updaterate");
	static ConVar* cvar_sv_maxupdaterate = Interfaces::CVar->FindVar("sv_maxupdaterate");
	static ConVar* cvar_sv_minupdaterate = Interfaces::CVar->FindVar("sv_minupdaterate");
	static ConVar* cvar_cl_interp_ratio = Interfaces::CVar->FindVar("cl_interp_ratio");

	IClientEntity* pLocal = hackManager.pLocal();

	if (!pLocal)
		return;

	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
	if (!pWeapon)
		return;

	float cl_interp = cvar_cl_interp->GetFloat();
	int cl_updaterate = cvar_cl_updaterate->GetInt();
	int sv_maxupdaterate = cvar_sv_maxupdaterate->GetInt();
	int sv_minupdaterate = cvar_sv_minupdaterate->GetInt();
	int cl_interp_ratio = cvar_cl_interp_ratio->GetInt();

	if (sv_maxupdaterate <= cl_updaterate)
		cl_updaterate = sv_maxupdaterate;

	if (sv_minupdaterate > cl_updaterate)
		cl_updaterate = sv_minupdaterate;

	float new_interp = (float)cl_interp_ratio / (float)cl_updaterate;

	if (new_interp > cl_interp)
		cl_interp = new_interp;

	float flSimTime = pLocal->GetSimulationTime();
	float flOldAnimTime = pLocal->GetAnimTime();

	int iTargetTickDiff = (int)(0.5f + (flSimTime - flOldAnimTime) / Interfaces::Globals->interval_per_tick);
}


// Functionality
void CRageBot::DoAimbot(CUserCmd *pCmd,bool &bSendPacket) // Ragebot, Aimbot
{
	IClientEntity* pTarget = nullptr;
	IClientEntity* pLocal = hackManager.pLocal();
	Vector Start = pLocal->GetViewOffset() + pLocal->GetOrigin();
	IClientEntity *pEntity = nullptr;
	bool FindNewTarget = true;
	//IsLocked = false;

	CSWeaponInfo* weapInfo = ((CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle()))->GetCSWpnData();

	// Don't aimbot with the knife..
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	// Auto Revolver
	if (GameUtils::IsRevolver(pWeapon) && Menu::Window.RageBotTab.AimbotAutoRevolver.GetState())
	{
		static int delay = 0;
		delay++;

		if (delay <= 15)
			pCmd->buttons |= IN_ATTACK;
		else
			delay = 0;
	}

	// Aimbot won't run if it's a knife or grenades
	if (pWeapon)
	{
		if (pWeapon->GetAmmoInClip() == 0 || !GameUtils::IsBallisticWeapon(pWeapon))
		{
			//TargetID = 0;
			//pTarget = nullptr;
			//HitBox = -1;
			return;
		}
	}
	else
		return;

	if (IsLocked && TargetID >= 0 && HitBox >= 0)
	{
		pTarget = Interfaces::EntList->GetClientEntity(TargetID);
		if (pTarget  && TargetMeetsRequirements(pTarget))
		{
			HitBox = HitScan(pTarget);
			if (HitBox >= 0)
			{
				Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
				Vector View;
				Interfaces::Engine->GetViewAngles(View);

				float FoV = FovToPlayer(ViewOffset, View, pTarget, HitBox);
				if (FoV < Menu::Window.RageBotTab.AimbotFov.GetValue())
					FindNewTarget = false;
			}
		}
	}

	// Find a new target, apparently we need to
	if (FindNewTarget)
	{
		TargetID = 0;
		pTarget = nullptr;
		HitBox = -1;

		// Target selection type
		switch (Menu::Window.RageBotTab.TargetSelection.GetIndex())
		{
		case 0:
			TargetID = GetTargetCrosshair();
			break;
		case 1:
			TargetID = GetTargetDistance();
			break;
		case 2:
			TargetID = GetTargetHealth();
			break;
		}

		// Memes
		if (TargetID >= 0)
		{
			pTarget = Interfaces::EntList->GetClientEntity(TargetID);
		}
		else
		{
			pTarget = nullptr;
			HitBox = -1;
		}
	} 

	Globals::Target = pTarget;
	Globals::TargetID = TargetID;

	// If we finally have a good target
	if (TargetID >= 0 && pTarget)
	{
		// Get the hitbox to shoot at
		HitBox = HitScan(pTarget);

		if (!CanOpenFire())
			return;

		// Key
		if (Menu::Window.RageBotTab.AimbotKeyPress.GetState())
		{
			int Key = Menu::Window.RageBotTab.AimbotKeyBind.GetKey();
			if (Key >= 0 && !GUI.GetKeyState(Key))
			{
				TargetID = -1;
				pTarget = nullptr;
				HitBox = -1;
				return;
			}
		}

		// Stop key
		int StopKey = Menu::Window.RageBotTab.AimbotStopKey.GetKey();
		if (StopKey >= 0 && GUI.GetKeyState(StopKey))
		{
			TargetID = -1;
			pTarget = nullptr;
			HitBox = -1;
			return;
		}

		float pointscale = Menu::Window.RageBotTab.TargetPointscale.GetValue() * 10 - 5;

		Vector Point;
		Vector AimPoint = GetHitboxPosition(pTarget, HitBox) + Vector(0, 0, pointscale);

		if (Menu::Window.RageBotTab.TargetMultipoint.GetState())
		{
			Point = BestPoint(pTarget, AimPoint);
		}
		else
		{
			Point = AimPoint;
		}

		Point = TickPrediction(Point, pTarget);

		if (Menu::Window.HvHTab.PerfectAccuracy.GetState())
			pCmd->tick_count = TIME_TO_TICKS(InterpolationFix());

		if (Menu::Window.HvHTab.PerfectAccuracy.GetState())
			StartLagCompensation(pLocal, pCmd);

		if (GameUtils::IsScopedWeapon(pWeapon) && !pWeapon->IsScoped() && Menu::Window.RageBotTab.AccuracyAutoScope.GetState()) // Autoscope
		{
			pCmd->buttons |= IN_ATTACK2;
		}
		else
		{
			if ((Menu::Window.RageBotTab.AccuracyHitchance.GetValue() * 1.5 <= hitchance(pLocal, pWeapon)) || Menu::Window.RageBotTab.AccuracyHitchance.GetValue() == 0 || *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() == 64)
			{
				if (AimAtPoint(pLocal, Point, pCmd, bSendPacket))
				{
					if (Menu::Window.RageBotTab.AimbotAutoFire.GetState())
					{
						pCmd->buttons |= IN_ATTACK;
					}
				}
				else if (Menu::Window.RageBotTab.AimbotAutoFire.GetState())
				{
					pCmd->buttons |= IN_ATTACK;
				}
			}
		}

		if (IsAbleToShoot(pLocal) && pCmd->buttons & IN_ATTACK)
			Globals::Shots += 1;

		// Stop and Crouch
		if (TargetID >= 0 && pTarget)
		{
			if (Menu::Window.RageBotTab.AccuracyAutoStop.GetState())
			{
				pCmd->forwardmove = 0.f;
				pCmd->sidemove = 0.f;
				pCmd->buttons |= IN_DUCK;
			}
		}
	}

	// Auto Pistol
	if (GameUtils::IsPistol(pWeapon) && Menu::Window.RageBotTab.AimbotAutoPistol.GetState())
	{
		if (pCmd->buttons & IN_ATTACK)
		{
			static bool WasFiring = false;
			WasFiring = !WasFiring;
			
			if (WasFiring)
			{
				pCmd->buttons |= IN_ATTACK2;
			}
		}
	}
}

bool CRageBot::TargetMeetsRequirements(IClientEntity* pEntity)
{
	// Is a valid player
	if (pEntity && pEntity->IsDormant() == false && pEntity->IsAlive() && pEntity->GetIndex() != hackManager.pLocal()->GetIndex())
	{
		// Entity Type checks
		ClientClass *pClientClass = pEntity->GetClientClass();
		player_info_t pinfo;
		if (pClientClass->m_ClassID == (int)CSGOClassID::CCSPlayer && Interfaces::Engine->GetPlayerInfo(pEntity->GetIndex(), &pinfo))
		{
			// Team Check
			if (pEntity->GetTeamNum() != hackManager.pLocal()->GetTeamNum() || Menu::Window.RageBotTab.TargetFriendlyFire.GetState())
			{
				// Spawn Check
				if (!pEntity->HasGunGameImmunity())
				{
					return true;
				}
			}
		}
	}

	// They must have failed a requirement
	return false;
}

float CRageBot::FovToPlayer(Vector ViewOffSet, Vector View, IClientEntity* pEntity, int aHitBox)
{
	CONST FLOAT MaxDegrees = 180.0f;
	Vector Angles = View;
	Vector Origin = ViewOffSet;
	Vector Delta(0, 0, 0);
	Vector Forward(0, 0, 0);
	AngleVectors(Angles, &Forward);
	Vector AimPos = GetHitboxPosition(pEntity, aHitBox);
	VectorSubtract(AimPos, Origin, Delta);
	Normalize(Delta, Delta);
	FLOAT DotProduct = Forward.Dot(Delta);
	return (acos(DotProduct) * (MaxDegrees / PI));
}

int CRageBot::GetTargetCrosshair()
{
	// Target selection
	int target = -1;
	float minFoV = Menu::Window.RageBotTab.AimbotFov.GetValue();

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++) //GetHighestEntityIndex()
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (fov < minFoV)
				{
					minFoV = fov;
					target = i;
				}
			}
		}
	}

	return target;
}

int CRageBot::GetTargetDistance()
{
	// New Distance Aimbot
	int target = -1;
	int minDist = 8192;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				Vector Difference = pLocal->GetOrigin() - pEntity->GetOrigin();
				int Distance = Difference.Length();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (Distance < minDist && fov < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					minDist = Distance;
					target = i;
				}
			}
		}
	}
	return target;

	/* 
	// Target selection
	int target = -1;
	int minDist = 9999999999;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				Vector Difference = pLocal->GetOrigin() - pEntity->GetOrigin();
				int Distance = Difference.Length();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (Distance < minDist && fov < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					minDist = Distance;
					target = i;
				}
			}
		}
	}

	return target;
	*/
}

int CRageBot::GetTargetHealth()
{
	int target = -1;
	int minHealth = 101;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				int Health = pEntity->GetHealth();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (Health < minHealth && fov < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					minHealth = Health;
					target = i;
				}
			}
		}
	}
	return target;

	/*
	// Target selection
	int target = -1;
	int minHealth = 101;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				int Health = pEntity->GetHealth();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (Health < minHealth && fov < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					minHealth = Health;
					target = i;
				}
			}
		}
	}

	return target;
	*/
}

int CRageBot::HitScan(IClientEntity* pEntity)
{
	std::vector<int> HitBoxesToScan;
	bool AWall = Menu::Window.RageBotTab.AccuracyAutoWall.GetState();

	// Get the hitboxes to scan
	int HitScanMode = Menu::Window.RageBotTab.TargetHitscan.GetIndex();
	if (HitScanMode == 0)
	{
		// No Hitscan, just a single hitbox
		switch (Menu::Window.RageBotTab.TargetHitbox.GetIndex())
		{
		case 0:
			HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
			break;
		case 1:
			HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
			HitBoxesToScan.push_back((int)CSGOHitboxID::NeckLower);
			break;
		case 2:
			HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
			break;
		case 3:
			HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
			break;
		case 4:
			HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
			break;
		}
	}
	else
	{
		switch (HitScanMode)
		{
		case 0:
			// Nothing
			break;
		case 1:
			HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
			HitBoxesToScan.push_back((int)CSGOHitboxID::NeckLower);
			HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
			break;
		case 2:
			HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
			HitBoxesToScan.push_back((int)CSGOHitboxID::NeckLower);
			HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh);
			break;
		case 3:
			HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
			HitBoxesToScan.push_back((int)CSGOHitboxID::NeckLower);
			HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftLowerArm);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightHand);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftHand);
			break;
		}
	}

	for (int HitBoxID : HitBoxesToScan)
	{
		if (AWall)
		{
			Vector Point = GetHitboxPosition(pEntity, HitBoxID);
			float Damage = 0.f;
			if (CanHit(Point, &Damage))
			{
				if (Damage >= Menu::Window.RageBotTab.AccuracyMinimumDamage.GetValue())
				{
					return HitBoxID;
				}
			}
		}
		else
		{
			if (GameUtils::IsVisible(hackManager.pLocal(), pEntity, HitBoxID))
				return HitBoxID;
		}
	}
	return -1;
}

void CRageBot::DoNoRecoil(CUserCmd *pCmd)
{
	IClientEntity* pLocal = hackManager.pLocal();
	if (pLocal)
	{
		Vector AimPunch = pLocal->localPlayerExclusive()->GetAimPunchAngle();
		if (AimPunch.Length2D() > 0 && AimPunch.Length2D() < 150)
		{
			pCmd->viewangles -= AimPunch * 2;
			GameUtils::NormaliseViewAngle(pCmd->viewangles);
		}
	}
}

bool CRageBot::AimAtPoint(IClientEntity* pLocal, Vector point, CUserCmd *pCmd, bool &bSendPacket)
{
	bool ReturnValue = false;
	// Get the full angles
	if (point.Length() == 0) return ReturnValue;

	Vector angles;
	Vector src = pLocal->GetOrigin() + pLocal->GetViewOffset();

	CalcAngle(src, point, angles);
	GameUtils::NormaliseViewAngle(angles);

	if (angles[0] != angles[0] || angles[1] != angles[1])
	{
		return ReturnValue;
	}


	IsLocked = true;
	//-----------------------------------------------

	// Aim Step Calcs
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	if (!IsAimStepping)
		LastAimstepAngle = LastAngle; // Don't just use the viewangs because you need to consider aa

	float fovLeft = FovToPlayer(ViewOffset, LastAimstepAngle, Interfaces::EntList->GetClientEntity(TargetID), 0);

	if (fovLeft > 25.0f && Menu::Window.RageBotTab.AimbotAimStep.GetState())
	{
		Vector AddAngs = angles - LastAimstepAngle;
		Normalize(AddAngs, AddAngs);
		AddAngs *= 25;
		LastAimstepAngle += AddAngs;
		GameUtils::NormaliseViewAngle(LastAimstepAngle);
		angles = LastAimstepAngle;
	}
	else
	{
		ReturnValue = true;
	}

	GameUtils::NormaliseViewAngle(angles);

	if (Menu::Window.RageBotTab.AimbotSlientSelection.GetIndex() == 1)
	{
		pCmd->viewangles = angles;
	}

	if (!Menu::Window.RageBotTab.AimbotSlientSelection.GetIndex() == 1 || !Menu::Window.RageBotTab.AimbotSlientSelection.GetIndex() == 2)
		Interfaces::Engine->SetViewAngles(angles);
		Vector Oldview = pCmd->viewangles;

	if (Menu::Window.RageBotTab.AimbotSlientSelection.GetIndex() == 2)
	{
		static int ChokedPackets = -1;
		ChokedPackets++;

		if (ChokedPackets < 6)
		{
			bSendPacket = false;
			pCmd->viewangles = angles;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles = Oldview;
			ChokedPackets = -1;
			ReturnValue = false;
		}

		//pCmd->viewangles.z = 0;
	}

	return ReturnValue;
	LastAngle = pCmd->viewangles;
}

namespace AntiAims
{
	static bool flip = false;
	// Pitches

	void JitterPitch(CUserCmd *pCmd)
	{
		static bool up = true;
		if (up)
		{
			pCmd->viewangles.x = 45;
			up = !up;
		}
		else
		{
			pCmd->viewangles.x = 89;
			up = !up;
		}
	}


	void kiduaJitter(CUserCmd *pCmd, bool &bSendPacket)
	{
		static int jitterangle = 0;
		bSendPacket = true;
		if (jitterangle <= 1)
		{
			pCmd->viewangles.y -= 90;
		}
		else if (jitterangle > 1 && jitterangle <= 3)
		{
			pCmd->viewangles.y += 90;
		}

		static int iChoked = -1;
		iChoked++;
		if (iChoked < Menu::Window.HvHTab.FakeLagChoke.GetValue())
		{
			bSendPacket = false;
			if (jitterangle <= 1)
			{
				pCmd->viewangles.y += 90;
				jitterangle += 1;
			}
			else if (jitterangle > 1 && jitterangle <= 3)
			{
				pCmd->viewangles.y += 90;
				jitterangle += 1;
			}
			else
			{
				jitterangle = -90;
			}

		}
	}

	float DegreesToRadians(float Angle)
	{
		return Angle * M_PI / 180.0f;
	}


	void Sinster1(CUserCmd *pCmd, bool &bSendPacket)
	{
		float setyaw = pCmd->viewangles.y;
		setyaw += (bSendPacket ? Interfaces::Globals->curtime * 8 : fmod(DegreesToRadians(Interfaces::Globals->framecount * Interfaces::Globals->curtime), 360));
	}

	int random;
	int maxJitter;
	float temp;

	void JitterBackward1(CUserCmd *pCmd, bool &bSendPacket)
	{
		bSendPacket = true; //fake angle
		pCmd->viewangles.y -= 180.0f;
		random = rand() % 100;
		maxJitter = rand() % (85 - 70 + 1) + 70;
		temp = maxJitter - (rand() % maxJitter);
		if (random < 35 + (rand() % 15))
			pCmd->viewangles.y -= temp;
		else if (random < 85 + (rand() % 15))
			pCmd->viewangles.y += temp;
	}


	void FakePitch(CUserCmd *pCmd, bool &bSendPacket)
	{
		static int ChokedPackets = -1;
		ChokedPackets++;
		if (ChokedPackets < Menu::Window.HvHTab.FakeLagChoke.GetValue())
		{
			bSendPacket = false;
			pCmd->viewangles.x = 89;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.x = 51;
			ChokedPackets = -1;
		}
	}

	void FakeHead(CUserCmd *pCmd)
	{
		static bool ySwitch = false;
		int veloc = hackManager.pLocal()->GetVelocity().Length();
		if (ySwitch)
			pCmd->viewangles.y -= 90 * veloc;
		else
			pCmd->viewangles.y += 90 * veloc;

		ySwitch = !ySwitch;
	}


	void MoveFix(CUserCmd *cmd, Vector &realvec)
	{
		Vector vMove(cmd->forwardmove, cmd->sidemove, cmd->upmove);
		float flSpeed = sqrt(vMove.x * vMove.x + vMove.y * vMove.y), flYaw;
		Vector vMove2;
		VectorAngles(vMove, vMove2);

		flYaw = DEG2RAD(cmd->viewangles.y - realvec.y + vMove2.y);
		cmd->forwardmove = cos(flYaw) * flSpeed;
		cmd->sidemove = sin(flYaw) * flSpeed;

		if (cmd->viewangles.x < -90.f || cmd->viewangles.x > 90.f)
			cmd->forwardmove = -cmd->forwardmove;
	}


	void StaticJitter(CUserCmd *pCmd)
	{
		static bool down = true;
		if (down)
		{
			pCmd->viewangles.x = 179.0f;
			down = !down;
		}
		else
		{
			pCmd->viewangles.x = 89.0f;
			down = !down;
		}
	}

	// Yaws

	void FastSpin(CUserCmd *pCmd)
	{
		static int y2 = -179;
		int spinBotSpeedFast = 100;

		y2 += spinBotSpeedFast;

		if (y2 >= 179)
			y2 = -179;

		pCmd->viewangles.y = y2;
	}

	void FakeEdges(IClientEntity* pLocal, CUserCmd *pCmd, bool &bSendPacket)
	{
		{
			Vector vEyePos = pLocal->GetAbsOrigin() + pLocal->GetEyePosition();

			CTraceFilter filter;
			filter.pSkip = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

			for (int y = 0; y < 360; y++)
			{
				Vector qTmp(10.0f, pCmd->viewangles.y, 0.0f);
				qTmp.y += y;

				if (qTmp.y > 180.0)
					qTmp.y -= 360.0;
				else if (qTmp.y < -180.0)
					qTmp.y += 360.0;

				//NormaliseViewAngle(qTmp);

				Vector vForward;

				VectorAngles3(qTmp, vForward);

				float fLength = (19.0f + (19.0f * sinf(DEG2RAD(10.0f)))) + 7.0f;
				vForward *= fLength;

				trace_t tr;

				Vector vTraceEnd = vEyePos + vForward;

				Ray_t ray;

				ray.Init(vEyePos, vTraceEnd);
				Interfaces::Trace->TraceRay(ray, MASK_PLAYERSOLID_BRUSHONLY, &filter, &tr);

				if (tr.fraction != 1.0f)
				{
					Vector angles;

					Vector vNegative = Vector(tr.plane.normal.x * -1.0f, tr.plane.normal.y * -1.0f, tr.plane.normal.z * -1.0f);

					VectorAngles3(vNegative, angles);

					//NormaliseViewAngle(angles);

					qTmp.y = angles.y;

					//NormaliseViewAngle(qTmp);

					trace_t trLeft, trRight;

					Vector vLeft, vRight;
					VectorAngles3(qTmp + Vector(0.0f, 30.0f, 0.0f), vLeft);
					VectorAngles3(qTmp + Vector(0.0f, 30.0f, 0.0f), vRight);

					vLeft *= (fLength + (fLength * sinf(DEG2RAD(30.0f))));
					vRight *= (fLength + (fLength * sinf(DEG2RAD(30.0f))));

					vTraceEnd = vEyePos + vLeft;

					ray.Init(vEyePos, vTraceEnd);
					Interfaces::Trace->TraceRay(ray, MASK_PLAYERSOLID_BRUSHONLY, &filter, &trLeft);

					vTraceEnd = vEyePos + vRight;

					ray.Init(vEyePos, vTraceEnd);
					Interfaces::Trace->TraceRay(ray, MASK_PLAYERSOLID_BRUSHONLY, &filter, &trRight);

					if ((trLeft.fraction == 1.0f)
						&& (trRight.fraction != 1.0f))
					{
						qTmp.y -= 10000000005.0f;

						// LEFT
					}
					else if ((trLeft.fraction != 1.0f)
						&& (trRight.fraction == 1.0f))
					{
						qTmp.y += 10000000005.0f;


						// RIGHT
					}
					pCmd->viewangles.y = -qTmp.y;

					return;
				}
			}
		}
	}


	void Arizona1(CUserCmd *pCmd, bool &bSendPacket) //Jitter Movement mit static bool Fast :)
	{
		static bool Fast = false;
		if (Fast)
		{
			bSendPacket = false; //true angle
			pCmd->viewangles.y = pCmd->viewangles.y - 134.0;
		}
		else
		{
			bSendPacket = true; //fake angle
			pCmd->viewangles.y = pCmd->viewangles.y - 226.0;
		}
		Fast = !Fast;
	}

	void FakeSpinLeft(CUserCmd* pCmd, bool &bSendPacket)
	{
		static bool fake;
		if (fake)
		{
			bSendPacket = true;
			Vector SpinSoSlow;
			SpinSoSlow.y = (Interfaces::Globals->curtime * 800.0);
			GameUtils::NormaliseViewAngle(SpinSoSlow);
			pCmd->viewangles.y = SpinSoSlow.y;
			fake = false;
		}
		else
		{
			bSendPacket = false;
			pCmd->viewangles.y += 90;
			fake = true;
		}
	}

	void CSpin(CUserCmd *pCmd, bool &bSendPacket)
	{
		static bool f_flip = true;
		f_flip = !f_flip;

		if (f_flip)
		{
			float CalculatedCurTime = (Interfaces::Globals->curtime * 350.0);
			pCmd->viewangles.y += CalculatedCurTime;
		}
		else if (!f_flip)
		{
			float CalculatedCurTime = (Interfaces::Globals->curtime * 350.0);
			pCmd->viewangles.y -= CalculatedCurTime;
		}

	}

	void halfSpin(CUserCmd *pCmd, bool &bSendPacket)
	{
		static bool f_flip = true;
		f_flip = !f_flip;


		if (f_flip)
		{
			bSendPacket = true;
			pCmd->viewangles.y = 0;
		}
		else if (!f_flip)
		{
			bSendPacket = true;
			pCmd->viewangles.y = -180;
		}
		else
			bSendPacket = false;
	}

	void FakeForward(CUserCmd *pCmd, bool &bSendPacket)
	{
		static int jitterangle = 0;

		if (jitterangle <= 1)
		{
			pCmd->viewangles.y += 0;
		}
		else if (jitterangle > 1 && jitterangle <= 3)
		{
			pCmd->viewangles.y += 0;
		}

		static int iChoked = -1;
		iChoked++;
		if (iChoked < 1)
		{
			bSendPacket = false;
			if (jitterangle <= 1)
			{
				pCmd->viewangles.y += 0;
				jitterangle += 1;
			}
			else if (jitterangle > 1 && jitterangle <= 3)
			{
				pCmd->viewangles.y -= 0;
				jitterangle += 1;
			}
			else
			{
				jitterangle = 0;
			}
		}
		else
		{
			bSendPacket = true;
			iChoked = -1;
		}
	}

	void TickOverRide(CUserCmd *pCmd, bool &bSendPacket)
	{
		bool jitter = false;
		int Add = 0;

		if (bSendPacket)
		{
			if (jitter)
				Add = -90;
			else
				Add = 90;
		}
		else
		{
			float flRandom = rand() % 5 + 1.f;
			switch (pCmd->tick_count % 4)
			{
			case 0:
				Add = -170.f - flRandom;
				break;
			case 3:
			case 1:
				Add = 180.f;
				break;
			case 2:
				Add = 170.f + flRandom;
				break;
			}
		}
	}


	void fakespin(CUserCmd *pCmd, bool &bSendPacket)
	{
		IClientEntity* pLocal = hackManager.pLocal();
		float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;

		if (bSendPacket)
		{
			pCmd->viewangles.y += (float)(fmod(server_time / 0.80f * 360.0f, 360.0f));
		}
		else
		{
			pCmd->viewangles.y -= (float)(fmod(server_time / 0.80f * 360.0f, 360.0f));
		}
	}

	void FakeTwoStep(CUserCmd* pCmd, bool& bSendPacket)
	{

		static bool bFlipYaw;
		float flInterval = Interfaces::Globals->interval_per_tick;
		float flTickcount = pCmd->tick_count;
		float flTime = flInterval * flTickcount;
		if (std::fmod(flTime, 1) == 0.f)
			bFlipYaw = !bFlipYaw;

		if (bSendPacket)
			pCmd->viewangles.y += bFlipYaw ? 90.f : -90.f;
		else
			pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() + bFlipYaw ? -90.f : 90.f;
	}




	void BackJitter(CUserCmd *pCmd)
	{
		int random = rand() % 100;

		// Small chance of starting fowards
		if (random < 98)
			// Look backwards
			pCmd->viewangles.y -= 180;

		// Some gitter
		if (random < 15)
		{
			float change = -70 + (rand() % (int)(140 + 1));
			pCmd->viewangles.y += change;
		}
		if (random == 69)
		{
			float change = -90 + (rand() % (int)(180 + 1));
			pCmd->viewangles.y += change;
		}
	}

	void FakeSideways(CUserCmd *pCmd, bool &bSendPacket)
	{

		static int ChokedPackets = -1;
		ChokedPackets++;
		if (ChokedPackets < Menu::Window.HvHTab.FakeLagChoke.GetValue())
		{
			bSendPacket = false;
			pCmd->viewangles.y += 90;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.y -= 180;
			ChokedPackets = -1;
		}
	}

	void Jitter(CUserCmd *pCmd)
	{
		static int jitterangle = 0;

		if (jitterangle <= 1)
		{
			pCmd->viewangles.y += 90;
		}
		else if (jitterangle > 1 && jitterangle <= 3)
		{
			pCmd->viewangles.y -= 90;
		}

		int re = rand() % 4 + 1;


		if (jitterangle <= 1)
		{
			if (re == 4)
				pCmd->viewangles.y += 180;
			jitterangle += 1;
		}
		else if (jitterangle > 1 && jitterangle <= 3)
		{
			if (re == 4)
				pCmd->viewangles.y -= 180;
			jitterangle += 1;
		}
		else
		{
			jitterangle = 0;
		}
	}

	void FakeStatic(CUserCmd *pCmd, bool &bSendPacket)
	{
		static int ChokedPackets = -1;
		ChokedPackets++;
		if (ChokedPackets < Menu::Window.HvHTab.FakeLagChoke.GetValue())
		{
			bSendPacket = false;
			static int y2 = -179;
			int spinBotSpeedFast = 360.0f / 1.618033988749895f;

			y2 += spinBotSpeedFast;

			if (y2 >= 179)
				y2 = -179;

			pCmd->viewangles.y = y2;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.y -= 180;
			ChokedPackets = -1;
		}
	}

	void TJitter(CUserCmd *pCmd)
	{
		static bool Turbo = true;
		if (Turbo)
		{
			pCmd->viewangles.y -= 90;
			Turbo = !Turbo;
		}
		else
		{
			pCmd->viewangles.y += 90;
			Turbo = !Turbo;
		}
	}


	void TFake(CUserCmd *pCmd, bool &bSendPacket)
	{

		static int ChokedPackets = -1;
		ChokedPackets++;
		if (ChokedPackets < Menu::Window.HvHTab.FakeLagChoke.GetValue())
		{
			bSendPacket = false;
			pCmd->viewangles.y = -90;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.y = 90;
			ChokedPackets = -1;
		}
	}

	void FakeJitter(CUserCmd* pCmd, bool &bSendPacket)
	{
		static int jitterangle = 0;

		if (jitterangle <= 1)
		{
			pCmd->viewangles.y += 135;
		}
		else if (jitterangle > 1 && jitterangle <= 3)
		{
			pCmd->viewangles.y += 225;
		}

		static int iChoked = -1;
		iChoked++;
		if (iChoked < Menu::Window.HvHTab.FakeLagChoke.GetValue())
		{
			bSendPacket = false;
			if (jitterangle <= 1)
			{
				pCmd->viewangles.y += 45;
				jitterangle += 1;
			}
			else if (jitterangle > 1 && jitterangle <= 3)
			{
				pCmd->viewangles.y -= 45;
				jitterangle += 1;
			}
			else
			{
				jitterangle = 0;
			}
		}
		else
		{
			bSendPacket = true;
			iChoked = -1;
		}
	}

	void opaf(CUserCmd* pCmd, bool &bSendPacket)
	{
		static int jitterangle = 0;

		if (jitterangle <= 1)
		{
			pCmd->viewangles.y += 90;
		}
		else if (jitterangle > 1 && jitterangle <= 2)
		{
			pCmd->viewangles.y += -90;
		}


		if (jitterangle <= 1)
		{
			bSendPacket = false;
			if (jitterangle <= 1)
			{
				pCmd->viewangles.y += 180;
				jitterangle += 1;
			}
			else if (jitterangle > 1 && jitterangle <= 2)
			{
				pCmd->viewangles.y -= -179;
				jitterangle += 1;
			}
			else
			{
				jitterangle = 0;
			}
		}
		else
		{
			bSendPacket = true;

		}
	}


	void fakelowerbody(CUserCmd *pCmd, bool &bSendPacket)
	{
		static bool f_flip = true;
		f_flip = !f_flip;

		if (f_flip)
		{
			pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() + 90.00f;
			bSendPacket = false;
		}
		else if (!f_flip)
		{
			pCmd->viewangles.y += hackManager.pLocal()->GetLowerBodyYaw() - 90.00f;
			bSendPacket = true;
		}
	}
	void YAW_fakeside_backj(CUserCmd *pCmd, bool &bSendPacket)
	{
		static bool f_flip = true;
		f_flip = !f_flip;
		if (f_flip)
		{
			bSendPacket = true;
			if (f_flip)
			{
				pCmd->viewangles.y += 50.000000f;
			}
			else if (!f_flip)
				pCmd->viewangles.y -= 70.000000f;
		}
		else
		{
			bSendPacket = false;
			if (f_flip)
				pCmd->viewangles.y += 190.000000f;
			else if (!f_flip)
				pCmd->viewangles.y -= 170.000000f;

		}
	}
	void YAW_FSTATIC3_rmake(CUserCmd *pCmd, bool &bSendPacket)
	{
		static bool f_flip = true;
		f_flip = !f_flip;

		if (f_flip)
		{
			bSendPacket = true;

			if (!f_flip)
			{
				pCmd->viewangles.y += 50.000000f;
			}
			else if (f_flip)
				pCmd->viewangles.y -= 70.000000f;
		}
		else
		{
			bSendPacket = false;
			if (f_flip)
				pCmd->viewangles.y += 212.000000f;
			else if (!f_flip)
				pCmd->viewangles.y -= 180.000000f;


		}
	}

	void YAW_FSTATIC_Remake(CUserCmd *pCmd, bool &bSendPacket)
	{
		static bool f_flip = true;
		f_flip = !f_flip;

		if (f_flip)
		{
			bSendPacket = true;

			if (!f_flip)
				pCmd->viewangles.y += 50.000000f;

			else
				pCmd->viewangles.y -= 60.000000f;
		}
		else
		{
			bSendPacket = false;
			pCmd->viewangles.y += 180.000000f;
		}
	}
	void LBYBreaker(CUserCmd *pCmd, bool &bSendPacket)
	{
		QAngle angle_for_yaw;
		static int counter = 0;
		static int motion = 0;
		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 2;

		bool flip = true;

		if (flip)
		{
			if (counter % 48 == 0)
				motion++;
			int value = ServerTime % 2;
			switch (value) {
			case 0:pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - 90.00f > 35 ? hackManager.pLocal()->GetLowerBodyYaw() - 180.f : hackManager.pLocal()->GetLowerBodyYaw() - 90.f; break;
				bSendPacket = false;
			case 1:pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 90.00f > 35 ? hackManager.pLocal()->GetLowerBodyYaw() - 90.f : hackManager.pLocal()->GetLowerBodyYaw() + 90.f; break;
				bSendPacket = true;
			}
			counter++;
		}
		else
		{


			if (counter % 48 == 0)
				motion++;
			int value = ServerTime % 2;
			switch (value) {
			case 0:pCmd->viewangles.y = angle_for_yaw.y + (rand() % 100 > 33 ? (rand() % 50 > 13 ? (rand() % 20 + 40) : -(rand() % 20 + 40)) : (rand() % 100 > 71 ? (rand() % 20 + 150) : -(rand() % 20 + 150))); break;
				bSendPacket = false;
			case 1:pCmd->viewangles.y = angle_for_yaw.y + (rand() % 100 > 33 ? (rand() % 50 > 13 ? (rand() % 20 + 40) : -(rand() % 20 + 40)) : (rand() % 100 > 71 ? (rand() % 20 + 150) : -(rand() % 20 + 150))); break;
				bSendPacket = true;
			}
			counter++;
		}
	}

	int BestTarget;

	void jewbag(CUserCmd *pCmd, bool &bSendPacket)
	{
		float server_time = Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 2;
		bool flip = true;
		QAngle angle_for_yaw;
		if (BestTarget > 0)
			CalcAngle(hackManager.pLocal()->GetOrigin(), Interfaces::EntList->GetClientEntity(BestTarget)->GetOrigin(), angle_for_yaw);
		else
			Interfaces::Engine->GetViewAngles(angle_for_yaw);

		if (flip)
		{
			bSendPacket = false;

			switch ((int)server_time % 3)
			{
			case 0: pCmd->viewangles.y = fabs(hackManager.pLocal()->GetLowerBodyYaw() - 180.f > 35 ? hackManager.pLocal()->GetLowerBodyYaw() - 180.f : angle_for_yaw.y - 90.f); break;
			case 1: pCmd->viewangles.y = fabs(hackManager.pLocal()->GetLowerBodyYaw() + 90.f > 35 ? hackManager.pLocal()->GetLowerBodyYaw() - 90.f : angle_for_yaw.y + 90.f); break;
			case 2: pCmd->viewangles.y = fabs(hackManager.pLocal()->GetLowerBodyYaw() - 90.f > 35 ? hackManager.pLocal()->GetLowerBodyYaw() + 90.f : angle_for_yaw.y - 90.f); break;
			}

		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.y = angle_for_yaw.y + (rand() % 100 > 33 ? (rand() % 50 > 13 ? (rand() % 20 + 40) : -(rand() % 20 + 40)) : (rand() % 100 > 71 ? (rand() % 20 + 150) : -(rand() % 20 + 150)));
		}
	}

	void LBY2(CUserCmd *pCmd, bool &bSendPacket)
	{
		QAngle angle_for_yaw;
		static int counter = 0;
		static int motion = 0;
		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 2;

		bool flip = true;

		if (flip)
		{
			if (counter % 48 == 0)
				motion++;
			int value = ServerTime % 2;
			switch (value) {
			case 0:pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - 90.00f > 35 ? hackManager.pLocal()->GetLowerBodyYaw() - 180.f : hackManager.pLocal()->GetLowerBodyYaw() - 90.f; break;
				bSendPacket = false;
			case 1:pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 90.00f > 35 ? hackManager.pLocal()->GetLowerBodyYaw() - 90.f : hackManager.pLocal()->GetLowerBodyYaw() + 90.f; break;
				bSendPacket = true;
			}
			counter++;
		}
		else
		{


			if (counter % 48 == 0)
				motion++;
			int value = ServerTime % 2;
			switch (value) {

			case 0:pCmd->viewangles.y = angle_for_yaw.y + (rand() % 100 > 33 ? (rand() % 50 > 13 ? (rand() % 20 + 40) : -(rand() % 20 + 40)) : (rand() % 100 > 71 ? (rand() % 20 + 150) : -(rand() % 20 + 150))); break;
				bSendPacket = true;
			}
			counter++;
		}
	}

	void sidestrash(CUserCmd *pCmd, bool &bSendPacket)
	{
		static float StoredYaw = 0;
		IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
		static bool flip = false;
		static bool flip2 = false;
		float flip2angle = 0.f;
		if (pLocal->GetLowerBodyYaw() != StoredYaw) //If lowerbody updates and is different than previous change the antiaim
			flip2 = !flip2;
		if (flip2)
			flip2angle = 180.f;
		else
			flip2angle = 0.f;


		if (flip)
		{
			pCmd->viewangles.y += 90.0 + flip2angle;
			bSendPacket = false;
		}
		else
		{
			pCmd->viewangles.y += -90.0 + flip2angle;
			bSendPacket = true;
		}
		StoredYaw = pLocal->GetLowerBodyYaw();
		flip = !flip;
	}

	void tolong(CUserCmd *pCmd, bool &bSendPacket)
	{
		static float StoredYaw = 0;
		IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
		static bool flip = false;
		static bool flip2 = false;
		float flip2angle = 0.f;
		if (pLocal->GetLowerBodyYaw() != StoredYaw) //If lowerbody updates and is different than previous change the antiaim
			flip2 = !flip2;
		if (flip2)
			flip2angle = 180.f;
		else
			flip2angle = 0.f;


		if (flip)
		{
			pCmd->viewangles.y += 90.0 + flip2angle;
			bSendPacket = false;
		}
		else
		{
			pCmd->viewangles.y += -90.0 + flip2angle;
			bSendPacket = true;
		}
		if (flip)
		{
			pCmd->viewangles.y += 100.0 + flip2angle;
			bSendPacket = false;
		}
		else
		{
			pCmd->viewangles.y += -100.0 + flip2angle;
			bSendPacket = true;
		}
		if (flip)
		{
			pCmd->viewangles.y += 110.0 + flip2angle;
			bSendPacket = false;
		}
		else
		{
			pCmd->viewangles.y += -110.0 + flip2angle;
			bSendPacket = true;
		}
		if (flip)
		{
			pCmd->viewangles.y += 120.0 + flip2angle;
			bSendPacket = false;
		}
		else
		{
			pCmd->viewangles.y += -120.0 + flip2angle;
			bSendPacket = true;
		}
		if (flip)
		{
			pCmd->viewangles.y += 130.0 + flip2angle;
			bSendPacket = false;
		}
		else
		{
			pCmd->viewangles.y += -130.0 + flip2angle;
			bSendPacket = true;
		}
		if (flip)
		{
			pCmd->viewangles.y += 310.0 + flip2angle;
			bSendPacket = false;
		}
		else
		{
			pCmd->viewangles.y += -310.0 + flip2angle;
			bSendPacket = true;
		}
		if (flip)
		{
			pCmd->viewangles.y += 300.0 + flip2angle;
			bSendPacket = false;
		}
		else
		{
			pCmd->viewangles.y += -300.0 + flip2angle;
			bSendPacket = true;
		}
		if (flip)
		{
			pCmd->viewangles.y += 290.0 + flip2angle;
			bSendPacket = false;
		}
		else
		{
			pCmd->viewangles.y += -290.0 + flip2angle;
			bSendPacket = true;
		}
		if (flip)
		{
			pCmd->viewangles.y += 280.0 + flip2angle;
			bSendPacket = false;
		}
		else
		{
			pCmd->viewangles.y += -280.0 + flip2angle;
			bSendPacket = true;
		}
		if (flip)
		{
			pCmd->viewangles.y += 270.0 + flip2angle;
			bSendPacket = false;
		}
		else
		{
			pCmd->viewangles.y += -270.0 + flip2angle;
			bSendPacket = true;
		}
		StoredYaw = pLocal->GetLowerBodyYaw();
		flip = !flip;
	}


	void fakestatic2(CUserCmd *pCmd, bool &bSendPacket)
	{
		static bool f_flip = true;
		f_flip = !f_flip;

		if (f_flip)
		{
			bSendPacket = false;
			pCmd->viewangles.y += 50.000000f;
		}
		else
		{
			bSendPacket = false;
			pCmd->viewangles.y -= 70.000000f;
		}

		if (!f_flip)
		{
			bSendPacket = true;
			pCmd->viewangles.y += 212.000000f;;
		}
		else if (!f_flip)
		{
			bSendPacket = true;
			pCmd->viewangles.y -= 180.000000f;
		}
	}

	void FakeJitter3(CUserCmd *pCmd, bool &bSendPacket)
	{
		static int jitterangle = 0;

		if (jitterangle <= 1)
		{
			pCmd->viewangles.y = 90;
		}
		else if (jitterangle > 1 && jitterangle <= 3)
		{
			pCmd->viewangles.y = -90;
		}

		static int iChoked = -1;
		iChoked++;
		if (iChoked < Menu::Window.HvHTab.FakeLagChoke.GetValue())
		{
			bSendPacket = false;
			if (jitterangle <= 1)
			{
				pCmd->viewangles.y += 45;
				jitterangle += 1;
			}
			else if (jitterangle > 1 && jitterangle <= 3)
			{
				pCmd->viewangles.y -= 45;
				jitterangle += 1;
			}
			else
			{
				jitterangle = 0;
			}
		}
		else
		{
			bSendPacket = true;
			iChoked = -1;
		}
	}
	void lagturnedup(CUserCmd *pCmd, bool &bSendPacket)
	{
		static int ChokedPackets = -1;
		ChokedPackets++;
		if (ChokedPackets < Menu::Window.HvHTab.FakeLagChoke.GetValue())
		{
			bSendPacket = false;
			pCmd->viewangles.y += 60;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.y -= 60;
			ChokedPackets = -1;
		}
	}
	void fakesideback(CUserCmd *pCmd, bool &bSendPacket)
	{
		static int ChokedPackets = -1;
		ChokedPackets++;
		if (ChokedPackets < Menu::Window.HvHTab.FakeLagChoke.GetValue())
		{
			bSendPacket = false;
			pCmd->viewangles.y -= 180;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.y -= 60;
			ChokedPackets = -1;
		}
	}
	void StaticWithFlag(CUserCmd *pCmd, bool &bSendPacket)
	{
		static int ChokedPackets = -1;
		ChokedPackets++;
		if (ChokedPackets < Menu::Window.HvHTab.FakeLagChoke.GetValue())
		{
			bSendPacket = false;
			pCmd->viewangles.y += 90;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.y -= 90;
			ChokedPackets = -1;
		}
	}

	void phase(CUserCmd *pCmd, bool &bSendPacket)
	{
		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 1.4000;
		bool flip = !flip;
		int value = ServerTime % 2;
		int iFlags = hackManager.pLocal()->GetFlags();
		int curYaw = pCmd->viewangles.y;
		static bool Turbo = false;

		static bool isMoving;
		float PlayerIsMoving = abs(hackManager.pLocal()->GetVelocity().Length());
		if (PlayerIsMoving > 0.1) isMoving = true;
		else if (PlayerIsMoving <= 0.1) isMoving = false;

		if (isMoving)
		{
			pCmd->viewangles.y = 180;
		}
		else {

			switch (value) {
			case 1:
				bSendPacket = true;
				if (Turbo)
				{
					pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() / 2 + 135;
					Turbo = !Turbo;
				}
				else
				{
					pCmd->viewangles.y += hackManager.pLocal()->GetLowerBodyYaw() / 2 + 135;
					Turbo = !Turbo;
				}
				if (curYaw > 5) {
					curYaw = 180;
				}
				break;
			case 2:
			{
				bSendPacket = false;
				if (curYaw != 180)
				{
					curYaw = 180;
				}
			}
			break;
			}
		}
	}

	void StaticPitch(CUserCmd *pCmd)
	{
		// Up
		pCmd->viewangles.x = 89.f;
	}

	void Fakelag(CUserCmd *pCmd, bool &bSendPacket)
	{
		int iChoke = Menu::Window.HvHTab.FakeLagChoke.GetValue();

		static int iFakeLag = -1;
		iFakeLag++;

		if (iFakeLag <= iChoke && iFakeLag > -1)
		{
			bSendPacket = false;
		}
		else
		{
			bSendPacket = true;
			iFakeLag = -1;
		}

	}



	void Emotion2(CUserCmd *pCmd, bool& bSendPacket) {
		pCmd->viewangles.x = -89.f;
		if (bSendPacket)
			pCmd->viewangles.x = 89.f;
	}
	void clickbait(CUserCmd *pCmd)
	{
		{
			int random = rand() % 100;
			int random2 = rand() % 1000;

			static bool dir;
			static float current_x = pCmd->viewangles.x;

			if (random == 1) dir = !dir;

			if (dir)
				current_x += 700;
			else
				current_x -= 34;

			pCmd->viewangles.x = current_x;

			if (random == random2)
				pCmd->viewangles.x += random;

		}

	}

	// Yaws
	void FastSpint(CUserCmd *pCmd)
	{
		int random = rand() % 100;
		int random2 = rand() % 1000;

		static bool dir;
		static float current_y = pCmd->viewangles.y;

		if (random == 1) dir = !dir;

		if (dir)
			current_y += 100;
		else
			current_y -= 100;

		pCmd->viewangles.y = current_y;

		if (random == random2)
			pCmd->viewangles.y += random;
	}

	void SlowSpin(CUserCmd *pCmd)
	{
		int random = rand() % 100;
		int random2 = rand() % 1000;

		static bool dir;
		static float current_y = pCmd->viewangles.y;

		if (random == 1) dir = !dir;

		if (dir)
			current_y += 10;
		else
			current_y -= 10;

		pCmd->viewangles.y = current_y;

		if (random == random2)
			pCmd->viewangles.y += random;

	}





}

void CRageBot::aimAtPlayer(CUserCmd *pCmd)
{
	IClientEntity* pLocal = hackManager.pLocal();

	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (!pLocal || !pWeapon)
		return;

	Vector eye_position = pLocal->GetEyePosition();

	float best_dist = pWeapon->GetCSWpnData()->m_flRange;

	IClientEntity* target = nullptr;

	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			if (Globals::TargetID != -1)
				target = Interfaces::EntList->GetClientEntity(Globals::TargetID);
			else
				target = pEntity;

			Vector target_position = target->GetEyePosition();

			float temp_dist = eye_position.DistTo(target_position);

			if (best_dist > temp_dist)
			{
				best_dist = temp_dist;
				CalcAngle(eye_position, target_position, pCmd->viewangles);
			}
		}
	}
}

void VectorAngles2(const Vector &vecForward, Vector &vecAngles)
{
	Vector vecView;
	if (vecForward[1] == 0.f && vecForward[0] == 0.f)
	{
		vecView[0] = 0.f;
		vecView[1] = 0.f;
	}
	else
	{
		vecView[1] = atan2(vecForward[1], vecForward[0]) * 180.f / M_PI;

		if (vecView[1] < 0.f)
			vecView[1] += 360.f;

		vecView[2] = sqrt(vecForward[0] * vecForward[0] + vecForward[1] * vecForward[1]);

		vecView[0] = atan2(vecForward[2], vecView[2]) * 180.f / M_PI;
	}

	vecAngles[0] = -vecView[0];
	vecAngles[1] = vecView[1];
	vecAngles[2] = 0.f;
}
void AngleVectors2(const Vector& qAngles, Vector& vecForward)
{
	float sp, sy, cp, cy;
	SinCos((float)(qAngles[1] * (M_PI / 180.f)), &sy, &cy);
	SinCos((float)(qAngles[0] * (M_PI / 180.f)), &sp, &cp);

	vecForward[0] = cp*cy;
	vecForward[1] = cp*sy;
	vecForward[2] = -sp;
}

namespace AntiAims // CanOpenFire checks for fake anti aims?
{


	/*
	________________________________________________________
	______             ______
	/      |           /      \
	$$$$$$/  _______  /$$$$$$  |______    _______
	$$ |  /       \ $$ |_ $$//      \  /       |
	$$ |  $$$$$$$  |$$   |  /$$$$$$  |/$$$$$$$/
	$$ |  $$ |  $$ |$$$$/   $$ |  $$ |$$      \
	_$$ |_ $$ |  $$ |$$ |    $$ \__$$ | $$$$$$  |
	/ $$   |$$ |  $$ |$$ |    $$    $$/ /     $$/
	$$$$$$/ $$/   $$/ $$/      $$$$$$/  $$$$$$$/
	________________________________________________________
	*/
	/*
	- 4 Boxen namen siehe unten
	- bSendPacket = true  <-- Fake Angle
	- bSendPacket = false <-- True Angle
	*/

	void Emotion(CUserCmd *pCmd)
	{
		pCmd->viewangles.x = 89.000000f;
	}

	void Down(CUserCmd *pCmd)
	{
		pCmd->viewangles.x = 179.000000f;
	}

	void Up(CUserCmd *pCmd)
	{
		pCmd->viewangles.x = -179.000000f;
	}

	void FakeDown1(CUserCmd *pCmd, bool &bSendPacket)
	{
		static bool flip = false;
		if (flip)
		{
			pCmd->viewangles.x = -179.000000f;
			bSendPacket = false;
		}
		else
		{
			pCmd->viewangles.x = 179.000000f;
			bSendPacket = true;
		}
	}

	void FakeUp1(CUserCmd *pCmd, bool &bSendPacket)
	{
		pCmd->viewangles.x = 89.000000f;
	}

	void Zero(CUserCmd *pCmd)
	{
		pCmd->viewangles.x = 0.f;
	}

	void FakeZero(CUserCmd *pCmd, bool &bSendPacket)
	{
		static int ChokedPackets = -1;
		ChokedPackets++;
		if (ChokedPackets < Menu::Window.HvHTab.FakeLagChoke.GetValue())
		{
			bSendPacket = false;
			pCmd->viewangles.x = -89;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.x = 0.f;
			ChokedPackets = -1;
		}
	}

	void Mixed(CUserCmd *pCmd)
	{
		static float pDance = 0.0f;

		pDance += 45.0f;
		if (pDance > 100)
			pDance = 0.0f;
		else if (pDance > 75.f)
			pCmd->viewangles.x = -11.295f;
		else if (pDance < 45.f)
			pCmd->viewangles.x = 26.3491651f;
		else if (pDance < 65)
			pCmd->viewangles.x = -9.91545f;
		else if (pDance < 75.f)
			pCmd->viewangles.x = 67.945324f;
		else if (pDance < 89.f)
			pCmd->viewangles.x = -72.62294519f;
		else if (pDance < 85.f)
			pCmd->viewangles.x = 35.19245635f;
	}

	void LegitFake(CUserCmd *pCmd, bool &bSendPacket)
	{
		static bool choke = false;
		if (choke)
		{
			bSendPacket = false; //true angle
			pCmd->viewangles.x = -90.f;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.x = 0.f; //fake angle
		}
		choke = !choke;
	}

	void LegitFakeJitter(CUserCmd *pCmd, bool &bSendPacket)
	{
		static bool choke = false;
		static bool s = false;
		if (choke)
		{
			bSendPacket = false; //true angle
			if (s) {
				pCmd->viewangles.x = 90.f;
			}
			else {
				pCmd->viewangles.x = -90.f;
			}
			s = !s;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.x = 0.f; //fake angle
		}
		choke = !choke;
	}

	// YAW

	#define RandomInt(min, max) (rand() % (max - min + 1) + min)

	void Backwards(CUserCmd *pCmd)
	{
		pCmd->viewangles.y -= 180.f;
	}

	void Static(CUserCmd *pCmd)
	{
		static bool flip = false;
		static bool flip2 = false;
		flip + !flip;
		flip2 = !flip2;
		{
			if (flip)
			{

				if (flip2)
					pCmd->viewangles.y += 90.000000f;

				else
					pCmd->viewangles.y -= 90.000000f;
			}
			else
			{
				pCmd->viewangles.y += 180.000000f;
			}
		}

	}

	void Static2(CUserCmd *pCmd)
	{
		static bool flip = false;
		static bool flip2 = false;
		flip + !flip;
		flip2 = !flip2;
		{
			if (flip)
			{
				if (flip2)
				{
					pCmd->viewangles.y += 90.000000f;
				}
				else
					pCmd->viewangles.y -= 90.000000f;
			}
			else
			{
				if (flip2)
					pCmd->viewangles.y -= 162.000000f;
				else if (!flip2)
					pCmd->viewangles.y -= 180.000000f;
			}
		}
	}

	void Static3(CUserCmd *pCmd)
	{
		static bool flip = false;
		static bool flip2 = false;
		flip + !flip;
		flip2 = !flip2;
		{
			if (flip)
			{
				if (flip2)
				{
					pCmd->viewangles.y += 90.000000f;
				}
				else
					pCmd->viewangles.y -= 90.000000f;
			}
			else
			{
				if (flip2)
					pCmd->viewangles.y += 90.000000f;
				else if (!flip2)
					pCmd->viewangles.y -= 180.000000f;
			}
		}
	}


	void Twitch1(CUserCmd *pCmd)
	{
			int random = rand() % 100;

			if (random < 98)
				pCmd->viewangles.y -= 180;

			if (random < 15)
			{
				float change = -70 + (rand() % (int)(140 + 1));
				pCmd->viewangles.y += change;
			}
			if (random == 69)
			{
				float change = -90 + (rand() % (int)(180 + 1));
				pCmd->viewangles.y += change;
			}
	}
	

	void ZeroYaw(CUserCmd *pCmd)
	{
		pCmd->viewangles.y = 0.f;
	}



	void LBYBreaker(CUserCmd *pCmd)
	{
		static bool wilupdate;
		static float LastLBYUpdateTime = 0;
		IClientEntity* pLocal = hackManager.pLocal();
		float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
		if (server_time >= LastLBYUpdateTime)
		{
			LastLBYUpdateTime = server_time + 1.125f;
			wilupdate = true;
			pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() + RandomInt(30, 61);
		}
		else
		{
			wilupdate = false;
			pCmd->viewangles.y += hackManager.pLocal()->GetLowerBodyYaw() - RandomInt(180, 360);
		}

	}

	void Lowerbody(CUserCmd *pCmd)
	{
		static bool wilupdate;
		static float LastLBYUpdateTime = 0;
		IClientEntity* pLocal = hackManager.pLocal();
		float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
		if (server_time >= LastLBYUpdateTime)
		{
			LastLBYUpdateTime = server_time + 1.125f;
			wilupdate = true;
			pCmd->viewangles.y -= 90.f;
		}
		else
		{
			wilupdate = false;
			pCmd->viewangles.y += 90.f;
		}
	}

	// Fake Yaw

	void FakeBackwards(CUserCmd *pCmd, bool &bSendPacket)
	{
		static bool choke = false;
		static bool s = false;
		if (choke)
		{
			bSendPacket = false; //true angle
			if (s) {
				pCmd->viewangles.y += 90.f;
			}
			else {
				pCmd->viewangles.y -= 90.f;
			}
			s = !s;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.y += 180.f; //fake angle
		}
		choke = !choke;
	}



	void FakeLowerbody(CUserCmd *pCmd, bool &bSendPacket)
	{
		bSendPacket = true;
		static bool wilupdate;
		static float LastLBYUpdateTime = 0;
		IClientEntity* pLocal = hackManager.pLocal();
		float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
		if (server_time >= LastLBYUpdateTime)
		{
			LastLBYUpdateTime = server_time + 1.125f;
			wilupdate = true;
			pCmd->viewangles.y -= 90.f;
		}
		else
		{
			wilupdate = false;
			pCmd->viewangles.y += 90.f;
		}
	}

	void LBYBreakerFake(CUserCmd *pCmd, bool &bSendPacket)
	{
		bSendPacket = true;
		static bool wilupdate;
		static float LastLBYUpdateTime = 0;
		IClientEntity* pLocal = hackManager.pLocal();
		float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
		if (server_time >= LastLBYUpdateTime)
		{
			LastLBYUpdateTime = server_time + 1.125f;
			wilupdate = true;
			pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() + RandomInt(30, 61);
		}
		else
		{
			wilupdate = false;
			pCmd->viewangles.y += hackManager.pLocal()->GetLowerBodyYaw() - RandomInt(180, 360);
		}

	}
};

bool EdgeAntiAim(IClientEntity* pLocalBaseEntity, CUserCmd* cmd, float flWall, float flCornor)
{
	Ray_t ray;
	trace_t tr;

	CTraceFilter traceFilter;
	traceFilter.pSkip = pLocalBaseEntity;

	auto bRetVal = false;
	auto vecCurPos = pLocalBaseEntity->GetEyePosition();

	for (float i = 0; i < 360; i++)
	{
		Vector vecDummy(10.f, cmd->viewangles.y, 0.f);
		vecDummy.y += i;

		NormalizeVector(vecDummy);

		Vector vecForward;
		AngleVectors2(vecDummy, vecForward);

		auto flLength = ((16.f + 3.f) + ((16.f + 3.f) * sin(DEG2RAD(10.f)))) + 7.f;
		vecForward *= flLength;

		ray.Init(vecCurPos, (vecCurPos + vecForward));
		Interfaces::Trace->TraceRay(ray, MASK_SHOT, (CTraceFilter *)&traceFilter, &tr);

		if (tr.fraction != 1.0f)
		{
			Vector qAngles;
			auto vecNegate = tr.plane.normal;

			vecNegate *= -1.f;
			VectorAngles2(vecNegate, qAngles);

			vecDummy.y = qAngles.y;

			NormalizeVector(vecDummy);
			trace_t leftTrace, rightTrace;

			Vector vecLeft;
			AngleVectors2(vecDummy + Vector(0.f, 30.f, 0.f), vecLeft);

			Vector vecRight;
			AngleVectors2(vecDummy - Vector(0.f, 30.f, 0.f), vecRight);

			vecLeft *= (flLength + (flLength * sin(DEG2RAD(30.f))));
			vecRight *= (flLength + (flLength * sin(DEG2RAD(30.f))));

			ray.Init(vecCurPos, (vecCurPos + vecLeft));
			Interfaces::Trace->TraceRay(ray, MASK_SHOT, (CTraceFilter*)&traceFilter, &leftTrace);

			ray.Init(vecCurPos, (vecCurPos + vecRight));
			Interfaces::Trace->TraceRay(ray, MASK_SHOT, (CTraceFilter*)&traceFilter, &rightTrace);

			if ((leftTrace.fraction == 1.f) && (rightTrace.fraction != 1.f))
				vecDummy.y -= flCornor; // left
			else if ((leftTrace.fraction != 1.f) && (rightTrace.fraction == 1.f))
				vecDummy.y += flCornor; // right			

			cmd->viewangles.y = vecDummy.y;
			cmd->viewangles.y -= flWall;
			cmd->viewangles.x = 89.f;
			bRetVal = true;
		}
	}
	return bRetVal;
}

// AntiAim
void CRageBot::DoAntiAim(CUserCmd *pCmd, bool &bSendPacket) // pCmd->viewangles.y = 0xFFFFF INT_MAX or idk
{
	IClientEntity* pLocal = hackManager.pLocal();
	IClientEntity *pEntity;
	IClientEntity* pLocalEntity;

	if (Menu::Window.HvHTab.AntiAimEnable.GetState())
	{
		AntiAims::Fakelag(pCmd, bSendPacket);
	}

	if ((pCmd->buttons & IN_USE) || pLocal->GetMoveType() == MOVETYPE_LADDER)
		return;
	
	// If the aimbot is doing something don't do anything
	if ((IsAimStepping || pCmd->buttons & IN_ATTACK) && !Menu::Window.RageBotTab.AimbotSlientSelection.GetIndex())
		return;

	// Weapon shit
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
	if (pWeapon)
	{
		CSWeaponInfo* pWeaponInfo = pWeapon->GetCSWpnData();
		// Knives or grenades
		if (!GameUtils::IsBallisticWeapon(pWeapon))
		{
			if (Menu::Window.HvHTab.AntiAimKnife.GetState())
			{
				if (!CanOpenFire() || pCmd->buttons & IN_ATTACK2)
					return;
			}
			else
			{
				return;
			}
		}
	}

	if (Menu::Window.HvHTab.AntiAimEdge.GetState()) {
		auto bEdge = EdgeAntiAim(hackManager.pLocal(), pCmd, 360.f, 45.f);
		if (bEdge)
			return;
	}

	if (Menu::Window.HvHTab.AntiAimTarget.GetState())
	{
		aimAtPlayer(pCmd);
	}

	// Anti-Aim Pitch
	switch (Menu::Window.HvHTab.AntiAimPitch.GetIndex()) // Magic pitch is 69.69?
	{
	case 0:
		// No Pitch AA
		break;
	case 1:
		// Down
		//AntiAims::StaticPitch(pCmd, false);
		pCmd->viewangles.x = 89.0f;
		break;
	case 2:
		// Half Down
		pCmd->viewangles.x = 51.f;
		break;
	case 3:
		// SMAC / Casual safe
		AntiAims::JitterPitch(pCmd);
		break;
	case 4:
		// Jitter
		pCmd->viewangles.x = 179.0f;
		break;
	case 5:
		// Fake Pitch
		pCmd->viewangles.x = 180.247f;
		break;
	case 6:
		// Static Down
		pCmd->viewangles.x = 1800089.0f;
		break;
	case 7:
		// Static Jitter
		pCmd->viewangles.x = -1800089.0f;
		break;
	case 8:
		// best nospread pitch
		pCmd->viewangles.x = 173.0f;
		break;
	case 9:
		// best nospread pitch
		pCmd->viewangles.x = 75.0f;
		break;
	case 10:
		// overdown
		pCmd->viewangles.x = 36000089.f;
		break;
	case 11:
		// best nospread pitch
		pCmd->viewangles.x = 35999911.f;
		break;
	case 12:
		// best nospread pitch
		pCmd->viewangles.x = 0.00000f;
		break;
	case 13:
		// best nospread pitch
		AntiAims::StaticJitter(pCmd);
		break;
	}

	//Anti-Aim Yaw
	switch (Menu::Window.HvHTab.AntiAimYaw.GetIndex())
	{
	case 0:
		// No Yaw AA
		break;
	case 1:
		// Fake Inverse
		AntiAims::StaticWithFlag(pCmd, bSendPacket);
		break;
	case 2:
		// Fake Sideways
		AntiAims::FakeSideways(pCmd, bSendPacket);
		break;
	case 3:
		// Fake Static
		AntiAims::FakeStatic(pCmd, bSendPacket);
		break;
	case 4:
		// Fake Inverse
		AntiAims::TFake(pCmd, bSendPacket);
		break;
	case 5:
		// Fake Jitter
		AntiAims::FakeJitter(pCmd, bSendPacket);
		break;
	case 6:
		// Jitter
		AntiAims::Static(pCmd);
		break;
	case 7:
		// T Jitter
		AntiAims::TJitter(pCmd);
		break;
	case 8:
		// Back Jitter
		AntiAims::BackJitter(pCmd);
		break;
	case 9:
		// T Inverse
		pCmd->viewangles.y -= 180;
		break;
	case 10:
		// T Inverse
		AntiAims::fakelowerbody(pCmd, bSendPacket);
		break;
	case 11:

		AntiAims::FakeJitter3(pCmd, bSendPacket);
		break;
	case 12:

		AntiAims::fakestatic2(pCmd, bSendPacket);
		break;
	case 13:

		AntiAims::YAW_fakeside_backj(pCmd, bSendPacket);
		break;
	case 14:

		AntiAims::YAW_FSTATIC3_rmake(pCmd, bSendPacket);
		break;
	case 15:

		AntiAims::YAW_FSTATIC_Remake(pCmd, bSendPacket);
		break;
	case 16:

		AntiAims::LBYBreaker(pCmd, bSendPacket);
		break;
	case 17:

		AntiAims::LBY2(pCmd, bSendPacket);
		break;
	case 18:

		AntiAims::Arizona1(pCmd, bSendPacket);
		break;
	case 19:

		AntiAims::jewbag(pCmd, bSendPacket);
		break;
	case 20:

		AntiAims::sidestrash(pCmd, bSendPacket);
		break;
	}

	switch (Menu::Window.HvHTab.AntiAimSpinYaw.GetIndex())
	{
	case 0:
		// No Yaw AA
		break;
	case 1:
	{
		AntiAims::FastSpin(pCmd);
		break;
	}
	case 2:
		// Fake Inverse
		AntiAims::CSpin(pCmd, bSendPacket);
		break;
	case 3:
	{
		AntiAims::halfSpin(pCmd, bSendPacket);
		break;
	}
	case 4:
	{
		AntiAims::FakeForward(pCmd, bSendPacket);
		break;
	}
	case 5:
	{
		AntiAims::FakeBackwards(pCmd, bSendPacket);
		break;
	}
	case 6:
	{
		AntiAims::fakespin(pCmd, bSendPacket);
		break;
	}

	case 7:
	{
		AntiAims::kiduaJitter(pCmd, bSendPacket);
		break;
	}

	case 8:
	{
		AntiAims::JitterBackward1(pCmd, bSendPacket);
		break;
	}

	case 9:
	{
		AntiAims::Sinster1(pCmd, bSendPacket);
		break;
	}
	case 10:
	{
		AntiAims::opaf(pCmd, bSendPacket);
		break;
	}
	case 11:
	{
		AntiAims::phase(pCmd, bSendPacket);
		break;
	}
	case 12:
	{
		AntiAims::tolong(pCmd, bSendPacket);
		break;
	}
	case 13:
	{
		AntiAims::FakeHead(pCmd);
		break;
	}
	case 14:
	{
		AntiAims::FakeTwoStep(pCmd, bSendPacket);
		break;
	}
	}

	if (Menu::Window.HvHTab.AntiAimJitterLBY.GetState())
	{
		static bool antiResolverFlip = false;
		if (pCmd->viewangles.y == pLocal->GetLowerBodyYaw())
		{
			if (antiResolverFlip)
				pCmd->viewangles.y += 60.f;
			else
				pCmd->viewangles.y -= 60.f;

			antiResolverFlip = !antiResolverFlip;
		}
	}
}