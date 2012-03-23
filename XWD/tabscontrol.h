#ifndef TABSCONTROL_H
#define TABSCONTROL_H

#include <afxwin.h>
#include <afxtempl.h>
#include <math.h>
#include "consts.h"
#include "DIB.h"

class CTabControl
{
public:
	CTabControl();
	~CTabControl();

public:
	CString text;
	CDIB *icon;
	bool selected;
	int id;
};

class CTabsControl: public CFrameWnd
{
public:
	CTabsControl();
	~CTabsControl();

	BOOL PreCreateWindow(CREATESTRUCT& cs);

	CTabControl* Add(CString text, CString icon, int id);
	void RemoveAll();

	CTabControl* FindItem(int id);

	CRect ItemRect(CTabControl *item);
	CTabControl* ItemAt(CPoint point);

	void Draw();
	void DrawItem(CTabControl *item, Gdiplus::Graphics &g);
	void DrawItems(Gdiplus::Graphics &g);

public:
	CDIB *dib;
	DWORD color1;
	DWORD color2;
	CList<CTabControl*> items;
	CTabControl *selected;

public:
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnPaint();
	afx_msg void OnWindowPosChanged(WINDOWPOS *lpwndpos);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP();
};

#endif /* TABSCONTROL_H */