#ifndef CORE_H
#define CORE_H

#include <afxwin.h>
#include <afxtempl.h>
#include "utils.h"

static const UINT WM_XWDCORENOTIFY = RegisterWindowMessage(L"WM_XWDCORENOTIFY");

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
	xwdCoreApplicationHasWindow,
	xwdCoreApplicationStack
};

static const UINT WM_MOUSENOTIFY = RegisterWindowMessage(L"WM_MOUSENOTIFY");

enum ApplicationStackState { ApplicationStackAdded = 0, ApplicationStackRemoved };

namespace Core
{
	void Run();
	void Stop();

	void MouseAddNotifer(CWnd *notifer);
	void MouseRemoveNotifer(CWnd *notifer);

	void DetachAddNotifer(CString path);
	void DetachRemoveNotifer(CString path);

	void WaitForAppWindowNotifer(CString path);

	void PushApplication(CString path);
	void PopApplication(CString path);

	bool IsApplicationWindow(HWND hWnd);
}

#endif /* CORE_H */