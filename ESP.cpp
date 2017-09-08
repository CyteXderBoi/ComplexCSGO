/*
Syn's AyyWare Framework 2015
*/

#include "ESP.h"
#include "Interfaces.h"
#include "RenderManager.h"
#include "MiscClasses.h"
#include "AntiAntiAim.h"


void CEsp::Init()
{
	BombCarrier = nullptr;
}

// Yeah dude we're defo gunna do some sick moves for the esp yeah
void CEsp::Move(CUserCmd *pCmd,bool &bSendPacket) 
{

}

// Main ESP Drawing loop
void CEsp::Draw()
{
	IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	{



		if (Menu::Window.MiscTab.OtherSpectators.GetState())
		{
			SpecList();
		}

		// Loop through all active entitys
		for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
		{
			// Get the entity
			IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
			player_info_t pinfo;

			// The entity isn't some laggy peice of shit or something
			if (pEntity &&  pEntity != pLocal && !pEntity->IsDormant())
			{
				// Radar
				if (Menu::Window.VisualsTab.OtherRadar.GetState())
				{
					DWORD m_bSpotted = NetVar.GetNetVar(0x839EB159);
					*(char*)((DWORD)(pEntity)+m_bSpotted) = 1;
				}

				// Is it a player?!
				if (Menu::Window.VisualsTab.FiltersPlayers.GetState() && Interfaces::Engine->GetPlayerInfo(i, &pinfo) && pEntity->IsAlive())
				{
					DrawPlayer(pEntity, pinfo);
				}



				// ~ Other ESP's here (items and shit) ~ //
				ClientClass* cClass = (ClientClass*)pEntity->GetClientClass();

				// Dropped weapons
				if (Menu::Window.VisualsTab.FiltersWeapons.GetState() && cClass->m_ClassID != (int)CSGOClassID::CBaseWeaponWorldModel && ((strstr(cClass->m_pNetworkName, "Weapon") || cClass->m_ClassID == (int)CSGOClassID::CDEagle || cClass->m_ClassID == (int)CSGOClassID::CAK47)))
				{
					DrawDrop(pEntity, cClass);
				}

				// If entity is the bomb
				if (Menu::Window.VisualsTab.FiltersC4.GetState())
				{
					if (cClass->m_ClassID == (int)CSGOClassID::CPlantedC4)
						DrawBombPlanted(pEntity, cClass);

					if (cClass->m_ClassID == (int)CSGOClassID::CC4)
						DrawBomb(pEntity, cClass);
				}

				// If entity is a chicken
				if (Menu::Window.VisualsTab.FiltersChickens.GetState())
				{
					if (cClass->m_ClassID == (int)CSGOClassID::CChicken)
						DrawChicken(pEntity, cClass);
				}
			}
		}

		// Anti Flash
		// pAntiFlash
		if (Menu::Window.VisualsTab.OtherNoFlash.GetState())
		{
			DWORD m_flFlashMaxAlpha = NetVar.GetNetVar(0xFE79FB98);
			*(float*)((DWORD)pLocal + m_flFlashMaxAlpha) = 0;
		}
		Vector* eyeAngles = pLocal->GetEyeAnglesPointer();

		// LBY indicator
		if (Menu::Window.MiscTab.OtherLBY.GetState() & pLocal->IsAlive())
		{
			if (pLocal->GetVelocity().Length() >= 1 && pLocal->GetFlags() & FL_ONGROUND)
			{
				Render::Text(10, 1000, Color(255, 0, 0, 255) , Render::Fonts::LBY, "LBY");
			}
			else
			{
				Render::Text(10, 1000, Color(0, 255, 0, 255), Render::Fonts::LBY, "LBY");
			}
		}
	}
}

