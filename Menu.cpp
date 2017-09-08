#include "Menu.h"
#include "Controls.h"
#include "Hooks.h"
#include "Interfaces.h"
#include "CRC32.h"

// Menu Window Size
#define WINDOW_WIDTH 780 // Menu Width
#define WINDOW_HEIGHT 575 // Menu Height

ComplexMenu Menu::Window;

// Save Config to CSGO directory
void SaveCallbk()
{
	switch (Menu::Window.ConfigBox.GetIndex())
	{
	case 0:
		GUI.SaveWindowState(&Menu::Window, "ComplexLegit.cfg");
		break;
	case 1:
		GUI.SaveWindowState(&Menu::Window, "ComplexRage.cfg");
		break;
	case 2:
		GUI.SaveWindowState(&Menu::Window, "ComplexMMHvH.cfg");
		break;
	case 3:
		GUI.SaveWindowState(&Menu::Window, "ComplexScoutHvH.cfg");
		break;
	case 4:
		GUI.SaveWindowState(&Menu::Window, "ComplexAWPHvH.cfg");
		break;
	case 5:
		GUI.SaveWindowState(&Menu::Window, "ComplexNSHvH.cfg");
		break;
	}
}

// Load Config from CSGO directory
void LoadCallbk()
{
	switch (Menu::Window.ConfigBox.GetIndex())
	{
	case 0:
		GUI.LoadWindowState(&Menu::Window, "ComplexLegit.cfg");
		break;
	case 1:
		GUI.LoadWindowState(&Menu::Window, "ComplexRage.cfg");
		break;
	case 2:
		GUI.LoadWindowState(&Menu::Window, "ComplexMMHvH.cfg");
		break;
	case 3:
		GUI.LoadWindowState(&Menu::Window, "ComplexScoutHvH.cfg");
		break;
	case 4:
		GUI.LoadWindowState(&Menu::Window, "ComplexAWPHvH.cfg");
		break;
	case 5:
		GUI.LoadWindowState(&Menu::Window, "ComplexNSHvH.cfg");
		break;
	}
}

// Unload Function
void UnLoadCallbk()
{
	DoUnload = true;
}

void KnifeApplyCallbk()
{
	static ConVar* Dank = Interfaces::CVar->FindVar("cl_fullupdate");
	Dank->nFlags &= ~FCVAR_CHEAT;
	Interfaces::Engine->ClientCmd_Unrestricted("cl_fullupdate");
	bGlovesNeedUpdate = true;
}

void ComplexMenu::Setup()
{
	SetPosition(120, 20);
	SetSize(WINDOW_WIDTH, WINDOW_HEIGHT); 
	SetTitle("C O M P L E X ( B E T A )");

	RegisterTab(&RageBotTab);
	RegisterTab(&HvHTab);
	RegisterTab(&LegitBotTab);
	RegisterTab(&VisualsTab);
	RegisterTab(&SkinTab);
	RegisterTab(&MiscTab);

	RECT Client = GetClientArea();
	Client.bottom -= 29;

	RageBotTab.Setup();
	HvHTab.Setup();
	LegitBotTab.Setup();
	VisualsTab.Setup();
	SkinTab.Setup();
	MiscTab.Setup();

#pragma region Bottom Buttons

	ConfigBox.SetFileId("cfg_box");
	ConfigBox.AddItem("Legit");
	ConfigBox.AddItem("Rage");
	ConfigBox.AddItem("MM HvH");
	ConfigBox.AddItem("Scout MM HvH");
	ConfigBox.AddItem("AWP MM HvH");
	ConfigBox.AddItem("Nospread HvH");
	ConfigBox.SetSize(100, 385);
	ConfigBox.SetPosition(10, Client.bottom - 113);

	SaveButton.SetText("Save");
	SaveButton.SetCallback(SaveCallbk);
	SaveButton.SetSize(45, 275);
	SaveButton.SetPosition(10, Client.bottom - 91);

	LoadButton.SetText("Load");
	LoadButton.SetCallback(LoadCallbk);
	LoadButton.SetSize(45, 275);
	LoadButton.SetPosition(65, Client.bottom - 91);;

	RageBotTab.RegisterControl(&LoadButton);
	HvHTab.RegisterControl(&LoadButton);
	LegitBotTab.RegisterControl(&LoadButton);
	VisualsTab.RegisterControl(&LoadButton);
	SkinTab.RegisterControl(&LoadButton);
	MiscTab.RegisterControl(&LoadButton);

	RageBotTab.RegisterControl(&SaveButton);
	HvHTab.RegisterControl(&SaveButton);
	LegitBotTab.RegisterControl(&SaveButton);
	VisualsTab.RegisterControl(&SaveButton);
	SkinTab.RegisterControl(&SaveButton);
	MiscTab.RegisterControl(&SaveButton);

	RageBotTab.RegisterControl(&ConfigBox);
	HvHTab.RegisterControl(&ConfigBox);
	LegitBotTab.RegisterControl(&ConfigBox);
	VisualsTab.RegisterControl(&ConfigBox);
	SkinTab.RegisterControl(&ConfigBox);
	MiscTab.RegisterControl(&ConfigBox);

#pragma endregion Setting up the settings buttons
}

