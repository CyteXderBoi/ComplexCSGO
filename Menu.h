#pragma once

#include "GUI.h"
#include "Controls.h"

class CRageBotTab : public CTab
{
public:
	void Setup();

	// Master Switch
	CLabel ActiveLabel;
	CCheckBox2 Active;

	// Aimbot Settings
	CGroupBox2 AimbotGroup;
	CCheckBox2 AimbotEnable;
	CCheckBox2 AimbotAutoFire;
	CSlider2  AimbotFov;

	// Slient Aim Selection
	CComboBox2 AimbotSlientSelection;

	CCheckBox2 AimbotAutoPistol;
	CCheckBox2 AimbotAutoRevolver;
	CCheckBox2 AimbotAimStep;
	CCheckBox2 AimbotKeyPress;
	CKeyBind  AimbotKeyBind;
	CKeyBind  AimbotStopKey;

	// Target Selection Settings
	CGroupBox2 TargetGroup;
	CComboBox2 TargetSelection;
	CCheckBox2 TargetFriendlyFire;
	CComboBox2 TargetHitbox;
	CComboBox2 TargetHitscan;
	CCheckBox2 TargetMultipoint;
	CSlider2   TargetPointscale;

	// Accuracy Settings
	CGroupBox2 AccuracyGroup;
	CCheckBox2 AccuracyRecoil;
	CCheckBox2 AccuracySpread;
	CCheckBox2 AccuracyAutoWall;
	CSlider2  AccuracyMinimumDamage;
	CCheckBox2 AccuracyAutoStop;
	CCheckBox2 AccuracyAutoCrouch;
	CCheckBox2 AccuracyAutoScope;
	CSlider2  AccuracyHitchance;
	 
	//boxgroup
	CGroupBox2 boxGroup;

};

class CHvHTab : public CTab
{
public:
	void Setup();

	// Master Switch
	CLabel ActiveLabel;
	CCheckBox2 Active;
	CGroupBox2 AccuracyGroup;

	// Engine Hake
	CCheckBox2 PositionAccuracy;
	CCheckBox2 PerfectAccuracy;
	CCheckBox2 AccuracyPrediction;
	CCheckBox2 AccuracyLagCompensate;
	CCheckBox2 AccuracyPositionAdjustment;

	// Anti-Aim Settings
	CGroupBox2 AntiAimGroup;
	CCheckBox2 AntiAimEnable;

	CComboBox2 AntiAimPitch;
	CComboBox2 AntiAimYaw;
	CComboBox2 AntiAimSpinYaw;
	CComboBox2 AntiAimYawFake;

	CCheckBox2 AntiAimJitterLBY;

	CSlider2  RealAntiAimYawSlider;

	// Fake Lag Settings
	CGroupBox2 FakeLagGroup;
	CCheckBox2 FakeLagEnable;
	CSlider2  FakeLagChoke;
	CSlider2	  FakeLagSend;
	CCheckBox2 ChokeRandomize;
	CCheckBox2 SendRandomize;

	// CSlider2	  AntiAimOffset;
	CSlider2  Slowmo;
	CCheckBox2 AntiAimKnife;
	CCheckBox2 AntiAimTarget;
	CCheckBox2 AntiAimEdge;
	CSlider2  AntiAimOffsetPitch;

	CCheckBox2 AccuracyResolverYaw;
	CComboBox2 AccuracyResolver;
	CComboBox2 AccuracyBrute;
	CSlider2  AccuracySmart;
};


class CLegitBotTab : public CTab
{
public:
	void Setup();

	// Master Switch
	CLabel ActiveLabel;
	CCheckBox2 Active;

	// Aimbot Settings
	CGroupBox2 AimbotGroup;
	CCheckBox2 AimbotEnable;
	CCheckBox2 AimbotAutoFire;
	CCheckBox2 AimbotFriendlyFire;
	CCheckBox2 AimbotKeyPress;
	CKeyBind  AimbotKeyBind;
	CCheckBox2 AimbotAutoPistol;
	CSlider2   AimbotInaccuracy;
	CKeyBind  AimbotDisableVis;