// Spectator List
void CEsp::SpecList()
{
	IClientEntity *pLocal = hackManager.pLocal();

	RECT scrn = Render::GetViewport();
	int ayy = 0;

	// Loop through all active entitys
	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		// Get the entity
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		player_info_t pinfo;

		// The entity isn't some laggy peice of shit or something
		if (pEntity &&  pEntity != pLocal)
		{
			if (Interfaces::Engine->GetPlayerInfo(i, &pinfo) && !pEntity->IsAlive() && !pEntity->IsDormant())
			{
				HANDLE obs = pEntity->GetObserverTargetHandle();

				if (obs)
				{
					IClientEntity *pTarget = Interfaces::EntList->GetClientEntityFromHandle(obs);
					player_info_t pinfo2;
					if (pTarget)
					{
						if (Interfaces::Engine->GetPlayerInfo(pTarget->GetIndex(), &pinfo2))
						{
							char buf[255]; sprintf_s(buf, "%s => %s", pinfo.name, pinfo2.name);
							RECT TextSize = Render::GetTextSize(Render::Fonts::ESP, buf);
							Render::Clear(scrn.right - 260, (scrn.bottom / 2) + (16 * ayy), 260, 16, Color(0, 0, 0, 140));
							Render::Text(scrn.right - TextSize.right - 4, (scrn.bottom / 2) + (16 * ayy), pTarget->GetIndex() == pLocal->GetIndex() ? Color(240, 70, 80, 255) : Color(255, 255, 255, 255), Render::Fonts::ESP, buf);
							ayy++;
						}
					}
				}
			}
		}
	}

	Render::Outline(scrn.right - 261, (scrn.bottom / 2) - 1, 262, (16 * ayy) + 2, Color(23, 23, 23, 255));
	Render::Outline(scrn.right - 260, (scrn.bottom / 2), 260, (16 * ayy), Color(90, 90, 90, 255));
}

//  Yeah m8
void CEsp::DrawPlayer(IClientEntity* pEntity, player_info_t pinfo)
{
	ESPBox Box;
	Color Color;

	// Show own team false? well gtfo teammate lol
	if (Menu::Window.VisualsTab.FiltersEnemiesOnly.GetState() && (pEntity->GetTeamNum() == hackManager.pLocal()->GetTeamNum()))
		return;

	if (GetBox(pEntity, Box))
	{
		Color = GetPlayerColor(pEntity);

		/*if (Menu::Window.VisualsTab.OptionsGlow.GetState())
		{
			int TeamNum = pEntity->GetTeamNum();

			if (TeamNum == TEAM_CS_T)
			{
				DrawGlow(pEntity, 255, 0, 0, 160);
			}
			else if (TeamNum == TEAM_CS_CT)
			{
				DrawGlow(pEntity, 0, 0, 255, 160);
			}
		}*/

		if (Menu::Window.VisualsTab.OptionsBox.GetState())
			DrawBox(Box, Color);

		if (Menu::Window.VisualsTab.OptionsName.GetState())
			DrawName(pinfo, Box);

		if (Menu::Window.VisualsTab.OptionHealthEnable.GetState())
			DrawHealth(pEntity, Box);

		if (Menu::Window.VisualsTab.OptionsInfo.GetState() || Menu::Window.VisualsTab.OptionsWeapon.GetState())
			DrawInfo(pEntity, Box);

		if (Menu::Window.VisualsTab.OptionsAimSpot.GetState())
			DrawCross(pEntity);

		if (Menu::Window.VisualsTab.OptionsSkeleton.GetState())
			DrawSkeleton(pEntity);

		if (Menu::Window.VisualsTab.OptionsArmur.GetState() == 1 && pEntity->ArmorValue() >= 1)
			DrawArmor(pEntity, Box);	

		if (Menu::Window.VisualsTab.OptionsFill.GetState())
			Fill(Box, Color);

		if (Menu::Window.VisualsTab.LBY.GetState())
			lbyup(pEntity, Box);

		if (Menu::Window.VisualsTab.SnapLines.GetState())
		{
			BulletTrace(pEntity, Color);
		}
		
	}
}