void CRageBotTab::Setup()
{
	SetTitle("RAGE");

	ActiveLabel.SetPosition(160, 8);
	ActiveLabel.SetText("Active");
	RegisterControl(&ActiveLabel);

	Active.SetFileId("active");
	Active.SetPosition(205, 8);
	RegisterControl(&Active);

#pragma region Aimbot

	AimbotGroup.SetPosition(160, 35);
	AimbotGroup.SetText("Aimbot");
	AimbotGroup.SetSize(260, 280);
	RegisterControl(&AimbotGroup);

	AimbotEnable.SetFileId("aim_enable");
	AimbotGroup.PlaceLabledControl("Enable", this, &AimbotEnable);

	AimbotKeyPress.SetFileId("aim_key");
	AimbotGroup.PlaceLabledControl("On Key", this, &AimbotKeyPress);

	AimbotKeyBind.SetFileId("aim_key");
	AimbotGroup.PlaceLabledControl("Key", this, &AimbotKeyBind);

	AimbotAutoFire.SetFileId("aim_autofire");
	AimbotGroup.PlaceLabledControl("Automatic Fire", this, &AimbotAutoFire);

	AimbotFov.SetFileId("aim_fov");
	AimbotFov.SetBoundaries(0.f, 180.f);
	AimbotFov.SetValue(39.f);
	AimbotGroup.PlaceLabledControl("FOV Range", this, &AimbotFov);

	AimbotSlientSelection.SetFileId("tgt_slientselection");
	AimbotSlientSelection.AddItem("Disabled");
	AimbotSlientSelection.AddItem("Client-Sided");
	AimbotSlientSelection.AddItem("Server-Sided");
	AimbotGroup.PlaceLabledControl("Silent", this, &AimbotSlientSelection);

	AimbotAutoPistol.SetFileId("aim_autopistol");
	AimbotGroup.PlaceLabledControl("Automatic Pistol", this, &AimbotAutoPistol);

	AimbotAutoRevolver.SetFileId("aim_autorevolver");
	AimbotGroup.PlaceLabledControl("Automatic Revolver", this, &AimbotAutoRevolver);

	TargetFriendlyFire.SetFileId("tgt_friendlyfire");
	AimbotGroup.PlaceLabledControl("Friendly Fire", this, &TargetFriendlyFire);

	AimbotAimStep.SetFileId("aim_aimstep");
	AimbotGroup.PlaceLabledControl("Aim Step", this, &AimbotAimStep);
#pragma endregion Aimbot Controls Get Setup in here

#pragma region Target
	TargetGroup.SetPosition(430, 230);
	TargetGroup.SetText("Target");
	TargetGroup.SetSize(260, 165);
	RegisterControl(&TargetGroup);;

	TargetSelection.SetFileId("tgt_selection");
	TargetSelection.AddItem("Field of View");
	TargetSelection.AddItem("Distance");
	TargetSelection.AddItem("Health");
	TargetGroup.PlaceLabledControl("Selection", this, &TargetSelection);

	TargetHitbox.SetFileId("tgt_hitbox");
	TargetHitbox.AddItem("Head");
	TargetHitbox.AddItem("Neck");
	TargetHitbox.AddItem("Chest");
	TargetHitbox.AddItem("Stomach");
	TargetHitbox.AddItem("Pelvis");
	TargetGroup.PlaceLabledControl("Hitbox", this, &TargetHitbox);

	TargetHitscan.SetFileId("tgt_hitscan");
	TargetHitscan.AddItem("Off"); // Hitscan Case 0
	TargetHitscan.AddItem("Low"); // Hitscan case 1
	TargetHitscan.AddItem("Medium"); // Hitscan Case 2
	TargetHitscan.AddItem("High"); // Hitscan Case 3
	TargetGroup.PlaceLabledControl("Multibox", this, &TargetHitscan);

	TargetMultipoint.SetFileId("tgt_multipoint");
	TargetGroup.PlaceLabledControl("Multipoint", this, &TargetMultipoint);

	TargetPointscale.SetFileId("tgt_pointscale");
	TargetPointscale.SetBoundaries(0, 1);
	TargetPointscale.SetValue(0.5);
	TargetGroup.PlaceLabledControl("Pointscale", this, &TargetPointscale);
#pragma endregion Targetting controls 

#pragma region Accuracy

	AccuracyGroup.SetPosition(430, 35);
	AccuracyGroup.SetText("Accuracy");
	AccuracyGroup.SetSize(270, 185);
	RegisterControl(&AccuracyGroup);

	AccuracyRecoil.SetFileId("acc_norecoil");
	AccuracyGroup.PlaceLabledControl("No Recoil", this, &AccuracyRecoil);

	AccuracyAutoWall.SetFileId("acc_awall");
	AccuracyGroup.PlaceLabledControl("Automatic penetration", this, &AccuracyAutoWall);

	AccuracyMinimumDamage.SetFileId("acc_mindmg");
	AccuracyMinimumDamage.SetBoundaries(0.f, 99.f);
	AccuracyMinimumDamage.SetValue(0.f);
	AccuracyGroup.PlaceLabledControl("Autowall Damage", this, &AccuracyMinimumDamage);

	AccuracyAutoStop.SetFileId("acc_stop");
	AccuracyGroup.PlaceLabledControl("Auto Stop & Crouch", this, &AccuracyAutoStop);

	AccuracyAutoScope.SetFileId("acc_scope");
	AccuracyGroup.PlaceLabledControl("Automatic Scope", this, &AccuracyAutoScope);

	AccuracyHitchance.SetFileId("acc_chance");
	AccuracyHitchance.SetBoundaries(0, 100);
	AccuracyHitchance.SetValue(0);
	AccuracyGroup.PlaceLabledControl("Hit Chance", this, &AccuracyHitchance);

#pragma endregion  Accuracy controls get Setup in here
}

