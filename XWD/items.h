#ifndef ITEMS_H
#define ITEMS_H

#include <afxwin.h>
#include <afxtempl.h>
#include <math.h>
#include <gdiplus.h>
#include <Tlhelp32.h>
#include "consts.h"
#include "dib.h"
#include "icons.h"
#include "oddocklets.h"
#include "menu.h"
#include "plugins.h"
#include "notifications.h"
#include "XWDPublicAPI.h"
#include "folderwatch.h"

static const UINT WM_DOCKITEMNOTIFY = RegisterWindowMessage(L"WM_DOCKITEMNOTIFY");

enum
{
	DockItemLMouseDown	= 1,
	DockItemLMouseUp,
	DockItemLClick,
	DockItemRMouseDown,
	DockItemRMouseUp,
	DockItemMoving,

	StateFlagPressed			= 1 << 0,
	StateFlagDropOver			= 1 << 1
};

class CDockItemLayer: public CFrameWnd
{
public:
	CDockItemLayer();
	~CDockItemLayer();

	virtual void LayerDraw(CDIB *dib = NULL);
	void LayerUpdate(CDIB *dib = NULL);

	void Event(unsigned short dockItemEvent);

public:
	CDIB *dib;
	CDIB *image;

	unsigned int state;
	bool mouseDown;

public:
	afx_msg void OnWindowPosChanged(WINDOWPOS *lpwndpos);
	afx_msg void OnClose();

	DECLARE_MESSAGE_MAP();
};

class CDockItemImage: public CDockItemLayer
{
public:
	CDockItemImage();
	~CDockItemImage();

	virtual void LayerDraw(CDIB *dib = NULL);

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCmdMsg(UINT NID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo);

public:
	CDIB *imageOverlay;

public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

	DECLARE_MESSAGE_MAP();
};

class CDockItemReflection: public CDockItemLayer
{
public:
	CDockItemReflection();
	~CDockItemReflection();

	virtual void LayerDraw(CDIB *dib = NULL);

public:
	bool paint;
	CDIB *imageOverlay;
};

class CDockItemIndicator: public CDockItemLayer
{
public:
	CDockItemIndicator();
	~CDockItemIndicator();

	virtual void LayerDraw(CDIB *dib = NULL);
};

enum DockItemType
{
	DockItemTypeIcon = 0,
	DockItemTypeSeparator,
	DockItemTypeODDocklet,
	DockItemTypeDocklet,
	DockItemTypeUpdate,
	DockItemTypePublicPlugin,
	DockItemTypeRunningApp
};
typedef enum DockItemType DockItemType;

enum DockItemShowAs
{
	DockItemShowAsNormal = 0,
	DockItemShowAsMinimized,
	DockItemShowAsMaximized
};
typedef enum DockItemShowAs DockItemShowAs;

enum DockItemExec
{
	DockItemExecNormal = 0,
	DockItemExecIfNotRun
};
typedef enum DockItemExec DockItemExec;

enum
{
	animationFlagMoving			= 1 << 0,
	animationFlagPoof			= 1 << 1,
	animationFlagReflectionShow	= 1 << 4,
	animationFlagIndicatorHide	= 1 << 5,
	animationFlagBounce			= 1 << 6,
	animationFlagBounceAttention = 1 << 7,
	animationFlagBounceAttentionCancel = 1 << 8,

	identificatorRecycleBin		= 1 << 0,
	identificatorDirectory		= 1 << 1,
	identificatorApplication	= 1 << 2,
	identificatorIndicator		= 1 << 3
};

enum DockItemBounce
{
	DockItemBounceNormal = 0,
	DockItemBounceAttention
};
typedef enum DockItemBounce DockItemBounce;

class CDockItem: public CFrameWnd
{
public:
	CDockItem();
	~CDockItem();

	void Show(bool visible = true);
	bool Move(int x, int y, bool animation = false);
	void Size(int width, int height);
	void Rect(CRect rect);
	void Rect(int x, int y, int width, int height);

	bool LoadImage();
	bool LoadPath(CString path);

	void SetBottomWindow(CWnd *wnd);

	void Delete(); // free all windows/data

	void AddDraw(float k);

	void Poof(); // auto-delete
	void PoofDraw(int step); // maximu is 5th step

	bool Exec(DockItemExec exec, bool runAs, bool bounce);
	bool Exec(CString fileName, bool runAs);

	void Close();

	void ReflectionShow(bool visible = true, bool animation = false);
	void IndicatorShow(bool visible = true, bool animation = false);

	void SetState(unsigned int flags);
	void RemoveState(unsigned int flags);

	void Event(unsigned short dockItemEvent);

	void NotificationUpdatePositions(); // XWD Docklets

	void TopMost(bool topMost = true);

	void Bounce(DockItemBounce bounce, int count = 0);
	void BounceStop();
	void BounceCancelAttention();

public:
	CString path;
	CString text;
	CString icon;
	int iconIndex;
	CString arguments;
	CString workDirectory;

	DockMode dockMode;
	DockPosition dockPosition;

	unsigned int reflectionSize;
	unsigned int reflectionSkipTop;
	unsigned int reflectionSkipBottom;
	unsigned int reflectionOffset;
	unsigned char reflectionOpacity;
	unsigned char reflectionOpacityFactor;

	bool iconShadowEnabled;

	DockItemType type;
	DockItemShowAs showAs;
	DockItemExec exec;

	// some indificators
	unsigned int identificator;

	CString additionalIcon;
	CString additionalIcon2;

	// dock's resizing
	DWORD resizeStartAt;
	float resizeFlag;

	// animation
	unsigned int animationFlags;

	// icon's moving
	CPoint movePt;
	CPoint moveNextPt;
	DWORD moveStartAt;

	// poof
	DWORD poofStartAt;
	CDIB *poof;

	// add
	DWORD addStartAt;
	CDIB *addTmp;

	// reflection - show
	CDIB *reflectionTmp;
	DWORD reflectionShowStartAt;

	// indicator - show
	CDIB *indicatorTmp;
	DWORD indicatorShowStartAt;

	// bounce
	DWORD bounceStartAt;
	CPoint bouncePt;
	CPoint bounceNextPt;
	CSize bounceReflectionSize;
	DockItemBounce bounceType;
	int bounceCounter;
	DWORD bounceDelay;
	int bounceCount;
	int bounceCountNow;
	bool bounceCancelOnFocus;
	HWND bounceWindow;

	// OD Docklet
	int odDockletId;

	// XWD Plugin
	int pluginId;
	bool pluginCanDrop;

	// XWD Docklet
	CList<CLMenuItem*> menu;
	CList<CNotification*> notifications;
	XWDVoid *pluginData;

	// XWD Public Plugin
	XWDPluginInfo *publicPluginInfo;
	CList<FolderWatcher::FolderInfo*> folderWatcherList;

public:
	CRect rect;
	
	int imageShadowSize3d;

	CDockItemImage *image;
	CDockItemReflection *reflection;
	CDockItemIndicator *indicator;
	CODDocklet *docklet;

	CPlugin *plugin;

public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP();
};

#endif /* ITEMS_H */