/* // glow shit
struct Glowobject
{
	IClientEntity* pEntity;
	Vector Color;
	float Alpha;
	byte PAD[16];
	bool RenderWhenOccluded;
	bool RenderWhenUnOccluded;
	bool FullBloomRender;
	byte PAD2[17];
};

// simple sexy glow
void CEsp::DrawGlow(IClientEntity *pEntity, int r, int g, int b, int a)
{
	static uintptr_t Module = (uintptr_t)GetModuleHandle("client.dll");

	Glowobject* GlowManager = *(Glowobject**)(Module + 0x4B71C6C);

	if (GlowManager)
	{
		Glowobject* GlowObject = &GlowManager[pEntity->GetGlowIndex()];

		if (GlowObject)
		{
			GlowObject->RenderWhenOccluded = 1;
			GlowObject->RenderWhenUnOccluded = 0;

			float glowr = (1 / 255.0f)*r;
			float glowg = (1 / 255.0f)*g;
			float glowb = (1 / 255.0f)*b;
			float glowa = (1 / 255.0f)*a;
			GlowObject->Color = Vector((1 / 255.0f)*r, (1 / 255.0f)*g, (1 / 255.0f)*b);
			GlowObject->Alpha = (1 / 255.0f)*a;
		}
	}
} */

// Gets the 2D bounding box for the entity
// Returns false on failure nigga don't fail me
bool CEsp::GetBox(IClientEntity* pEntity, CEsp::ESPBox &result)
{
	// Variables
	Vector  vOrigin, min, max, sMin, sMax, sOrigin,
		flb, brt, blb, frt, frb, brb, blt, flt;
	float left, top, right, bottom;

	// Get the locations
	vOrigin = pEntity->GetOrigin();
	min = pEntity->collisionProperty()->GetMins() + vOrigin;
	max = pEntity->collisionProperty()->GetMaxs() + vOrigin;

	// Points of a 3d bounding box
	Vector points[] = { Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z) };

	// Get screen positions
	if (!Render::WorldToScreen(points[3], flb) || !Render::WorldToScreen(points[5], brt)
		|| !Render::WorldToScreen(points[0], blb) || !Render::WorldToScreen(points[4], frt)
		|| !Render::WorldToScreen(points[2], frb) || !Render::WorldToScreen(points[1], brb)
		|| !Render::WorldToScreen(points[6], blt) || !Render::WorldToScreen(points[7], flt))
		return false;

	// Put them in an array (maybe start them off in one later for speed?)
	Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };

	// Init this shit
	left = flb.x;
	top = flb.y;
	right = flb.x;
	bottom = flb.y;

	// Find the bounding corners for our box
	for (int i = 1; i < 8; i++)
	{
		if (left > arr[i].x)
			left = arr[i].x;
		if (bottom < arr[i].y)
			bottom = arr[i].y;
		if (right < arr[i].x)
			right = arr[i].x;
		if (top > arr[i].y)
			top = arr[i].y;
	}

	// Width / height
	result.x = left;
	result.y = top;
	result.w = right - left;
	result.h = bottom - top;

	return true;
}

// Get an entities color depending on team and vis ect
Color CEsp::GetPlayerColor(IClientEntity* pEntity)
{
	int TeamNum = pEntity->GetTeamNum();
	bool IsVis = GameUtils::IsVisible(hackManager.pLocal(), pEntity, (int)CSGOHitboxID::Head);

	int rctvisesp = Menu::Window.VisualsTab.RBoxCTV.GetValue();
	int gctvisesp = Menu::Window.VisualsTab.GBoxCTV.GetValue();
	int bctvisesp = Menu::Window.VisualsTab.BBoxCTV.GetValue();
	int actvisesp = Menu::Window.VisualsTab.ABoxCTV.GetValue();

	int rctesp = Menu::Window.VisualsTab.RBoxCT.GetValue();
	int gctesp = Menu::Window.VisualsTab.GBoxCT.GetValue();
	int bctesp = Menu::Window.VisualsTab.BBoxCT.GetValue();
	int actesp = Menu::Window.VisualsTab.ABoxCT.GetValue();

	Color color;

	if (TeamNum == TEAM_CS_T)
	{
		if (IsVis)
			color = Color(235, 200, 0, 255);
		else
			color = Color(235, 50, 0, 255);
	}
	else
	{
		if (IsVis)
			color = Color(rctvisesp, gctvisesp, bctvisesp, actvisesp);
		else
			color = Color(rctesp, gctesp, bctesp, actesp);
	}


	return color;
}

