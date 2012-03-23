#ifndef MENU_H
#define MENU_H

#include <afxwin.h>
#include <afxtempl.h>
#include <math.h>
#include "consts.h"
#include "DIB.h"
#include "monitors.h"

static const UINT WM_MENUNOTIFY = RegisterWindowMessage(L"WM_MENUNOTIFY");

enum {
	MenuSelect = 1,
	MenuHide
};

class CLMenu;

class CLMenuItem
{
public:
	CLMenuItem();
	~CLMenuItem();

	CString text;
	CDIB *dib;
	CDIB *icon;
	CLMenu *menu;
	int nID;
	bool selected;
	bool enabled;
	bool visible;
	bool checkbox;
	bool checked;
	bool isLine;

	// additional
	HWND hWnd;
	int AliasId;
	int AliasParentId;
};

class CLMenu: public CFrameWnd
{
public:
	CLMenu();
	~CLMenu();

	void Create(CWnd *Parent);

	CLMenuItem *Add(CString text, int nID = NULL, bool checkBox = false);
	CLMenuItem *Insert(CLMenuItem *prevItem, CString text, int nID = NULL, bool checkBox = false, bool addTail = true);
	CLMenuItem *AddLine();
	CLMenuItem *InsertLine(CLMenuItem *prevItem, bool addTail = true);
	bool Remove(CLMenuItem *item);
	void RemoveAll();

	int ItemCount(bool onlyVisible = FALSE, bool onlyEnabled = FALSE);

	CRect ItemRect(CLMenuItem *item);
	CLMenuItem *ItemAt(int x, int y);
	void DrawItem(CLMenuItem *item);
	void UpdateItem(Gdiplus::Graphics *g, CLMenuItem *item);

	int GetMaxItemWidth();
	bool IsVisible();

	void SetSelected(CLMenuItem *item);
	void EnableItem(CLMenuItem *item, bool bEnabled = TRUE);
	void VisibleItem(CLMenuItem *item, bool bVisible = TRUE);
	void CheckedItem(CLMenuItem *item, bool bChecked = TRUE);

	CLMenuItem *GetItem(int nID);
	void EnableItem(int nID, bool bEnabled = TRUE);
	void VisibleItem(int nID, bool bVisible = TRUE);
	void CheckedItem(int nID, bool bChecked = TRUE);

	CLMenuItem *GetItemByAlias(int AliasId);

	bool IsEnableItem(CLMenuItem *item);
	bool IsVisibleItem(CLMenuItem *item);
	bool IsCheckboxItem(CLMenuItem *item);
	bool IsCheckedItem(CLMenuItem *item);
	bool IsEnableItem(int nID);
	bool IsVisibleItem(int nID);
	bool IsCheckboxItem(int nID);
	bool IsCheckedItem(int nID);

	void DrawBckg();
	void DrawLayer(CDIB *dib = NULL);
	void UpdateLayer(CDIB *dib = NULL);

	bool Popup(CLMenu *Parent, DockPosition position, int X, int Y);
	void Hide(bool clicked = false);

	void UpdatePosition(CPoint pt);

public:
	CList<CLMenuItem*> items;
	CDIB *dib;
	CDIB *tmp;
	CDIB *bckg;
	CWnd *wndParent;
	CLMenu *general;
	CLMenu *parent;
	CLMenu *childMenu;
	CLMenuItem *selected;
	DWORD showStartAt;
	DockPosition position;
	int arrowOffset;

public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd *pNewWnd);

	DECLARE_MESSAGE_MAP();
};

#endif /* MENU_H */