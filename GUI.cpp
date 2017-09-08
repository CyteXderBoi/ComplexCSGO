/*
Syn's AyyWare Framework 2015
*/

#include "GUI.h"

#include "RenderManager.h"
#include "MetaInfo.h"
#include "Menu.h" //Falsch?
#include <algorithm>
#include "tinyxml2.h"
#include "Controls.h"
#include "Hacks.h" //falsch?

CGUI GUI;

CGUI::CGUI()
{

}

// Draws all windows 
void CGUI::Draw()
{
	bool ShouldDrawCursor = false;

	for (auto window : Windows)
	{
		if (window->m_bIsOpen)
		{
			ShouldDrawCursor = true;
			DrawWindow(window);
		}

	}

	if (ShouldDrawCursor)
	{
		/*static Vertex_t MouseVt[3];

		MouseVt[0].Init(Vector2D(Mouse.x, Mouse.y));
		MouseVt[1].Init(Vector2D(Mouse.x + 8, Mouse.y));
		MouseVt[2].Init(Vector2D(Mouse.x, Mouse.y + 16));

		Render::PolygonOutline(3, MouseVt, Color(48, 232, 168, 1), Color(48, 110, 232, 1));*/
		int CursorOutLineR = 155;
		int CursorOutLineG = 255;
		int CursorOutLineB = 50;

		int CursorR = 155;
		int CursorG = 255;
		int CursorB = 50;

		Render::Clear(Mouse.x + 1, Mouse.y, 1, 17, Color(CursorOutLineR, CursorOutLineG, CursorOutLineB, 255));
		for (int i = 0; i < 11; i++)
			Render::Clear(Mouse.x + 2 + i, Mouse.y + 1 + i, 1, 1, Color(CursorOutLineR, CursorOutLineG, CursorOutLineB, 255));
		Render::Clear(Mouse.x + 8, Mouse.y + 12, 5, 1, Color(CursorOutLineR, CursorOutLineG, CursorOutLineB, 255));
		Render::Clear(Mouse.x + 8, Mouse.y + 13, 1, 1, Color(CursorOutLineR, CursorOutLineG, CursorOutLineB, 255));
		Render::Clear(Mouse.x + 9, Mouse.y + 14, 1, 2, Color(CursorOutLineR, CursorOutLineG, CursorOutLineB, 255));
		Render::Clear(Mouse.x + 10, Mouse.y + 16, 1, 2, Color(CursorOutLineR, CursorOutLineG, CursorOutLineB, 255));
		Render::Clear(Mouse.x + 8, Mouse.y + 18, 2, 1, Color(CursorOutLineR, CursorOutLineG, CursorOutLineB, 255));
		Render::Clear(Mouse.x + 7, Mouse.y + 16, 1, 2, Color(CursorOutLineR, CursorOutLineG, CursorOutLineB, 255));
		Render::Clear(Mouse.x + 6, Mouse.y + 14, 1, 2, Color(CursorOutLineR, CursorOutLineG, CursorOutLineB, 255));
		Render::Clear(Mouse.x + 5, Mouse.y + 13, 1, 1, Color(CursorOutLineR, CursorOutLineG, CursorOutLineB, 255));
		Render::Clear(Mouse.x + 4, Mouse.y + 14, 1, 1, Color(CursorOutLineR, CursorOutLineG, CursorOutLineB, 255));
		Render::Clear(Mouse.x + 3, Mouse.y + 15, 1, 1, Color(CursorOutLineR, CursorOutLineG, CursorOutLineB, 255));
		Render::Clear(Mouse.x + 2, Mouse.y + 16, 1, 1, Color(CursorOutLineR, CursorOutLineG, CursorOutLineB, 255));
		for (int i = 0; i < 4; i++)
			Render::Clear(Mouse.x + 2 + i, Mouse.y + 2 + i, 1, 14 - (i * 2), Color(CursorR, CursorG, CursorB, 255));
		Render::Clear(Mouse.x + 6, Mouse.y + 6, 1, 8, Color(CursorR, CursorG, CursorB, 255));
		Render::Clear(Mouse.x + 7, Mouse.y + 7, 1, 9, Color(CursorR, CursorG, CursorB, 255));
		for (int i = 0; i < 4; i++)
			Render::Clear(Mouse.x + 8 + i, Mouse.y + 8 + i, 1, 4 - i, Color(CursorR, CursorG, CursorB, 255));
		Render::Clear(Mouse.x + 8, Mouse.y + 14, 1, 4, Color(CursorR, CursorG, CursorB, 255));
		Render::Clear(Mouse.x + 9, Mouse.y + 16, 1, 2, Color(CursorR, CursorG, CursorB, 255));
	}
}