// 2D  Esp box
void CEsp::DrawBox(CEsp::ESPBox size, Color color)
{
	static float rainbow;
	rainbow += 0.005f;
	if (rainbow > 1.f) rainbow = 0.f;
	//if (PlayerBoxes->GetStringIndex() == 1)
	//{
		// Full Box
	//Render::Clear(size.x, size.y, size.w, size.h, color);
	//Render::Clear(size.x - 1, size.y - 1, size.w + 2, size.h + 2, Color(10, 10, 10, 150)); 
	//Render::Clear(size.x + 1, size.y + 1, size.w - 2, size.h - 2, Color(10, 10, 10, 150));
	//}
	//else
	if (Menu::Window.VisualsTab.BoxDesign.GetIndex() == 0)
	{
		// Corner Box
		int VertLine = (((float)size.w) * (0.20f));
		int HorzLine = (((float)size.h) * (0.20f));

		Render::Clear(size.x, size.y - 1, VertLine, 1, Color(10, 10, 10, 150));
		Render::Clear(size.x + size.w - VertLine, size.y - 1, VertLine, 1, Color(10, 10, 10, 150));
		Render::Clear(size.x, size.y + size.h - 1, VertLine, 1, Color(10, 10, 10, 150));
		Render::Clear(size.x + size.w - VertLine, size.y + size.h - 1, VertLine, 1, Color(10, 10, 10, 150));

		Render::Clear(size.x - 1, size.y, 1, HorzLine, Color(10, 10, 10, 150));
		Render::Clear(size.x - 1, size.y + size.h - HorzLine, 1, HorzLine, Color(10, 10, 10, 150));
		Render::Clear(size.x + size.w - 1, size.y, 1, HorzLine, Color(10, 10, 10, 150));
		Render::Clear(size.x + size.w - 1, size.y + size.h - HorzLine, 1, HorzLine, Color(10, 10, 10, 150));

		Render::Clear(size.x, size.y, VertLine, 1, color);
		Render::Clear(size.x + size.w - VertLine, size.y, VertLine, 1, color);
		Render::Clear(size.x, size.y + size.h, VertLine, 1, color);
		Render::Clear(size.x + size.w - VertLine, size.y + size.h, VertLine, 1, color);

		Render::Clear(size.x, size.y, 1, HorzLine, color);
		Render::Clear(size.x, size.y + size.h - HorzLine, 1, HorzLine, color);
		Render::Clear(size.x + size.w, size.y, 1, HorzLine, color);
		Render::Clear(size.x + size.w, size.y + size.h - HorzLine, 1, HorzLine, color);
	}
	else if (Menu::Window.VisualsTab.BoxDesign.GetIndex() == 1)
	{
		// Full Box
		Render::Outline(size.x, size.y, size.w, size.h, color);
		Render::Outline(size.x - 1, size.y - 1, size.w + 2, size.h + 2, Color(10, 10, 10, 150));
		Render::Outline(size.x + 1, size.y + 1, size.w - 2, size.h - 2, Color(10, 10, 10, 150));
	}
	else if (Menu::Window.VisualsTab.BoxDesign.GetIndex() == 2)
	{
		// Full Box
		Render::Outline(size.x, size.y, size.w, size.h, color);
		Render::Outline(size.x - 1, size.y - 1, size.w + 2, size.h + 2, Color::FromHSB(rainbow, 1.f, 1.f));
		Render::Outline(size.x + 1, size.y + 1, size.w - 2, size.h - 2, Color::FromHSB(rainbow, 1.f, 1.f));
	}
}

