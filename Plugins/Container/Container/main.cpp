#include <afxwin.h>
#include <afxdlgs.h>
#include "stdafx.h"
#include "XWDAPI.h"
#include "ContainerLib.h"

static CWinApp theApp;

using namespace Gdiplus;

XWDPluginType XWDAPICALL XWDGetPluginType()
{
	return XWDPluginDocklet;
}

XWDBool XWDAPICALL XWDGetPluginIcon(XWDString buff)
{
	wcscpy_s(buff, XWDStringLength, L"Container-Empty.png");
	return XWDTrue;
}

XWDVoid XWDAPICALL XWDGetPluginInformation(XWDString name, XWDString author, XWDString description, XWDUInt32 *version)
{
	wcscpy_s(name, XWDStringLength, L"Container");
	wcscpy_s(author, XWDStringLength, L"Lichonos Vladimir");
	wcscpy_s(description, XWDStringLength, L"Contain your all apps at the same place.");
	(*version) = 0x00000001; // 0.0.0.1
}

class CXWDHost;

typedef enum
{
	ViewAutomatic = 0,
	ViewAsFan,
	ViewAsGrid
} StackViewAs;

class XWDData
{
public:
	CContainer *container;
	CXWDHost *host;

	CString path;
	HANDLE hFile;
	bool threadActive;
	CWinThread *thread;
	HHOOK hMouse;
	int renderedCount;
	int oldState;
	int canLoadIcons;
	StackViewAs viewAs;

	XWDId id;
	XWDId imageStack;
	XWDId imageEmpty;
	XWDId imageOpened;
	XWDId imageTransparent;
	XWDId imageUser;
	XWDId menuChooseFolder;
	XWDId menuOpenFolder;
	XWDId menuSetIcon;
	XWDId menuViewAs;
	XWDId menuViewAsFan;
	XWDId menuViewAsGrid;
	XWDId menuViewAsAutomatic;
};

static const UINT WM_XWDREADCHANGES = RegisterWindowMessage(L"WM_XWDREADCHANGES");

void XWDLoadIcons(XWDData *data, CString path);
void XWDDrawIcon(XWDData *data);

class CXWDHost: public CFrameWnd
{
public:
	XWDData *data;

public:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		if(message == WM_XWDREADCHANGES)
		{
			XWDData *data = (XWDData*)lParam;
			CXWDHost::data = data;
			KillTimer(1);
			SetTimer(1, 1000, NULL);
		}
		else
		if(message == WM_TIMER)
		{
			switch(wParam)
			{
			case 1:
				{
					if(!data->container->bckg->IsWindowVisible() && !data->canLoadIcons)
					{
						KillTimer(1);
						XWDLoadIcons(data, data->path);
						XWDExec(XWDDockletBounce, data->id, 1, 1);
					}
				}
				break;
			}
		}
		else
		if(message == WM_CONTAINEREVENT)
		{
			XWDData *data = (XWDData*)lParam;
			switch(wParam)
			{
			case ContainerEventPopupAnimation:
				{
					XWDExec(XWDDockletSetImage, data->id, data->imageOpened);
				}
				break;

			case ContainerEventHideIcons:
				{
					bool render = false;
					int n = 0;
					POSITION p = data->container->icons.GetHeadPosition();
					while(p)
					{
						CContainerIcon *icon = data->container->icons.GetAt(p);
						if(icon->rendered)
						{
							render = true;
							icon->rendered = false;
						}
						if(icon->thumbReady)
						{
							n++;
						}
						data->container->icons.GetNext(p);
					}
					if((n == data->renderedCount) && !render)
					{
						XWDExec(XWDDockletSetImage, data->id, data->imageStack);
					}
				}
				break;

			case ContainerEventItemBeginDrag:
			case ContainerEventKillFocus:
			case ContainerEventItemExec:
				{
					if(data->container->bckg->IsWindowVisible())
					{
						bool render = false;
						int n = 0;
						POSITION p = data->container->icons.GetHeadPosition();
						while(p)
						{
							CContainerIcon *icon = data->container->icons.GetAt(p);
							if(icon->rendered)
							{
								render = true;
								icon->rendered = false;
							}
							if(icon->thumbReady)
							{
								n++;
							}
							data->container->icons.GetNext(p);
						}

						XWDExec(XWDDockletSetImage, data->id, data->imageTransparent);
						data->container->Hide((GetKeyState(VK_SHIFT) & 0x8000) == 0x8000);
						XWDExec(XWDDockletLockMouseEffect, data->id, XWDFalse);

						if((n != data->renderedCount) || render)
						{
							data->oldState = (data->renderedCount == 0 ? 3 : 1);
							data->renderedCount = n;
							XWDDrawIcon(data);
							XWDExec(XWDDockletSetImage, data->id, data->imageStack);
						}
					}
				}
				break;
			}
		}
		return CFrameWnd::WindowProc(message, wParam, lParam);
	}
};