// Handle all input etc
void CGUI::Update()
{
	//Key Array
	std::copy(keys, keys + 255, oldKeys);
	for (int x = 0; x < 255; x++)
	{
		//oldKeys[x] = oldKeys[x] & keys[x];
		keys[x] = static_cast<bool>(GetAsyncKeyState(x));
	}

	// Mouse Location
	POINT mp; GetCursorPos(&mp);
	ScreenToClient(GetForegroundWindow(), &mp);
	Mouse.x = mp.x; Mouse.y = mp.y;

	RECT Screen = Render::GetViewport();

	// Window Binds
	for (auto& bind : WindowBinds)
	{
		if (GetKeyPress(bind.first))
		{
			bind.second->Toggle();
		}
	}

	// Stop dragging
	if (IsDraggingWindow && !GetKeyState(VK_LBUTTON))
	{
		IsDraggingWindow = false;
		DraggingWindow = nullptr;
	}

	// If we are in the proccess of dragging a window
	if (IsDraggingWindow && GetKeyState(VK_LBUTTON) && !GetKeyPress(VK_LBUTTON))
	{
		if (DraggingWindow)
		{
			DraggingWindow->m_x = Mouse.x - DragOffsetX;
			DraggingWindow->m_y = Mouse.y - DragOffsetY;
		}
	}

	// Process some windows
	for (auto window : Windows)
	{
		if (window->m_bIsOpen)
		{
			// Used to tell the widget processing that there could be a click
			bool bCheckWidgetClicks = false;

			// If the user clicks inside the window
			if (GetKeyPress(VK_LBUTTON))
			{
				if (IsMouseInRegion(window->m_x, window->m_y, window->m_x + window->m_iWidth, window->m_y + window->m_iHeight))
				{
					// Close Button
					if (IsMouseInRegion(window->m_x + window->m_iWidth - UI_WIN_CLOSE_X, window->m_y, window->m_x + window->m_iWidth, window->m_y + UI_WIN_CLOSE_X))
					{
						window->Toggle();
					}
					else
						// User is selecting a new tab
						if (IsMouseInRegion(window->GetTabArea()))
						{

							bCheckWidgetClicks = true;

							int iTab = 0;
							int TabCount = window->Tabs.size();
							if (TabCount) // If there are some tabs
							{
								int TabCount = window->Tabs.size();
								//RECT TabArea = { window->m_x, window->m_y + UI_WIN_TOPHEIGHT + UI_WIN_TITLEHEIGHT + (i*TabSize) , window->m_iWidth - window->m_iHeight, TabSize };
								int TabSize = (window->m_iWidth - UI_TAB_WIDTH - UI_WIN_TITLEHEIGHT - 400) / TabCount;;
								int Dist = Mouse.y - (window->m_y + UI_WIN_TITLEHEIGHT + UI_WIN_TOPHEIGHT);
								if (Dist < (TabSize*TabCount))
								{
									while (Dist > TabSize)
									{
										if (Dist > TabSize)
										{
											iTab++;
											Dist -= TabSize;
										}
										if (iTab == (TabCount - 1))
										{
											break;
										}
									}
									window->SelectedTab = window->Tabs[iTab];

									// Loose focus on the control
									bCheckWidgetClicks = false;
									window->IsFocusingControl = false;
									window->FocusedControl = nullptr;
								}
							}

						}
					// Is it inside the client area?
						else if (IsMouseInRegion(window->GetClientArea()))
						{
							bCheckWidgetClicks = true;
						}
						else
						{
							// Must be in the around the title or side of the window
							// So we assume the user is trying to drag the window
							IsDraggingWindow = true;
							DraggingWindow = window;
							DragOffsetX = Mouse.x - window->m_x;
							DragOffsetY = Mouse.y - window->m_y;

							// Loose focus on the control
							window->IsFocusingControl = false;
							window->FocusedControl = nullptr;
						}
				}
				else
				{
					// Loose focus on the control
					window->IsFocusingControl = false;
					window->FocusedControl = nullptr;
				}
			}



			// Controls 
			if (window->SelectedTab != nullptr)
			{
				// Focused widget
				bool SkipWidget = false;
				CControl* SkipMe = nullptr;

				// this window is focusing on a widget??
				if (window->IsFocusingControl)
				{
					if (window->FocusedControl != nullptr)
					{
						// We've processed it once, skip it later
						SkipWidget = true;
						SkipMe = window->FocusedControl;

						POINT cAbs = window->FocusedControl->GetAbsolutePos();
						RECT controlRect = { cAbs.x, cAbs.y, window->FocusedControl->m_iWidth, window->FocusedControl->m_iHeight };
						window->FocusedControl->OnUpdate();

						if (window->FocusedControl->Flag(UIFlags::UI_Clickable) && IsMouseInRegion(controlRect) && bCheckWidgetClicks)
						{
							window->FocusedControl->OnClick();

							// If it gets clicked we loose focus
							window->IsFocusingControl = false;
							window->FocusedControl = nullptr;
							bCheckWidgetClicks = false;
						}
					}
				}

				// Itterate over the rest of the control
				for (auto control : window->SelectedTab->Controls)
				{
					if (control != nullptr)
					{
						if (SkipWidget && SkipMe == control)
							continue;

						control->parent = window;

						POINT cAbs = control->GetAbsolutePos();
						RECT controlRect = { cAbs.x, cAbs.y, control->m_iWidth, control->m_iHeight };
						control->OnUpdate();

						if (control->Flag(UIFlags::UI_Clickable) && IsMouseInRegion(controlRect) && bCheckWidgetClicks)
						{
							control->OnClick();
							bCheckWidgetClicks = false;

							// Change of focus
							if (control->Flag(UIFlags::UI_Focusable))
							{
								window->IsFocusingControl = true;
								window->FocusedControl = control;
							}
							else
							{
								window->IsFocusingControl = false;
								window->FocusedControl = nullptr;
							}

						}
					}
				}

				// We must have clicked whitespace
				if (bCheckWidgetClicks)
				{
					// Loose focus on the control
					window->IsFocusingControl = false;
					window->FocusedControl = nullptr;
				}
			}
		}
	}
}