void CEsp::DrawArmor(IClientEntity* pEntity, CEsp::ESPBox size)
{
	ESPBox ArmorBar = size;
	if (Menu::Window.VisualsTab.OptionsArmur.GetState() == 1)
		ArmorBar.y += (ArmorBar.h + 6);
	else
		ArmorBar.y += (ArmorBar.h + 0);

	ArmorBar.h = 4;

	float ArmorValue = pEntity->ArmorValue();
	float ArmorPerc = ArmorValue / 100.f;
	float Width = (size.w * ArmorPerc);
	ArmorBar.w = Width;

	// — Main Bar — //
	Vertex_t Verts[4];
	Verts[0].Init(Vector2D(ArmorBar.x, ArmorBar.y));
	Verts[1].Init(Vector2D(ArmorBar.x + size.w, ArmorBar.y));
	Verts[2].Init(Vector2D(ArmorBar.x + size.w, ArmorBar.y + 5));
	Verts[3].Init(Vector2D(ArmorBar.x, ArmorBar.y + 5));

	Render::PolygonOutline(4, Verts, Color(0, 0, 0, 255), Color(0, 0, 0, 255));

	Vertex_t Verts2[4];
	Verts2[0].Init(Vector2D(ArmorBar.x + 1, ArmorBar.y + 1));
	Verts2[1].Init(Vector2D(ArmorBar.x + ArmorBar.w, ArmorBar.y + 1));
	Verts2[2].Init(Vector2D(ArmorBar.x + ArmorBar.w, ArmorBar.y + 5));
	Verts2[3].Init(Vector2D(ArmorBar.x, ArmorBar.y + 5));

	Color c = Color(0, 205, 247, 255);
	Render::Polygon(4, Verts2, c);

	Verts2[0].Init(Vector2D(ArmorBar.x + 1, ArmorBar.y + 1));
	Verts2[1].Init(Vector2D(ArmorBar.x + ArmorBar.w, ArmorBar.y + 1));
	Verts2[2].Init(Vector2D(ArmorBar.x + ArmorBar.w, ArmorBar.y + 2));
	Verts2[3].Init(Vector2D(ArmorBar.x, ArmorBar.y + 2));

	Render::Polygon(4, Verts2, Color(0, 255, 0, 255));
}



// Unicode Conversions
static wchar_t* CharToWideChar(const char* text)
{
	size_t size = strlen(text) + 1;
	wchar_t* wa = new wchar_t[size];
	mbstowcs_s(NULL, wa, size/4, text, size);
	return wa;
}

// Player name
void CEsp::DrawName(player_info_t pinfo, CEsp::ESPBox size)
{
	RECT nameSize = Render::GetTextSize(Render::Fonts::ESP, pinfo.name);
	Render::Text(size.x + (size.w / 2) - (nameSize.right / 2), size.y - 16, Color(255, 255, 255, 255), Render::Fonts::ESP, pinfo.name);
}

// Draw a health bar. For Tf2 when a bar is bigger than max health a second bar is displayed
void CEsp::DrawHealth(IClientEntity* pEntity, CEsp::ESPBox size)
{
	if (Menu::Window.VisualsTab.OptionsHealth.GetIndex() == 0)
	{
		ESPBox HealthBar = size;
		HealthBar.y += (HealthBar.h + 6);
		HealthBar.h = 4;

		float HealthValue = pEntity->GetHealth();
		float HealthPerc = HealthValue / 100.f;
		float Width = (size.w * HealthPerc);
		HealthBar.w = Width;

		int health = pEntity->GetHealth();

		if (health > 100)
			health = 100;

		int r = 255 - health * 2.55;
		int g = health * 2.55;

		int healthBar = size.h / 100 * health;
		int healthBarDelta = size.h - healthBar;

		//int iClampedHealth = pEntity->GetHealth();
		//	if (iClampedHealth >= 100)
		//	iClampedHealth = 100;

		Render::Outline(size.x - 4, size.y + 1, 1, size.h * 0.01 * health, Color(r, g, 0, 255));
		Render::Outline(size.x - 5, size.y - 1, 3, size.h + 2, Color(0, 0, 0, 150));
	}
	else if (Menu::Window.VisualsTab.OptionsHealth.GetIndex() == 1)
	{
		ESPBox HealthBar = size;
		HealthBar.y += (HealthBar.h + 6);
		HealthBar.h = 4;

		float HealthValue = pEntity->GetHealth();
		float HealthPerc = HealthValue / 100.f;
		float Width = (size.w * HealthPerc);
		HealthBar.w = Width;

		int health = pEntity->GetHealth();

		if (health > 100)
			health = 100;

		int r = 255 - health * 2.55;
		int g = health * 2.55;

		int healthBar = size.h / 100 * health;
		int healthBarDelta = size.h - healthBar;

		//int iClampedHealth = pEntity->GetHealth();
		//	if (iClampedHealth >= 100)
		//	iClampedHealth = 100;

		char buffer[24];
		sprintf_s(buffer, "%.f", HealthValue);
		RECT txtSize = Render::GetTextSize(Render::Fonts::Menu, buffer);
		if (HealthPerc == 0)
			Render::Text(size.x - 8 , size.y * 0.01 * 100, Color(244, 244, 244, 255), Render::Fonts::Menu, buffer);
		else
			Render::Text(size.x - 8, size.y * 0.01 * 100, Color(244, 244, 244, 255), Render::Fonts::Menu, buffer);
	}
	else if (Menu::Window.VisualsTab.OptionsHealth.GetIndex() == 2)
	{
		ESPBox HealthBar = size;
		HealthBar.y += (HealthBar.h + 6);
		HealthBar.h = 4;

		float HealthValue = pEntity->GetHealth();
		float HealthPerc = HealthValue / 100.f;
		float Width = (size.w * HealthPerc);
		HealthBar.w = Width;

		int health = pEntity->GetHealth();

		if (health > 100)
			health = 100;

		int r = 255 - health * 2.55;
		int g = health * 2.55;

		int healthBar = size.h / 100 * health;
		int healthBarDelta = size.h - healthBar;

		//int iClampedHealth = pEntity->GetHealth();
		//	if (iClampedHealth >= 100)
		//	iClampedHealth = 100;

		Render::Outline(size.x - 4, size.y + 1, 1, size.h * 0.01 * health, Color(r, g, 0, 255));
		Render::Outline(size.x - 5, size.y - 1, 3, size.h + 2, Color(0, 0, 0, 150));

		char buffer[24];
		sprintf_s(buffer, "%.f", HealthValue);
		RECT txtSize = Render::GetTextSize(Render::Fonts::Menu, buffer);
		if (HealthPerc == 0)
			Render::Text(size.x - 8, size.y * 0.01 * 100, Color(244, 244, 244, 255), Render::Fonts::Menu, buffer);
		else
			Render::Text(size.x - 8, size.y * 0.01 * 100, Color(244, 244, 244, 255), Render::Fonts::Menu, buffer);
	}
}

