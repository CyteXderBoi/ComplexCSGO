/*
Syn's AyyWare Framework 2015
*/

#pragma once

#include "GUI.h"

class CCheckBox : public CControl
{
public:
	CCheckBox();
	void SetState(bool s);
	bool GetState();
protected:
	bool Checked;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
};

class CCheckBox2 : public CControl
{
public:
	CCheckBox2();
	void SetState(bool s);
	bool GetState();
protected:
	bool Checked;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
};

class CLabel : public CControl
{
public:
	CLabel();
	void SetText(std::string text);
protected:
	std::string Text;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
};

class CGroupBox : public CControl
{
public:
	CGroupBox();
	void SetText(std::string text);
	void PlaceLabledControl(std::string Label, CTab *Tab, CControl* control);
	void PlaceLabledControl1(std::string Label, CTab *Tab, CControl* control, int c, int r);
	inline void SetColumns(int c) { Columns = c; }
protected:
	int Columns;
	int MaxRow;
	int Items;
	std::string Text;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
};

class CGroupBox2 : public CControl
{
public:
	CGroupBox2();
	void SetText(std::string text);
	void PlaceLabledControl(std::string Label, CTab *Tab, CControl* control);
protected:
	int Items;
	std::string Text;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
};

class CGroupBox3 : public CControl
{
public:
	CGroupBox3();
	void SetText(std::string text);
	void PlaceLabledControl(std::string Label, CTab *Tab, CControl* control);
protected:
	int Items;
	std::string Text;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
};

class CSlider : public CControl
{
public:
	CSlider();
	float GetValue();
	void SetValue(float v);
	void SetBoundaries(float min, float max);
protected:
	float Value;
	float Min;
	float Max;
	bool DoDrag;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
};

class CSlider2 : public CControl
{
public:
	CSlider2();
	float GetValue();
	void SetValue(float v);
	void SetBoundaries(float min, float max);
protected:
	float Value;
	float Min;
	float Max;
	bool DoDrag;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
};

class CSlider3 : public CControl
{
public:
	CSlider3();
	float GetValue();
	void SetValue(float v);
	void SetBoundaries(float min, float max);
protected:
	float Value;
	float Min;
	float Max;
	bool DoDrag;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
};
//
class CKeyBind : public CControl
{
public:
	CKeyBind();
	int GetKey();
	void SetKey(int key);
protected:
	int Key;
	bool IsGettingKey;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
};

class CButton : public CControl
{
public:
	typedef void(*ButtonCallback_t)(void);
	CButton();
	void SetCallback(ButtonCallback_t callback);
	void SetText(std::string text);
protected:
	ButtonCallback_t CallBack;
	std::string Text;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
};

class CComboBox : public CControl
{
public:
	CComboBox();
	void AddItem(std::string text);
	void SelectIndex(int idx);
	int GetIndex();
	std::string GetItem();
protected:
	std::vector<std::string> Items;
	int SelectedIndex;
	bool IsOpen;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
};

class CComboBox2 : public CControl
{
public:
	CComboBox2();
	void AddItem(std::string text);
	void SelectIndex(int idx);
	int GetIndex();
	void SetIndex(int);
	std::string GetItem();
protected:
	std::vector<std::string> Items;
	int SelectedIndex;
	bool IsOpen;
	int winWidth;
	int winHeight;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
};

class CTextField : public CControl
{
public:
	CTextField();
	std::string getText();
	void SetText(std::string);
private:
	std::string text;
	bool IsGettingKey;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
};

class CTextField2 : public CControl
{
public:
	CTextField2();
	std::string getText();
	void SetText(std::string);
private:
	std::string text;
	bool IsGettingKey;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
};

class CListBox : public CControl
{
public:
	CListBox();
	void AddItem(std::string text, int value = 0);
	void SelectIndex(int idx);
	inline int GetIndex() { return SelectedIndex; }
	std::string GetItem();
	inline int GetValue() { if (SelectedIndex >= 0 && SelectedIndex < Items.size()) return Values[SelectedIndex]; else return -1; };
	void ClearItems() { Items.clear(); Values.clear(); }
	void SetHeightInItems(int items);
	inline void SelectItem(int idx) { if (idx >= 0 && idx < Items.size()) SelectedIndex = idx; }

protected:
	std::vector<std::string> Items;
	std::vector<int> Values;

	int ScrollTop;

	int SelectedIndex;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
};