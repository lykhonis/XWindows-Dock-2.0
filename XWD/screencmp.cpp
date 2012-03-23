#include "screencmp.h"

enum
{
	HandlerNone = 0,
	HandlerStop,
	HandlerRun,
	HandlerPause,
	HandlerContinue
};

struct ScreenCmp
{
	bool init;
	CWnd *notifer;
	CDIB *buffer;
	CDIB *buffProc;
	CRect rect;
	HANDLE signal;
	int handlerType;
	HANDLE handler;
};
typedef struct ScreenCmp ScreenCmp;

struct EnumData
{
	ScreenCmp *screenCmp;
	CDIB *buff;
	HWND wnd[200];
	int count;
};
typedef struct EnumData EnumData;

void ScreenCmpDrawWindow(HWND hWnd, CDIB *dib, CRect cmpRect, bool)
{
	CRect rect(0, 0, 0, 0);
	CRect inrect(0, 0, 0, 0);
	GetWindowRect(hWnd, &rect);
	if(rect.IsRectEmpty() || !inrect.IntersectRect(&rect, &cmpRect))
	{
		return;
	}
	BITMAPINFO bi = {0};
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biHeight = inrect.bottom - cmpRect.top - max(0, inrect.top - cmpRect.top);
	bi.bmiHeader.biWidth = inrect.Width();
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biSizeImage = bi.bmiHeader.biWidth * bi.bmiHeader.biHeight * 4;

	void *bits = NULL;
	HBITMAP bitmap = CreateDIBSection(0, &bi, DIB_RGB_COLORS, (void**)&bits, 0, 0);
	if(!bitmap)
	{
		return;
	}
	HDC dc = GetWindowDC(hWnd);
	HDC memdc = CreateCompatibleDC(dc);
	HBITMAP oldBitmap = (HBITMAP)SelectObject(memdc, bitmap);

	BitBlt(memdc, 0, 0, bi.bmiHeader.biWidth, bi.bmiHeader.biHeight, 
		dc, max(0, cmpRect.left - rect.left), max(0, inrect.top - rect.top), SRCCOPY);

	ReleaseDC(hWnd, dc);

	if(bits)
	{
		for(int y = 0; y < bi.bmiHeader.biHeight; y++)
		{
			DIB_ARGB *ps = (DIB_ARGB*)((int)bits + ((bi.bmiHeader.biHeight - 1 - y) * bi.bmiHeader.biWidth) * 4);
			DIB_ARGB *pd = (DIB_ARGB*)((int)dib->scan0 + 
				((dib->Height() - 1 - y - max(0, inrect.top - cmpRect.top)) * dib->Width() + 
				max(0, inrect.left - cmpRect.left)) * 4);
			for(int x = 0; x < bi.bmiHeader.biWidth; x++, ps++, pd++)
			{
				//if(ps->c)
				//{
					/*if(alpha)
					{
						float k1 = pd->a * ps->a / 255.0f / 255.0f;
						float k2 = 1.0f - k1;
						pd->r = (unsigned char)(pd->r * k2 + ps->r * k1);
						pd->g = (unsigned char)(pd->g * k2 + ps->g * k1);
						pd->b = (unsigned char)(pd->b * k2 + ps->b * k1);
					}
					else*/
					{
						pd->a = 255;
						pd->r = ps->r;
						pd->g = ps->g;
						pd->b = ps->b;
					}
				//}
			}
		}
	}

	SelectObject(memdc, oldBitmap);
	DeleteObject(memdc);
	DeleteObject(bitmap);

	/*BitBlt(dib->dc,
		rect.left - cmpRect.left,
		0,
		rect.Width(),
		inrect.Height(),
		dc,
		0,
		inrect.top - rect.top,
		SRCCOPY);*/
}

BOOL CALLBACK ScreenCmpEnumWindows(HWND hWnd, LPARAM lParam)
{
	EnumData *data = (EnumData*)lParam;
	if((GetParent(hWnd) != NULL) || !IsWindowVisible(hWnd))
	{
		return TRUE;
	}
	DWORD exStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	if(exStyle & WS_EX_LAYERED)
	{
		return TRUE;
	}
	DWORD pid = NULL;
	GetWindowThreadProcessId(hWnd, &pid);
	if(!pid || (pid == GetCurrentProcessId()))
	{
		return TRUE;		
	}
	wchar_t buff[256];
	GetClassName(hWnd, buff, 256);
	if((wcscmp(buff, L"Progman") == 0) || (wcscmp(buff, L"WorkerW") == 0))
	{
		return TRUE;
	}
	data->wnd[data->count] = hWnd;
	data->count++;
	return TRUE;
}