// Cleans the internal class name up to something human readable and nice
std::string CleanItemName(std::string name)
{
	std::string Name = name;
	// Tidy up the weapon Name
	if (Name[0] == 'C')
		Name.erase(Name.begin());

	// Remove the word Weapon
	auto startOfWeap = Name.find("Weapon");
	if (startOfWeap != std::string::npos)
		Name.erase(Name.begin() + startOfWeap, Name.begin() + startOfWeap + 6);

	return Name;
}

// Anything else: weapons, class state? idk
void CEsp::DrawInfo(IClientEntity* pEntity, CEsp::ESPBox size)
{
	std::vector<std::string> Info;

	// Player Weapon ESP
	IClientEntity* pWeapon = Interfaces::EntList->GetClientEntityFromHandle((HANDLE)pEntity->GetActiveWeaponHandle());
	if (Menu::Window.VisualsTab.OptionsWeapon.GetState() && pWeapon)
	{
		ClientClass* cClass = (ClientClass*)pWeapon->GetClientClass();
		if (cClass)
		{
			// Draw it
			Info.push_back(CleanItemName(cClass->m_pNetworkName));
		}
	}

	// Bomb Carrier
	if (Menu::Window.VisualsTab.OptionsInfo.GetState() && pEntity == BombCarrier)
	{
		Info.push_back("Bomb Carrier");
	}

	static RECT Size = Render::GetTextSize(Render::Fonts::Default, "Hi");
	int i = 0;
	for (auto Text : Info)
	{
		// Render both Weapon and Bomb Carrier
		Render::Text(size.x + size.w + 3, size.y + (i*(Size.bottom + 2)), Color(255, 255, 255, 255), Render::Fonts::ESP, Text.c_str());
		i++;
	}
}

// Little cross on their heads
void CEsp::DrawCross(IClientEntity* pEntity)
{

	Vector cross = pEntity->GetHeadPos(), screen;
	static int Scale = 2;
	if (Render::WorldToScreen(cross, screen))
	{

		Render::DrawCircle(screen.x - Scale, screen.y - (Scale * 2), (Scale * 2), (Scale * 4), Color(255, 255, 255, 255));
	}

}