void CHvHTab::Setup()
{
	SetTitle("HvH");

	ActiveLabel.SetPosition(160, 8);
	ActiveLabel.SetText("Active");
	RegisterControl(&ActiveLabel);

	Active.SetFileId("active");
	Active.SetPosition(205, 8);
	RegisterControl(&Active);


	AccuracyGroup.SetPosition(195, 45);
	AccuracyGroup.SetText("HvH");
	AccuracyGroup.SetSize(270, 175);
	RegisterControl(&AccuracyGroup);

	AccuracyPrediction.SetFileId("acc_prediction");
	AccuracyGroup.PlaceLabledControl("Engine prediction", this, &AccuracyPrediction);

	PerfectAccuracy.SetFileId("acc_paccuracy");
	AccuracyGroup.PlaceLabledControl("Lag Compensation", this, &PerfectAccuracy);

	AccuracyPositionAdjustment.SetFileId("acc_disableinterp");
	AccuracyGroup.PlaceLabledControl("Disable Interpolation", this, &AccuracyPositionAdjustment);

	AccuracyResolverYaw.SetFileId("acc_resolver");
	AccuracyGroup.PlaceLabledControl("Resolver", this, &AccuracyResolverYaw);

	AccuracySmart.SetFileId("acc_smart");
	AccuracySmart.SetBoundaries(0, 20);
	AccuracySmart.SetValue(0);
	AccuracyGroup.PlaceLabledControl("Baim after shots", this, &AccuracySmart);

#pragma region Anti-Aim

	// Anti-Aim
	AntiAimGroup.SetPosition(485, 45);
	AntiAimGroup.SetText("Anti-Aim");
	AntiAimGroup.SetSize(270, 225);
	RegisterControl(&AntiAimGroup);

	AntiAimEnable.SetFileId("aa_enable");
	AntiAimGroup.PlaceLabledControl("Enable", this, &AntiAimEnable);

	AntiAimPitch.SetFileId("aa_x");
	AntiAimPitch.AddItem("None");
	AntiAimPitch.AddItem("Down");
	AntiAimPitch.AddItem("Half Down");
	AntiAimPitch.AddItem("Jitter");
	AntiAimPitch.AddItem("Static");
	AntiAimPitch.AddItem("Fake Down");
	AntiAimPitch.AddItem("Lisp Down");
	AntiAimPitch.AddItem("Lisp Up");
	AntiAimPitch.AddItem("NS UT");
	AntiAimPitch.AddItem("FakeZero");
	AntiAimPitch.AddItem("Overflow-Down");
	AntiAimPitch.AddItem("Overflow-Up");
	AntiAimPitch.AddItem("Fake-Overflow");
	AntiAimPitch.AddItem("StaticJitter UT");
	AntiAimGroup.PlaceLabledControl("Pitch", this, &AntiAimPitch);

	AntiAimYaw.SetFileId("aa_y");
	AntiAimYaw.AddItem("None");
	AntiAimYaw.AddItem("Fake Edge");
	AntiAimYaw.AddItem("Fake Sideways");
	AntiAimYaw.AddItem("Fake Static");
	AntiAimYaw.AddItem("T Fake");
	AntiAimYaw.AddItem("Fake Jitter");
	AntiAimYaw.AddItem("Jitter");
	AntiAimYaw.AddItem("T Jitter");
	AntiAimYaw.AddItem("Back Jitter");
	AntiAimYaw.AddItem("Backwards");
	AntiAimYaw.AddItem("Fake Lowerbody");
	AntiAimYaw.AddItem("-Fake Jitter");
	AntiAimYaw.AddItem("Fake Static 2");
	AntiAimYaw.AddItem("FakeSide RealBack");
	AntiAimYaw.AddItem("Fake Stactic 3");
	AntiAimYaw.AddItem("Fake Static Remake");
	AntiAimYaw.AddItem("LBY des");
	AntiAimYaw.AddItem("suicide");
	AntiAimYaw.AddItem("Arizona");
	AntiAimYaw.AddItem("Jew Distortion");
	AntiAimYaw.AddItem("SidesTrash");
	AntiAimGroup.PlaceLabledControl("Yaw", this, &AntiAimYaw);

	AntiAimSpinYaw.SetFileId("aa_ynumtwo");
	AntiAimSpinYaw.AddItem("None");
	AntiAimSpinYaw.AddItem("SpinBot");
	AntiAimSpinYaw.AddItem("CSpin");
	AntiAimSpinYaw.AddItem("HalfSpin");
	AntiAimSpinYaw.AddItem("Fake Forward");
	AntiAimSpinYaw.AddItem("Fake Backwards");
	AntiAimSpinYaw.AddItem("Fake Spin");
	AntiAimSpinYaw.AddItem("Kidua Jitter");
	AntiAimSpinYaw.AddItem("Fake 180 Jitter");
	AntiAimSpinYaw.AddItem("Sinster Roll");
	AntiAimSpinYaw.AddItem("Opaf");
	AntiAimSpinYaw.AddItem("phase");
	AntiAimSpinYaw.AddItem("SidesTrash2");
	AntiAimSpinYaw.AddItem("Crash");
	AntiAimSpinYaw.AddItem("FakeTwoStep");
	AntiAimGroup.PlaceLabledControl("Fake Yaw", this, &AntiAimSpinYaw);

	AntiAimJitterLBY.SetFileId("aa_jitterlby");
	AntiAimGroup.PlaceLabledControl("Anti Resolver", this, &AntiAimJitterLBY);

	AntiAimKnife.SetFileId("aa_knife");
	AntiAimGroup.PlaceLabledControl("Anti Aim on Knife", this, &AntiAimKnife);

	AntiAimTarget.SetFileId("aa_target");
	AntiAimGroup.PlaceLabledControl("Anti Aim At Target", this, &AntiAimTarget);

	AntiAimEdge.SetFileId("aa_edge");
	AntiAimGroup.PlaceLabledControl("Edge", this, &AntiAimEdge);

#pragma endregion  AntiAim controls get setup in here

#pragma region FakeLag
	FakeLagGroup.SetPosition(195, 230);
	FakeLagGroup.SetSize(260, 70);
	FakeLagGroup.SetText("Fakelag");
	RegisterControl(&FakeLagGroup);

	FakeLagChoke.SetFileId("fakelag_choke");
	FakeLagChoke.SetBoundaries(0, 16);
	FakeLagChoke.SetValue(0);
	FakeLagGroup.PlaceLabledControl("Factor", this, &FakeLagChoke);
#pragma endregion fakelag shit

}

