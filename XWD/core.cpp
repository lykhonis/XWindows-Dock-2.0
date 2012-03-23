#include "core.h"

typedef BOOL(WINAPI *IW64PFP)(HANDLE, BOOL*);

struct ApplicationStackItem
{
	CString path;
	DWORD ticks;
	int state;
};
typedef struct ApplicationStackItem ApplicationStackItem;

struct CoreData
{
	bool init;
	bool active;
	HMODULE dll;

	CWinThread *mouseThread;

	CWinThread *detachThread;
	CRITICAL_SECTION detachSection;

	HHOOK mouseHook;
	HOOKPROC mouseProc;
	CList<CWnd*> mouseNotifers;

	HHOOK windowHook;
	HOOKPROC windowProc;

	HHOOK cbtHook;
	HOOKPROC cbtProc;

	CStringList detachNotifers;

	CStringList waitAppWindowPaths;
	DWORD waitAppWindowCurrentPID;
	bool waitAppWindowFound;

	CList<ApplicationStackItem*> applicationsStack;
};
typedef struct CoreData CoreData;

UINT MouseThread(LPVOID param)
{
	CoreData *coreData = (CoreData*)param;
	
	HANDLE mouseEvent = CreateEvent(NULL, FALSE, FALSE, L"XWDMouseEvent");
	if(!mouseEvent)
	{
		return 0;
	}

	for(;coreData->active;)
	{
		DWORD ret = WaitForSingleObject(mouseEvent, 100);
		if(ret == WAIT_OBJECT_0)
		{
			POSITION p = coreData->mouseNotifers.GetHeadPosition();
			while(p)
			{
				coreData->mouseNotifers.GetAt(p)->PostMessage(WM_MOUSENOTIFY);
				coreData->mouseNotifers.GetNext(p);
			}
		}
	}

	CloseHandle(mouseEvent);
	return 0;
}

bool Core::IsApplicationWindow(HWND hWnd)
{
	if (!::IsWindowVisible(hWnd))
	{
		return false;
	}
	//DWORD style = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
	//if((style & WS_EX_TOOLWINDOW) || ((::GetParent(hWnd) != NULL) && ((style & WS_EX_APPWINDOW) == 0)))
	//{
	//	return false;
	//}
	wchar_t buff[MAX_PATH];
	GetClassName(hWnd, buff, MAX_PATH);
	if((wcscmp(buff, L"Progman") == 0) || (wcscmp(buff, L"WorkerW") == 0) || 
		(wcscmp(buff, L"Shell_TrayWnd") == 0) || (wcscmp(buff, L"XWindowsDockClass") == 0))
	{
		return false;
	}
	return true;
}

BOOL CALLBACK FindAddAppEnumWindows(HWND hWnd, LPARAM lParam)
{
	DWORD pid = NULL;
	GetWindowThreadProcessId(hWnd, &pid);
	CoreData *coreData = (CoreData*)lParam;
	if((coreData->waitAppWindowCurrentPID == pid) && Core::IsApplicationWindow(hWnd))
	{
		coreData->waitAppWindowFound = true;
		return FALSE;
	}
	return TRUE;
}