// Returns 
bool CGUI::GetKeyPress(unsigned int key)
{
	if (keys[key] == true && oldKeys[key] == false)
		return true;
	else
		return false;
}

bool CGUI::GetKeyState(unsigned int key)
{
	if (key >= 0 && key <= 255)
		return keys[key];
	else
		return false;
}

bool CGUI::IsMouseInRegion(int x, int y, int x2, int y2)
{
	if (Mouse.x > x && Mouse.y > y && Mouse.x < x2 && Mouse.y < y2)
		return true;
	else
		return false;
}

bool CGUI::IsMouseInRegion(RECT region)
{
	return IsMouseInRegion(region.left, region.top, region.left + region.right, region.top + region.bottom);
}

POINT CGUI::GetMouse()
{
	return Mouse;
}

bool CGUI::DrawWindow(CWindow* window)
{
	// Main window and title

	////RECT TextSize = Render::GetTextSize(Render::Fonts::INTERX, window->Title.c_str());
	////int TextX = window->m_x + (window->m_iWidth / 2) - (TextSize.left / 2);

	//Render::Clear(window->m_x, window->m_y + UI_TAB_HEIGHT, window->m_iWidth, 15, UI_COL_MAIN); //blauer balken //war 10
	int TopBarR = 30;
	int TopBarG = 30;
	int TopBarB = 30;

	Render::Clear(window->m_x, window->m_y + 50, window->m_iWidth - 100, window->m_iHeight - 50, Color(25, 25, 25, 255));
	Render::Clear(window->m_x, window->m_y + UI_WIN_TOPHEIGHT, window->m_iWidth, UI_WIN_TITLEHEIGHT, Color(TopBarR, TopBarG, TopBarB, 255));
	Render::GradientV(window->m_x, window->m_y + UI_WIN_TOPHEIGHT, window->m_iWidth, UI_WIN_TITLEHEIGHT, UI_COL_SHADOW, Color(TopBarR, TopBarG, TopBarB, 255)); //Balken
	Render::Outline(window->m_x, window->m_y + UI_WIN_TOPHEIGHT, window->m_iWidth, UI_WIN_TITLEHEIGHT, UI_COL_SHADOW); //Innen
	Render::Outline(window->m_x - 1, window->m_y + 21, window->m_iWidth + 2, window->m_iHeight - 22, UI_COL_SHADOW); //UMRANDUNG

	/*Title*/
	Render::Text(window->m_x + 16, window->m_y + UI_WIN_TITLEHEIGHT + 1, Color(255, 255, 255, 255), Render::Fonts::MenuBold, window->Title.c_str());

	// Client
	if (window->m_HasTabs)
		Render::Clear(window->m_x + UI_TAB_WIDTH, window->m_y + UI_WIN_TOPHEIGHT + UI_WIN_TITLEHEIGHT, window->m_iWidth - UI_TAB_WIDTH, window->m_iHeight - UI_WIN_TOPHEIGHT - UI_WIN_TITLEHEIGHT, Color(25, 25, 25, 255));
	else
		Render::Clear(window->m_x, window->m_y + UI_WIN_TOPHEIGHT + UI_WIN_TITLEHEIGHT, window->m_iWidth, window->m_iHeight - UI_WIN_TOPHEIGHT - UI_WIN_TITLEHEIGHT, Color(25, 25, 25, 255));

	Render::Clear(window->m_x + UI_TAB_WIDTH, window->m_y + UI_WIN_TOPHEIGHT + UI_WIN_TITLEHEIGHT, 1, window->m_iHeight - UI_WIN_TOPHEIGHT - UI_WIN_TITLEHEIGHT, Color(0, 0, 0, 255));
	Render::Clear(window->m_x + UI_TAB_WIDTH + 1, window->m_y + UI_WIN_TOPHEIGHT + UI_WIN_TITLEHEIGHT, 1, window->m_iHeight - UI_WIN_TOPHEIGHT - UI_WIN_TITLEHEIGHT, Color(48, 48, 48, 255));
	// Tabs
	int TabCount = window->Tabs.size();
	int TabSize = (window->m_iWidth - UI_TAB_WIDTH - UI_WIN_TITLEHEIGHT - 400) / TabCount;
	if (TabCount) // If there are some tabs
	{
		for (int i = 0; i < TabCount; i++)
		{
			RECT TabArea = { window->m_x, window->m_y + UI_WIN_TOPHEIGHT + UI_WIN_TITLEHEIGHT + (i*TabSize) , window->m_iWidth - window->m_iHeight - 200, TabSize };
			CTab *tab = window->Tabs[i];

			Color txtColor = UI_COL_TABSEPERATOR;

			int TabSelectedR = 255;
			int TabSelectedG = 255;
			int TabSelectedB = 255;
			if (window->SelectedTab == tab)
			{
				// Selected
				txtColor = Color(TabSelectedR, TabSelectedG, TabSelectedB, 255);
			}
			else if (IsMouseInRegion(TabArea))
			{
				// Hover
				txtColor = Color(TabSelectedR - 30, TabSelectedG - 30, TabSelectedB - 30, 255);
			}

			Render::Text(TabArea.left + 58 - UI_WIN_TOPHEIGHT - UI_WIN_TITLEHEIGHT + 5, TabArea.top + 8 - UI_WIN_TOPHEIGHT - UI_WIN_TITLEHEIGHT + 45, txtColor, Render::Fonts::Impact, tab->Title.c_str());
		}
	}

	//Window Outline
	Render::Outline(window->m_x - 1, window->m_y + 27, window->m_iWidth + 2, window->m_iHeight - 28, Color(30, 30, 30, 255));
	Render::Outline(window->m_x - 2, window->m_y + 26, window->m_iWidth + 4, window->m_iHeight - 26, Color(60, 60, 60, 255));
	Render::Outline(window->m_x - 3, window->m_y + 25, window->m_iWidth + 6, window->m_iHeight - 24, Color(40, 40, 40, 255));
	Render::Outline(window->m_x - 4, window->m_y + 24, window->m_iWidth + 8, window->m_iHeight - 22, Color(40, 40, 40, 255));
	Render::Outline(window->m_x - 5, window->m_y + 23, window->m_iWidth + 10, window->m_iHeight - 20, Color(40, 40, 40, 255));
	Render::Outline(window->m_x - 6, window->m_y + 22, window->m_iWidth + 12, window->m_iHeight - 18, Color(60, 60, 60, 255));
	Render::Outline(window->m_x - 7, window->m_y + 21, window->m_iWidth + 14, window->m_iHeight - 16, Color(31, 31, 31, 255));



	// Controls 
	if (window->SelectedTab != nullptr)
	{
		// Focused widget
		bool SkipWidget = false;
		CControl* SkipMe = nullptr;

		// this window is focusing on a widget??
		if (window->IsFocusingControl)
		{
			if (window->FocusedControl != nullptr)
			{
				// We need to draw it last, so skip it in the regular loop
				SkipWidget = true;
				SkipMe = window->FocusedControl;
			}
		}


		// Itterate over all the other controls
		for (auto control : window->SelectedTab->Controls)
		{
			if (SkipWidget && SkipMe == control)
				continue;

			if (control != nullptr && control->Flag(UIFlags::UI_Drawable))
			{
				control->parent = window;
				POINT cAbs = control->GetAbsolutePos();
				RECT controlRect = { cAbs.x, cAbs.y, control->m_iWidth, control->m_iHeight };
				bool hover = false;
				if (IsMouseInRegion(controlRect))
				{
					hover = true;
				}
				control->Draw(hover);
			}
		}

		// Draw the skipped widget last
		if (SkipWidget)
		{
			auto control = window->FocusedControl;

			if (control != nullptr && control->Flag(UIFlags::UI_Drawable))
			{
				POINT cAbs = control->GetAbsolutePos();
				RECT controlRect = { cAbs.x, cAbs.y, control->m_iWidth, control->m_iHeight };
				bool hover = false;
				if (IsMouseInRegion(controlRect))
				{
					hover = true;
				}
				control->Draw(hover);
			}
		}

	}


	return true;
}

