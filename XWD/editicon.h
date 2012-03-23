#ifndef EDITICON_H
#define EDITICON_H

#include <afxwin.h>
#include <gdiplus.h>
#include <afxole.h>
#include "consts.h"
#include "shellio.h"
#include "dib.h"
#include "icons.h"
#include "imagebutton.h"
#include "items.h"
#include "oledatasourceex.h"

class CEditIcon: public CFrameWnd, public COleDropTarget
{
public:
	CEditIcon();
	~CEditIcon();

	BOOL PreCreateWindow(CREATESTRUCT &cs);
	BOOL PreTranslateMessage(MSG *pMSG);

	void Draw();

	void DragIcon(CPoint point);
	void OpenFileDialog();

public:
	DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	void OnDragLeave(CWnd* pWnd);

public:
	CDockItem *item;
	CWnd *notifer;
	int nID;
//	CDIB *logo;
	CDIB *icon;
	CDIB *dib;
	CString iconPath;

	CEdit *editDesc;
	CEdit *editArguments;
	CEdit *editWorkDir;

	CImageButton *btnClose;

	bool drawIcon;
	bool mouseDown;
	CPoint mousePt;

	bool dropIsIcon;
	LPITEMIDLIST dropRoot;
	LPITEMIDLIST dropFile;

	bool hideArgWorkDir;
	
	bool userDropTargetHelper;
	CComPtr<IDropTargetHelper> dropTargetHelper;

public:
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnPaint();
	afx_msg void OnNcPaint();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS *lpncsp);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnWindowPosChanged(WINDOWPOS *lpwndpos);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnControlNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP();
};

#endif /* EDITICON_H */