	// Main
	CGroupBox2 TriggerGroup;
	CCheckBox2 TriggerEnable;
	CCheckBox2 TriggerKeyPress;
	CKeyBind  TriggerKeyBind;
	CSlider2   TriggerDelay;

	// Main
	CGroupBox2 WeaponMainGroup;
	CSlider2   WeaponMainSpeed;
	CSlider2   WeaponMainFoV;
	CCheckBox2 WeaponMainRecoil;
	CCheckBox2 WeaponMainPSilent;
	CSlider2   WeaponMainInacc;
	CComboBox2 WeaponMainHitbox;

	// Pistol
	CGroupBox2 WeaponPistGroup;
	CSlider2   WeaponPistSpeed;
	CSlider2   WeaponPistFoV;
	CCheckBox2 WeaponPistRecoil;
	CCheckBox2 WeaponPistPSilent;
	CSlider2   WeaponPistInacc;
	CComboBox2 WeaponPistHitbox;

	// Sniper
	CGroupBox2 WeaponSnipGroup;
	CSlider2   WeaponSnipSpeed;
	CSlider2   WeaponSnipFoV;
	CCheckBox2 WeaponSnipRecoil;
	CCheckBox2 WeaponSnipPSilent;
	CSlider2   WeaponSnipInacc;
	CComboBox2 WeaponSnipHitbox;
};

class CVisualTab : public CTab
{
public:
	void Setup();

	// Master Switch
	CLabel ActiveLabel;
	CCheckBox2 Active;

	// Options Settings
	CGroupBox2 OptionsGroup;
	CCheckBox2 OptionsBox;
	CComboBox2 BoxDesign;
	CCheckBox2 OptionsName;
	CCheckBox2 OptionHealthEnable;
	CComboBox2 OptionsHealth;
	CCheckBox2 OptionsArmur;
	CCheckBox2 OptionsWeapon;
	CCheckBox2 OptionsInfo;
	CCheckBox2 OptionsHelmet;
	CCheckBox2 OptionsKit;
	CCheckBox2 OptionsDefuse;
	//CCheckBox2 OptionsGlow;
	CComboBox2 OptionsChams;
	CCheckBox2 OptionsSkeleton;
	CCheckBox2 OptionsAimSpot;
	CCheckBox2 OptionsCompRank;

	// Skeleton ESP Colour
	CGroupBox2 ESPColorSkeleton;
	CSlider2   RSkele;
	CSlider2   GSkele;
	CSlider2   BSkele;
	CSlider2   ASkele;

	// Box ESP Colour
	CGroupBox2 ESPColorBox;
	CSlider2   RBoxCT;
	CSlider2   GBoxCT;
	CSlider2   BBoxCT;
	CSlider2   ABoxCT;
	CSlider2   RBoxCTV;
	CSlider2   GBoxCTV;
	CSlider2   BBoxCTV;
	CSlider2   ABoxCTV;

	// Filters Settings
	CGroupBox2 FiltersGroup;
	CCheckBox2 FiltersAll;
	CCheckBox2 FiltersPlayers;
	CCheckBox2 FiltersEnemiesOnly;
	CCheckBox2 FiltersWeapons;
	CCheckBox2 FiltersChickens;
	CCheckBox2 FiltersC4;
	CSlider2  Filterspos;
	// Other Settings
	CGroupBox2 OtherGroup;
	CComboBox2 OtherCrosshair;
	CComboBox2 OtherRecoilCrosshair;
	CCheckBox2 OtherHitlerCrosshair;
	CCheckBox2 OtherHitmarker;
	CCheckBox2 OtherRadar;
	CCheckBox2 OtherNoVisualRecoil;
	CCheckBox2 OtherNoSky; 
	CCheckBox2 OtherNoFlash; 
	CCheckBox2 OtherNoSmoke;
	CCheckBox2 OtherAsusWalls;
	CComboBox2 OtherNoHands;
	CCheckBox2 OtherThirdperson;
	CSlider2 OtherThirdpersonRange;
	CKeyBind OtherThirdpersonKey;
	CSlider2  OtherViewmodelFOV;
	CSlider2  OtherFOV;
	CCheckBox2 CustomSky;
	CComboBox2 OtherNoScope;
	CCheckBox2 OptionsFill;
	CCheckBox2 LBY;
	CCheckBox2 SnapLines;
	CCheckBox2 NightMode;
	CCheckBox2 BulletTrace;
	CSlider2   TraceLength;

};