UINT AFX_CDECL XWDReadChangesThread(LPVOID param)
{
	XWDData *data = (XWDData*)param;
	const int buffSize = 60 * 1024;
	void *buff = malloc(buffSize);
	DWORD ret;
	while(data->threadActive)
	{
		if(ReadDirectoryChangesW(data->hFile, buff, buffSize, FALSE,
			FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
			FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | 
			FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION, &ret, NULL, NULL) && ret && data->hFile)
		{
			bool notify = false;
			FILE_NOTIFY_INFORMATION *fni = (FILE_NOTIFY_INFORMATION*)buff;
			while(!notify)
			{
				switch(fni->Action)
				{
				case FILE_ACTION_ADDED:
				case FILE_ACTION_MODIFIED:
					{
						CString fileName = data->path;
						fileName.Append(fni->FileName, fni->FileNameLength / sizeof(wchar_t));

						DWORD errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
						DWORD attr = GetFileAttributes(fileName.GetBuffer());
						SetErrorMode(errorMode);

						if(attr != INVALID_FILE_ATTRIBUTES)
						{
							if(attr & FILE_ATTRIBUTE_DIRECTORY)
							{
								notify = true;
							}
							else
							{
								HANDLE hFile = CreateFile((LPCWSTR)fileName.GetBuffer(), GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, NULL, NULL);
								if(hFile != INVALID_HANDLE_VALUE)
								{
									CloseHandle(hFile);
									notify = true;
								}
							}
						}
					}
					break;

				default:
					{
						notify = true;
					}
					break;
				}
				if(!fni->NextEntryOffset)
				{
					break;
				}
				fni = (FILE_NOTIFY_INFORMATION*)((DWORD)fni + fni->NextEntryOffset);
			}
			if(notify)
			{
				data->host->PostMessage(WM_XWDREADCHANGES, NULL, (LPARAM)data);
			}
		}
		Sleep(100);
	}
	free(buff);
	return 0;
}

