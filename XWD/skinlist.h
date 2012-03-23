#ifndef SKINLIST_H
#define SKINLIST_H

#include <afxwin.h>
#include "consts.h"
#include "DIB.h"
#include "skin.h"
#include "dock3d.h"

class CSkinList: public CFrameWnd
{
public:
	CSkinList();
	~CSkinList();

	BOOL PreCreateWindow(CREATESTRUCT& cs);

	void Draw();
	void Draw(CDIB &tmp);

public:
	CDIB *bckg;
	CDIB *dib;
	CSkinLoader *loader;
	CSkin *skin;
	DockMode preferMode;

public:
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnPaint();
	afx_msg void OnWindowPosChanged(WINDOWPOS *lpwndpos);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	DECLARE_MESSAGE_MAP();
};

#endif /* SKINLIST_H */