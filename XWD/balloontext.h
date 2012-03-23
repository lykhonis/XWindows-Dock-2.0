#ifndef BALLOONTEXT_H
#define BALLOONTEXT_H

#include <afxwin.h>
#include <math.h>
#include <gdiplus.h>
#include "consts.h"
#include "dib.h"
#include "monitors.h"

class CBalloonText: public CFrameWnd
{
public:
	CBalloonText();
	~CBalloonText();

	void LayerDraw(CDIB *dib = NULL);
	void LayerUpdate(CDIB *dib = NULL);

	void Prepare(CPoint point, DockPosition position);
	void Prepare();
	void Popup();
	void Hide();

	void UpdatePosition(CPoint point, DockPosition position);

public:
	CDIB *dib;
	CDIB *tmp;
	CPoint point;
	DockPosition position;
	float radius;
	bool hiding;
	DWORD hideStartAt;
	float fontSize;
	int arrowOffset;
	bool prepearing;

public:
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP();
};

#endif /* BALLOONTEXT_H */