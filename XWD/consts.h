#ifndef CONSTS_H
#define CONSTS_H

#include <afxwin.h>
#include <shlobj.h>

static const float PI = 3.14159265358979323846264338327950288f;

// Type
struct RectI
{
	int X;
	int Y;
	int Width;
	int Height;
};
typedef struct RectI RectI;

// Dock/Skin/Preferences...
enum DockPosition
{
	DockPositionNone	= 0,
	DockPositionLeft	= 1 << 0,
	DockPositionTop		= 1 << 1,
	DockPositionRight	= 1 << 2,
	DockPositionBottom	= 1 << 3,
};

enum DockMode
{
	DockModeNone	= 0,
	DockMode2D		= 1 << 0,
	DockMode3D		= 1 << 1
};

// Control Dock
static const UINT WM_SETUPDOCK = RegisterWindowMessage(L"WM_SETUPDOCK");

enum
{
	// lo-wParam				hi-wParam
	SetupDockPosition = 1,		// DockPosition
	SetupDockMode,				// DockMode
	SetupDockIconSize,			// 20-256
	SetupDockMonitor,			// 0-... depeneds on monitors' count in the system
	SetupDockSkin,				// name of the skin
};

// Drag&Drop
static const CLIPFORMAT CF_SHELLIDLIST = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST);

struct DROPDATA
{
	UINT format;
	CString path;
	// use only if format is CF_SHELIDLIST
	LPITEMIDLIST pIdl;
	LPITEMIDLIST pParentIdl;
};
typedef struct DROPDATA DROPDATA;

// Controls

static const UINT WM_CONTROLNOTIFY = RegisterWindowMessage(L"WM_CONTROLNOTIFY");

enum 
{
	ButtonNormal = 0,
	ButtonPressed = 1,
	ButtonHover = 2,
	ButtonDisabled = 4,

	ScrollBarButtonPressed = 1,
	ScrollBarButtonUpPressed = 2,
	ScrollBarButtonDownPressed = 4
};

#endif /* CONSTS_H */