void CGUI::RegisterWindow(CWindow* window)
{
	Windows.push_back(window);

	// Resorting to put groupboxes at the start
	for (auto tab : window->Tabs)
	{
		for (auto control : tab->Controls)
		{
			if (control->Flag(UIFlags::UI_RenderFirst))
			{
				CControl * c = control;
				tab->Controls.erase(std::remove(tab->Controls.begin(), tab->Controls.end(), control), tab->Controls.end());
				tab->Controls.insert(tab->Controls.begin(), control);
			}
		}
	}
}

void CGUI::BindWindow(unsigned char Key, CWindow* window)
{
	if (window)
		WindowBinds[Key] = window;
	else
		WindowBinds.erase(Key);
}

//Save Settings

void CGUI::SaveWindowState(CWindow* window, std::string Filename)
{
	// Create a whole new document and we'll just save over top of the old one
	tinyxml2::XMLDocument Doc;

	// Root Element is called "ayy"
	tinyxml2::XMLElement *Root = Doc.NewElement("Complex");
	Doc.LinkEndChild(Root);

	Utilities::Log("Saving Window %s", window->Title.c_str());

	// If the window has some tabs..
	if (Root && window->Tabs.size() > 0)
	{
		for (auto Tab : window->Tabs)
		{
			// Add a new section for this tab
			tinyxml2::XMLElement *TabElement = Doc.NewElement(Tab->Title.c_str());
			Root->LinkEndChild(TabElement);

			Utilities::Log("Saving Tab %s", Tab->Title.c_str());

			// Now we itterate the controls this tab contains
			if (TabElement && Tab->Controls.size() > 0)
			{
				for (auto Control : Tab->Controls)
				{
					// If the control is ok to be saved
					if (Control && Control->Flag(UIFlags::UI_SaveFile) && Control->FileIdentifier.length() > 1 && Control->FileControlType)
					{
						// Create an element for the control
						tinyxml2::XMLElement *ControlElement = Doc.NewElement(Control->FileIdentifier.c_str());
						TabElement->LinkEndChild(ControlElement);

						Utilities::Log("Saving control %s", Control->FileIdentifier.c_str());

						if (!ControlElement)
						{
							Utilities::Log("Errorino :("); // s0 cute
							return;
						}

						CCheckBox* cbx = nullptr;
						CComboBox* cbo = nullptr;
						CKeyBind* key = nullptr;
						CSlider* sld = nullptr;

						// Figure out what kind of control and data this is
						switch (Control->FileControlType)
						{
						case UIControlTypes::UIC_CheckBox:
							cbx = (CCheckBox*)Control;
							ControlElement->SetText(cbx->GetState());
							break;
						case UIControlTypes::UIC_ComboBox:
							cbo = (CComboBox*)Control;
							ControlElement->SetText(cbo->GetIndex());
							break;
						case UIControlTypes::UIC_KeyBind:
							key = (CKeyBind*)Control;
							ControlElement->SetText(key->GetKey());
							break;
						case UIControlTypes::UIC_Slider:
							sld = (CSlider*)Control;
							ControlElement->SetText(sld->GetValue());
							break;
						}
					}
				}
			}
		}
	}

	//Save the file
	if (Doc.SaveFile(Filename.c_str()) != tinyxml2::XML_NO_ERROR)
	{
		MessageBox(NULL, "Failed To Save Config File!", "Complex", MB_OK);
	}

}