void CLegitBotTab::Setup()
{
	SetTitle("LEGIT");

	ActiveLabel.SetPosition(160, 8);
	ActiveLabel.SetText("Active");
	RegisterControl(&ActiveLabel);

	Active.SetFileId("active");
	Active.SetPosition(205, 8);
	RegisterControl(&Active);

#pragma region Aimbot
	AimbotGroup.SetPosition(160, 35);
	AimbotGroup.SetText("Aimbot");
	AimbotGroup.SetSize(240, 210);
	RegisterControl(&AimbotGroup);

	AimbotEnable.SetFileId("aim_enable");
	AimbotGroup.PlaceLabledControl("Enable", this, &AimbotEnable);

	AimbotAutoFire.SetFileId("aim_autofire");
	AimbotGroup.PlaceLabledControl("Auto Fire", this, &AimbotAutoFire);

	AimbotKeyPress.SetFileId("aim_key");
	AimbotGroup.PlaceLabledControl("On Key", this, &AimbotKeyPress);

	AimbotKeyBind.SetFileId("aim_key");
	AimbotGroup.PlaceLabledControl("Key", this, &AimbotKeyBind);
	
	AimbotAutoPistol.SetFileId("aim_apistol");
	AimbotGroup.PlaceLabledControl("Auto Pistol", this, &AimbotAutoPistol);

#pragma endregion Aimbot shit

#pragma region Main Weapon

	WeaponMainGroup.SetPosition(160, 260);
	WeaponMainGroup.SetText("Rifles/Other");
	WeaponMainGroup.SetSize(210, 210);
	RegisterControl(&WeaponMainGroup);

	WeaponMainSpeed.SetFileId("main_speed");
	WeaponMainSpeed.SetBoundaries(0.1f, 2.f);
	WeaponMainSpeed.SetValue(1.0f);
	WeaponMainGroup.PlaceLabledControl("Max Speed", this, &WeaponMainSpeed);

	WeaponMainFoV.SetFileId("main_fov");
	WeaponMainFoV.SetBoundaries(0.1f, 30.f);
	WeaponMainFoV.SetValue(5.f);
	WeaponMainGroup.PlaceLabledControl("FoV", this, &WeaponMainFoV);

	WeaponMainRecoil.SetFileId("main_recoil");
	WeaponMainGroup.PlaceLabledControl("Recoil", this, &WeaponMainRecoil);

	WeaponMainPSilent.SetFileId("main_psilent");
	WeaponMainGroup.PlaceLabledControl("Perfect Silent", this, &WeaponMainPSilent);

	WeaponMainInacc.SetFileId("main_inacc");
	WeaponMainInacc.SetBoundaries(0.f, 15.f);
	WeaponMainGroup.PlaceLabledControl("Inaccuracy", this, &WeaponMainInacc);

	WeaponMainHitbox.SetFileId("main_hitbox");
	WeaponMainHitbox.AddItem("Head");
	WeaponMainHitbox.AddItem("Neck");
	WeaponMainHitbox.AddItem("Chest");
	WeaponMainHitbox.AddItem("Stomach");
	WeaponMainGroup.PlaceLabledControl("Hitbox", this, &WeaponMainHitbox);

#pragma endregion

#pragma region Pistols
	WeaponPistGroup.SetPosition(380, 260);
	WeaponPistGroup.SetText("Pistols");
	WeaponPistGroup.SetSize(210, 210);
	RegisterControl(&WeaponPistGroup);

	WeaponPistSpeed.SetFileId("pist_speed");
	WeaponPistSpeed.SetBoundaries(0.1f, 2.f);
	WeaponPistSpeed.SetValue(1.0f);
	WeaponPistGroup.PlaceLabledControl("Max Speed", this, &WeaponPistSpeed);

	WeaponPistFoV.SetFileId("pist_fov");
	WeaponPistFoV.SetBoundaries(0.1f, 30.f);
	WeaponPistFoV.SetValue(5.f);
	WeaponPistGroup.PlaceLabledControl("FoV", this, &WeaponPistFoV);

	WeaponPistRecoil.SetFileId("pist_recoil");
	WeaponPistGroup.PlaceLabledControl("Recoil", this, &WeaponPistRecoil);

	WeaponPistPSilent.SetFileId("pist_psilent");
	WeaponPistGroup.PlaceLabledControl("Perfect Silent", this, &WeaponPistPSilent);

	WeaponPistInacc.SetFileId("pist_inacc");
	WeaponPistInacc.SetBoundaries(0.f, 15.f);
	WeaponPistGroup.PlaceLabledControl("Inaccuracy", this, &WeaponPistInacc);

	WeaponPistHitbox.SetFileId("pist_hitbox");
	WeaponPistHitbox.AddItem("Head");
	WeaponPistHitbox.AddItem("Neck");
	WeaponPistHitbox.AddItem("Chest");
	WeaponPistHitbox.AddItem("Stomach");
	WeaponPistGroup.PlaceLabledControl("Hitbox", this, &WeaponPistHitbox);
#pragma endregion

#pragma region Snipers
	WeaponSnipGroup.SetPosition(600, 260);
	WeaponSnipGroup.SetText("Snipers");
	WeaponSnipGroup.SetSize(210, 210);
	RegisterControl(&WeaponSnipGroup);

	WeaponSnipSpeed.SetFileId("snip_speed");
	WeaponSnipSpeed.SetBoundaries(0.1f, 2.f);
	WeaponSnipSpeed.SetValue(1.0f);
	WeaponSnipGroup.PlaceLabledControl("Max Speed", this, &WeaponSnipSpeed);

	WeaponSnipFoV.SetFileId("snip_fov");
	WeaponSnipFoV.SetBoundaries(0.1f, 30.f);
	WeaponSnipFoV.SetValue(5.f);
	WeaponSnipGroup.PlaceLabledControl("FoV", this, &WeaponSnipFoV);

	WeaponSnipRecoil.SetFileId("snip_recoil");
	WeaponSnipGroup.PlaceLabledControl("Recoil", this, &WeaponSnipRecoil);

	WeaponSnipPSilent.SetFileId("snip_psilent");
	WeaponSnipGroup.PlaceLabledControl("Perfect Silent", this, &WeaponSnipPSilent);

	WeaponSnipInacc.SetFileId("snip_inacc");
	WeaponSnipInacc.SetBoundaries(0.f, 15.f);
	WeaponSnipGroup.PlaceLabledControl("Inaccuracy", this, &WeaponSnipInacc);

	WeaponSnipHitbox.SetFileId("snip_hitbox");
	WeaponSnipHitbox.AddItem("Head");
	WeaponSnipHitbox.AddItem("Neck");
	WeaponSnipHitbox.AddItem("Chest");
	WeaponSnipHitbox.AddItem("Stomach");
	WeaponSnipGroup.PlaceLabledControl("Hitbox", this, &WeaponSnipHitbox);
#pragma endregion
}

