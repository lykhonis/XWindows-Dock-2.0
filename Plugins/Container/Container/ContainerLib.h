#ifndef CONTAINERLIB_H
#define CONTAINERLIB_H

#include <afxwin.h>
#include <afxtempl.h>
#include <math.h>
#include "dib.h"
#include "icons.h"
#include "video.h"

typedef enum
{
	ContainerPositionLeft = 0,
	ContainerPositionTop,
	ContainerPositionRight,
	ContainerPositionBottom
} ContainerPosition;

typedef enum
{
	ContainerViewModeGrid = 0,
	ContainerViewModeFan
} ContainerViewMode;

static const float PI = 3.14159265358979323846264338327950288f;

class CContainer;

class CContainerItem: public CFrameWnd
{
public:
	CContainerItem();
	~CContainerItem();

	void UpdateLayer(CDIB *dib = NULL);
	virtual void DrawLayer(CDIB *dib = NULL);

	virtual void DrawStep(CDIB *dst, CDIB *src, float step, bool blend);

public:
	CDIB *dib;
	CDIB *tmp;
	CContainer *container;
};

class CContainerPopupText: public CContainerItem
{
public:
	virtual void DrawLayer(CDIB *dib = NULL);
	virtual void DrawStep(CDIB *dst, CDIB *src, float step, bool blend);

	void Prepare();
	void Popup(CRect rect, CPoint pt);
	void Delete();

public:
	CString text;
	float radius;
	DWORD startAt;

public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP();
};

class CContainerBckg: public CContainerItem
{
public:
	virtual void DrawLayer(CDIB *dib = NULL);
	virtual void DrawStep(CDIB *dst, CDIB *src, float step, bool blend);

public:
	ContainerPosition position;
	ContainerViewMode viewMode;
	int ArrowOffset;

public:
	DECLARE_MESSAGE_MAP();
};

class CContainerIcon: public CContainerItem
{
public:
	CContainerIcon();
	~CContainerIcon();

	virtual void DrawLayer(CDIB *dib = NULL);
	virtual void DrawStep(CDIB *dst, CDIB *src, float step, bool blend);

	void LoadIcon();
	void LoadThumb();

	void BeginDrag(CPoint point);

	void Exec();

public:
	ContainerPosition position;
	CString path;
	CRect dstRect;
	CRect srcRect;
	CString text;
	CPoint pressedPt;
	bool pressed;
	float pressedAlpha;
	CDIB *icon;
	int index;
	bool dragging;
	bool thumbReady;
	bool thumbWorking;
	bool animating;
	bool rendered;
	bool isVideo;
	double videoPosition;
	bool mouseIsOver;
	bool shortText;
	CContainerPopupText *popupText;
	bool shutingDown;
	CWinThread *thumbThread;

public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();

	DECLARE_MESSAGE_MAP();
};

static const UINT WM_CONTAINEREVENT = RegisterWindowMessage(L"WM_CONTAINEREVENT");

// Events
enum
{
	ContainerEventPopupAnimation = 0,
	ContainerEventHideIcons,
	ContainerEventItemExec,
	ContainerEventItemBeginDrag,
	ContainerEventKillFocus
};

class CContainer
{
public:
	CContainer(HWND hParent = NULL);
	~CContainer();

	CContainerIcon* AddIcon();
	void RemoveIcon(CContainerIcon *icon);
	void RemoveIconAll();

	CRect IconRect(CContainerIcon *icon);
	CContainerIcon* IconAt(CPoint pt);

	static int TextWidth(CString s);
	void DrawIcon(CContainerIcon *icon);
	void DrawCaption();

	void Hide(bool slow = false);
	void Show(CPoint point, CRect rect, bool slow = false);

public:
	CWnd *callBack;
	void *param;
	ContainerPosition position;
	ContainerViewMode viewMode;
	CContainerBckg *bckg;
	CList<CContainerIcon*> icons;
	CString caption;

	// Customization
	int limitIconsGrid;
	int limitIconsFan;
	int textHeight;
	int iconSize;
	int cellSize;
	int separatorTextSize;
	int separatorSize;
};

#endif /* CONTAINERLIB_H */