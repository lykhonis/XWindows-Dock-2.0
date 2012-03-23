#ifndef GCOMBOBOX_H
#define GCOMBOBOX_H

#include <afxwin.h>
#include "consts.h"
#include "DIB.h"
#include "menu.h"

class CGComboBoxItem
{
public:
	CGComboBoxItem();
	~CGComboBoxItem();

public:
	CString text;
	int id;
};

class CGComboBox: public CFrameWnd
{
public:
	CGComboBox();
	~CGComboBox();

	BOOL PreCreateWindow(CREATESTRUCT& cs);

	CGComboBoxItem* Add(CString text, int id);
	void RemoveAll();

	CGComboBoxItem* FindItem(int id);

	void Draw(CDIB &dib);

public:
	CDIB *bckg;
	CList<CGComboBoxItem*> items;
	CGComboBoxItem *selected;
	CLMenu *menu;

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMenuNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEnable(BOOL bEnable);

	DECLARE_MESSAGE_MAP();
};

#endif /* GCOMBOBOX_H */