void CVisualTab::Setup()
{
	SetTitle("VISUALS");

	ActiveLabel.SetPosition(160, 8);
	ActiveLabel.SetText("Active");
	RegisterControl(&ActiveLabel);

	Active.SetFileId("active");
	Active.SetPosition(205, 8);
	RegisterControl(&Active);

#pragma region Options
	OptionsGroup.SetText("Options");
	OptionsGroup.SetPosition(160, 35);
	OptionsGroup.SetSize(185, 350);
	RegisterControl(&OptionsGroup);

	OptionsBox.SetFileId("opt_box");
	OptionsGroup.PlaceLabledControl("Box", this, &OptionsBox);

	BoxDesign.SetFileId("opt_boxtype");
	BoxDesign.AddItem("Corners");
	BoxDesign.AddItem("Whole");
	BoxDesign.AddItem("Rainbow");
	OptionsGroup.PlaceLabledControl("Box Type", this, &BoxDesign);

	OptionsFill.SetFileId("opt_fill");
	OptionsGroup.PlaceLabledControl("Fill Box", this, &OptionsFill);

	OptionsName.SetFileId("opt_name");
	OptionsGroup.PlaceLabledControl("Name", this, &OptionsName);

	OptionHealthEnable.SetFileId("opt_health");
	OptionsGroup.PlaceLabledControl("Health", this, &OptionHealthEnable);

	OptionsHealth.SetFileId("opt_hp");
	OptionsHealth.AddItem("Bar");
	OptionsHealth.AddItem("Number");
	OptionsHealth.AddItem("Both");
	//OptionsHealth.AddItem("interwebz");
	//OptionsHealth.AddItem("skeet");
	OptionsGroup.PlaceLabledControl("Health", this, &OptionsHealth);

	OptionsArmur.SetFileId("opt_armor");
	OptionsGroup.PlaceLabledControl("Armor", this, &OptionsArmur);

	OptionsWeapon.SetFileId("opt_weapon");
	OptionsGroup.PlaceLabledControl("Weapon", this, &OptionsWeapon);

	OptionsInfo.SetFileId("opt_info");
	OptionsGroup.PlaceLabledControl("Info", this, &OptionsInfo);

	OptionsChams.SetFileId("opt_chams");
	OptionsChams.AddItem("Off");
	OptionsChams.AddItem("Normal");
	OptionsChams.AddItem("Flat");
	OptionsGroup.PlaceLabledControl("Chams", this, &OptionsChams);

	OptionsSkeleton.SetFileId("opt_bone");
	OptionsGroup.PlaceLabledControl("Skeleton", this, &OptionsSkeleton);

	OptionsAimSpot.SetFileId("opt_aimspot");
	OptionsGroup.PlaceLabledControl("Head Cross", this, &OptionsAimSpot);
	
	OptionsCompRank.SetFileId("opt_comprank");
	OptionsGroup.PlaceLabledControl("Player Ranks", this, &OptionsCompRank);

	LBY.SetFileId("opt_LBY");
	OptionsGroup.PlaceLabledControl("LBY", this, &LBY);

#pragma endregion Setting up the Options controls

#pragma region Filters
	FiltersGroup.SetText("Filters");
	FiltersGroup.SetPosition(355, 35);
	FiltersGroup.SetSize(150, 135);
	RegisterControl(&FiltersGroup);

	FiltersPlayers.SetFileId("ftr_players");
	FiltersGroup.PlaceLabledControl("Players", this, &FiltersPlayers);

	FiltersEnemiesOnly.SetFileId("ftr_enemyonly");
	FiltersGroup.PlaceLabledControl("Enemies Only", this, &FiltersEnemiesOnly);

	FiltersWeapons.SetFileId("ftr_weaps");
	FiltersGroup.PlaceLabledControl("Weapons", this, &FiltersWeapons);

	FiltersChickens.SetFileId("ftr_chickens");
	FiltersGroup.PlaceLabledControl("Chickens", this, &FiltersChickens);

	FiltersC4.SetFileId("ftr_c4");
	FiltersGroup.PlaceLabledControl("C4", this, &FiltersC4);
#pragma endregion Setting up the Filters controls

#pragma region Other
	OtherGroup.SetText("Other");
	OtherGroup.SetPosition(515, 35);
	OtherGroup.SetSize(290, 350);
	RegisterControl(&OtherGroup);

	OtherCrosshair.SetFileId("otr_xhair");
	OtherCrosshair.AddItem("Off");
	OtherCrosshair.AddItem("Rainbow X");
	OtherGroup.PlaceLabledControl("Crosshair", this, &OtherCrosshair);

	OtherRadar.SetFileId("otr_radar");
	OtherGroup.PlaceLabledControl("Engine Radar", this, &OtherRadar);

	OtherNoVisualRecoil.SetFileId("otr_visrecoil");
	OtherGroup.PlaceLabledControl("No Visual Recoil", this, &OtherNoVisualRecoil);

	OtherNoFlash.SetFileId("otr_noflash");
	OtherGroup.PlaceLabledControl("No Flash", this, &OtherNoFlash);

	OtherNoHands.SetFileId("otr_hands");
	OtherNoHands.AddItem("Off");
	OtherNoHands.AddItem("None");
	OtherNoHands.AddItem("Transparent");
	OtherNoHands.AddItem("Chams");
	OtherNoHands.AddItem("Rainbow");
	OtherGroup.PlaceLabledControl("Hands", this, &OtherNoHands);

	OtherThirdperson.SetFileId("aa_thirdpsr");
	OtherGroup.PlaceLabledControl("Thirdperson", this, &OtherThirdperson);

	OtherThirdpersonRange.SetFileId("otr_thirdpersonfov");
	OtherThirdpersonRange.SetBoundaries(0.f, 360.f);
	OtherThirdpersonRange.SetValue(0.f);
	OtherGroup.PlaceLabledControl("Thirdperson Range", this, &OtherThirdpersonRange);

	OtherThirdpersonKey.SetFileId("aa_thirdpsrkey");
	OtherGroup.PlaceLabledControl("Thirdperson Key", this, &OtherThirdpersonKey);

	OtherFOV.SetFileId("otr_fov");
	OtherFOV.SetBoundaries(0.f, 120.f);
	OtherFOV.SetValue(90.f);
	OtherGroup.PlaceLabledControl("FOV", this, &OtherFOV);

	OtherViewmodelFOV.SetFileId("otr_viewfov");
	OtherViewmodelFOV.SetBoundaries(0.f, 120.f);
	OtherViewmodelFOV.SetValue(0.f);
	OtherGroup.PlaceLabledControl("Viewmodel FOV", this, &OtherViewmodelFOV);

	OtherNoScope.SetFileId("otr_noscope");
	OtherNoScope.AddItem("None");
	OtherNoScope.AddItem("On");
	OtherNoScope.AddItem("On + Lines");
	OtherGroup.PlaceLabledControl("Disable Sniper Scope", this, &OtherNoScope);

	SnapLines.SetFileId("otr_snapline");
	OtherGroup.PlaceLabledControl("Snap Line", this, &SnapLines);

#pragma endregion Setting up the Other controls
}

