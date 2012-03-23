#ifndef TRACKBAR_H
#define TRACKBAR_H

#include <afxwin.h>
#include "consts.h"
#include "DIB.h"

class CTrackBar: public CFrameWnd
{
public:
	CTrackBar();
	~CTrackBar();

	BOOL PreCreateWindow(CREATESTRUCT& cs);

	bool SetPosition(int value);

	CRect GetTrackRect();

	void Draw(CDIB &dib);

public:
	int min;
	int position;
	int max;
	bool tracking;
	CDIB *button;
	CDIB *bckg;
	CDIB *bckgTrack;
	int state;

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP();
};

#endif /* TRACKBAR_H */