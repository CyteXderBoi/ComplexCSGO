#include "Hooks.h"
#include "Hacks.h"
#include "Chams.h"
#include "Menu.h"

#include "Interfaces.h"
#include "RenderManager.h"
#include "MiscHacks.h"
#include "CRC32.h"
#include "Resolver.h"
#include "IPrediction.h"
#include "ResolverMeme.h"

#include <math.h>
#include <cmath>
#include <ctime>

#include <stdio.h>
#include <time.h>
#include <iostream>



#define RandomInt(nMin, nMax) (rand() % (nMax - nMin + 1) + nMin)
Vector LastAngleAA;
#define M_PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679821480865132823066470938446095505822317253594081284811174502841027019385211055596446229489549303819644288109756659334461284756482337867831652712019091456485669234603486104543266482133936072602491412737245870066063155881748815209209628292540917153643678925903600113305305488204665213841469519415116094330572703657595919530921861173819326117931051185480744623799627495673518857527248912279381830119491298336733624406566430860213949463952247371907021798609437027705392171762931767523846748184676694051320005681271452635608277857713427577896091736371787214684409012249534301465495853710507922796892589235420199561121290219608640344181598136297747713099605187072113499999983729780499510597317328160963185950244594553469083026425223082533446850352619311881710100031378387528865875332083814206171776691473035982534904287554687311595628638823537875937519577818577805321712268066130019278766111959092164201989380952572010654858632788659361533818279682303019520353018529689957736225994138912497217752834791315155748572424541506959508295331168617278558890750983817546374649393192550604009277016711390098488240128583616035637076601047101819429555961989467678374494482553797747268471040475346462080466842590694912933136770289891521047521620569660240580
inline float DegreesToRadians(float Angle) { return Angle * M_PI / 180.0f; }

// Funtion Typedefs
typedef void(__thiscall* DrawModelEx_)(void*, void*, void*, const ModelRenderInfo_t&, matrix3x4*);
typedef void(__thiscall* PaintTraverse_)(PVOID, unsigned int, bool, bool);
typedef bool(__thiscall* InPrediction_)(PVOID);
typedef void(__stdcall *FrameStageNotifyFn)(ClientFrameStage_t);
typedef void(__thiscall* RenderViewFn)(void*, CViewSetup&, CViewSetup&, int, int);

using OverrideViewFn = void(__fastcall*)(void*, void*, CViewSetup*);
typedef float(__stdcall *oGetViewModelFOV)();

// Function Pointers to the originals
PaintTraverse_ oPaintTraverse;
DrawModelEx_ oDrawModelExecute;
FrameStageNotifyFn oFrameStageNotify;
OverrideViewFn oOverrideView;
RenderViewFn oRenderView;

// Hook function prototypes
void __fastcall PaintTraverse_Hooked(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce);
bool __stdcall Hooked_InPrediction();
void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld);
bool __stdcall CreateMoveClient_Hooked(/*void* self, int edx,*/ float frametime, CUserCmd* pCmd);
void  __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage);
void __fastcall Hooked_OverrideView(void* ecx, void* edx, CViewSetup* pSetup);
float __stdcall GGetViewModelFOV();
void __fastcall Hooked_RenderView(void* ecx, void* edx, CViewSetup &setup, CViewSetup &hudViewSetup, int nClearFlags, int whatToDraw);

// VMT Managers
namespace Hooks
{;
	// VMT Managers
	Utilities::Memory::VMTManager VMTPanel; // Hooking drawing functions
	Utilities::Memory::VMTManager VMTClient; // Maybe CreateMove
	Utilities::Memory::VMTManager VMTClientMode; // CreateMove for functionality
	Utilities::Memory::VMTManager VMTModelRender; // DrawModelEx for chams
	Utilities::Memory::VMTManager VMTPrediction; // InPrediction for no vis recoil
	Utilities::Memory::VMTManager VMTPlaySound; // Autoaccept 
	Utilities::Memory::VMTManager VMTRenderView;
};

// Undo our hooks
void Hooks::UndoHooks()
{
	VMTPanel.UnHook();
	VMTPrediction.UnHook();
	VMTModelRender.UnHook();
	VMTClientMode.UnHook();
	//VMTClient.RestoreOriginal();
}

// Initialise all our hooks
void Hooks::Initialise()
{
	// Panel hooks for drawing to the screen via surface functions
	VMTPanel.Initialise((DWORD*)Interfaces::Panels);
	oPaintTraverse = (PaintTraverse_)VMTPanel.HookMethod((DWORD)&PaintTraverse_Hooked, Offsets::VMT::Panel_PaintTraverse);
	//Utilities::Log("Paint Traverse Hooked");

	// No Visual Recoi	l
	VMTPrediction.Initialise((DWORD*)Interfaces::Prediction);
	VMTPrediction.HookMethod((DWORD)&Hooked_InPrediction, 14);
	//Utilities::Log("InPrediction Hooked");

	// Chams
	VMTModelRender.Initialise((DWORD*)Interfaces::ModelRender);
	oDrawModelExecute = (DrawModelEx_)VMTModelRender.HookMethod((DWORD)&Hooked_DrawModelExecute, Offsets::VMT::ModelRender_DrawModelExecute);
	//Utilities::Log("DrawModelExecute Hooked");

	// Setup ClientMode Hooks
	VMTClientMode.Initialise((DWORD*)Interfaces::ClientMode);
	VMTClientMode.HookMethod((DWORD)CreateMoveClient_Hooked, 24);

	oOverrideView = (OverrideViewFn)VMTClientMode.HookMethod((DWORD)&Hooked_OverrideView, 18);
	VMTClientMode.HookMethod((DWORD)&GGetViewModelFOV, 35);

	// Setup client hooks
	VMTClient.Initialise((DWORD*)Interfaces::Client);
	oFrameStageNotify = (FrameStageNotifyFn)VMTClient.HookMethod((DWORD)&Hooked_FrameStageNotify, 36);

}

void MovementCorrection(CUserCmd* pCmd)
{

}

//---------------------------------------------------------------------------------------------------------
//                                         Hooked Functions
//---------------------------------------------------------------------------------------------------------

// Animated ClanTag Function
void SetClanTag(const char* tag, const char* name)
{
	static auto pSetClanTag = reinterpret_cast<void(__fastcall*)(const char*, const char*)>(((DWORD)Utilities::Memory::FindPattern("engine.dll", (PBYTE)"\x53\x56\x57\x8B\xDA\x8B\xF9\xFF\x15\x00\x00\x00\x00\x6A\x24\x8B\xC8\x8B\x30", "xxxxxxxxx????xxxxxx")));
	pSetClanTag(tag, name);
}

// Blank Clantag
void NoClantag()
{
	SetClanTag("", "");
}