void CSkinTab::Setup()
{
	SetTitle("SKINS");

#pragma region Knife
	KnifeGroup.SetPosition(430, 80);
	KnifeGroup.SetSize(345, 115);
	KnifeGroup.SetText("Knife Changer");
	RegisterControl(&KnifeGroup);

	SkinEnable.SetFileId("knife_enable");
	KnifeGroup.PlaceLabledControl("Enable", this, &SkinEnable);

	KnifeModel.SetFileId("knife_model");
	KnifeModel.AddItem("Karambit");
	KnifeModel.AddItem("M9 Bayonet");
	KnifeModel.AddItem("Huntsman");
	KnifeModel.AddItem("Butterfly");
	KnifeModel.AddItem("Bowie");
	KnifeModel.AddItem("Flip");
	KnifeModel.AddItem("Bayonet");
	KnifeModel.AddItem("Falchion");
	KnifeModel.AddItem("Gut");
	KnifeModel.AddItem("Daggers");
	KnifeGroup.PlaceLabledControl("Knife", this, &KnifeModel);

	KnifeSkin.SetFileId("knife_skin");
	KnifeSkin.SetText("000");
	KnifeGroup.PlaceLabledControl("Knife Skin", this, &KnifeSkin);
#pragma endregion Knife

#pragma region Gloves
	GloveGroup.SetPosition(430, -10);
	GloveGroup.SetText("Gloves");
	GloveGroup.SetSize(345, 80);
	RegisterControl(&GloveGroup);

	GloveModel.SetFileId("glove_model");
	GloveModel.AddItem("None");
	GloveModel.AddItem("BloodHound");
	GloveModel.AddItem("HandWrap");
	GloveModel.AddItem("Driver");
	GloveModel.AddItem("Sport");
	GloveModel.AddItem("Motorcycle");
	GloveModel.AddItem("Specialist");
	GloveGroup.PlaceLabledControl("Glove Model", this, &GloveModel);

	GloveSkin.SetFileId("glove_skin");
	GloveSkin.AddItem("None");
	GloveSkin.AddItem("Snakebite"); //10007
	GloveSkin.AddItem("Charred");// 10006
	GloveSkin.AddItem("Guerrilla");// 10039
	GloveSkin.AddItem("Bronzed");// 10008
	GloveSkin.AddItem("Slaughter"); // 10021
	GloveSkin.AddItem("Badlands");// 10036
	GloveSkin.AddItem("Leather");// 10009
	GloveSkin.AddItem("Spruce DDPAT");// 10010
	GloveSkin.AddItem("Crimson Weave");// 10016
	GloveSkin.AddItem("Lunar Weave");// 10013
	GloveSkin.AddItem("Diamondback");// 10040
	GloveSkin.AddItem("Convoy");// 10015
	GloveSkin.AddItem("Pandora's Box");// 10037
	GloveSkin.AddItem("Hedge Maze");// 10038
	GloveSkin.AddItem("Superconductor");// 10018
	GloveSkin.AddItem("Arid");// 10019
	GloveSkin.AddItem("Spearmint");// 10026
	GloveSkin.AddItem("Cool Mint");// 10028
	GloveSkin.AddItem("Boom");// 10027
	GloveSkin.AddItem("Eclipse");// 10024
	GloveSkin.AddItem("Crimson Kimono");// 10033
	GloveSkin.AddItem("Emerald Web");// 10034
	GloveSkin.AddItem("Foundation");// 10035
	GloveSkin.AddItem("Forest DDPAT");// 10030
	GloveGroup.PlaceLabledControl("Skin", this, &GloveSkin);
#pragma endregion Gloves

#pragma region Skins
	RifleMPGroup.SetPosition(135, -10);
	RifleMPGroup.SetSize(285, 315);
	RifleMPGroup.SetText("Rifle|MP");
	RegisterControl(&RifleMPGroup);

	//Rifles
	AK47.SetFileId("ak47_skin");
	AK47.SetText("000");
	RifleMPGroup.PlaceLabledControl("AK47 Skin", this, &AK47);

	M4A4.SetFileId("m4a4_skin");
	M4A4.SetText("000");
	RifleMPGroup.PlaceLabledControl("M4A4 Skin", this, &M4A4);

	M4A1.SetFileId("m4a1_skin");
	M4A1.SetText("000");
	RifleMPGroup.PlaceLabledControl("M4A1 Skin", this, &M4A1);

	Galil.SetFileId("galil_skin");
	Galil.SetText("000");
	RifleMPGroup.PlaceLabledControl("Galil Skin", this, &Galil);

	Famas.SetFileId("famas_skin");
	Famas.SetText("000");
	RifleMPGroup.PlaceLabledControl("Famas Skin", this, &Famas);

	AWP.SetFileId("awp_skin");
	AWP.SetText("000");
	RifleMPGroup.PlaceLabledControl("AWP Skin", this, &AWP);

	SSG08.SetFileId("ssg08_skin");
	SSG08.SetText("000");
	RifleMPGroup.PlaceLabledControl("SSG08 Skin", this, &SSG08);

	AUG.SetFileId("aug_skin");
	AUG.SetText("000");
	RifleMPGroup.PlaceLabledControl("AUG Skin", this, &AUG);

	Scar20.SetFileId("scar20_skin");
	Scar20.SetText("000");
	RifleMPGroup.PlaceLabledControl("Scar20 Skin", this, &Scar20);

	G3SG1.SetFileId("g3sg1_skin");
	G3SG1.SetText("000");
	RifleMPGroup.PlaceLabledControl("G3SG1 Skin", this, &G3SG1);

	SG553.SetFileId("sg553_skin");
	SG553.SetText("000");
	RifleMPGroup.PlaceLabledControl("SG553 Skin", this, &SG553);

	//MPs
	UMP.SetFileId("ump_skin");
	UMP.SetText("000");
	RifleMPGroup.PlaceLabledControl("UMP Skin", this, &UMP);

	MP9.SetFileId("mp9_skin");
	MP9.SetText("000");
	RifleMPGroup.PlaceLabledControl("MP9 Skin", this, &MP9);

	MP7.SetFileId("mp7_skin");
	MP7.SetText("000");
	RifleMPGroup.PlaceLabledControl("MP7 Skin", this, &MP7);

	P90.SetFileId("p90_skin");
	P90.SetText("000");
	RifleMPGroup.PlaceLabledControl("P90 Skin", this, &P90);

	PPBizon.SetFileId("ppbizon_skin");
	PPBizon.SetText("000");
	RifleMPGroup.PlaceLabledControl("PPBizon Skin", this, &PPBizon);

	MAC10.SetFileId("mac10_skin");
	MAC10.SetText("000");
	RifleMPGroup.PlaceLabledControl("MAC10 Skin", this, &MAC10);

	//Heavy
	HeavyGroup.SetPosition(135, 315);
	HeavyGroup.SetSize(285, 135);
	HeavyGroup.SetText("Heavy");
	RegisterControl(&HeavyGroup);

	Nova.SetFileId("nova_skin");
	Nova.SetText("000");
	HeavyGroup.PlaceLabledControl("Nova Skin", this, &Nova);

	XM1014.SetFileId("xm1014_skin");
	XM1014.SetText("000");
	HeavyGroup.PlaceLabledControl("XM1014 Skin", this, &XM1014);

	SawedOff.SetFileId("sawedoff_skin");
	SawedOff.SetText("000");
	HeavyGroup.PlaceLabledControl("SawedOff Skin", this, &SawedOff);

	MAG7.SetFileId("mag7_skin");
	MAG7.SetText("000");
	HeavyGroup.PlaceLabledControl("MAG7 Skin", this, &MAG7);

	M249.SetFileId("m249_skin");
	M249.SetText("000");
	HeavyGroup.PlaceLabledControl("M249 Skin", this, &M249);

	Negev.SetFileId("negev_skin");
	Negev.SetText("000");
	HeavyGroup.PlaceLabledControl("Negev Skin", this, &Negev);

	//Pistols
	PistolGroup.SetPosition(430, 205);
	PistolGroup.SetSize(345, 190);
	PistolGroup.SetText("Pistol");
	RegisterControl(&PistolGroup);

	Glock18.SetFileId("glock_skin");
	Glock18.SetText("000");
	PistolGroup.PlaceLabledControl("Glock-18 Skin", this, &Glock18);

	USP.SetFileId("usp_skin");
	USP.SetText("000");
	PistolGroup.PlaceLabledControl("USP Skin", this, &USP);

	P2000.SetFileId("p2000_skin");
	P2000.SetText("000");
	PistolGroup.PlaceLabledControl("P2000 Skin", this, &P2000);

	Deagle.SetFileId("deagle_skin");
	Deagle.SetText("000");
	PistolGroup.PlaceLabledControl("Deagle Skin", this, &Deagle);

	TEC9.SetFileId("tec9_skin");
	TEC9.SetText("000");
	PistolGroup.PlaceLabledControl("TEC9 Skin", this, &TEC9);

	FiveSeven.SetFileId("fiveseven_skin");
	FiveSeven.SetText("000");
	PistolGroup.PlaceLabledControl("FiveSeven Skin", this, &FiveSeven);

	P250.SetFileId("p250_skin");
	P250.SetText("000");
	PistolGroup.PlaceLabledControl("P250 Skin", this, &P250);

	Duals.SetFileId("duals_skin");
	Duals.SetText("000");
	PistolGroup.PlaceLabledControl("Duals Skin", this, &Duals);

	CZ75.SetFileId("cz75_skin");
	CZ75.SetText("000");
	PistolGroup.PlaceLabledControl("CZ75 Skin", this, &CZ75);

	R8.SetFileId("r8_skin");
	R8.SetText("000");
	PistolGroup.PlaceLabledControl("R8 Skin", this, &R8);

	SkinApply.SetText("Apply Changes");
	SkinApply.SetCallback(KnifeApplyCallbk);
	KnifeGroup.PlaceLabledControl("", this, &SkinApply);

#pragma endregion Skins
}

