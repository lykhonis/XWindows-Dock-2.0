#ifndef XWD_H
#define XWD_H

#include <afxwin.h>
#include <gdiplus.h>
#include <afxole.h>
#include <afxdlgs.h>
#include <time.h>
#include <math.h>
#include "stdafx.h"
#include "consts.h"
#include "skin.h"
#include "monitors.h"
#include "screencmp.h"
#include "core.h"
#include "items.h"
#include "balloontext.h"
#include "menu.h"
#include "dock3d.h"
#include "about.h"
#include "editicon.h"
#include "preferences.h"
#include "plugins.h"
#include "oddocklets.h"
#include "XWDAPI.h"
#include "XWDPublicAPI.h"
#include "update.h"
#include "expose.h"
#include "folderwatch.h"
#include "minimizer.h"

class CXWD: public CFrameWnd, public COleDropTarget
{
public:
	CXWD();
	~CXWD();

	bool IsFirstTimeStart();

	void LayerDraw(CDIB *dib);
	void LayerUpdate(CDIB *dib);

	void DrawDock2D(CDIB *dib, Gdiplus::Graphics &g, Gdiplus::RectF &rf);
	void DrawDock3D(CDIB *dib, Gdiplus::Graphics &g, Gdiplus::RectF &rf);
	void DrawDock3DReflection(CDIB *dib);

	bool MakeBlurBehind(bool disabled = false);

	void DockAdjustSize(bool animating = false, bool resize = false);
	void DockAdjustPosition(bool animating = false);
	CRect DockRect(unsigned int position);

	bool DockSwitchMode(DockMode mode);
	bool DockSwitchPosition(DockPosition position);

	void CheckFullScreen();

	void ItemsSave();
	void ItemsLoad();
	void ItemsLoadDefault();

	void DoItemDropSetupIcon(CDockItem *item);
	void DoItemDropRunIn(CDockItem *item);
	void DoItemDropCopyTo(CDockItem *item, bool move = false);
	void DoItemDropMoveToRecyclerBin();
	void DoItemExpose(CDockItem *item);

	bool IsDockCoveredByAnyOfWindows();

	CSize ItemSize(CDockItem *item, unsigned int position);
	CRect ItemRect(CDockItem *item, unsigned int position, bool onScreen = false, bool offset = true);
	CDockItem* ItemAt(CPoint point, bool onScreen = false);
	CRect ItemIndicatorRect(CDockItem *item);
	//CRect ItemIndicatorRect(CDockItem *item, CRect itemRect);

	void DockRectOffset(CRect &rect);
	void DockRectOffset(CPoint &point);

	void UpdateReserveScreen();

	void SettingsSave();

	// Animations
	void AddItem(CDockItem *item, bool resizeDock = true);
	void RemoveItem(CDockItem *item);

	void AddRunningItem(CString path);
	void RemoveRunningItem(CString path);

	void StopBouncing();

	void MakeDockScreenshot();

public:
	void OnItemsItemMoving(CDockItem *item);
	void OnItemsLClick(CDockItem *item);
	void OnItemsLMouseDown(CDockItem *item);
	void OnItemsLMouseUp(CDockItem *item);
	void OnItemsRMouseDown(CDockItem *item);
	void OnItemsDragBegin();
	void OnItemsDragEnter(CDockItem *item);
	void OnItemsDragLeave(CDockItem *item);
	bool OnItemsDrop(CDockItem *item);

	void OnDragEnter(CPoint point, void *plugin);
	void OnDragOver(CPoint point, void *plugin);
	bool OnDrop();
	void OnDragLeave();

	void OnMenuSelect(int nID);
	void OnMenuPopup(CDockItem *item, bool dockMenu, bool drop);
	void OnMenuHide(bool clicked);

public:
	DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	void OnDragLeave(CWnd* pWnd);

public:
	static BOOL CALLBACK FullScreenEnumWindows(HWND hWnd, LPARAM lParam);
	static BOOL CALLBACK FindMenuEnumWindows(HWND hWnd, LPARAM lParam);
	static BOOL CALLBACK WindowIsCoveredEnumWindows(HWND hWnd, LPARAM lParam);

	// XWD API
	static XWDBool XWDExecProc(XWDFunction function, XWDError &xwdLastError, va_list args);

