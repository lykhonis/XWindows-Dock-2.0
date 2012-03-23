#ifndef ABOUT_H
#define ABOUT_H

#include <afxwin.h>
#include "consts.h"
#include "dib.h"
#include "icons.h"
#include "version.h"

class CAbout: public CFrameWnd
{
public:
	CAbout();
	~CAbout();

	void LayerDraw();
	void LayerUpdate();

public:
	CDIB *dib;
	CDIB *imageClose;
	CWnd *notifer;
	int nID;

public:
	afx_msg void OnClose();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP();
};

#endif /* ABOUT_H */