// Clantag Functions
void ClanTag()
{
	int speed = Menu::Window.MiscTab.OtherClantagspeed.GetValue();
	static int counter = 0;
	switch (Menu::Window.MiscTab.OtherClantag.GetIndex())
	{
	case 0:
		// No 
		break;
	case 1:
	{
		static int motion = 0;
		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * speed;

		if (counter % 48 == 0)
			motion++;
		int value = ServerTime % 4;
		switch (value) {
		case 0:SetClanTag("[Owner]", "[Owner]"); break;
		case 1:SetClanTag("[Owner]", "[Owner]"); break;
		case 2:SetClanTag("[Owner]", "[Owner]"); break;
		}
		counter++;
	}
	break;
	case 2:
	{
		static int motion = 0;
		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * speed;

		if (counter % 48 == 0)
			motion++;
		int value = ServerTime % 42;
		switch (value) {
		case 0: SetClanTag("/      ", "Complex"); break;
		case 1: SetClanTag("-      ", "Complex"); break;
		case 2: SetClanTag("C      ", "Complex"); break;
		case 3: SetClanTag("C/     ", "Complex"); break;
		case 4: SetClanTag("C-     ", "Complex"); break;
		case 5: SetClanTag("Co     ", "Complex"); break;
		case 6: SetClanTag("Co/    ", "Complex"); break;
		case 7: SetClanTag("Co-    ", "Complex"); break;
		case 8: SetClanTag("Com    ", "Complex"); break;
		case 9: SetClanTag("Com/   ", "Complex"); break;
		case 10:SetClanTag("Com-   ", "Complex"); break;
		case 11:SetClanTag("Comp   ", "Complex"); break;
		case 12:SetClanTag("Comp/  ", "Complex"); break;
		case 13:SetClanTag("Comp-  ", "Complex"); break;
		case 14:SetClanTag("Compl  ", "Complex"); break;
		case 15:SetClanTag("Compl/ ", "Complex"); break;
		case 16:SetClanTag("Compl- ", "Complex"); break;
		case 17:SetClanTag("Comple ", "Complex"); break;
		case 18:SetClanTag("Comple/", "Complex"); break;
		case 19:SetClanTag("Comple-", "Complex"); break;
		case 20:SetClanTag("Complex", "Complex"); break;
		case 21:SetClanTag("Comple-", "Complex"); break;
		case 22:SetClanTag("Comple/", "Complex"); break;
		case 23:SetClanTag("Comple ", "Complex"); break;
		case 24:SetClanTag("Compl- ", "Complex"); break;
		case 25:SetClanTag("Compl/ ", "Complex"); break;
		case 26:SetClanTag("Compl  ", "Complex"); break;
		case 27:SetClanTag("Comp-  ", "Complex"); break;
		case 28:SetClanTag("Comp/  ", "Complex"); break;
		case 29:SetClanTag("Comp   ", "Complex"); break;
		case 30:SetClanTag("Com-   ", "Complex"); break;
		case 31:SetClanTag("Com/   ", "Complex"); break;
		case 32:SetClanTag("Com    ", "Complex"); break;
		case 33:SetClanTag("Co-    ", "Complex"); break;
		case 34:SetClanTag("Co/    ", "Complex"); break;
		case 35:SetClanTag("Co     ", "Complex"); break;
		case 36:SetClanTag("C-     ", "Complex"); break;
		case 37:SetClanTag("C/     ", "Complex"); break;
		case 38:SetClanTag("C      ", "Complex"); break;
		case 39:SetClanTag("-      ", "Complex"); break;
		case 40:SetClanTag("/      ", "Complex"); break;
		case 41:SetClanTag("       ", "Complex"); break;
		}
		counter++;
	}
	break;
	case 3:
	{
		static int motion = 0;
		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * speed;

		if (counter % 48 == 0)
			motion++;
		int value = ServerTime % 3;
		switch (value) {
		case 0:SetClanTag("[Admin]", "[Admin]"); break;
		case 1:SetClanTag("[Admin]", "[Admin]"); break;
		}
		counter++;
	}
	break;
	case 4:
		// stainless
		SetClanTag("\r", "\r");
		break;
	case 5:
		SetClanTag("[VALV\xE1\xB4\xB1]", "Valve");
		break;
	case 6:
	{
		static int motion = 0;
		int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * speed;

		if (counter % 48 == 0)
			motion++;
		int value = ServerTime % 7;
		switch (value) {
		case 0:SetClanTag("[Owner]", "pasteware"); break;
		case 1:SetClanTag("[Admin]", "pasteware"); break;
		case 2:SetClanTag("[Moderator]", "pasteware"); break;
		case 3:SetClanTag("[VIP]", "pasteware"); break;
		case 4:SetClanTag("[Newbie]", "pasteware"); break;
		case 5:SetClanTag("[Banned]", "pasteware"); break;
		case 6:SetClanTag("[Dead]", "pasteware"); break;
		}
		counter++;
	}
	break;
	case 7:
		time_t now = time(0);
		char timestamp[10] = "";
		strftime(timestamp, 10, "%H:%M:%S", localtime(&now));
		SetClanTag(timestamp, "Time");
		break;

	}
}

// Rank Revealer
void MsgFunc_ServerRankRevealAll()
{
	using MsgFunc_ServerRankRevealAllFn = bool(__cdecl*)(float*);
	static MsgFunc_ServerRankRevealAllFn MsgFunc_ServerRankRevealAll = reinterpret_cast<MsgFunc_ServerRankRevealAllFn>((PDWORD)Utilities::Memory::FindPattern("client.dll", (PBYTE)"\x55\x8B\xEC\x8B\x0D\x00\x00\x00\x00\x68\x00\x00\x00\x00", "xxxxx????x????"));

	float fArray[3];
	fArray[0] = 0.f;
	fArray[1] = 0.f;
	fArray[2] = 0.f;

	MsgFunc_ServerRankRevealAll(fArray);
}

BYTE bMoveData[0x200];

// Movement Prediction
void Prediction(CUserCmd* pCmd, IClientEntity* LocalPlayer)
{
	if (Interfaces::MoveHelper && Menu::Window.RageBotTab.AimbotEnable.GetState() && Menu::Window.HvHTab.AccuracyPrediction.GetState() && LocalPlayer->IsAlive())
	{
		float curtime = Interfaces::Globals->curtime;
		float frametime = Interfaces::Globals->frametime;
		int iFlags = LocalPlayer->GetFlags();

		Interfaces::Globals->curtime = (float)LocalPlayer->GetTickBase() * Interfaces::Globals->interval_per_tick;
		Interfaces::Globals->frametime = Interfaces::Globals->interval_per_tick;

		Interfaces::MoveHelper->SetHost(LocalPlayer);

		Interfaces::GamePrediction->SetupMove(LocalPlayer, pCmd, nullptr, bMoveData);
		Interfaces::GameMovement->ProcessMovement(LocalPlayer, bMoveData);
		Interfaces::GamePrediction->FinishMove(LocalPlayer, pCmd, bMoveData);

		Interfaces::MoveHelper->SetHost(0);

		Interfaces::Globals->curtime = curtime;
		Interfaces::Globals->frametime = frametime;
		*LocalPlayer->GetPointerFlags() = iFlags;
	}
}

// Create moves
bool __stdcall CreateMoveClient_Hooked(float frametime, CUserCmd* pCmd)
{
	if (!pCmd->command_number)
		return true;



	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{

		PVOID pebp;
		__asm mov pebp, ebp;
		bool* pbSendPacket = (bool*)(*(DWORD*)pebp - 0x1C);
		bool& bSendPacket = *pbSendPacket;
		CClientState* pClient;
		INetChannel* pNet;
		CInput* pInput;

		if (Menu::Window.MiscTab.OtherClantag.GetIndex() > 0)
			ClanTag();

		//	CUserCmd* cmdlist = *(CUserCmd**)((DWORD)Interfaces::pInput + 0xEC);
		//	CUserCmd* pCmd = &cmdlist[sequence_number % 150];


		// Backup for safety
		Vector origView = pCmd->viewangles;
		Vector viewforward, viewright, viewup, aimforward, aimright, aimup;
		Vector qAimAngles;
		qAimAngles.Init(0.0f, pCmd->viewangles.y, 0.0f);
		AngleVectors(qAimAngles, &viewforward, &viewright, &viewup);

		// Do da hacks
		IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && pLocal && pLocal->IsAlive())
			Hacks::MoveHacks(pCmd, bSendPacket);

		// Movement Fix
		qAimAngles.Init(0.0f, GetAutostrafeView().y, 0.0f);
		AngleVectors(qAimAngles, &viewforward, &viewright, &viewup);
		qAimAngles.Init(0.0f, pCmd->viewangles.y, 0.0f);
		AngleVectors(qAimAngles, &aimforward, &aimright, &aimup);
		Vector vForwardNorm;		Normalize(viewforward, vForwardNorm);
		Vector vRightNorm;			Normalize(viewright, vRightNorm);
		Vector vUpNorm;				Normalize(viewup, vUpNorm);

		// Movement Prediction
		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && pLocal)
		{
			if (pLocal->IsAlive())
			{
				Prediction(pCmd, pLocal);
			}
		}

		// Original shit for movement correction
		float forward = pCmd->forwardmove;
		float right = pCmd->sidemove;
		float up = pCmd->upmove;
		if (forward > 450) forward = 450;
		if (right > 450) right = 450;
		if (up > 450) up = 450;
		if (forward < -450) forward = -450;
		if (right < -450) right = -450;
		if (up < -450) up = -450;
		pCmd->forwardmove = DotProduct(forward * vForwardNorm, aimforward) + DotProduct(right * vRightNorm, aimforward) + DotProduct(up * vUpNorm, aimforward);
		pCmd->sidemove = DotProduct(forward * vForwardNorm, aimright) + DotProduct(right * vRightNorm, aimright) + DotProduct(up * vUpNorm, aimright);
		pCmd->upmove = DotProduct(forward * vForwardNorm, aimup) + DotProduct(right * vRightNorm, aimup) + DotProduct(up * vUpNorm, aimup);

		// Angle normalisation
		if (Menu::Window.MiscTab.OtherSafeMode.GetState())
		{
			GameUtils::NormaliseViewAngle(pCmd->viewangles);

			if (pCmd->viewangles.z != 0.0f)
			{
				pCmd->viewangles.z = 0.00;
			}

			if (pCmd->viewangles.x < -89 || pCmd->viewangles.x > 89 || pCmd->viewangles.y < -180 || pCmd->viewangles.y > 180)
			{
				GameUtils::NormaliseViewAngle(pCmd->viewangles);
				Beep(750, 800);
				if (pCmd->viewangles.x < -89 || pCmd->viewangles.x > 89 || pCmd->viewangles.y < -180 || pCmd->viewangles.y > 180)
				{
					pCmd->viewangles = origView;
					pCmd->sidemove = right;
					pCmd->forwardmove = forward;
				}
			}
		}

		if (pCmd->viewangles.x > 90)
		{
			pCmd->forwardmove = -pCmd->forwardmove;
		}

		if (pCmd->viewangles.x < -90)
		{
			pCmd->forwardmove = -pCmd->forwardmove;
		}

		if (bSendPacket)
			LastAngleAA = pCmd->viewangles;
	}

	return false;
}