void AngleVectors123(const Vector &angles, Vector *forward)
{
	Assert(s_bMathlibInitialized);
	Assert(forward);

	float	sp, sy, cp, cy;

	sy = sin(DEG2RAD(angles[1]));
	cy = cos(DEG2RAD(angles[1]));

	sp = sin(DEG2RAD(angles[0]));
	cp = cos(DEG2RAD(angles[0]));

	forward->x = cp*cy;
	forward->y = cp*sy;
	forward->z = -sp;
}


void CEsp::BulletTrace(IClientEntity* pEntity, Color color)
{
	Vector src3D, dst3D, forward, src, dst;
	Vector vParent, vChild, sParent, sChild;
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	AngleVectors123(pEntity->GetEyePosition(), &forward);
	filter.pSkip = pEntity;
	vParent = pEntity->GetBonePos(6) - Vector(0, 0, 0);
	dst3D = vParent + (forward * 200.f);

	ray.Init(vParent, dst3D);

	Interfaces::Trace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	if (Render::WorldToScreen(vParent, sParent) && Render::WorldToScreen(vChild, sChild))
		return;

	Render::Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(255, 255, 255, 255));

}

// Draws a dropped CS:GO Item
void CEsp::DrawDrop(IClientEntity* pEntity, ClientClass* cClass)
{
	Vector Box;
	CBaseCombatWeapon* Weapon = (CBaseCombatWeapon*)pEntity;
	IClientEntity* plr = Interfaces::EntList->GetClientEntityFromHandle((HANDLE)Weapon->GetOwnerHandle());
	if (!plr && Render::WorldToScreen(Weapon->GetOrigin(), Box))
	{
		if (Menu::Window.VisualsTab.OptionsBox.GetState())
		{
			Render::Outline(Box.x - 2, Box.y - 2, 4, 4, Color(255, 255, 255, 255));
			Render::Outline(Box.x - 3, Box.y - 3, 6, 6, Color(10, 10, 10, 150));
		}

		if (Menu::Window.VisualsTab.OptionsInfo.GetState())
		{
			std::string ItemName = CleanItemName(cClass->m_pNetworkName);
			RECT TextSize = Render::GetTextSize(Render::Fonts::ESP, ItemName.c_str());
			Render::Text(Box.x - (TextSize.right / 2), Box.y - 16, Color(255, 255, 255, 255), Render::Fonts::ESP, ItemName.c_str());
		}
	}
}

void CEsp::lbyup(IClientEntity* pEntity, CEsp::ESPBox size)
{
	if (pEntity->GetFlags() & FL_ONGROUND && pEntity->GetVelocity().Length2D() != 1)
	{
		RECT defSize = Render::GetTextSize(Render::Fonts::ESP, "");
		Render::Text(size.x + size.w + 3, size.y + (0.3*(defSize.bottom + 15)),
			Color(0, 255, 0, 255), Render::Fonts::ESP, "LBY");
	}

	if (pEntity->GetFlags() & FL_ONGROUND && pEntity->GetVelocity().Length2D() != 0)
	{
		RECT defSize = Render::GetTextSize(Render::Fonts::ESP, "");
		Render::Text(size.x + size.w + 3, size.y + (0.3*(defSize.bottom + 15)),
			Color(255, 0, 0, 255), Render::Fonts::ESP, "LBY");
	}
}

// Draws a chicken
void CEsp::DrawChicken(IClientEntity* pEntity, ClientClass* cClass)
{
	ESPBox Box;

	if (GetBox(pEntity, Box))
	{
		player_info_t pinfo; strcpy_s(pinfo.name, "Chicken");
		if (Menu::Window.VisualsTab.OptionsBox.GetState())
			DrawBox(Box, Color(255,255,255,255));

		if (Menu::Window.VisualsTab.OptionsName.GetState())
			DrawName(pinfo, Box);
	}
}