LRESULT CALLBACK XWDMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode == HC_ACTION)
	{
		switch(wParam)
		{
		case WM_MOUSEWHEEL:
			{
				PMOUSEHOOKSTRUCTEX pmhs = (PMOUSEHOOKSTRUCTEX)lParam;
				HWND hWnd = WindowFromPoint(pmhs->pt);
				if(hWnd && ((int)GetProp(hWnd, L"IsContainerIcon") == TRUE))
				{
					CContainerIcon *icon = (CContainerIcon*)GetProp(hWnd, L"ContainerIcon");
					if(icon && icon->isVideo)
					{
						icon->videoPosition += ((int)HIWORD(pmhs->mouseData) < 0 ? -0.05 : 0.05);
						if(icon->videoPosition < 0) icon->videoPosition = 1;
						if(icon->videoPosition > 1) icon->videoPosition = 0;
						icon->thumbReady = false;
						icon->rendered = true;
						icon->LoadThumb();
					}
				}
			}
			break;
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

XWDData* XWDAPICALL XWDPluginInitialize(XWDId id)
{
	XWDData *data = new XWDData();

	HWND hWnd;
	XWDExec(XWDDockletGetPanelHWND, &hWnd);

	data->host = new CXWDHost();
	data->host->CreateEx(WS_EX_TOOLWINDOW, NULL, NULL, WS_POPUP, 0, 0, 0, 0, NULL, NULL);

	data->renderedCount = 0;
	data->canLoadIcons = 0;
	data->id = id;
	data->hFile = NULL;
	data->threadActive = true;
	
	data->container = new CContainer(hWnd);
	data->container->param = data;
	data->container->callBack = data->host;

	data->thread = AfxBeginThread(XWDReadChangesThread, (LPVOID)data, THREAD_PRIORITY_IDLE, 0, CREATE_SUSPENDED);
	if(data->thread)
	{
		data->thread->m_bAutoDelete = FALSE;
		data->thread->ResumeThread();
	}

	data->hMouse = SetWindowsHookEx(WH_MOUSE, XWDMouseProc, NULL, GetCurrentThreadId());

	return data;
}

XWDVoid XWDAPICALL XWDPluginDeinitialize(XWDId, XWDData *data)
{
	if(data->hMouse)
	{
		UnhookWindowsHookEx(data->hMouse);
	}
	if(data->hFile)
	{
		CloseHandle(data->hFile);
		data->hFile = NULL;
	}
	if(data->thread)
	{
		data->threadActive = false;
		WaitForSingleObject(data->thread->m_hThread, INFINITE);
		delete data->thread;
	}
	data->host->DestroyWindow();
	delete data->container;
	delete data;
}

void XWDDrawIcon(XWDData *data)
{
	XWDRect rect;
	XWDExec(XWDDockletGetRect, data->id, &rect);
	XWDExec(XWDImageResize, data->imageStack, rect.width, rect.height);

	if(data->imageUser)
	{
		Bitmap *src;
		XWDExec(XWDImageGetGdiplusBitmap, data->imageStack, &src);

		Graphics g(src);
		g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

		Bitmap *bmp;
		XWDExec(XWDImageGetGdiplusBitmap, data->imageUser, &bmp);
		
		float k = min((float)rect.width / bmp->GetWidth(), (float)rect.height / bmp->GetHeight());

		RectF rf;
		rf.Width = bmp->GetWidth() * k;
		rf.Height = bmp->GetHeight() * k;
		rf.X += (rect.width - rf.Width) / 2;
		rf.Y = 0;

		g.DrawImage(bmp, rf);
	}
	else
	if(data->container->icons.IsEmpty())
	{
		Bitmap *src;
		XWDExec(XWDImageGetGdiplusBitmap, data->imageStack, &src);

		Graphics g(src);
		g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

		Bitmap *bmp;
		XWDExec(XWDImageGetGdiplusBitmap, data->imageEmpty, &bmp);
		
		float k = min((float)rect.width / bmp->GetWidth(), (float)rect.height / bmp->GetHeight());

		RectF rf;
		rf.Width = bmp->GetWidth() * k;
		rf.Height = bmp->GetHeight() * k;
		rf.X += (rect.width - rf.Width) / 2;
		rf.Y = 0;

		g.DrawImage(bmp, rf);
	}
	else
	{
		HDC dc;
		XWDExec(XWDImageGetHDC, data->imageStack, &dc);

		Graphics g(dc);
		g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

		int n = min(data->container->icons.GetCount() - 1, 2);
		int i = n;
		POSITION p = data->container->icons.FindIndex(i);
		while(p)
		{
			CDIB *dib = data->container->icons.GetAt(p)->icon;

			float nk = 0.9f - 0.24f * i / max(n, 1);
			float k = min((float)rect.width / dib->Width(), (float)rect.height / dib->Height());

			RectF rf;
			rf.Width = dib->Width() * k * nk;
			rf.Height = dib->Height() * k * nk;
			rf.X = (rect.width - rf.Width) / 2;
			rf.Y = rect.height * 0.34f * i / max(n, 1);

			g.DrawImage(dib->bmp, rf);

			i--;
			data->container->icons.GetPrev(p);
		}
	}
}

void XWDLoadIcons(XWDData *data, CString path)
{
	data->renderedCount = 0;
	data->oldState = 1;
	if(data->container->icons.IsEmpty() || data->path.IsEmpty())
	{
		data->oldState = 0;
	}
	data->container->RemoveIconAll();
	if(path.IsEmpty())
	{
		XWDExec(XWDDockletSetLabel, data->id, L"Container - Empty");
		XWDExec(XWDDockletSetImage, data->id, data->imageEmpty);
		return;
	}

	if(path.Mid(path.GetLength() - 1) != L'\\')
	{
		path += L'\\';
	}

	WIN32_FIND_DATA findData = {0};
	HANDLE hFind;

	// check or setup stack's view's mode
	switch(data->viewAs)
	{
	case ViewAutomatic:
		{
			data->container->viewMode = ContainerViewModeFan;
			int n = 0;
			hFind = FindFirstFile(CString(path + L"*").GetBuffer(), &findData);
			if(hFind != INVALID_HANDLE_VALUE)
			{
				do
				{
					if(((findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == 0) &&
						((findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0))
					{
						if((wcscmp(findData.cFileName, L".") != 0) && (wcscmp(findData.cFileName, L"..") != 0))
						{
							n++;
							if(n > data->container->limitIconsFan)
							{
								data->container->viewMode = ContainerViewModeGrid;
								break;
							}
						}
					}
				}
				while(FindNextFile(hFind, &findData));
				FindClose(hFind);
			}
		}
		break;

	case ViewAsFan:
		{
			data->container->viewMode = ContainerViewModeFan;
		}
		break;

	case ViewAsGrid:
		{
			data->container->viewMode = ContainerViewModeGrid;
		}
		break;
	}

	int nFiles = 0;

	hFind = FindFirstFile(CString(path + L"*").GetBuffer(), &findData);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(((findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == 0) &&
				((findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0) &&
				((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0))
			{
				if((wcscmp(findData.cFileName, L".") != 0) && (wcscmp(findData.cFileName, L"..") != 0))
				{
					CContainerIcon *icon = data->container->AddIcon();
					if(icon)
					{
						icon->text = findData.cFileName;
						icon->path = path + findData.cFileName;
						icon->LoadIcon();
					}
					nFiles++;
				}
			}
		}
		while(FindNextFile(hFind, &findData));
		FindClose(hFind);
	}

	hFind = FindFirstFile(CString(path + L"*").GetBuffer(), &findData);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(((findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == 0) &&
				((findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0) &&
				((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY))
			{
				if((wcscmp(findData.cFileName, L".") != 0) && (wcscmp(findData.cFileName, L"..") != 0))
				{
					CContainerIcon *icon = data->container->AddIcon();
					if(icon)
					{
						icon->text = findData.cFileName;
						icon->path = path + findData.cFileName;
						icon->LoadIcon();
					}
					nFiles++;
				}
			}
		}
		while(FindNextFile(hFind, &findData));
		FindClose(hFind);
	}

	CString buf;
	buf = path;
	buf.Delete(buf.GetLength() - 1); // L'\\'
	buf = buf.Mid(buf.ReverseFind(L'\\') + 1);
	data->container->caption = buf;

	buf.Format(L"%s - %d files", data->container->caption.GetBuffer(), nFiles);
	XWDExec(XWDDockletSetLabel, data->id, buf.GetBuffer());

	if((path != data->path) || !data->hFile)
	{
		data->path = path;

		if(data->hFile)
		{
			CloseHandle(data->hFile);
		}
		data->hFile = CreateFile(data->path.GetBuffer(), GENERIC_READ, 
			FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);

		if(data->hFile == INVALID_HANDLE_VALUE)
		{
			data->hFile = NULL;
		}
	}

	XWDDrawIcon(data);
	XWDExec(XWDDockletSetImage, data->id, data->imageStack);
}

XWDBool XWDAPICALL XWDPluginEvent(XWDId id, XWDData *data, XWDEvent uEvent, va_list args)
{
	switch(uEvent)
	{
	case XWDEventCreate:
		{
			XWDString s = {0};

			XWDExec(XWDDockletMenuAdd, id, XWDNull, NULL, NULL, XWDMenuFlagsLine);
			XWDExec(XWDDockletMenuAdd, id, XWDNull, &data->menuViewAs, L"View as", XWDMenuFlagsNone);
			XWDExec(XWDDockletMenuAdd, id, data->menuViewAs, &data->menuViewAsAutomatic, L"Automatic", XWDMenuFlagsCheckbox);
			XWDExec(XWDDockletMenuAdd, id, data->menuViewAs, &data->menuViewAsFan, L"Fan", XWDMenuFlagsCheckbox);
			XWDExec(XWDDockletMenuAdd, id, data->menuViewAs, &data->menuViewAsGrid, L"Grid", XWDMenuFlagsCheckbox);
			XWDExec(XWDDockletMenuAdd, id, XWDNull, &data->menuSetIcon, L"Set Icon", XWDMenuFlagsNone);
			XWDExec(XWDDockletMenuAdd, id, XWDNull, &data->menuOpenFolder, L"Open Folder", XWDMenuFlagsNone);
			XWDExec(XWDDockletMenuAdd, id, XWDNull, &data->menuChooseFolder, L"Choose Folder", XWDMenuFlagsNone);

			XWDExec(XWDImageCreate, &data->imageTransparent);
			XWDExec(XWDImageResize, data->imageTransparent, 1, 1);

			XWDExec(XWDDockletGetPath, id, s);
			wcscat_s(s, XWDStringLength, L"Container-Empty.png");
			XWDExec(XWDImageCreate, &data->imageEmpty);
			XWDExec(XWDImageAssignFile, data->imageEmpty, s);

			XWDExec(XWDDockletGetPath, id, s);
			wcscat_s(s, XWDStringLength, L"Container-Opened.png");
			XWDExec(XWDImageCreate, &data->imageOpened);
			XWDExec(XWDImageAssignFile, data->imageOpened, s);

			XWDExec(XWDImageCreate, &data->imageStack);

			XWDExec(XWDDockletGetSettingsPath, id, s);
			wcscat_s(s, XWDStringLength, L"Settings.ini");

			XWDUInt32 pluginId;
			XWDExec(XWDDockletGetPluginId, id, &pluginId);

			CString buf;
			buf.Format(L"%d", pluginId);

			data->viewAs = (StackViewAs)GetPrivateProfileInt(buf.GetBuffer(), L"ViewMode", ViewAutomatic, s);

			XWDString userIcon = {0};
			data->imageUser = XWDNull;
			if(GetPrivateProfileString(buf.GetBuffer(), L"UserIcon", NULL, userIcon, XWDStringLength, s))
			{
				XWDExec(XWDImageCreate, &data->imageUser);
				if(XWDExec(XWDImageAssignFile, data->imageUser, userIcon) == XWDTrue)
				{
					XWDExec(XWDDockletMenuSetText, data->id, data->menuSetIcon, L"Remove Icon");
				}
				else
				{
					XWDExec(XWDImageDelete, data->imageUser);
					data->imageUser = XWDNull;
				}
			}

			XWDUInt8 position;
			XWDExec(XWDDockletGetDockEdge, &position);

			XWDExec(XWDDockletMenuSetEnabled, id, data->menuViewAsAutomatic, position == 3 ? XWDTrue : XWDFalse);
			XWDExec(XWDDockletMenuSetEnabled, id, data->menuViewAsFan, position == 3 ? XWDTrue : XWDFalse);

			if((position != 3) && (data->viewAs != ViewAsGrid))
			{
				data->viewAs = ViewAsGrid;
			}

			GetPrivateProfileString(buf.GetBuffer(), L"Path", NULL, s, XWDStringLength, s);
			XWDLoadIcons(data, s);

			return XWDTrue;
		}
		break;

	case XWDEventDockDestroy:
		{
			// kill parent
			SetWindowLong(data->container->bckg->m_hWnd, GWL_HWNDPARENT, NULL);
		}
		break;

	case XWDEventDestroy:
		{
			XWDExec(XWDImageDelete, data->imageStack);
			XWDExec(XWDImageDelete, data->imageEmpty);
			XWDExec(XWDImageDelete, data->imageOpened);
			XWDExec(XWDImageDelete, data->imageTransparent);
			if(data->imageUser != XWDNull)
			{
				XWDExec(XWDImageDelete, data->imageUser);
			}
			return XWDTrue;
		}
		break;

	case XWDEventAdjustIconSize:
		{
			XWDDrawIcon(data);
			XWDExec(XWDDockletSetImage, data->id, data->imageStack);
		}
		break;

	case XWDEventLButtonClick:
		{
			bool shift = ((GetKeyState(VK_SHIFT) & 0x8000) == 0x8000); 
			bool ctrl = ((GetKeyState(VK_CONTROL) & 0x8000) == 0x8000); 

			if(data->container->bckg->IsWindowVisible())
			{
				bool render = false;
				int n = 0;
				POSITION p = data->container->icons.GetHeadPosition();
				while(p)
				{
					CContainerIcon *icon = data->container->icons.GetAt(p);
					if(icon->rendered)
					{
						render = true;
						icon->rendered = false;
					}
					if(icon->thumbReady)
					{
						n++;
					}
					data->container->icons.GetNext(p);
				}

				XWDExec(XWDDockletSetImage, id, data->imageTransparent);
				data->container->Hide(shift);
				XWDExec(XWDDockletLockMouseEffect, id, XWDFalse);
				
				if((n != data->renderedCount) || render)
				{
					data->oldState = (data->renderedCount == 0 ? 3 : 1);
					data->renderedCount = n;
					XWDDrawIcon(data);
					XWDExec(XWDDockletSetImage, data->id, data->imageStack);
				}
			}
			else
			{
				if(data->path.IsEmpty())
				{
					HWND hWnd;
					XWDExec(XWDDockletGetPanelHWND, &hWnd);

					data->canLoadIcons++;
					CString dir;
					if(ShellIO::SelectDirectory(hWnd, L"Select Directory:", dir, true, false, true))
					{
						XWDLoadIcons(data, dir);

						XWDString s = {0};
						XWDExec(XWDDockletGetSettingsPath, id, s);
						wcscat_s(s, XWDStringLength, L"Settings.ini");

						XWDUInt32 pluginId;
						XWDExec(XWDDockletGetPluginId, id, &pluginId);

						CString buf;
						buf.Format(L"%d", pluginId);

						WritePrivateProfileString(buf.GetBuffer(), L"Path", dir.GetBuffer(), s);
					}
					data->canLoadIcons--;
				}
				else
				if(ctrl)
				{
					ShellExecute(NULL, L"open", data->path.GetBuffer(), NULL, NULL, SW_SHOWNORMAL);
				}
				else
				if(!data->container->icons.IsEmpty())
				{
					XWDUInt8 position;
					XWDExec(XWDDockletGetDockEdge, &position);

					XWDRect rect;
					XWDExec(XWDDockletGetRect, id, &rect);

					CRect rc(rect.left, rect.top, rect.left + rect.width, rect.top + rect.height);
					CPoint pt;
					switch(position)
					{
					case 3: // bottom
						{
							pt.x = rect.left + rect.width / 2;
							pt.y = rect.top;
						}
						break;

					case 0: // left
						{
							pt.x = rect.left + rect.width;
							pt.y = rect.top + rect.height / 2;
						}
						break;

					case 1: // top
						{
							pt.x = rect.left + rect.width / 2;
							pt.y = rect.top + rect.height;
						}
						break;

					case 2: // right
						{
							pt.x = rect.left;
							pt.y = rect.top + rect.height / 2;
						}
						break;
					}

					XWDExec(XWDDockletLockMouseEffect, id, XWDTrue);

					data->container->position = (ContainerPosition)position;
					data->container->Show(pt, rc, shift);
				}
			}
			return XWDTrue;
		}
		break;

	case XWDEventAdjustDockEdge:
		{
			XWDUInt8 position;
			XWDExec(XWDDockletGetDockEdge, &position);

			XWDExec(XWDDockletMenuSetEnabled, id, data->menuViewAsAutomatic, position == 3 ? XWDTrue : XWDFalse);
			XWDExec(XWDDockletMenuSetEnabled, id, data->menuViewAsFan, position == 3 ? XWDTrue : XWDFalse);

			if((position != 3) && (data->viewAs != ViewAsGrid))
			{
				data->viewAs = ViewAsGrid;
				XWDLoadIcons(data, data->path);
			}
		}
		break;

	case XWDEventMenuItemClick:
		{
			XWDId menuId = va_arg(args, XWDId);
			HWND hWnd;
			XWDExec(XWDDockletGetPanelHWND, &hWnd);

			if((menuId == data->menuViewAsAutomatic) || (menuId == data->menuViewAsFan) || (menuId == data->menuViewAsGrid))
			{
				data->canLoadIcons++;

				if(menuId == data->menuViewAsFan)
				{
					data->viewAs = ViewAsFan;
				}
				else
				if(menuId == data->menuViewAsGrid)
				{
					data->viewAs = ViewAsGrid;
				}
				else
				{
					data->viewAs = ViewAutomatic;
				}
				XWDLoadIcons(data, data->path);
				
				XWDString s = {0};
				XWDExec(XWDDockletGetSettingsPath, id, s);
				wcscat_s(s, XWDStringLength, L"Settings.ini");

				XWDUInt32 pluginId;
				XWDExec(XWDDockletGetPluginId, id, &pluginId);

				CString buf;
				buf.Format(L"%d", pluginId);
				CString vm;
				vm.Format(L"%d", data->viewAs);

				WritePrivateProfileString(buf.GetBuffer(), L"ViewMode", vm.GetBuffer(), s);

				data->canLoadIcons--;
			}
			else
			if(menuId == data->menuChooseFolder)
			{
				data->canLoadIcons++;
				CString dir;
				if(ShellIO::SelectDirectory(hWnd, L"Select Directory:", dir, true, false, true))
				{
					XWDLoadIcons(data, dir);

					XWDString s = {0};
					XWDExec(XWDDockletGetSettingsPath, id, s);
					wcscat_s(s, XWDStringLength, L"Settings.ini");

					XWDUInt32 pluginId;
					XWDExec(XWDDockletGetPluginId, id, &pluginId);

					CString buf;
					buf.Format(L"%d", pluginId);

					WritePrivateProfileString(buf.GetBuffer(), L"Path", dir.GetBuffer(), s);
				}
				data->canLoadIcons--;
			}
			else
			if(menuId == data->menuOpenFolder)
			{
				ShellExecute(NULL, L"open", data->path.GetBuffer(), NULL, NULL, SW_SHOWNORMAL);
			}
			else
			if(menuId == data->menuSetIcon)
			{
				if(data->imageUser)
				{
					XWDExec(XWDDockletMenuSetText, data->id, data->menuSetIcon, L"Set Icon");
					XWDExec(XWDImageDelete, data->imageUser);
					data->imageUser = XWDNull;
					XWDDrawIcon(data);
					XWDExec(XWDDockletSetImage, data->id, data->imageStack);

					XWDString s = {0};
					XWDExec(XWDDockletGetSettingsPath, id, s);
					wcscat_s(s, XWDStringLength, L"Settings.ini");

					XWDUInt32 pluginId;
					XWDExec(XWDDockletGetPluginId, id, &pluginId);

					CString buf;
					buf.Format(L"%d", pluginId);

					WritePrivateProfileString(buf.GetBuffer(), L"UserIcon", NULL, s);
				}
				else
				{
					CFileDialog *dlg = new CFileDialog(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, 
						L"Images (*.png;*.ico;*.bmp;*.gif;*.jpg;*.jpeg)|*.png;*.ico;*.bmp;*.gif;*.jpg;*.jpeg");
					if(dlg->DoModal() == IDOK)
					{
						XWDExec(XWDImageCreate, &data->imageUser);
						if(XWDExec(XWDImageAssignFile, data->imageUser, dlg->GetPathName()) == XWDTrue)
						{
							XWDExec(XWDDockletMenuSetText, data->id, data->menuSetIcon, L"Remove Icon");
							XWDDrawIcon(data);
							XWDExec(XWDDockletSetImage, data->id, data->imageStack);

							XWDString s = {0};
							XWDExec(XWDDockletGetSettingsPath, id, s);
							wcscat_s(s, XWDStringLength, L"Settings.ini");

							XWDUInt32 pluginId;
							XWDExec(XWDDockletGetPluginId, id, &pluginId);

							CString buf;
							buf.Format(L"%d", pluginId);

							WritePrivateProfileString(buf.GetBuffer(), L"UserIcon", dlg->GetPathName(), s);
						}
						else
						{
							XWDExec(XWDImageDelete, data->imageUser);
							data->imageUser = XWDNull;
							MessageBox(hWnd, L"Failed. Cannot load this icon.", L"Container", MB_ICONEXCLAMATION);
						}
					}
					delete dlg;
				}
			}
			return XWDTrue;
		}
		break;

	case XWDEventDelete:
		{
			XWDString s = {0};
			XWDExec(XWDDockletGetSettingsPath, id, s);
			wcscat_s(s, XWDStringLength, L"Settings.ini");

			XWDUInt32 pluginId;
			XWDExec(XWDDockletGetPluginId, id, &pluginId);

			CString buf;
			buf.Format(L"%d", pluginId);

			WritePrivateProfileString(buf.GetBuffer(), NULL, NULL, s);

			return XWDTrue;
		}
		break;

	case XWDEventMenuPopup:
		{
			XWDExec(XWDDockletMenuSetEnabled, id, data->menuOpenFolder, data->path.IsEmpty() ? XWDFalse : XWDTrue);
			XWDExec(XWDDockletMenuSetEnabled, id, data->menuViewAs, data->path.IsEmpty() ? XWDFalse : XWDTrue);
			XWDExec(XWDDockletMenuSetChecked, id, data->menuViewAsAutomatic, data->viewAs == ViewAutomatic ? XWDTrue : XWDFalse);
			XWDExec(XWDDockletMenuSetChecked, id, data->menuViewAsGrid, data->viewAs == ViewAsGrid ? XWDTrue : XWDFalse);
			XWDExec(XWDDockletMenuSetChecked, id, data->menuViewAsFan, data->viewAs == ViewAsFan ? XWDTrue : XWDFalse);
		}
		break;

	case XWDEventDropEnter:
		{
			LPITEMIDLIST root = va_arg(args, LPITEMIDLIST);
			LPITEMIDLIST *items = va_arg(args, LPITEMIDLIST*);
			XWDUInt16 count = va_arg(args, XWDUInt16);

			if(count >= 1)
			{
				if(data->path.IsEmpty())
				{
					CString path = ShellIO::PIDLToString((*items), root, SHGDN_FORPARSING);

					DWORD mode = SetErrorMode(SEM_FAILCRITICALERRORS);
					DWORD attr = GetFileAttributes(path.GetBuffer());
					SetErrorMode(mode);

					if((attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY))
					{
						return XWDTrue;
					}
				}
				else
				{
					return XWDTrue;
				}
			}
		}
		break;

	case XWDEventDropLeave:
		{
		}
		break;

	case XWDEventDrop:
		{
			LPITEMIDLIST root = va_arg(args, LPITEMIDLIST);
			LPITEMIDLIST *items = va_arg(args, LPITEMIDLIST*);
			XWDUInt16 count = va_arg(args, XWDUInt16);

			// check whether dragging by me
			if(count == 1)
			{
				POSITION p = data->container->icons.GetHeadPosition();
				while(p)
				{
					CContainerIcon *icon = data->container->icons.GetAt(p);
					if(icon->dragging)
					{
						CString path = ShellIO::PIDLToString((*items), root, SHGDN_FORPARSING);
						if(icon->path.CompareNoCase(path) == 0)
						{
							return XWDTrue;
						}
					}
					data->container->icons.GetNext(p);
				}
			}

			if(count >= 1)
			{
				if(data->path.IsEmpty())
				{
					data->canLoadIcons++;
					CString path = ShellIO::PIDLToString((*items), root, SHGDN_FORPARSING);

					XWDLoadIcons(data, path);

					XWDString s = {0};
					XWDExec(XWDDockletGetSettingsPath, id, s);
					wcscat_s(s, XWDStringLength, L"Settings.ini");

					XWDUInt32 pluginId;
					XWDExec(XWDDockletGetPluginId, id, &pluginId);

					CString buf;
					buf.Format(L"%d", pluginId);

					WritePrivateProfileString(buf.GetBuffer(), L"Path", path.GetBuffer(), s);
					data->canLoadIcons--;
				}
				else
				{
					// prepearing source files/folders
					int srcSize = 2 * sizeof(wchar_t); // indicates the end of paths

					// calculating size
					LPITEMIDLIST *pId = items;
					for(int i = 0; i < count; i++, pId++)
					{
						CString path = ShellIO::PIDLToString((*pId), root, SHGDN_FORPARSING);
						srcSize += (path.GetLength() + 1) * sizeof(wchar_t);
					}

					wchar_t *src = (wchar_t*)malloc(srcSize);
					memset(src, 0, srcSize);
					wchar_t *n = src;

					// filling in data
					pId = items;
					for(int i = 0; i < count; i++, pId++)
					{
						CString path = ShellIO::PIDLToString((*pId), root, SHGDN_FORPARSING);
						memcpy(n, path.GetBuffer(),path.GetLength() * sizeof(wchar_t));
						n += path.GetLength() + 1;
					}

					int dstSize = (data->path.GetLength() + 2) * sizeof(wchar_t);
					wchar_t *dst = (wchar_t*)malloc(dstSize);
					memset(dst, 0, dstSize);
					memcpy(dst, data->path.GetBuffer(), data->path.GetLength() * sizeof(wchar_t));

					SHFILEOPSTRUCT fop = {0};
					fop.wFunc = ((GetKeyState(VK_CONTROL) & 0x8000) == 0x8000) ? FO_COPY : FO_MOVE; 
					fop.pFrom = src;
					fop.pTo = dst;

					SHFileOperation(&fop);

					free(src);
					free(dst);
				}
			}
		}
		break;
	}
	return XWDFalse;
}