UINT AFX_CDECL ScreenCmpThread(LPVOID param)
{
	ScreenCmp *screenCmp = (ScreenCmp*)param;
	//EnumData enumData = {screenCmp, &buff};
	int width = 0;
	int height = 0;
	bool ready = false;
	int pause = 0;
	//HWND hwnd = GetDesktopWindow();
	//HDC hdc = GetDC(hwnd);
	HWND hProgman = FindWindow(L"Progman", NULL);
	HDC dc = GetWindowDC(hProgman);
	for(;;)
	{
		DWORD ret = WaitForSingleObject(screenCmp->handler, 10);
		if(ret == WAIT_OBJECT_0)
		{
			if(screenCmp->handlerType == HandlerStop)
			{
				SetEvent(screenCmp->signal);
				//CloseHandle(screenCmp->signal);
				//CloseHandle(screenCmp->handler);
				break;
			}
			else
			if(screenCmp->handlerType == HandlerRun)
			{
				if(!screenCmp->rect.IsRectEmpty())
				{
					width = screenCmp->rect.Width();
					height = screenCmp->rect.Height();

					screenCmp->buffer->Resize(width, height);
					screenCmp->buffProc->Resize(width, height);

					ready = screenCmp->buffProc->Ready() && screenCmp->buffer->Ready();
				}
				pause = 0;
				SetEvent(screenCmp->signal);
			}
			else
			if(screenCmp->handlerType == HandlerPause)
			{
				pause++;
				SetEvent(screenCmp->signal);
			}
			else
			if(screenCmp->handlerType == HandlerContinue)
			{
				if(pause)
				{
					pause--;
				}
				SetEvent(screenCmp->signal);
			}
		}
		if(ready && screenCmp->notifer && !pause)
		{
			//enumData.count = 0;
			//EnumWindows(ScreenCmpEnumWindows, (LPARAM)&enumData);

			//enumData.buff->Fill();
			//ScreenCmpDrawWindow(FindWindow(L"Progman", NULL), enumData.buff, enumData.screenCmp->rect, false); // wallpaper
			/*for(int i = enumData.count - 1; i >= 0; i--)
			{
				ScreenCmpDrawWindow(enumData.wnd[i], enumData.buff, enumData.screenCmp->rect, true);
			}*/

			//BitBlt(buff.dc, 0, 0, width, height, hdc, screenCmp->rect.left, screenCmp->rect.top, SRCCOPY);

			CRect rect;
			GetWindowRect(hProgman, &rect);

			BitBlt(screenCmp->buffProc->dc,
				0,
				0,
				rect.Width(),
				screenCmp->rect.Height(),
				dc,
				rect.left,
				screenCmp->rect.top - rect.top,
				SRCCOPY);

			void *scan0 = screenCmp->buffer->scan0;
			void *dstScan0 = screenCmp->buffProc->scan0;
			bool same = true;
			__asm 
			{
				push edi
				push esi
				push ecx

				mov eax,width
				mul height
				mov ecx,eax

				mov edi,scan0
				mov esi,dstScan0

			l1:
				mov eax,[esi]
				cmp eax,[edi]
				je l2
				mov same,0
				mov [edi],eax
			l2:
				lea edi,[edi+4]
				lea esi,[esi+4]
				loop l1

				pop ecx
				pop esi
				pop edi
			}
			if(!same)
			{
				screenCmp->notifer->PostMessage(WM_SCREENCMPNOTIFY);
			}
		}
	}
	ReleaseDC(hProgman, dc);
	//ReleaseDC(hwnd, hdc);
	return 0;
}

static ScreenCmp screenCmp = {0};

void SceenCompare::Draw(CDIB *buffer)
{
	if(!screenCmp.init)
	{
		return;
	}
	buffer->Resize(screenCmp.rect.Width(), screenCmp.rect.Height());
	if(!buffer->Ready())
	{
		return;
	}
	HWND hProgman = FindWindow(L"Progman", NULL);
	HDC dc = GetWindowDC(hProgman);
	CRect rect;
	GetWindowRect(hProgman, &rect);
	BitBlt(buffer->dc,
		0,
		0,
		rect.Width(),
		screenCmp.rect.Height(),
		dc,
		rect.left,
		screenCmp.rect.top - rect.top,
		SRCCOPY);
	ReleaseDC(hProgman, dc);
}

void SceenCompare::Run(CRect rect, CDIB *buffer, CWnd *notifer)
{
	if(screenCmp.init)
	{
		return;
	}
	screenCmp.rect = rect;
	screenCmp.notifer = notifer;
	screenCmp.buffer = buffer;
	screenCmp.buffProc = new CDIB();
	screenCmp.handler = CreateEvent(NULL, FALSE, FALSE, NULL);
	screenCmp.signal = CreateEvent(NULL, FALSE, FALSE, NULL);
	screenCmp.init = true;

	AfxBeginThread(ScreenCmpThread, &screenCmp);

	screenCmp.handlerType = HandlerRun;
	SetEvent(screenCmp.handler);
	WaitForSingleObject(screenCmp.signal, INFINITE);
}

bool SceenCompare::Runing()
{
	return screenCmp.init;
}

void SceenCompare::Pause(bool pause)
{
	if(!screenCmp.init)
	{
		return;
	}
	screenCmp.handlerType = pause ? HandlerPause : HandlerRun;
	SetEvent(screenCmp.handler);
	WaitForSingleObject(screenCmp.signal, INFINITE);
}

void SceenCompare::Rect(CRect rect)
{
	if(!screenCmp.init)
	{
		return;
	}
	screenCmp.handlerType = HandlerPause;
	SetEvent(screenCmp.handler);
	WaitForSingleObject(screenCmp.signal, INFINITE);

	screenCmp.rect = rect;

	screenCmp.handlerType = HandlerRun;
	SetEvent(screenCmp.handler);
	WaitForSingleObject(screenCmp.signal, INFINITE);
}

void SceenCompare::Stop()
{
	if(!screenCmp.init)
	{
		return;
	}
	screenCmp.notifer = NULL;
	screenCmp.handlerType = HandlerStop;
	SetEvent(screenCmp.handler);
	WaitForSingleObject(screenCmp.signal, INFINITE);
	delete screenCmp.buffProc;
	CloseHandle(screenCmp.signal);
	CloseHandle(screenCmp.handler);
	screenCmp.init = false;
}