UINT DetachThread(LPVOID param)
{
	HWND hWnd = FindWindow(L"XWindowsDockClass", NULL);
	if(!hWnd)
	{
		return 0;
	}
	CoreData *coreData = (CoreData*)param;
	for(;coreData->active;)
	{
		EnterCriticalSection(&coreData->detachSection);
		
		CStringList windowFoundList;
		CStringList deleteList;
		POSITION p = coreData->detachNotifers.GetHeadPosition();
		while(p)
		{
			deleteList.AddTail(coreData->detachNotifers.GetAt(p));
			coreData->detachNotifers.GetNext(p);
		}

		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if(hSnapshot)
		{
			PROCESSENTRY32 processEntry = {0};
			processEntry.dwSize = sizeof(PROCESSENTRY32);
			if(Process32First(hSnapshot, &processEntry))
			do
			{
				CString path = GetPathByPID(processEntry.th32ProcessID);

				POSITION p = coreData->detachNotifers.GetHeadPosition();
				while(p)
				{
					if(coreData->detachNotifers.GetAt(p).CompareNoCase(path) == 0)
					{
						if(POSITION p2 = deleteList.Find(path))
						{
							deleteList.RemoveAt(p2);
						}
					}
					coreData->detachNotifers.GetNext(p);
				}

				p = coreData->waitAppWindowPaths.GetHeadPosition();
				while (p)
				{
					if(coreData->waitAppWindowPaths.GetAt(p).CompareNoCase(path) == 0)
					{
						coreData->waitAppWindowFound = false;
						coreData->waitAppWindowCurrentPID = processEntry.th32ProcessID;

						EnumWindows(FindAddAppEnumWindows, (LPARAM)coreData);

						if (coreData->waitAppWindowFound)
						{
							windowFoundList.AddTail(path);
						}
					}
					coreData->waitAppWindowPaths.GetNext(p);
				}
			}
			while(Process32Next(hSnapshot, &processEntry));
			CloseHandle(hSnapshot);
		}

		p = deleteList.GetHeadPosition();
		while(p)
		{
			CString path = deleteList.GetAt(p);

			PostMessage(hWnd, WM_XWDCORENOTIFY, MAKEWPARAM(xwdCoreProcessDetach, NULL), (LPARAM)(new CString(path)));

			if (POSITION p2 = coreData->detachNotifers.Find(path))
			{
				coreData->detachNotifers.RemoveAt(p2);
			}
			deleteList.GetNext(p);
		}

		p = windowFoundList.GetHeadPosition();
		while(p)
		{
			CString path = windowFoundList.GetAt(p);

			PostMessage(hWnd, WM_XWDCORENOTIFY, MAKEWPARAM(xwdCoreApplicationHasWindow, NULL), (LPARAM)(new CString(path)));

			if (POSITION p2 = coreData->waitAppWindowPaths.Find(path))
			{
				coreData->waitAppWindowPaths.RemoveAt(p2);
			}
			windowFoundList.GetNext(p);
		}

		p = coreData->applicationsStack.GetHeadPosition();
		while(p)
		{
			ApplicationStackItem *item = coreData->applicationsStack.GetAt(p);

			if (GetTickCount() - item->ticks >= 500)
			{
				PostMessage(hWnd, WM_XWDCORENOTIFY, MAKEWPARAM(xwdCoreApplicationStack, item->state), (LPARAM)(new CString(item->path)));

				delete item;
				coreData->applicationsStack.RemoveAt(p);

				p = coreData->applicationsStack.GetHeadPosition();
			}
			else
			{
				coreData->applicationsStack.GetNext(p);
			}
		}

		LeaveCriticalSection(&coreData->detachSection);
		Sleep(1000);
	}
	return 0;
}

static CoreData coreData = {0};

void Core::Run()
{
	Stop();

#ifdef _DEBUG
	CString path = L"E:\\Coding_C\\XWD\\Release\\";
#else
	wchar_t buff[MAX_PATH];
	GetModuleFileName(AfxGetInstanceHandle(), buff, MAX_PATH);
	CString path = buff;
	path = path.Mid(0, path.ReverseFind(L'\\') + 1);
#endif
	
	coreData.dll = LoadLibrary(CString(path + L"XWDCore.dll").GetBuffer());
	if(!coreData.dll)
	{
		return;
	}
	coreData.mouseProc = (HOOKPROC)GetProcAddress(coreData.dll, "MouseProc");
	coreData.windowProc = (HOOKPROC)GetProcAddress(coreData.dll, "WindowProc");
	coreData.cbtProc = (HOOKPROC)GetProcAddress(coreData.dll, "CBTProc");
	if(!coreData.mouseProc || !coreData.windowProc || !coreData.cbtProc)
	{
		FreeLibrary(coreData.dll);
		return;
	}
	
	coreData.mouseHook = SetWindowsHookEx(WH_MOUSE, coreData.mouseProc, coreData.dll, NULL);
	coreData.windowHook = SetWindowsHookEx(WH_CALLWNDPROCRET, coreData.windowProc, coreData.dll, NULL);
	coreData.cbtHook = SetWindowsHookEx(WH_CBT, coreData.cbtProc, coreData.dll, NULL);

	BOOL is64Bit = FALSE;
	IW64PFP IW64P = (IW64PFP)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "IsWow64Process");
	if(IW64P != NULL)
	{
		IW64P(GetCurrentProcess(), &is64Bit);
	}

	if (is64Bit)
	{
		ShellExecute(NULL, NULL, CString(path + L"XWDCore64.exe"), NULL, NULL, SW_HIDE);
	}

	InitializeCriticalSection(&coreData.detachSection);

	coreData.active = true;
	coreData.init = true;
	coreData.mouseThread = AfxBeginThread(MouseThread, &coreData);
	coreData.detachThread = AfxBeginThread(DetachThread, &coreData, THREAD_PRIORITY_IDLE);
}

