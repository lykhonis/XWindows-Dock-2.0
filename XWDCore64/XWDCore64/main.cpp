#include <windows.h>
#include "intercept.h"

#pragma comment(lib, "user32.lib")

enum
{
	xwdCoreProcessAttach = 1,
	xwdCoreProcessDetach,
	xwdCoreWindowChangedSize,
	xwdCoreWindowShow,
	xwdCoreWindowHide,
	xwdCoreWindowFlash,
	xwdCoreWindowGotFocus,
	xwdCoreWindowMinimize,
	xwdCoreWindowRestore,
	xwdCoreWindowClose,
	xwdCoreApplicationHasWindow
};

static const UINT WM_XWDCORENOTIFY = RegisterWindowMessage(L"WM_XWDCORENOTIFY");

#define XWINDOWS_DOCK_CLASS L"XWindowsDockClass"
//#define XWINDOWS_DOCK_MOUSEEVENT L"MouseEvent"

HWND dockWindow = NULL;

void FlashWindowXWD(HWND hWnd, WORD Flags)
{
	if(dockWindow)
	{
		PostMessage(dockWindow, WM_XWDCORENOTIFY, MAKEWPARAM(xwdCoreWindowFlash, Flags), (LPARAM)hWnd);
	}
}

INTERCEPT_DECLARE_INFO(FlashWindow);
BOOL WINAPI icFlashWindow(HWND hWnd, BOOL bInvert) 
{
	FlashWindowXWD(hWnd, bInvert ? FLASHW_ALL : FLASHW_STOP);
	return ((BOOL(WINAPI*)(HWND, BOOL))INTERCEPT_CALLREAL(FlashWindow))(hWnd, bInvert);
}

INTERCEPT_DECLARE_INFO(FlashWindowEx);
BOOL WINAPI icFlashWindowEx(PFLASHWINFO pfwi) 
{
	if(pfwi)
	{
		FlashWindowXWD(pfwi->hwnd, LOWORD(pfwi->dwFlags));
	}
	return ((BOOL(WINAPI*)(PFLASHWINFO))INTERCEPT_CALLREAL(FlashWindowEx))(pfwi);
}

INTERCEPT_DECLARE_INFO(SetFocus);
HWND WINAPI icSetFocus(HWND hWnd)
{
	if(dockWindow)
	{
		PostMessage(dockWindow, WM_XWDCORENOTIFY, MAKEWPARAM(xwdCoreWindowGotFocus, NULL), (LPARAM)hWnd);
	}
	return ((HWND(WINAPI*)(HWND))INTERCEPT_CALLREAL(SetFocus))(hWnd);
}

BOOL WINAPI DllMain(__in HINSTANCE/* hinstDLL*/, __in DWORD fdwReason, __in LPVOID/* lpvReserved*/)
{
	switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		{
			dockWindow = FindWindow(XWINDOWS_DOCK_CLASS, NULL);
			if(dockWindow)
			{
				PostMessage(dockWindow, WM_XWDCORENOTIFY, MAKEWPARAM(xwdCoreProcessAttach, NULL), GetCurrentProcessId());
			}
			
			ThreadsState(true);
			//INTERCEPT_API(L"User32.dll", SetFocus);
			//INTERCEPT_API(L"User32.dll", FlashWindowEx);
			//INTERCEPT_API(L"User32.dll", FlashWindow);
			ThreadsState(false);
		}
		break;

	case DLL_PROCESS_DETACH:
		{
			ThreadsState(true);
			//INTERCEPT_RESTORE(SetFocus);
			//INTERCEPT_RESTORE(FlashWindowEx);
			//INTERCEPT_RESTORE(FlashWindow);
			ThreadsState(false);
		}
		break;
	}
	return TRUE;
}

LRESULT CALLBACK CBTProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (dockWindow)
	{
		switch(code)
		{
		case HCBT_MINMAX:
			{
				switch (LOWORD(lParam))
				{
				case SW_MINIMIZE:
				case SW_SHOWMINIMIZED:
				case SW_FORCEMINIMIZE:
				case SW_SHOWMINNOACTIVE:
					{
						if(IsWindowVisible((HWND)wParam) && !IsIconic((HWND)wParam))
						{
							SendMessage(dockWindow, WM_XWDCORENOTIFY, MAKEWPARAM(xwdCoreWindowMinimize, NULL), wParam);
						}
					}
					break;

				case SW_RESTORE:
					{
						PostMessage(dockWindow, WM_XWDCORENOTIFY, MAKEWPARAM(xwdCoreWindowRestore, NULL), wParam);
					}
					break;
				}
			}
			break;

		case HCBT_DESTROYWND:
			{
				PostMessage(dockWindow, WM_XWDCORENOTIFY, MAKEWPARAM(xwdCoreWindowClose, NULL), wParam);
			}
			break;
		}
	}
	return CallNextHookEx(0, code, wParam, lParam);
}

LRESULT CALLBACK WindowProc(int code, WPARAM wParam, LPARAM lParam)
{
	if((code == HC_ACTION) && dockWindow)
	{
		PCWPRETSTRUCT pcw = (PCWPRETSTRUCT)lParam;
		switch(pcw->message)
		{
		case WM_WINDOWPOSCHANGED:
			{
				WINDOWPOS *wndPos = (WINDOWPOS*)pcw->lParam;
				if(!(wndPos->flags & SWP_NOSIZE))
				{
					PostMessage(dockWindow, WM_XWDCORENOTIFY, MAKEWPARAM(xwdCoreWindowChangedSize, NULL), (LPARAM)pcw->hwnd);
				}
			}
			break;

		case WM_SHOWWINDOW:
			{
				if (pcw->wParam)
				{
					PostMessage(dockWindow, WM_XWDCORENOTIFY, MAKEWPARAM(xwdCoreWindowShow, NULL), (LPARAM)pcw->hwnd);
				}
				else
				{
					PostMessage(dockWindow, WM_XWDCORENOTIFY, MAKEWPARAM(xwdCoreWindowHide, NULL), (LPARAM)pcw->hwnd);
				}
			}
			break;

		/*case WM_SYSCOMMAND:
			{
				if (pcw->wParam == SC_CLOSE)
				{
					PostMessage(dockWindow, WM_XWDCORENOTIFY, MAKEWPARAM(xwdCoreWindowClose, NULL), (LPARAM)pcw->hwnd);
				}
			}
			break;*/
		}
	}
	return CallNextHookEx(0, code, wParam, lParam);
}

LRESULT CALLBACK MouseProc(int code, WPARAM wParam, LPARAM lParam)
{
	if(code == HC_ACTION)
	{
		switch(wParam)
		{
		case WM_NCMOUSEMOVE:
		case WM_MOUSEMOVE:
			{
				HANDLE h = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"XWDMouseEvent");
				if(h)
				{
					SetEvent(h);
					CloseHandle(h);
				}
			}
			break;
		}
	}
	return CallNextHookEx(0, code, wParam, lParam);
}
