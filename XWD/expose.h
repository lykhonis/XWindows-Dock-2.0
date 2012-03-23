#ifndef EXPOSE_H
#define EXPOSE_H

#include <afxwin.h>
#include <afxole.h>
#include <math.h>
#include "consts.h"
#include "utils.h"
#include "safedwmapi.h"
#include "dib.h"
#include "monitors.h"

static const UINT WM_EXPOSENOTIFY = RegisterWindowMessage(L"WM_EXPOSENOTIFY");

class CExposeThumbnail
{
public:
	CExposeThumbnail(HWND hParent, HWND hWnd);
	~CExposeThumbnail();

	void Initialize();
	void SetRect(CRect &rect);
	void SetVisible(bool visible = true);
	void Animate(float step);

public:
	HWND hParent;
	HWND hWnd;
	CString text;
	CRect rect;
	CSize thumbSize;
	bool visible;
	HTHUMBNAIL handle;
	int index;
	unsigned char opacity;
	bool isIconic;
	bool isZoomed;

	CRect srcRect;
	CRect dstRect;
	CRect zoomRect;
};

class CExpose: public CFrameWnd, public COleDropTarget
{
public:
	CExpose();
	~CExpose();

	void Hide(bool slow);
	bool Expose(HWND hParent, CRect rect, CDIB *icon, CString exePath, DockPosition position, bool slow);

	void SetIconRect(CRect rect, CDIB *icon);

	void DrawStep(CDIB *dib, CDIB *src, float step);
	void DrawLayer(CDIB *dib = NULL);
	void UpdateLayer(CDIB *dib = NULL);

	CRect ItemRect(CExposeThumbnail *item);
	CExposeThumbnail* ItemAt(CPoint point);
	void RemoveAll();

	void ItemDrawText(CExposeThumbnail *item);

public:
	DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	void OnDragLeave(CWnd* pWnd);

public:
	CDIB *dib;
	CMonitors *monitors;
	CMonitor *monitor;
	CRect iconRect;
	CDIB *iconDib;
	CRect clientRect;
	DWORD pid;
	CList<CExposeThumbnail*> items;
	int textHeight;
	bool dragging;
	CExposeThumbnail *itemOver;
	CPoint ptOver;
	bool readyToWork;

	bool userDropTargetHelper;
	CComPtr<IDropTargetHelper> dropTargetHelper;

private:
	static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);

public:
	afx_msg void OnClose();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP();
};

#endif /* EXPOSE_H */