// Paint Traverse Hooked function
void __fastcall PaintTraverse_Hooked(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce)
{
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (Menu::Window.VisualsTab.OtherNoScope.GetIndex() && pLocal && pLocal->IsScoped()) {
		if (!strcmp("HudZoom", Interfaces::Panels->GetName(vguiPanel))) {
			return;
		}
	}

	oPaintTraverse(pPanels, vguiPanel, forceRepaint, allowForce);

	static unsigned int FocusOverlayPanel = 0;
	static bool FoundPanel = false;

	if (!FoundPanel)
	{
		PCHAR szPanelName = (PCHAR)Interfaces::Panels->GetName(vguiPanel);
		if (strstr(szPanelName, "MatSystemTopPanel"))
		{
			FocusOverlayPanel = vguiPanel;
			FoundPanel = true;
		}
	}
	else if (FocusOverlayPanel == vguiPanel)
	{	
		if (Menu::Window.MiscTab.WaterMark.GetState() == true)
		{
			static float rainbow;
			rainbow += 0.005f;
			if (rainbow > 1.f) rainbow = 0.f;



			Render::Text(10, 10, Color::FromHSB(rainbow, 1.f, 1.f), Render::Fonts::Menu, "Complex");
		}
		if (!FoundPanel)
		{
			PCHAR szPanelName = (PCHAR)Interfaces::Panels->GetName(vguiPanel);
			if (strstr(szPanelName, "MatSystemTopPanel"))
			{
				FocusOverlayPanel = vguiPanel;
				FoundPanel = true;
			}
		}
		else if (FocusOverlayPanel == vguiPanel)
		{
			if (Menu::Window.MiscTab.Info.GetState() == true)
			{
				if (Menu::Window.MiscTab.OtherSafeMode.GetState() == true)
				{
					Render::Text(10, 30, Color(33, 255, 0, 255), Render::Fonts::Slider, "Anti-Untrusted: On");
				}
				else
				{
					Render::Text(10, 30, Color(255, 4, 0, 255), Render::Fonts::Slider, "Anti-Untrusted: Off");
				}

				if (Menu::Window.HvHTab.AntiAimEnable.GetState() == true)
				{
					Render::Text(10, 40, Color(242, 255, 0, 255), Render::Fonts::Slider, "Anti Aim: On");
				}
				else
				{
					Render::Text(10, 40, Color(255, 255, 255, 255), Render::Fonts::Slider, "Anti Aim: Off");
				}
				if (Menu::Window.HvHTab.AntiAimPitch.GetIndex() == 0)
				{
					Render::Text(10, 50, Color(255, 255, 255, 255), Render::Fonts::Slider, "Pitch: None");
				}

				if (Menu::Window.HvHTab.AntiAimPitch.GetIndex() == 1)
				{
					Render::Text(10, 50, Color(255, 4, 0, 255), Render::Fonts::Slider, "Pitch: Down");
				}
				if (Menu::Window.HvHTab.AntiAimPitch.GetIndex() == 2)
				{
					Render::Text(10, 50, Color(255, 4, 0, 255), Render::Fonts::Slider, "Pitch: Half Down");
				}
				if (Menu::Window.HvHTab.AntiAimPitch.GetIndex() == 3)
				{
					Render::Text(10, 50, Color(255, 4, 0, 255), Render::Fonts::Slider, "Pitch: Jitter");
				}
				if (Menu::Window.HvHTab.AntiAimPitch.GetIndex() == 4)
				{
					Render::Text(10, 50, Color(255, 4, 0, 255), Render::Fonts::Slider, "Pitch: Static");
				}
				if (Menu::Window.HvHTab.AntiAimPitch.GetIndex() == 5)
				{
					Render::Text(10, 50, Color(255, 4, 0, 255), Render::Fonts::Slider, "Pitch: Fake Down");
				}
				if (Menu::Window.HvHTab.AntiAimPitch.GetIndex() == 6)
				{
					Render::Text(10, 50, Color(255, 4, 0, 255), Render::Fonts::Slider, "Pitch: Lisp Down");
				}
				if (Menu::Window.HvHTab.AntiAimPitch.GetIndex() == 7)
				{
					Render::Text(10, 50, Color(255, 4, 0, 255), Render::Fonts::Slider, "Pitch: Lisp Up");
				}
				if (Menu::Window.HvHTab.AntiAimPitch.GetIndex() == 8)
				{
					Render::Text(10, 50, Color(255, 4, 0, 255), Render::Fonts::Slider, "Pitch: NS UT");
				}
				if (Menu::Window.HvHTab.AntiAimPitch.GetIndex() == 9)
				{
					Render::Text(10, 50, Color(255, 4, 0, 255), Render::Fonts::Slider, "Pitch: FakeZero");
				}
				if (Menu::Window.HvHTab.AntiAimPitch.GetIndex() == 10)
				{
					Render::Text(10, 50, Color(255, 4, 0, 255), Render::Fonts::Slider, "Pitch: Overflow-Down");
				}
				if (Menu::Window.HvHTab.AntiAimPitch.GetIndex() == 11)
				{
					Render::Text(10, 50, Color(255, 4, 0, 255), Render::Fonts::Slider, "Pitch: Overflow-Up");
				}
				if (Menu::Window.HvHTab.AntiAimPitch.GetIndex() == 12)
				{
					Render::Text(10, 50, Color(255, 4, 0, 255), Render::Fonts::Slider, "Pitch: Fake-Overflow");
				}
				if (Menu::Window.HvHTab.AntiAimPitch.GetIndex() == 13)
				{
					Render::Text(10, 50, Color(255, 4, 0, 255), Render::Fonts::Slider, "Pitch: StaticJitter UT");
				}

				if (Menu::Window.HvHTab.AntiAimYaw.GetIndex() == 0)
				{
					Render::Text(10, 60, Color(255, 255, 255, 255), Render::Fonts::Slider, "Yaw: None");
				}
				if (Menu::Window.HvHTab.AntiAimYaw.GetIndex() == 1)
				{
					Render::Text(10, 60, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw: Fake Edge");
				}
				if (Menu::Window.HvHTab.AntiAimYaw.GetIndex() == 2)
				{
					Render::Text(10, 60, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw: Fake Sideways");
				}
				if (Menu::Window.HvHTab.AntiAimYaw.GetIndex() == 3)
				{
					Render::Text(10, 60, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw: Fake Static");
				}
				if (Menu::Window.HvHTab.AntiAimYaw.GetIndex() == 4)
				{
					Render::Text(10, 60, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw: T Fake");
				}
				if (Menu::Window.HvHTab.AntiAimYaw.GetIndex() == 5)
				{
					Render::Text(10, 60, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw: Fake Jitter");
				}
				if (Menu::Window.HvHTab.AntiAimYaw.GetIndex() == 6)
				{
					Render::Text(10, 60, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw: Jitter");
				}
				if (Menu::Window.HvHTab.AntiAimYaw.GetIndex() == 7)
				{
					Render::Text(10, 60, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw: T Jitter");
				}
				if (Menu::Window.HvHTab.AntiAimYaw.GetIndex() == 8)
				{
					Render::Text(10, 60, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw: Back Jitter");
				}
				if (Menu::Window.HvHTab.AntiAimYaw.GetIndex() == 9)
				{
					Render::Text(10, 60, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw: Backwards");
				}
				if (Menu::Window.HvHTab.AntiAimYaw.GetIndex() == 10)
				{
					Render::Text(10, 60, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw: Fake Lowerbody");
				}
				if (Menu::Window.HvHTab.AntiAimYaw.GetIndex() == 11)
				{
					Render::Text(10, 60, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw: -Fake Jitter");
				}
				if (Menu::Window.HvHTab.AntiAimYaw.GetIndex() == 12)
				{
					Render::Text(10, 60, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw: Fake Static 2");
				}
				if (Menu::Window.HvHTab.AntiAimYaw.GetIndex() == 13)
				{
					Render::Text(10, 60, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw: FakeSide RealBack");
				}
				if (Menu::Window.HvHTab.AntiAimYaw.GetIndex() == 14)
				{
					Render::Text(10, 60, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw: Fake Stactic 3");
				}
				if (Menu::Window.HvHTab.AntiAimYaw.GetIndex() == 15)
				{
					Render::Text(10, 60, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw: Fake Static Remake");
				}
				if (Menu::Window.HvHTab.AntiAimYaw.GetIndex() == 16)
				{
					Render::Text(10, 60, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw: LBY des");
				}
				if (Menu::Window.HvHTab.AntiAimYaw.GetIndex() == 17)
				{
					Render::Text(10, 60, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw: suicide");
				}
				if (Menu::Window.HvHTab.AntiAimYaw.GetIndex() == 18)
				{
					Render::Text(10, 60, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw: Arizona");
				}
				if (Menu::Window.HvHTab.AntiAimYaw.GetIndex() == 19)
				{
					Render::Text(10, 60, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw: Jew Distortion");
				}
				if (Menu::Window.HvHTab.AntiAimYaw.GetIndex() == 20)
				{
					Render::Text(10, 60, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw: SidesTrash");
				}

				if (Menu::Window.HvHTab.AntiAimSpinYaw.GetIndex() == 0)
				{
					Render::Text(10, 70, Color(255, 255, 255, 255), Render::Fonts::Slider, "Yaw2: None");
				}
				if (Menu::Window.HvHTab.AntiAimSpinYaw.GetIndex() == 1)
				{
					Render::Text(10, 70, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw2: SpinBot");
				}
				if (Menu::Window.HvHTab.AntiAimSpinYaw.GetIndex() == 2)
				{
					Render::Text(10, 70, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw2: CSpin");
				}
				if (Menu::Window.HvHTab.AntiAimSpinYaw.GetIndex() == 3)
				{
					Render::Text(10, 70, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw2: HalfSpin");
				}
				if (Menu::Window.HvHTab.AntiAimSpinYaw.GetIndex() == 4)
				{
					Render::Text(10, 70, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw2: Fake Forward");
				}
				if (Menu::Window.HvHTab.AntiAimSpinYaw.GetIndex() == 5)
				{
					Render::Text(10, 70, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw2: Fake Backwards");
				}
				if (Menu::Window.HvHTab.AntiAimSpinYaw.GetIndex() == 6)
				{
					Render::Text(10, 70, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw2: Fake Spin");
				}
				if (Menu::Window.HvHTab.AntiAimSpinYaw.GetIndex() == 7)
				{
					Render::Text(10, 70, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw2: Kidua Jitter");
				}
				if (Menu::Window.HvHTab.AntiAimSpinYaw.GetIndex() == 8)
				{
					Render::Text(10, 70, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw2: Fake 180 Jitter");
				}
				if (Menu::Window.HvHTab.AntiAimSpinYaw.GetIndex() == 9)
				{
					Render::Text(10, 70, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw2: Sinster Roll");
				}
				if (Menu::Window.HvHTab.AntiAimSpinYaw.GetIndex() == 10)
				{
					Render::Text(10, 70, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw2: Opaf");
				}
				if (Menu::Window.HvHTab.AntiAimSpinYaw.GetIndex() == 11)
				{
					Render::Text(10, 70, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw2: phase");
				}
				if (Menu::Window.HvHTab.AntiAimSpinYaw.GetIndex() == 12)
				{
					Render::Text(10, 70, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw2: SidesTrash2");
				}
				if (Menu::Window.HvHTab.AntiAimSpinYaw.GetIndex() == 13)
				{
					Render::Text(10, 70, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw2: Crash");
				}
				if (Menu::Window.HvHTab.AntiAimSpinYaw.GetIndex() == 14)
				{
					Render::Text(10, 70, Color(255, 4, 0, 255), Render::Fonts::Slider, "Yaw2: FakeTwoStep");
				}
			}



		}

		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
			Hacks::DrawHacks();

		// Update and draw the menu
		Menu::DoUIFrame();
	}
}

// InPrediction Hooked Function
bool __stdcall Hooked_InPrediction()
{
	bool result;
	static InPrediction_ origFunc = (InPrediction_)Hooks::VMTPrediction.GetOriginalFunction(14);
	static DWORD *ecxVal = Interfaces::Prediction;
	result = origFunc(ecxVal);

	// If we are in the right place where the player view is calculated
	// Calculate the change in the view and get rid of it
	if (Menu::Window.VisualsTab.OtherNoVisualRecoil.GetState() && (DWORD)(_ReturnAddress()) == Offsets::Functions::dwCalcPlayerView)
	{
		IClientEntity* pLocalEntity = NULL;

		float* m_LocalViewAngles = NULL;

		__asm
		{
			MOV pLocalEntity, ESI
			MOV m_LocalViewAngles, EBX
		}

		Vector viewPunch = pLocalEntity->localPlayerExclusive()->GetViewPunchAngle();
		Vector aimPunch = pLocalEntity->localPlayerExclusive()->GetAimPunchAngle();

		m_LocalViewAngles[0] -= (viewPunch[0] + (aimPunch[0] * 2 * 0.4499999f));
		m_LocalViewAngles[1] -= (viewPunch[1] + (aimPunch[1] * 2 * 0.4499999f));
		m_LocalViewAngles[2] -= (viewPunch[2] + (aimPunch[2] * 2 * 0.4499999f));
		return true;
	}

	return result;
}

// DrawModelExec for chams and shit
void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld)
{
	Color color;
	float flColor[3] = { 0.f };
	static IMaterial* CoveredLit = CreateMaterial(true);
	static IMaterial* OpenLit = CreateMaterial(false);
	static IMaterial* CoveredFlat = CreateMaterial(true, false);
	static IMaterial* OpenFlat = CreateMaterial(false, false);
	bool DontDraw = false;

	const char* ModelName = Interfaces::ModelInfo->GetModelName((model_t*)pInfo.pModel);
	IClientEntity* pModelEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(pInfo.entity_index);
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (Menu::Window.VisualsTab.Active.GetState())
	{
		// Player Chams
		int ChamsStyle = Menu::Window.VisualsTab.OptionsChams.GetIndex();
		int HandsStyle = Menu::Window.VisualsTab.OtherNoHands.GetIndex();
		if (ChamsStyle != 0 && Menu::Window.VisualsTab.FiltersPlayers.GetState() && strstr(ModelName, "models/player"))
		{
			if (pLocal /* && (!Menu::Window.VisualsTab.FiltersEnemiesOnly.GetState() || pModelEntity->GetTeamNum() != pLocal->GetTeamNum())*/)
			{
				IMaterial *covered = ChamsStyle == 1 ? CoveredLit : CoveredFlat;
				IMaterial *open = ChamsStyle == 1 ? OpenLit : OpenFlat;

				IClientEntity* pModelEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(pInfo.entity_index);
				if (pModelEntity)
				{
					IClientEntity *local = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
					if (local)
					{
						if (pModelEntity->IsAlive() && pModelEntity->GetHealth() > 0 /*&& pModelEntity->GetTeamNum() != local->GetTeamNum()*/)
						{
							float alpha = 1.f;

							if (pModelEntity->HasGunGameImmunity())
								alpha = 0.5f;

							if (pModelEntity->GetTeamNum() == 2)
							{
								flColor[0] = 240.f / 255.f;
								flColor[1] = 30.f / 255.f;
								flColor[2] = 35.f / 255.f;
							}
							else
							{
								flColor[0] = 63.f / 255.f;
								flColor[1] = 72.f / 255.f;
								flColor[2] = 205.f / 255.f;
							}

							Interfaces::RenderView->SetColorModulation(flColor);
							Interfaces::RenderView->SetBlend(alpha);
							Interfaces::ModelRender->ForcedMaterialOverride(covered);
							oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

							if (pModelEntity->GetTeamNum() == 2)
							{
								flColor[0] = 255.f / 255.f;
								flColor[1] = 0.f / 255.f;
								flColor[2] = 0.f / 255.f;
							}
							else
							{
								flColor[0] = 255.f / 255.f;
								flColor[1] = 0.f / 255.f;
								flColor[2] = 239.f / 255.f;
							}

							Interfaces::RenderView->SetColorModulation(flColor);
							Interfaces::RenderView->SetBlend(alpha);
							Interfaces::ModelRender->ForcedMaterialOverride(open);
						}
						else
						{
							color.SetColor(255, 255, 255, 255);
							ForceMaterial(color, open);
						}
					}
				}
			}
		}
		else if (HandsStyle != 0 && strstr(ModelName, "arms"))
		{
			if (HandsStyle == 1)
			{
				DontDraw = true;
			}
			else if (HandsStyle == 2)
			{
				Interfaces::RenderView->SetBlend(0.3);
			}
			else if (HandsStyle == 3)
			{
				IMaterial *covered = ChamsStyle == 1 ? CoveredLit : CoveredFlat;
				IMaterial *open = ChamsStyle == 1 ? OpenLit : OpenFlat;
				if (pLocal)
				{
					if (pLocal->IsAlive())
					{
						int alpha = pLocal->HasGunGameImmunity() ? 150 : 255;

						if (pLocal->GetTeamNum() == 2)
							color.SetColor(240, 30, 35, alpha);
						else
							color.SetColor(63, 72, 205, alpha);

						ForceMaterial(color, covered);
						oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

						if (pLocal->GetTeamNum() == 2)
							color.SetColor(247, 180, 20, alpha);
						else
							color.SetColor(32, 180, 57, alpha);
					}
					else
					{
						color.SetColor(255, 255, 255, 255);
					}

					ForceMaterial(color, open);
				}
			}
			else
			{
				static int counter = 0;
				static float colors[3] = { 1.f, 0.f, 0.f };

				if (colors[counter] >= 1.0f)
				{
					colors[counter] = 1.0f;
					counter += 1;
					if (counter > 2)
						counter = 0;
				}
				else
				{
					int prev = counter - 1;
					if (prev < 0) prev = 2;
					colors[prev] -= 0.05f;
					colors[counter] += 0.05f;
				}

				Interfaces::RenderView->SetColorModulation(colors);
				Interfaces::RenderView->SetBlend(0.3);
				Interfaces::ModelRender->ForcedMaterialOverride(OpenLit);
			}
		}
		else if (ChamsStyle != 0 && Menu::Window.VisualsTab.FiltersWeapons.GetState() && strstr(ModelName, "_dropped.mdl"))
		{
			IMaterial *covered = ChamsStyle == 1 ? CoveredLit : CoveredFlat;
			color.SetColor(255, 255, 255, 255);
			ForceMaterial(color, covered);
		}
	}

	if (!DontDraw)
		oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
	Interfaces::ModelRender->ForcedMaterialOverride(NULL);
}

int RandomInt1(int min, int max)
{
	return rand() % max + min;
}
bool bGlovesNeedUpdate;
void ApplyCustomGloves(IClientEntity* pLocal)
{

	if (bGlovesNeedUpdate || !pLocal->IsAlive())
	{
		DWORD* hMyWearables = (DWORD*)((size_t)pLocal + 0x2EF4);

		if (!Interfaces::EntList->GetClientEntity(hMyWearables[0] & 0xFFF))
		{
			for (ClientClass* pClass = Interfaces::Client->GetAllClasses(); pClass; pClass = pClass->m_pNext)
			{
				if (pClass->m_ClassID != (int)CSGOClassID::CEconWearable)
					continue;

				int iEntry = (Interfaces::EntList->GetHighestEntityIndex() + 1);
				int	iSerial = RandomInt1(0x0, 0xFFF);

				pClass->m_pCreateFn(iEntry, iSerial);
				hMyWearables[0] = iEntry | (iSerial << 16);

				break;
			}
		}

		player_info_t LocalPlayerInfo;
		Interfaces::Engine->GetPlayerInfo(Interfaces::Engine->GetLocalPlayer(), &LocalPlayerInfo);

		CBaseCombatWeapon* glovestochange = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntity(hMyWearables[0] & 0xFFF);

		if (!glovestochange)
			return;


		switch (Menu::Window.SkinTab.GloveModel.GetIndex())
		{
		case 1:
		{
			*glovestochange->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 5027;
			glovestochange->SetModelIndex(Interfaces::ModelInfo->GetModelIndex("models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl"));
			//*glovestochange->ViewModelIndex() = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl");
			//*glovestochange->WorldModelIndex() = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl") + 1;
			break;
		}
		case 2:
		{
			*glovestochange->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 5032;
			glovestochange->SetModelIndex(Interfaces::ModelInfo->GetModelIndex("models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl"));
			break;
		}
		case 3:
		{
			*glovestochange->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 5031;
			glovestochange->SetModelIndex(Interfaces::ModelInfo->GetModelIndex("models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl"));
			break;
		}
		case 4:
		{
			*glovestochange->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 5030;
			glovestochange->SetModelIndex(Interfaces::ModelInfo->GetModelIndex("models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl"));
			break;
		}
		case 5:
		{
			*glovestochange->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 5033;
			glovestochange->SetModelIndex(Interfaces::ModelInfo->GetModelIndex("models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl"));
			break;
		}
		case 6:
		{
			*glovestochange->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 5034;
			glovestochange->SetModelIndex(Interfaces::ModelInfo->GetModelIndex("models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl"));
			break;
		}
		default:
			break;
		}

		switch (Menu::Window.SkinTab.GloveSkin.GetIndex())
		{
		case 0:
			break;
		case 1:
			*glovestochange->FallbackPaintKit() = 10007;
			break;
		case 2:
			*glovestochange->FallbackPaintKit() = 10006;
			break;
		case 3:
			*glovestochange->FallbackPaintKit() = 10039;
			break;
		case 4:
			*glovestochange->FallbackPaintKit() = 10008;
			break;
		case 5:
			*glovestochange->FallbackPaintKit() = 10021;
			break;
		case 6:
			*glovestochange->FallbackPaintKit() = 10036;
			break;
		case 7:
			*glovestochange->FallbackPaintKit() = 10009;
			break;
		case 8:
			*glovestochange->FallbackPaintKit() = 10010;
			break;
		case 9:
			*glovestochange->FallbackPaintKit() = 10016;
			break;
		case 10:
			*glovestochange->FallbackPaintKit() = 10013;
			break;
		case 11:
			*glovestochange->FallbackPaintKit() = 10040;
			break;
		case 12:
			*glovestochange->FallbackPaintKit() = 10015;
			break;
		case 13:
			*glovestochange->FallbackPaintKit() = 10037;
			break;
		case 14:
			*glovestochange->FallbackPaintKit() = 10038;
			break;
		case 15:
			*glovestochange->FallbackPaintKit() = 10018;
			break;
		case 16:
			*glovestochange->FallbackPaintKit() = 10019;
			break;
		case 17:
			*glovestochange->FallbackPaintKit() = 10026;
			break;
		case 18:
			*glovestochange->FallbackPaintKit() = 10028;
			break;
		case 19:
			*glovestochange->FallbackPaintKit() = 10027;
			break;
		case 20:
			*glovestochange->FallbackPaintKit() = 10024;
			break;
		case 21:
			*glovestochange->FallbackPaintKit() = 10033;
			break;
		case 22:
			*glovestochange->FallbackPaintKit() = 10034;
			break;
		case 23:
			*glovestochange->FallbackPaintKit() = 10035;
			break;
		case 24:
			*glovestochange->FallbackPaintKit() = 10030;
			break;
		}
		//*glovestochange->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = whatever item definition index you want;
		//glovestochange->SetModelIndex(desired model index e.g.GLOVE_BLOOD in your case);
		//*glovestochange->FallbackPaintKit() = 10008;
		*glovestochange->m_AttributeManager()->m_Item()->ItemIDHigh() = -1; //you need to set this to apply the custom stuff you're setting (this is probably your issue)
		*glovestochange->FallbackWear() = 0.001f;
		*glovestochange->m_AttributeManager()->m_Item()->AccountID() = LocalPlayerInfo.m_nXuidLow;


		glovestochange->PreDataUpdate(0);
		bGlovesNeedUpdate = false;
	}
}

// Hooked FrameStageNotify for removing visual recoil
void  __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage)
{
	DWORD eyeangles = NetVar.GetNetVar(0xBFEA4E7B);
	IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	


	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && curStage == FRAME_RENDER_START)
	{

		if (pLocal->IsAlive())
		{
			if (*(bool*)((DWORD)Interfaces::pInput + 0xA5))
				*(Vector*)((DWORD)pLocal + 0x31C8) = LastAngleAA;
		}

		if(Menu::Window.VisualsTab.OtherThirdperson.GetState())
		{
			if(Menu::Window.VisualsTab.OtherThirdpersonRange.GetValue() > 0)
			{
				*(bool *)((DWORD)Interfaces::pInput + 0xA5) = true;
				*(float *)((DWORD)Interfaces::pInput + 0xA8 + 0x8) = Menu::Window.VisualsTab.OtherThirdpersonRange.GetValue();
			}
			else
			{
			*(bool *)((DWORD)Interfaces::pInput + 0xA5) = false;
			*(float *)((DWORD)Interfaces::pInput + 0xA8 + 0x8) = 0;
			}
				if(Menu::Window.VisualsTab.OtherThirdpersonKey.GetKey())
			{
				*(bool *)((DWORD)Interfaces::pInput + 0xA5) = false;
				*(float *)((DWORD)Interfaces::pInput + 0xA8 + 0x8) = 0;

				if(GetKeyState(Menu::Window.VisualsTab.OtherThirdpersonKey.GetKey()))
				{
					*(bool *)((DWORD)Interfaces::pInput + 0xA5) = true;
					*(float *)((DWORD)Interfaces::pInput + 0xA8 + 0x8) = Menu::Window.VisualsTab.OtherThirdpersonRange.GetValue();
				}
			}
		}
		if(!Menu::Window.VisualsTab.OtherThirdperson.GetState())
		{
			*(bool *)((DWORD)Interfaces::pInput + 0xA5) = false;
			*(float *)((DWORD)Interfaces::pInput + 0xA8 + 0x8) = 0;
		}
	}

	// PVS Fix
	if (curStage == FRAME_RENDER_START)
	{
		for (int i = 1; i <= Interfaces::Globals->maxClients; i++)
		{
			if (i == Interfaces::Engine->GetLocalPlayer()) continue;

			IClientEntity* pCurEntity = Interfaces::EntList->GetClientEntity(i);
			if (!pCurEntity) continue;

			*(int*)((uintptr_t)pCurEntity + 0xA30) = Interfaces::Globals->framecount; //we'll skip occlusion checks now
			*(int*)((uintptr_t)pCurEntity + 0xA28) = 0;//clear occlusion flags
		}
	}


	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{

		if (pLocal->IsAlive())
		{
			R::Resolve();
		}

		//Utilities::Log("APPLY SKIN APPLY SKIN");
		IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
		int iBayonet = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_bayonet.mdl");
		int iButterfly = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_butterfly.mdl");
		int iFlip = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_flip.mdl");
		int iGut = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_gut.mdl");
		int iKarambit = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_karam.mdl");
		int iM9Bayonet = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_m9_bay.mdl");
		int iHuntsman = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_tactical.mdl");
		int iFalchion = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_falchion_advanced.mdl");
		int iDagger = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_push.mdl");
		int iBowie = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_survival_bowie.mdl");

		int iGunGame = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_gg.mdl");

		for (int i = 0; i <= Interfaces::EntList->GetHighestEntityIndex(); i++) // CHANGE
		{
			IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);

			if (pEntity)
			{
				ApplyCustomGloves(pLocal);
				ULONG hOwnerEntity = *(PULONG)((DWORD)pEntity + 0x148);

				IClientEntity* pOwner = Interfaces::EntList->GetClientEntityFromHandle((HANDLE)hOwnerEntity);

				if (pOwner)
				{
					if (pOwner == pLocal)
					{
						std::string sWeapon = Interfaces::ModelInfo->GetModelName(pEntity->GetModel());

						if (!(sWeapon.find("models/weapons", 0) != std::string::npos))
							continue;

						if (sWeapon.find("c4_planted", 0) != std::string::npos)
							continue;

						if (sWeapon.find("thrown", 0) != std::string::npos)
							continue;

						if (sWeapon.find("smokegrenade", 0) != std::string::npos)
							continue;

						if (sWeapon.find("flashbang", 0) != std::string::npos)
							continue;

						if (sWeapon.find("fraggrenade", 0) != std::string::npos)
							continue;

						if (sWeapon.find("molotov", 0) != std::string::npos)
							continue;

						if (sWeapon.find("decoy", 0) != std::string::npos)
							continue;

						if (sWeapon.find("incendiarygrenade", 0) != std::string::npos)
							continue;

						if (sWeapon.find("ied", 0) != std::string::npos)
							continue;

						if (sWeapon.find("w_eq_", 0) != std::string::npos)
							continue;

						CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)pEntity;

						ClientClass *pClass = Interfaces::Client->GetAllClasses();

						if (Menu::Window.SkinTab.SkinEnable.GetState())
						{
							int Model = Menu::Window.SkinTab.KnifeModel.GetIndex();

							int weapon = *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex();

							switch (weapon)
							{
							case 7: // AK47 
							{
								if (Menu::Window.SkinTab.AK47.getText().c_str() != NULL && Menu::Window.SkinTab.AK47.getText().c_str() != "")
								{
									int AK47 = atoi(Menu::Window.SkinTab.AK47.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = AK47;
								}
							}
							break;
							case 16: // M4A4
							{
								if (Menu::Window.SkinTab.M4A4.getText().c_str() != NULL && Menu::Window.SkinTab.M4A4.getText().c_str() != "")
								{
									int M4A4 = atoi(Menu::Window.SkinTab.M4A4.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = M4A4;
								}
							}
							break;
							case 60: // M4A1
							{
								if (Menu::Window.SkinTab.M4A1.getText().c_str() != NULL && Menu::Window.SkinTab.M4A1.getText().c_str() != "")
								{
									int M4A1 = atoi(Menu::Window.SkinTab.M4A1.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = M4A1;
								}
							}
							break;
							case 9: // AWP
							{
								if (Menu::Window.SkinTab.AWP.getText().c_str() != NULL && Menu::Window.SkinTab.AWP.getText().c_str() != "")
								{
									int AWP = atoi(Menu::Window.SkinTab.AWP.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = AWP;
								}
							}
							break;
							case 61: // USP
							{
								if (Menu::Window.SkinTab.USP.getText().c_str() != NULL && Menu::Window.SkinTab.USP.getText().c_str() != "")
								{
									int USP = atoi(Menu::Window.SkinTab.USP.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = USP;
								}
							}
							break;
							case 4: // Glock
							{
								if (Menu::Window.SkinTab.Glock18.getText().c_str() != NULL && Menu::Window.SkinTab.Glock18.getText().c_str() != "")
								{
									int Glock18 = atoi(Menu::Window.SkinTab.Glock18.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = Glock18;
								}
							}
							break;
							case 1: // Deagle
							{
								if (Menu::Window.SkinTab.Deagle.getText().c_str() != NULL && Menu::Window.SkinTab.Deagle.getText().c_str() != "")
								{
									int Deagle = atoi(Menu::Window.SkinTab.Deagle.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = Deagle;
								}
							}
							break;
							case 2: // Duals
							{
								if (Menu::Window.SkinTab.Duals.getText().c_str() != NULL && Menu::Window.SkinTab.Duals.getText().c_str() != "")
								{
									int Duals = atoi(Menu::Window.SkinTab.Duals.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = Duals;
								}
							}
							break;
							case 3: // Five Seven
							{
								if (Menu::Window.SkinTab.FiveSeven.getText().c_str() != NULL && Menu::Window.SkinTab.FiveSeven.getText().c_str() != "")
								{
									int FiveSeven = atoi(Menu::Window.SkinTab.FiveSeven.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = FiveSeven;
								}
							}
							break;
							case 8: // AUG
							{
								if (Menu::Window.SkinTab.AUG.getText().c_str() != NULL && Menu::Window.SkinTab.AUG.getText().c_str() != "")
								{
									int AUG = atoi(Menu::Window.SkinTab.AUG.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = AUG;
								}
							}
							break;
							case 10: // Famas
							{
								if (Menu::Window.SkinTab.Famas.getText().c_str() != NULL && Menu::Window.SkinTab.Famas.getText().c_str() != "")
								{
									int Famas = atoi(Menu::Window.SkinTab.Famas.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = Famas;
								}
							}
							break;
							case 11: // G3SG1
							{
								if (Menu::Window.SkinTab.G3SG1.getText().c_str() != NULL && Menu::Window.SkinTab.G3SG1.getText().c_str() != "")
								{
									int G3SG1 = atoi(Menu::Window.SkinTab.G3SG1.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = G3SG1;
								}
							}
							break;
							case 13: // Galil
							{
								if (Menu::Window.SkinTab.Galil.getText().c_str() != NULL && Menu::Window.SkinTab.Galil.getText().c_str() != "")
								{
									int Galil = atoi(Menu::Window.SkinTab.Galil.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = Galil;
								}
							}
							break;
							case 14: // M249
							{
								if (Menu::Window.SkinTab.M249.getText().c_str() != NULL && Menu::Window.SkinTab.M249.getText().c_str() != "")
								{
									int M249 = atoi(Menu::Window.SkinTab.M249.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = M249;
								}
							}
							break;
							case 17: // Mac 10
							{
								if (Menu::Window.SkinTab.MAC10.getText().c_str() != NULL && Menu::Window.SkinTab.MAC10.getText().c_str() != "")
								{
									int MAC10 = atoi(Menu::Window.SkinTab.MAC10.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = MAC10;
								}
							}
							break;
							case 19: // P90
							{
								if (Menu::Window.SkinTab.P90.getText().c_str() != NULL && Menu::Window.SkinTab.P90.getText().c_str() != "")
								{
									int P90 = atoi(Menu::Window.SkinTab.P90.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = P90;
								}
							}
							break;
							case 24: // UMP-45
							{
								if (Menu::Window.SkinTab.UMP.getText().c_str() != NULL && Menu::Window.SkinTab.UMP.getText().c_str() != "")
								{
									int UMP = atoi(Menu::Window.SkinTab.UMP.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = UMP;
								}
							}
							break;
							case 25: // XM1014
							{
								if (Menu::Window.SkinTab.XM1014.getText().c_str() != NULL && Menu::Window.SkinTab.XM1014.getText().c_str() != "")
								{
									int XM1014 = atoi(Menu::Window.SkinTab.XM1014.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = XM1014;
								}
							}
							break;
							case 26: // Bizon
							{
								if (Menu::Window.SkinTab.PPBizon.getText().c_str() != NULL && Menu::Window.SkinTab.PPBizon.getText().c_str() != "")
								{
									int PPBizon = atoi(Menu::Window.SkinTab.PPBizon.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = PPBizon;
								}
							}
							break;
							case 27: // Mag 7
							{
								if (Menu::Window.SkinTab.MAG7.getText().c_str() != NULL && Menu::Window.SkinTab.MAG7.getText().c_str() != "")
								{
									int MAG7 = atoi(Menu::Window.SkinTab.MAG7.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = MAG7;
								}
							}
							break;
							case 28: // Negev
							{
								if (Menu::Window.SkinTab.Negev.getText().c_str() != NULL && Menu::Window.SkinTab.Negev.getText().c_str() != "")
								{
									int Negev = atoi(Menu::Window.SkinTab.Negev.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = Negev;
								}
							}
							break;
							case 29: // Sawed Off
							{
								if (Menu::Window.SkinTab.SawedOff.getText().c_str() != NULL && Menu::Window.SkinTab.SawedOff.getText().c_str() != "")
								{
									int SawedOff = atoi(Menu::Window.SkinTab.SawedOff.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = SawedOff;
								}
							}
							break;
							case 30: // Tec 9
							{
								if (Menu::Window.SkinTab.TEC9.getText().c_str() != NULL && Menu::Window.SkinTab.TEC9.getText().c_str() != "")
								{
									int TEC9 = atoi(Menu::Window.SkinTab.TEC9.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = TEC9;
								}
							}
							break;
							case 32: // P2000
							{
								if (Menu::Window.SkinTab.P2000.getText().c_str() != NULL && Menu::Window.SkinTab.P2000.getText().c_str() != "")
								{
									int P2000 = atoi(Menu::Window.SkinTab.P2000.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = P2000;
								}
							}
							break;
							case 33: // MP7
							{
								if (Menu::Window.SkinTab.MP7.getText().c_str() != NULL && Menu::Window.SkinTab.MP7.getText().c_str() != "")
								{
									int MP7 = atoi(Menu::Window.SkinTab.MP7.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = MP7;
								}
							}
							break;
							case 34: // MP9
							{
								if (Menu::Window.SkinTab.MP9.getText().c_str() != NULL && Menu::Window.SkinTab.MP9.getText().c_str() != "")
								{
									int MP9 = atoi(Menu::Window.SkinTab.MP9.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = MP9;
								}
							}
							break;
							case 35: // Nova
							{
								if (Menu::Window.SkinTab.Nova.getText().c_str() != NULL && Menu::Window.SkinTab.Nova.getText().c_str() != "")
								{
									int Nova = atoi(Menu::Window.SkinTab.Nova.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = Nova;
								}
							}
							break;
							case 36: // P250
							{
								if (Menu::Window.SkinTab.P250.getText().c_str() != NULL && Menu::Window.SkinTab.P250.getText().c_str() != "")
								{
									int P250 = atoi(Menu::Window.SkinTab.P250.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = P250;
								}
							}
							break;
							case 38: // Scar 20
							{
								if (Menu::Window.SkinTab.Scar20.getText().c_str() != NULL && Menu::Window.SkinTab.Scar20.getText().c_str() != "")
								{
									int Scar20 = atoi(Menu::Window.SkinTab.Scar20.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = Scar20;
								}
							}
							break;
							case 39: // SG553
							{
								if (Menu::Window.SkinTab.SG553.getText().c_str() != NULL && Menu::Window.SkinTab.SG553.getText().c_str() != "")
								{
									int SG553 = atoi(Menu::Window.SkinTab.SG553.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = SG553;
								}
							}
							break;
							case 40: // SSG08
							{
								if (Menu::Window.SkinTab.SSG08.getText().c_str() != NULL && Menu::Window.SkinTab.SSG08.getText().c_str() != "")
								{
									int SSG08 = atoi(Menu::Window.SkinTab.SSG08.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = SSG08;
								}
							}
							break;
							case 64: // Revolver
							{
								if (Menu::Window.SkinTab.R8.getText().c_str() != NULL && Menu::Window.SkinTab.R8.getText().c_str() != "")
								{
									int R8 = atoi(Menu::Window.SkinTab.R8.getText().c_str());

									if (Menu::Window.SkinTab.SkinEnable.GetState())
										*pWeapon->FallbackPaintKit() = R8;
								}
							}
							break;
							default:
								break;
							}


							if (pEntity->GetClientClass()->m_ClassID == (int)CSGOClassID::CKnife)
							{
								if (Model == 0) // Karambit
								{
									*pWeapon->ModelIndex() = iKarambit; // m_nModelIndex
									*pWeapon->ViewModelIndex() = iKarambit;
									*pWeapon->WorldModelIndex() = iKarambit + 1;
									*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 507;

									if (Menu::Window.SkinTab.KnifeSkin.getText().c_str() != NULL && Menu::Window.SkinTab.KnifeSkin.getText().c_str() != "")
									{
										int KnifeSkin = atoi(Menu::Window.SkinTab.KnifeSkin.getText().c_str());

										if (Menu::Window.SkinTab.SkinEnable.GetState())
											*pWeapon->FallbackPaintKit() = KnifeSkin;
									}
								}
								else if (Model == 1) // M9 Bayonet
								{
									*pWeapon->ModelIndex() = iM9Bayonet; // m_nModelIndex
									*pWeapon->ViewModelIndex() = iM9Bayonet;
									*pWeapon->WorldModelIndex() = iM9Bayonet + 1;
									*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 508;

									if (Menu::Window.SkinTab.KnifeSkin.getText().c_str() != NULL && Menu::Window.SkinTab.KnifeSkin.getText().c_str() != "")
									{
										int KnifeSkin = atoi(Menu::Window.SkinTab.KnifeSkin.getText().c_str());

										if (Menu::Window.SkinTab.SkinEnable.GetState())
											*pWeapon->FallbackPaintKit() = KnifeSkin;
									}
								}
								else if (Model == 2) // Huntsman Knife
								{
									*pWeapon->ModelIndex() = iHuntsman; // m_nModelIndex
									*pWeapon->ViewModelIndex() = iHuntsman;
									*pWeapon->WorldModelIndex() = iHuntsman + 1;
									*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 509;

									if (Menu::Window.SkinTab.KnifeSkin.getText().c_str() != NULL && Menu::Window.SkinTab.KnifeSkin.getText().c_str() != "")
									{
										int KnifeSkin = atoi(Menu::Window.SkinTab.KnifeSkin.getText().c_str());

										if (Menu::Window.SkinTab.SkinEnable.GetState())
											*pWeapon->FallbackPaintKit() = KnifeSkin;
									}
								}
								else if (Model == 3) // Butterfly Knife
								{
									*pWeapon->ModelIndex() = iButterfly; // m_nModelIndex
									*pWeapon->ViewModelIndex() = iButterfly;
									*pWeapon->WorldModelIndex() = iButterfly + 1;
									*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 515;

									if (Menu::Window.SkinTab.KnifeSkin.getText().c_str() != NULL && Menu::Window.SkinTab.KnifeSkin.getText().c_str() != "")
									{
										int KnifeSkin = atoi(Menu::Window.SkinTab.KnifeSkin.getText().c_str());

										if (Menu::Window.SkinTab.SkinEnable.GetState())
											*pWeapon->FallbackPaintKit() = KnifeSkin;
									}
								}
								else if (Model == 4) // Bowie Knife
								{
									*pWeapon->ModelIndex() = iBowie; // m_nModelIndex
									*pWeapon->ViewModelIndex() = iBowie;
									*pWeapon->WorldModelIndex() = iBowie + 1;
									*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 514;

									if (Menu::Window.SkinTab.KnifeSkin.getText().c_str() != NULL && Menu::Window.SkinTab.KnifeSkin.getText().c_str() != "")
									{
										int KnifeSkin = atoi(Menu::Window.SkinTab.KnifeSkin.getText().c_str());

										if (Menu::Window.SkinTab.SkinEnable.GetState())
											*pWeapon->FallbackPaintKit() = KnifeSkin;
									}
								}
								else if (Model == 5) // Flip Knife
								{
									*pWeapon->ModelIndex() = iFlip; // m_nModelIndex
									*pWeapon->ViewModelIndex() = iFlip;
									*pWeapon->WorldModelIndex() = iFlip + 1;
									*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 505;

									if (Menu::Window.SkinTab.KnifeSkin.getText().c_str() != NULL && Menu::Window.SkinTab.KnifeSkin.getText().c_str() != "")
									{
										int KnifeSkin = atoi(Menu::Window.SkinTab.KnifeSkin.getText().c_str());

										if (Menu::Window.SkinTab.SkinEnable.GetState())
											*pWeapon->FallbackPaintKit() = KnifeSkin;
									}
								}
								else if (Model == 6) // Bayonet
								{
									*pWeapon->ModelIndex() = iBayonet; // m_nModelIndex
									*pWeapon->ViewModelIndex() = iBayonet;
									*pWeapon->WorldModelIndex() = iBayonet + 1;
									*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 500;

									if (Menu::Window.SkinTab.KnifeSkin.getText().c_str() != NULL && Menu::Window.SkinTab.KnifeSkin.getText().c_str() != "")
									{
										int KnifeSkin = atoi(Menu::Window.SkinTab.KnifeSkin.getText().c_str());

										if (Menu::Window.SkinTab.SkinEnable.GetState())
											*pWeapon->FallbackPaintKit() = KnifeSkin;
									}
								}
								else if (Model == 7) // Falchion Knife
								{
									*pWeapon->ModelIndex() = iFalchion; // m_nModelIndex
									*pWeapon->ViewModelIndex() = iFalchion;
									*pWeapon->WorldModelIndex() = iFalchion + 1;
									*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 512;

									if (Menu::Window.SkinTab.KnifeSkin.getText().c_str() != NULL && Menu::Window.SkinTab.KnifeSkin.getText().c_str() != "")
									{
										int KnifeSkin = atoi(Menu::Window.SkinTab.KnifeSkin.getText().c_str());

										if (Menu::Window.SkinTab.SkinEnable.GetState())
											*pWeapon->FallbackPaintKit() = KnifeSkin;
									}
								}
								else if (Model == 8) // Gut Knife
								{
									*pWeapon->ModelIndex() = iGut; // m_nModelIndex
									*pWeapon->ViewModelIndex() = iGut;
									*pWeapon->WorldModelIndex() = iGut + 1;
									*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 506;

									if (Menu::Window.SkinTab.KnifeSkin.getText().c_str() != NULL && Menu::Window.SkinTab.KnifeSkin.getText().c_str() != "")
									{
										int KnifeSkin = atoi(Menu::Window.SkinTab.KnifeSkin.getText().c_str());

										if (Menu::Window.SkinTab.SkinEnable.GetState())
											*pWeapon->FallbackPaintKit() = KnifeSkin;
									}
								}
								else if (Model == 9) // Shadow Daggers
								{
									*pWeapon->ModelIndex() = iDagger; // m_nModelIndex
									*pWeapon->ViewModelIndex() = iDagger;
									*pWeapon->WorldModelIndex() = iDagger + 1;
									*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() = 516;

									if (Menu::Window.SkinTab.KnifeSkin.getText().c_str() != NULL && Menu::Window.SkinTab.KnifeSkin.getText().c_str() != "")
									{
										int KnifeSkin = atoi(Menu::Window.SkinTab.KnifeSkin.getText().c_str());

										if (Menu::Window.SkinTab.SkinEnable.GetState())
											*pWeapon->FallbackPaintKit() = KnifeSkin;
									}
								}
							}

							*pWeapon->OwnerXuidLow() = 0;
							*pWeapon->OwnerXuidHigh() = 0;
							*pWeapon->FallbackWear() = 0.001f;
							*pWeapon->m_AttributeManager()->m_Item()->ItemIDHigh() = 1;

						}
					}
				}

			}
		}
	}
	oFrameStageNotify(curStage);
}

void __fastcall Hooked_OverrideView(void* ecx, void* edx, CViewSetup* pSetup)
{
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{
		if (Menu::Window.VisualsTab.Active.GetState() && pLocal->IsAlive() && !pLocal->IsScoped())
		{
			if (pSetup->fov = 90)
				pSetup->fov = Menu::Window.VisualsTab.OtherFOV.GetValue();
		}

		oOverrideView(ecx, edx, pSetup);
	}

}

void GetViewModelFOV(float& fov)
{
	IClientEntity* localplayer = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{

		if (!localplayer)
			return;


		if (Menu::Window.VisualsTab.Active.GetState())
		fov = Menu::Window.VisualsTab.OtherViewmodelFOV.GetValue() - 45;
	}
}

float __stdcall GGetViewModelFOV()
{
	float fov = Hooks::VMTClientMode.GetMethod<oGetViewModelFOV>(35)();

	GetViewModelFOV(fov);

	return fov;
}

void __fastcall Hooked_RenderView(void* ecx, void* edx, CViewSetup &setup, CViewSetup &hudViewSetup, int nClearFlags, int whatToDraw)
{
	static DWORD oRenderView = Hooks::VMTRenderView.GetOriginalFunction(6);

	IClientEntity* pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	__asm
	{
		PUSH whatToDraw
		PUSH nClearFlags
		PUSH hudViewSetup
		PUSH setup
		MOV ECX, ecx
		CALL oRenderView
	}
}