class CMiscTab : public CTab
{
public:
	void Setup();

	// Other Settings
	CGroupBox2 OtherGroup;
	CCheckBox2 OtherSafeMode;
	CComboBox2 OtherChatSpam;
	CCheckBox2 OtherTeamChat;
	CSlider2	  OtherChatDelay;
	CKeyBind  OtherAirStuck;
	CKeyBind  OtherLagSwitch;
	CCheckBox2 OtherSpectators;
	
	//CCheckBox2 OtherAutoAccept;
/*	CCheckBox2 OtherWalkbot;
	CSlider2   Distance;
	CSlider2   Edgeam;*/

	// Clan Tag Settings
	CComboBox2 OtherClantag;
	CSlider2  OtherClantagspeed;


	//CCheckBox2 FakeLagWhileShooting;

	// Teleport shit cause we're cool
	CGroupBox2 TeleportGroup;
	CCheckBox2 TeleportEnable;
	CKeyBind  TeleportKey;

	// Strafing
	CGroupBox2 StrafingGroup;
	CCheckBox2 OtherAutoJump;
	CComboBox2 OtherAutoStrafe;
	CSlider2   OtherStrafeSpeed;
	CCheckBox OtherForwardMove;
	CCheckBox2 OtherEdgeJump;
	CCheckBox2 OtherCircle;
	CKeyBind OtherCircleKey;


	// Rank Reveal
	CGroupBox2 RankGroup;
	CCheckBox2 WaterMark;
	CCheckBox2 Info;
	CCheckBox2 OtherSlowmo;
	CCheckBox2 OtherLBY;
};

class CSkinTab : public CTab
{
public:
	void Setup();

	//Skin Enable|Apply
	CCheckBox SkinEnable;
	CButton   SkinApply;

	//Knife
	CGroupBox3 KnifeGroup;
	CComboBox KnifeModel;
	CTextField KnifeSkin;

	//Glove
	CGroupBox3 GloveGroup;
	CComboBox GloveModel;
	CComboBox GloveSkin;

	//Skin
	CGroupBox3 RifleMPGroup;
	CGroupBox3 HeavyGroup;
	CGroupBox3 PistolGroup;

	//Pistols
	CTextField Glock18;
	CTextField USP;
	CTextField Deagle;
	CTextField R8;
	CTextField Duals;
	CTextField TEC9;
	CTextField FiveSeven;
	CTextField CZ75;
	CTextField P2000;
	CTextField P250;

	//Rifles
	CTextField AK47;
	CTextField M4A1;
	CTextField M4A4;
	CTextField Famas;
	CTextField Galil;
	CTextField AWP;
	CTextField SSG08;
	CTextField AUG;
	CTextField Scar20;
	CTextField G3SG1;
	CTextField SG553;

	//MPs
	CTextField UMP;
	CTextField MP9;
	CTextField P90;
	CTextField MP7;
	CTextField PPBizon;
	CTextField MAC10;

	//Shotguns
	CTextField Nova;
	CTextField XM1014;
	CTextField SawedOff;
	CTextField MAG7;

	//MGs
	CTextField M249;
	CTextField Negev;
};

// Elements that can be placed anywhere in any tabs, should be decleared here.
class ComplexMenu : public CWindow
{
public:
	void Setup();

	CHvHTab HvHTab;
	CRageBotTab RageBotTab;
	CLegitBotTab LegitBotTab;
	CVisualTab VisualsTab;
	CMiscTab MiscTab;
	CSkinTab SkinTab;

	CButton SaveButton;
	CButton LoadButton;
	CButton UnloadButton;
	CComboBox2 ConfigBox;
};

namespace Menu
{
	void SetupMenu();
	void DoUIFrame();

	extern ComplexMenu Window;
};