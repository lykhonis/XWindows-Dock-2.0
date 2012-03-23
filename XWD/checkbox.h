#ifndef CHECKBOX_H
#define CHECKBOX_H

#include <afxwin.h>
#include "consts.h"
#include "DIB.h"

class CCheckBox: public CFrameWnd
{
public:
	CCheckBox();
	~CCheckBox();

	BOOL PreCreateWindow(CREATESTRUCT& cs);

	void Draw(CDIB &dib);

	void Checked(bool value = true);

public:
	CDIB *bckg;
	bool radio;
	bool checked;

public:
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnPaint();
	afx_msg void OnWindowPosChanged(WINDOWPOS *lpwndpos);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnEnable(BOOL bEnable);

	DECLARE_MESSAGE_MAP();
};

#endif /* CHECKBOX_H */