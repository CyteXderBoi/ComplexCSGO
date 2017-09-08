#include "Resolver.h"

int Globals::Shots = 0;
bool Globals::change;
bool Globals::LBYUpdate;
CUserCmd* Globals::UserCmd;
int Globals::TargetID;
IClientEntity* Globals::Target;
float YawDelta[64];
float reset[64];
float Delta[64];
float OldLowerBodyYaw[64];
float Resolved_angles[64];
int iSmart;
static int jitter = -1;

void NormalizeVector1337(float& lowerDelta) {
	for (int i = 0; i < 3; ++i) {
		while (lowerDelta > 180.f)
			lowerDelta -= 360.f;

		while (lowerDelta < -180.f)
			lowerDelta += 360.f;
	}
	lowerDelta = 0.f;
}


int GetEstimatedServerTickCount(float latency)
{
	return (int)floorf((float)((float)(latency) / (float)((uintptr_t)&Interfaces::Globals->interval_per_tick)) + 0.5) + 1 + (int)((uintptr_t)&Interfaces::Globals->tickcount);
}

void R::Resolve()
{
	IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	for (int i = 0; i < Interfaces::Engine->GetMaxClients(); ++i)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);

		if (!pEntity || pEntity->IsDormant() || !pEntity->IsAlive())
			continue;

		if (pEntity->GetTeamNum() == pLocal->GetTeamNum() || !pLocal->IsAlive())
			continue;

		Vector* eyeAngles = pEntity->GetEyeAnglesPointer();

		float oldlowerbodyyaw;
		bool bLowerBodyUpdated = false;
		bool IsUsingFakeAngles = false;
		static bool isMoving;
		float PlayerIsMoving = abs(pEntity->GetVelocity().Length());
		if (PlayerIsMoving > 0.1) isMoving = true;
		else if (PlayerIsMoving <= 0.1) isMoving = false;
		float TLBY = pEntity->GetLowerBodyYaw();
		float backtrack;

		if (Menu::Window.HvHTab.AccuracyResolverYaw.GetState() && Menu::Window.MiscTab.OtherSafeMode.GetState())
		{
			bool bLowerBodyUpdated = false;
			bool IsUsingFakeAngles = false;
			float oldlowerbodyyaw;
			if (oldlowerbodyyaw != TLBY)
			{
				bLowerBodyUpdated = true;
			}
			float bodyeyedelta = pEntity->GetEyeAngles().y - eyeAngles->y;
			if (PlayerIsMoving || bLowerBodyUpdated)// || LastUpdatedNetVars->eyeangles.x != CurrentNetVars->eyeangles.x || LastUpdatedNetVars->eyeyaw != CurrentNetVars->eyeangles.y)
			{
				if (bLowerBodyUpdated || (PlayerIsMoving && bodyeyedelta >= 35.0f))
				{
					eyeAngles->y = TLBY;
					oldlowerbodyyaw = eyeAngles->y;
				}

				IsUsingFakeAngles = false;
			}
			else
			{
				if (bodyeyedelta > 35.0f)
				{
					eyeAngles->y = oldlowerbodyyaw;
					IsUsingFakeAngles = true;
				}
				else
				{
					IsUsingFakeAngles = false;
				}
			}
			if (IsUsingFakeAngles)
			{
				int com = GetEstimatedServerTickCount(90);

				if (com % 2)
				{
					eyeAngles->y += 90;
				}
				else if (com % 3)
					eyeAngles->y -= 90;
				else
					eyeAngles->y -= 0;
			}
		}

		if (Menu::Window.HvHTab.AccuracyResolverYaw.GetState() && !Menu::Window.MiscTab.OtherSafeMode.GetState())
		{
			float spin;
			spin++;
			eyeAngles->y = spin;
		}

		if (Menu::Window.HvHTab.AccuracyResolverYaw.GetState() && !Menu::Window.MiscTab.OtherSafeMode.GetState()) //pitch
		{
			std::string strPitch = std::to_string(eyeAngles->x);

			if (eyeAngles->x < -179.f) eyeAngles->x += 360.f;
			else if (eyeAngles->x > 90.0 || eyeAngles->x < -90.0) eyeAngles->x = 89.f;
			else if (eyeAngles->x > 89.0 && eyeAngles->x < 91.0) eyeAngles->x -= 90.f;
			else if (eyeAngles->x > 179.0 && eyeAngles->x < 181.0) eyeAngles->x -= 180;
			else if (eyeAngles->x > -179.0 && eyeAngles->x < -181.0) eyeAngles->x += 180;
			else if (fabs(eyeAngles->x) == 0) eyeAngles->x = std::copysign(89.0f, eyeAngles->x);
		}
	}
}