void Core::Stop()
{
	if(!coreData.init)
	{
		return;
	}

	// do not forget about x64
	HWND hCore64 = FindWindow(L"XWindowsDockCore64", NULL);
	if (hCore64)
	{
		PostMessage(hCore64, WM_DESTROY, NULL, NULL);
	}

	UnhookWindowsHookEx(coreData.mouseHook);
	UnhookWindowsHookEx(coreData.windowHook);
	UnhookWindowsHookEx(coreData.cbtHook);
	coreData.active = false;
	HANDLE h[2];
	h[0] = coreData.mouseThread->m_hThread;
	h[1] = coreData.detachThread->m_hThread;
	WaitForMultipleObjects(2, h, TRUE, INFINITE);
	DeleteCriticalSection(&coreData.detachSection);
	FreeLibrary(coreData.dll);
	coreData.init = false;

	coreData.detachNotifers.RemoveAll();
	coreData.mouseNotifers.RemoveAll();

	POSITION p = coreData.applicationsStack.GetHeadPosition();
	while (p)
	{
		ApplicationStackItem *item = coreData.applicationsStack.GetAt(p);
		delete item;
		coreData.applicationsStack.GetNext(p);
	}
	coreData.applicationsStack.RemoveAll();
}

void Core::MouseAddNotifer(CWnd *notifer)
{
	coreData.mouseNotifers.AddTail(notifer);
}
	
void Core::MouseRemoveNotifer(CWnd *notifer)
{
	POSITION p = coreData.mouseNotifers.Find(notifer);
	if(p)
	{
		coreData.mouseNotifers.RemoveAt(p);
	}
}

void Core::DetachAddNotifer(CString path)
{
	if(coreData.init)
	{
		EnterCriticalSection(&coreData.detachSection);
	}
	coreData.detachNotifers.AddTail(path);
	if(coreData.init)
	{
		LeaveCriticalSection(&coreData.detachSection);
	}
}

void Core::DetachRemoveNotifer(CString path)
{
	if(coreData.init)
	{
		EnterCriticalSection(&coreData.detachSection);
	}
	POSITION p = coreData.detachNotifers.Find(path);
	if(p)
	{
		coreData.detachNotifers.RemoveAt(p);
	}
	if(coreData.init)
	{
		LeaveCriticalSection(&coreData.detachSection);
	}
}

void Core::WaitForAppWindowNotifer(CString path)
{
	if(coreData.init)
	{
		EnterCriticalSection(&coreData.detachSection);
	}
	if (!coreData.waitAppWindowPaths.Find(path))
	{
		coreData.waitAppWindowPaths.AddTail(path);
	}
	if(coreData.init)
	{
		LeaveCriticalSection(&coreData.detachSection);
	}
}

void Core::PushApplication(CString path)
{
	if(coreData.init)
	{
		EnterCriticalSection(&coreData.detachSection);
	}
	bool found = false;
	POSITION p = coreData.applicationsStack.GetHeadPosition();
	while (p)
	{
		ApplicationStackItem *item = coreData.applicationsStack.GetAt(p);
		if (item->path.CompareNoCase(path) == 0)
		{
			item->state = ApplicationStackAdded;
			item->ticks = GetTickCount();
			found = true;
			break;
		}
		coreData.applicationsStack.GetNext(p);
	}
	if (!found)
	{
		ApplicationStackItem *item = new ApplicationStackItem();
		item->path = path;
		item->ticks = GetTickCount();
		item->state = ApplicationStackAdded;
		coreData.applicationsStack.AddTail(item);
	}
	if(coreData.init)
	{
		LeaveCriticalSection(&coreData.detachSection);
	}
}

void Core::PopApplication(CString path)
{
	if(coreData.init)
	{
		EnterCriticalSection(&coreData.detachSection);
	}
	bool found = false;
	POSITION p = coreData.applicationsStack.GetHeadPosition();
	while (p)
	{
		ApplicationStackItem *item = coreData.applicationsStack.GetAt(p);
		if (item->path.CompareNoCase(path) == 0)
		{
			item->state = ApplicationStackRemoved;
			item->ticks = GetTickCount();
			found = true;
			break;
		}
		coreData.applicationsStack.GetNext(p);
	}
	if (!found)
	{
		ApplicationStackItem *item = new ApplicationStackItem();
		item->path = path;
		item->ticks = GetTickCount();
		item->state = ApplicationStackRemoved;
		coreData.applicationsStack.AddTail(item);
	}
	if(coreData.init)
	{
		LeaveCriticalSection(&coreData.detachSection);
	}
}