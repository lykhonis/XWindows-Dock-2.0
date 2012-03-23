#ifndef IMAGEBUTTON_H
#define IMAGEBUTTON_H

#include <afxwin.h>
#include "consts.h"
#include "dib.h"

class CImageButton: public CFrameWnd
{
public:
	CImageButton();
	~CImageButton();

	BOOL PreCreateWindow(CREATESTRUCT& cs);

	void Draw(CDIB &dib);
	void Click();
	void SetPressed(bool value);

public:
	int state;
	bool down;
	bool checked;
	bool pressed;
	CDIB *image;
	DWORD color1;
	DWORD color2;
	DWORD textColor;

public:
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnEnable(BOOL bEnable);

	DECLARE_MESSAGE_MAP();
};

#endif /* IMAGEBUTTON_H */