// Draw the planted bomb and timer
void CEsp::DrawBombPlanted(IClientEntity* pEntity, ClientClass* cClass) 
{
	// Null it out incase bomb has been dropped or planted
	BombCarrier = nullptr;

	Vector vOrig; Vector vScreen;
	vOrig = pEntity->GetOrigin();
	CCSBomb* Bomb = (CCSBomb*)pEntity;

	if (Render::WorldToScreen(vOrig, vScreen))
	{
		float flBlow = Bomb->GetC4BlowTime();
		float TimeRemaining = flBlow - (Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase());
		char buffer[64];
		sprintf_s(buffer, "Bomb Explosion [ %.1f ]", TimeRemaining);
		Render::Text(vScreen.x, vScreen.y, Color(0, 255, 205, 255), Render::Fonts::ESP, buffer);
	}
}

void CEsp::Fill(CEsp::ESPBox size, Color color)
{

	if (Menu::Window.VisualsTab.OptionsFill.GetState())
	{
		Render::Clear(size.x, size.y, size.w, size.h, Color(20, 20, 20, 120));
	}
	else {

	}
}//+

// Draw the bomb if it's dropped, or store the player who's carrying 
void CEsp::DrawBomb(IClientEntity* pEntity, ClientClass* cClass)
{
	// Null it out incase bomb has been dropped or planted
	BombCarrier = nullptr;
	CBaseCombatWeapon *BombWeapon = (CBaseCombatWeapon *)pEntity;
	Vector vOrig; Vector vScreen;
	vOrig = pEntity->GetOrigin();
	bool adopted = true;
	HANDLE parent = BombWeapon->GetOwnerHandle();
	if (parent || (vOrig.x == 0 && vOrig.y == 0 && vOrig.z == 0))
	{
		IClientEntity* pParentEnt = (Interfaces::EntList->GetClientEntityFromHandle(parent));
		if (pParentEnt && pParentEnt->IsAlive())
		{
			BombCarrier = pParentEnt;
			adopted = false;
		}
	}

	if (adopted)
	{
		if (Render::WorldToScreen(vOrig, vScreen))
		{
			Render::Text(vScreen.x, vScreen.y, Color(255, 0, 0, 255), Render::Fonts::ESP, "Dropped Bomb");
		}
	}
}

void DrawBoneArray(int* boneNumbers, int amount, IClientEntity* pEntity, Color color)
{
	Vector LastBoneScreen;
	for (int i = 0; i < amount; i++)
	{
		Vector Bone = pEntity->GetBonePos(boneNumbers[i]);
		Vector BoneScreen;

		if (Render::WorldToScreen(Bone, BoneScreen))
		{
			if (i>0)
			{
				Render::Line(LastBoneScreen.x, LastBoneScreen.y, BoneScreen.x, BoneScreen.y, color);
			}
		}
		LastBoneScreen = BoneScreen;
	}
}

void DrawBoneTest(IClientEntity *pEntity)
{
	for (int i = 0; i < 127; i++)
	{
		Vector BoneLoc = pEntity->GetBonePos(i);
		Vector BoneScreen;
		if (Render::WorldToScreen(BoneLoc, BoneScreen))
		{
			char buf[10];
			_itoa_s(i, buf, 10);
			Render::Text(BoneScreen.x, BoneScreen.y, Color(255, 255, 255, 180), Render::Fonts::ESP, buf);
		}
	}
}

void CEsp::DrawSkeleton(IClientEntity* pEntity)
{
	int rskele = Menu::Window.VisualsTab.RSkele.GetValue();
	int gskele = Menu::Window.VisualsTab.GSkele.GetValue();
	int bskele = Menu::Window.VisualsTab.BSkele.GetValue();
	int askele = Menu::Window.VisualsTab.ASkele.GetValue();

	studiohdr_t* pStudioHdr = Interfaces::ModelInfo->GetStudiomodel(pEntity->GetModel());

	if (!pStudioHdr)
		return;

	Vector vParent, vChild, sParent, sChild;

	for (int j = 0; j < pStudioHdr->numbones; j++)
	{
		mstudiobone_t* pBone = pStudioHdr->GetBone(j);

		if (pBone && (pBone->flags & BONE_USED_BY_HITBOX) && (pBone->parent != -1))
		{
			vChild = pEntity->GetBonePos(j);
			vParent = pEntity->GetBonePos(pBone->parent);

			if (Render::WorldToScreen(vParent, sParent) && Render::WorldToScreen(vChild, sChild))
			{
				Render::Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(rskele, gskele, bskele, askele));
			}
		}
	}
}