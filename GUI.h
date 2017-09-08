/*
Syn's AyyWare Framework 2015
*/

#pragma once

#include "CommonIncludes.h"

#include <map>
//////////////////////////////////////////////////////////////
#define UI_CURSORSIZE		12
#define UI_CURSORFILL		Color(255,255,255)
#define UI_CURSOROUTLINE	Color(20,20,20,140)

#define UI_WIN_TOPHEIGHT	26
#define UI_WIN_TITLEHEIGHT	32


#define UI_TAB_WIDTH		125
#define UI_TAB_HEIGHT		150

#define UI_WIN_CLOSE_X		20
#define UI_WIN_CLOSE_Y      6

#define UI_CHK_SIZE			16

#define UI_COL_TEST			Color(255, 0, 255, 255) //pink
#define UI_COL_TEST2		Color(105, 5, 153) //lila
#define UI_COL_TEXT3		Color(255, 0, 0, 255) //rot
#define UI_FILLER			Color(0, 0, 0, 0) //nix
#define COL_WHITE			Color(255, 255, 255) //weiß
#define UI_COL_MAIN			Color(37, 137, 255, 255) //blau
#define UI_COL_MAINDARK		Color(28, 136, 0, 255) //Grün
#define UI_COL_FADEMAIN		Color(28, 136, 0, 255) // grün
#define UI_COL_SHADOW		Color(0, 0, 0, 255) // schwarz / schwarz transparent
#define UI_COL_CLIENTBACK	Color(30, 30, 30, 255) //schwarz
#define UI_COL_TABSEPERATOR	Color(132, 132, 132, 200) //Transparent
#define UI_COL_TABTEXT		Color(145, 145, 145, 255) //hellgrau
#define UI_COL_GROUPOUTLINE Color(222, 222, 222, 255) //gräulich

///////////////////////////////////////////////////////////////

class CControl;
class CTab;
class CWindow;
class CGUI;

extern CGUI GUI;

enum UIFlags
{
	UI_None = 0x00, // No Flags
	UI_Drawable = 0x01, // This control should be drawn
	UI_Clickable = 0x02, // This control has a click event
	UI_Focusable = 0x04, // This control can hold an input focus
	UI_RenderFirst = 0x08, // This control should be drawn under all others
	UI_SaveFile = 0x10 // This control can be saved to the config files
};

enum UIControlTypes
{
	UIC_CheckBox = 1,
	UIC_Slider,
	UIC_KeyBind,
	UIC_ComboBox
};

// Base class for GUI controls
class CControl
{
	friend class CGUI;
	friend class CTab;
	friend class CWindow;

public:
	void SetPosition(int x, int y);
	void SetSize(int w, int h);
	void GetSize(int &w, int &h);
	void SetFileId(std::string fid);
	inline void SetFileID(DWORD ID) { FileID = ID; }
	//void SetFileID(DWORD ID) { FileID = ID; }

	bool Flag(int f);

protected:
	int m_x, m_y;
	int m_iWidth, m_iHeight;
	int m_Flags;
	DWORD FileID;

	std::string FileIdentifier;
	int FileControlType;
	CWindow* parent;

	virtual void Draw(bool) = 0;
	virtual void OnUpdate() = 0;
	virtual void OnClick() = 0;

	POINT GetAbsolutePos();
};

// A GUI Control Container
class CTab
{
	friend class CControl;
	friend class CGUI;
	friend class CWindow;

public:
	void SetTitle(std::string title);
	void RegisterControl(CControl* control);
private:
	std::string Title;
	std::vector<CControl*> Controls;
	CWindow* parent;
};

// Base class for a simple GUI window
class CWindow
{
	friend class CControl;
	friend class CGUI;

public:
	//void SetPosition(int x, int y);
	inline void SetPosition(int x, int y) { m_x = x; m_y = y; }
	void SetSize(int w, int h);
	void SetTitle(std::string title);
	void Open();
	void Close();
	inline bool IsOpen() { return m_bIsOpen; }
	void Toggle();
	CControl* GetFocus();

	void RegisterTab(CTab* Tab);
	inline void EnableTabs(bool tabs) { m_HasTabs = tabs; }

	RECT GetClientArea();
	RECT GetTabArea();
	/*void inline SetConfigFile(std::string p) { ConfigFilePath = p; }
	void SaveToCurrentConfig();*/

private:
	void DrawControls();
	//std::string ConfigFilePath;

	bool m_bIsOpen;

	bool m_HasTabs;

	std::vector<CTab*> Tabs;
	CTab* SelectedTab;

	bool IsFocusingControl;
	CControl* FocusedControl;

	std::string Title;
	int m_x;
	int m_y;
	int m_iWidth;
	int m_iHeight;

};

// User interface manager
class CGUI
{
public:
	CGUI();

	// Draws all windows 
	void Draw();

	// Handle all input etc
	void Update();

	// Draws a single window
	bool DrawWindow(CWindow* window);

	// Registers a window
	void RegisterWindow(CWindow* window);

	// Config saving/loading
	void SaveWindowState(CWindow* window, std::string Filename);
	void LoadWindowState(CWindow* window, std::string Filename);

	// Window Binds
	void BindWindow(unsigned char Key, CWindow* window);

	// Input
	bool GetKeyPress(unsigned int key);
	bool GetKeyState(unsigned int key);
	bool IsMouseInRegion(int x, int y, int x2, int y2);
	bool IsMouseInRegion(RECT region);
	POINT GetMouse();

private:
	// Input
	// keyboard
	bool keys[256];
	bool oldKeys[256];
	// Mouse
	POINT Mouse;
	bool MenuOpen;

	// Window Dragging
	bool IsDraggingWindow;
	CWindow* DraggingWindow;
	int DragOffsetX; int DragOffsetY;

	// Windows
	std::vector<CWindow*> Windows;

	// KeyBinds -> Windows Map
	std::map<int, CWindow*> WindowBinds;


	//char buf[255];

};