#ifndef PLUGINLIST_H
#define PLUGINLIST_H

#include <afxwin.h>
#include <afxtempl.h>
#include <afxole.h>
#include "consts.h"
#include "DIB.h"
#include "oledatasourceex.h"

static const CLIPFORMAT CF_PLUGINLIST = (CLIPFORMAT)RegisterClipboardFormat(L"CF_PLUGINLIST");

struct PluginListDrop
{
	void *plugin;
};
typedef struct PluginListDrop PluginListDrop;

class CPluginListItem
{
public:
	CPluginListItem()
	{
		icon = new CDIB();
		dib = new CDIB();
		data = NULL;
	}
	
	~CPluginListItem()
	{
		delete icon;
		delete dib;
	}

public:
	CString text;
	CString description;
	CDIB *icon;
	CDIB *dib;
	void *data;
};

class CPluginList: public CFrameWnd, public COleDropTarget
{
public:
	CPluginList();
	~CPluginList();

	BOOL PreCreateWindow(CREATESTRUCT& cs);

	void AdjustSize(bool drawItems = true);

	void Draw(CDIB &dib);

	void DrawItem(CPluginListItem *item);
	void DrawItems();

	void UpdateItem(Gdiplus::Graphics *g, CPluginListItem *item);
	void UpdateItems(Gdiplus::Graphics *g);

	CPluginListItem* Add();
	void RemoveAll();

	CRect GetItemRect(CPluginListItem *item);
	CPluginListItem* GetItemAt(CPoint point);

	bool IsItemVisible(CPluginListItem *item);

	CRect ScrollBarRect();
	void DrawScrollBar(Gdiplus::Graphics *g);
	bool ScrollTo(int value);
	bool ScrollUp();
	bool ScrollDown();

	void DragItem(CPluginListItem *item, CPoint point);

public:
	DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	void OnDragLeave(CWnd* pWnd);

public:
	CPoint downPt;
	bool mouseDown;
	CPluginListItem *selected;
	CPluginListItem *hover;
	CList<CPluginListItem*> items;
	int scrollState;
	int scroll;
	int scrollMax;
	bool scrollVisible;
	bool scrolling;
	CDIB *scrollTop;
	CDIB *scrollMiddle;
	CDIB *scrollBottom;
	CDIB *scrollButton;
	CDIB *bckg;
	bool draging;

	bool userDropTargetHelper;
	CComPtr<IDropTargetHelper> dropTargetHelper;

public:
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnWindowPosChanged(WINDOWPOS *lpwndpos);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	DECLARE_MESSAGE_MAP();
};

#endif /* PLUGINLIST_H */