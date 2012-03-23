#ifndef MINIMIZER_H
#define MINIMIZER_H

#include <afxwin.h>
#include <afxole.h>
#include <math.h>
#include "consts.h"
#include "utils.h"
#include "dib.h"
#include "monitors.h"
#include "safedwmapi.h"

class CMinimizer: public CFrameWnd
{
public:
	CMinimizer();
	~CMinimizer();

	void DrawLayer(CDIB *dib = NULL);
	void UpdateLayer(CDIB *dib = NULL);

	void Minimize(HWND hParent, HWND hWindow);
	
public:
	CDIB *dib;
	CMonitors *monitors;
	CMonitor *monitor;
	CFrameWnd *hideWindow;
	WINDOWPLACEMENT orgWndPlacement;
	HWND orgWndParent;
	
public:
	afx_msg void OnClose();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP();
};

#endif /* MINIMIZER_H */