	// OD API
	static BOOL ODDockletIsVisible(HWND hwndDocklet);
	static BOOL ODDockletGetRect(HWND hwndDocklet, RECT *rcDocklet);
	static int ODDockletGetLabel(HWND hwndDocklet, char *szLabel);
	static int ODDockletSetLabel(HWND hwndDocklet, char *szLabel);
	static Gdiplus::Bitmap* ODDockletLoadGDIPlusImage(char *szImage);
	static void ODDockletSetImage(HWND hwndDocklet, Gdiplus::Image *lpImageNew, BOOL bAutomaticallyDeleteImage);
	static void ODDockletSetImageFile(HWND hwndDocklet, char *szImage);
	static void ODDockletSetImageOverlay(HWND hwndDocklet, Gdiplus::Image *imageOverlay, BOOL bAutomaticallyDeleteImage);
	static BOOL ODDockletBrowseForImage(HWND hwndParent, char *szImage, char *szAlternateRelativeRoot);
	static void ODDockletLockMouseEffect(HWND hwndDocklet, BOOL bLock);
	static void ODDockletDoAttentionAnimation(HWND hwndDocklet);
	static void ODDockletGetRelativeFolder(HWND hwndDocklet, char *szFolder);
	static void ODDockletGetRootFolder(HWND hwndDocklet, char *szFolder);
	static void ODDockletDefaultConfigDialog(HWND hwndDocklet);
	static int ODDockletQueryDockEdge(HWND hwndDocklet);
	static int ODDockletSetDockEdge(HWND hwndDocklet, int iNewEdge);
	static int ODDockletQueryDockAlign(HWND hwndDocklet);
	static int ODDockletSetDockAlign(HWND hwndDocklet, int iNewAlign);

	// XWD Public Plugin API
	CDockItem* ItemByPublicPluginInfo(XWDPluginInfo *plugin);

public:
	int dockWidth;
	int dockHeight;
	float dockHeightOffset3d;

	unsigned int animationFlags;

	DWORD animationPopupStartAt;
	float animationPopup;

	bool showAllRunningAppsInDock;
	HRGN dockBlurRegion;
	DockMode dockMode;
	DockPosition dockPosition;
	bool itemsLock;
	bool iconShadowEnabled;
	bool windowsReflectionEnabled;
	int iconSize;
	unsigned int balloonTextSize;
	bool dockIsTopMost;
	bool dockIsTopMostNow;
	bool nowIsFullScreen;

public:
	CString appPath;
	CString dataPath;

	CDIB *dib;
	CDIB *bckg;
	CDIB *poof;
	CDIB *reflection;

	CDockItem *hoverItem;
	CDockItem *menuItem;
	CDockItem *dropItem;
	CDockItem *dropItemOver;
	CDockItem *dropLastItemOver;
	CDockItem *dragItem;
	CDockItem *exposedItem;
	bool draging;
	bool draged;
	bool exposed;
	bool portable;
	CPoint dragItemPt;
	bool recycleBinFull;
	int lockMouseEffect;
	bool dropIsIcon;
	bool dropIsPlugin;
	bool checkForUpdates;
	bool reserveScreen;
	bool reserveScreenOk;
	APPBARDATA barData;

	LPITEMIDLIST dropRoot;
	CList<LPITEMIDLIST> dropFiles;
	CPreferences *preferences;
	CAbout *about;
	CEditIcon *editIcon;

public:
	CSkinLoader *skinLoader;
	CSkin *skin;
	CExpose *expose;
	CMinimizer *minimizer;

	CMonitors *monitors;
	CMonitor *monitor;

	CPlugins *plugins;
	CODDocklets *odDocklets;

	CBalloonText *balloonText;

	CLMenu *menu;

	CList<CDockItem*> *items;

	bool userDropTargetHelper;
	CComPtr<IDropTargetHelper> dropTargetHelper;

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnScreenCmpNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDockItemNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMouseNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMenuNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnExposeNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCoreNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetupDock(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnControlNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnXWDUpdateNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg LRESULT OnDisplayChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSettingChange(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT *pCopyDataStruct);
	afx_msg LRESULT OnFileChangesNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	DECLARE_MESSAGE_MAP();
};

#endif /* XWD_H */