void CMiscTab::Setup()
{
	SetTitle("MISC");

#pragma region Other
	OtherGroup.SetPosition(420, 10);
	OtherGroup.SetSize(270, 450);
	OtherGroup.SetText("Misc");
	RegisterControl(&OtherGroup);

	/*OtherChatSpam.SetFileId("otr_spam");
	OtherChatSpam.AddItem("Off");
	OtherChatSpam.AddItem("Namestealer");
	OtherChatSpam.AddItem("Regular");
	OtherChatSpam.AddItem("Interwebz");
	OtherChatSpam.AddItem("Disperse Name");
	OtherGroup.PlaceLabledControl("Chat Spam", this, &OtherChatSpam);*/

	OtherClantag.SetFileId("otr_spam");
	OtherClantag.AddItem("Off");
	OtherClantag.AddItem("Owner");
	OtherClantag.AddItem("Complex");
	OtherClantag.AddItem("Admin");
	OtherClantag.AddItem("Invisible");
	OtherClantag.AddItem("Valve");
	OtherClantag.AddItem("Random");
	OtherClantag.AddItem("Time");


	OtherGroup.PlaceLabledControl("Clantag", this, &OtherClantag);

	OtherClantagspeed.SetFileId("otr_clantagspeed");
	OtherClantagspeed.SetBoundaries(0, 10);
	OtherClantagspeed.SetValue(1);
	OtherGroup.PlaceLabledControl("Clantag Speed", this, &OtherClantagspeed);

	OtherAirStuck.SetFileId("otr_astuck");
	OtherGroup.PlaceLabledControl("Meme Stuck", this, &OtherAirStuck);

	OtherSpectators.SetFileId("otr_speclist");
	OtherGroup.PlaceLabledControl("Spectators List", this, &OtherSpectators);

	OtherSafeMode.SetFileId("otr_safemode");
	OtherSafeMode.SetState(true);
	OtherGroup.PlaceLabledControl("Anti Untrusted", this, &OtherSafeMode);

	/*OtherWalkbot.SetFileId("otr_walkb");
	OtherGroup.PlaceLabledControl("WalkBot[Not HvHBot]", this, &OtherWalkbot);

	Edgeam.SetFileId("otr_walk");
	Edgeam.SetBoundaries(0, 15);
	Edgeam.SetValue(0);
	OtherGroup.PlaceLabledControl("WalkBot TurnSpeed", this, &Edgeam);

	Distance.SetFileId("otr_walkdis");
	Distance.SetBoundaries(0, 1000);
	Distance.SetValue(110);
	OtherGroup.PlaceLabledControl("WalkBot Distance", this, &Distance);*/

#pragma endregion other random options

#pragma region Strafing
	StrafingGroup.SetPosition(160, 100);
	StrafingGroup.SetSize(250, 170);
	StrafingGroup.SetText("Other");
	RegisterControl(&StrafingGroup);

	OtherAutoJump.SetFileId("otr_autojump");
	StrafingGroup.PlaceLabledControl("Auto Jump", this, &OtherAutoJump);

	OtherAutoStrafe.SetFileId("otr_strafe");
	OtherAutoStrafe.AddItem("Off");
	OtherAutoStrafe.AddItem("Legit");
	OtherAutoStrafe.AddItem("Rage");
	StrafingGroup.PlaceLabledControl("Auto Strafer", this, &OtherAutoStrafe);

	/*OtherCircle.SetFileId("otr_circle");
	StrafingGroup.PlaceLabledControl("Circle Strafe", this, &OtherCircle);

	OtherCircleKey.SetFileId("otr_circlkey");
	StrafingGroup.PlaceLabledControl("Circle Key", this, &OtherCircleKey);*/

#pragma endregion

#pragma region Rank
	RankGroup.SetPosition(160, 280);
	RankGroup.SetSize(250, 110);
	RankGroup.SetText("extra");
	RegisterControl(&RankGroup);

	WaterMark.SetFileId("otr_wtrmrk");
	WaterMark.SetState(true);
	RankGroup.PlaceLabledControl("Watermark", this, &WaterMark);

	Info.SetFileId("otr_info");
	Info.SetState(true);
	RankGroup.PlaceLabledControl("Info", this, &Info);

	OtherSlowmo.SetFileId("otr_slowwalk");
	OtherSlowmo.SetState(true);
	RankGroup.PlaceLabledControl("FakeWalk", this, &OtherSlowmo);

	OtherLBY.SetFileId("otr_lbyind");
	RankGroup.PlaceLabledControl("LBY indicator", this, &OtherLBY);
#pragma endregion
}

void Menu::SetupMenu()
{
	Window.Setup();

	GUI.RegisterWindow(&Window);
	GUI.BindWindow(VK_INSERT, &Window);
}

void Menu::DoUIFrame()
{
	// General Processing

	// If the "all filter is selected tick all the others
	if (Window.VisualsTab.FiltersAll.GetState())
	{
		Window.VisualsTab.FiltersC4.SetState(true);
		Window.VisualsTab.FiltersChickens.SetState(true);
		Window.VisualsTab.FiltersPlayers.SetState(true);
		Window.VisualsTab.FiltersWeapons.SetState(true);
	}

	GUI.Update();
	GUI.Draw();

}