//Load Settings

void CGUI::LoadWindowState(CWindow* window, std::string Filename)
{
	// Lets load our meme
	tinyxml2::XMLDocument Doc;
	if (Doc.LoadFile(Filename.c_str()) == tinyxml2::XML_NO_ERROR)
	{
		tinyxml2::XMLElement *Root = Doc.RootElement();

		// The root "ayy" element
		if (Root)
		{
			// If the window has some tabs..
			if (Root && window->Tabs.size() > 0)
			{
				for (auto Tab : window->Tabs)
				{
					// We find the corresponding element for this tab
					tinyxml2::XMLElement *TabElement = Root->FirstChildElement(Tab->Title.c_str());
					if (TabElement)
					{
						// Now we itterate the controls this tab contains
						if (TabElement && Tab->Controls.size() > 0)
						{
							for (auto Control : Tab->Controls)
							{
								// If the control is ok to be saved
								if (Control && Control->Flag(UIFlags::UI_SaveFile) && Control->FileIdentifier.length() > 1 && Control->FileControlType)
								{
									// Get the controls element
									tinyxml2::XMLElement *ControlElement = TabElement->FirstChildElement(Control->FileIdentifier.c_str());

									if (ControlElement)
									{
										CCheckBox* cbx = nullptr;
										CComboBox* cbo = nullptr;
										CKeyBind* key = nullptr;
										CSlider* sld = nullptr;

										// Figure out what kind of control and data this is
										switch (Control->FileControlType)
										{
										case UIControlTypes::UIC_CheckBox:
											cbx = (CCheckBox*)Control;
											cbx->SetState(ControlElement->GetText()[0] == '1' ? true : false);
											break;
										case UIControlTypes::UIC_ComboBox:
											cbo = (CComboBox*)Control;
											cbo->SelectIndex(atoi(ControlElement->GetText()));
											break;
										case UIControlTypes::UIC_KeyBind:
											key = (CKeyBind*)Control;
											key->SetKey(atoi(ControlElement->GetText()));
											break;
										case UIControlTypes::UIC_Slider:
											sld = (CSlider*)Control;
											sld->SetValue(atof(ControlElement->GetText()));
											break;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}