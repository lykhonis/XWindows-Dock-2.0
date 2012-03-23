#include "expose.h"

using namespace Gdiplus;

CExposeThumbnail::CExposeThumbnail(HWND hParent, HWND hWnd)
{
	CExposeThumbnail::hParent = hParent;
	CExposeThumbnail::hWnd = hWnd;
	
	wchar_t buff[MAX_PATH] = {0};
	::GetWindowText(hWnd, buff, MAX_PATH);

	opacity = 255;
	text = buff;
	isZoomed = false;
	visible = false;
	handle = NULL;
	thumbSize.SetSize(0, 0);
	isIconic = (::IsIconic(hWnd) == TRUE);
}

CExposeThumbnail::~CExposeThumbnail()
{
	if(handle)
	{
		DwmApi::DwmUnregisterThumbnail(handle);
	}
}

void CExposeThumbnail::Initialize()
{
	if(!handle && SUCCEEDED(DwmApi::DwmRegisterThumbnail(hParent, hWnd, &handle)))
	{
		DwmApi::DwmQueryThumbnailSourceSize(handle, &thumbSize);
	}
}

void CExposeThumbnail::SetVisible(bool visible)
{
	if(handle && (CExposeThumbnail::visible != visible))
	{
		CExposeThumbnail::visible = visible;

		DWM_THUMBNAIL_PROPERTIES prop;
		prop.dwFlags = DWM_TNP_VISIBLE | DWM_TNP_OPACITY;
		prop.fVisible = (visible ? TRUE : FALSE);
		prop.opacity = opacity;
		DwmApi::DwmUpdateThumbnailProperties(handle, &prop);
	}
}

void CExposeThumbnail::SetRect(CRect &rect)
{
	if(handle)
	{
		DWM_THUMBNAIL_PROPERTIES prop;
		prop.dwFlags = DWM_TNP_RECTDESTINATION | DWM_TNP_OPACITY;
		prop.rcDestination = rect;
		prop.opacity = opacity;
		DwmApi::DwmUpdateThumbnailProperties(handle, &prop);
	}
}

void CExposeThumbnail::Animate(float step)
{
	if(handle)
	{
		CRect *dr = &dstRect;
		if(isZoomed)
		{
			dr = &zoomRect;
		}
		if(opacity < 255)
		{
			opacity = (unsigned char)(255 * (0.3f + 0.7f * (1 - step)));
		}
		CRect r;
		r.left = (int)(srcRect.left * (1 - step) + dr->left * step);
		r.top = (int)(srcRect.top * (1 - step) + dr->top * step);
		r.right = r.left + (int)(srcRect.Width() * (1 - step) + dr->Width() * step);
		r.bottom = r.top + (int)(srcRect.Height() * (1 - step) + dr->Height() * step);
		SetRect(r);
	}
}

BEGIN_MESSAGE_MAP(CExpose, CFrameWnd)
	ON_WM_CLOSE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_TIMER()
END_MESSAGE_MAP()

CExpose::CExpose()
{
	CreateEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST, NULL, L"XWindows Dock - Expose", WS_POPUP, CRect(0, 0, 0, 0), NULL, NULL);

	DwmApi::Initialize();

	readyToWork = false;
	dragging = false;
	itemOver = NULL;
	dib = new CDIB();
	iconDib = new CDIB();
	monitors = new CMonitors();

	HDC dc = CreateCompatibleDC(0);
	Graphics g(dc);

	Font font(L"Arial", 11.0f);
	RectF r;
	StringFormat *stringFormat = new StringFormat();
	stringFormat->SetAlignment(StringAlignmentCenter);
	stringFormat->SetLineAlignment(StringAlignmentCenter);
	stringFormat->SetFormatFlags(StringFormatFlagsLineLimit);
	stringFormat->SetTrimming(StringTrimmingEllipsisCharacter);

	r.X = 0;
	r.Y = 0;
	r.Width = 0;
	r.Height = 0;
	g.MeasureString(L"W", 1, &font, r, stringFormat, &r);

	delete stringFormat;
	DeleteObject(dc);

	textHeight = (int)r.Height + 2;

	userDropTargetHelper = SUCCEEDED(CoCreateInstance(CLSID_DragDropHelper, NULL, 
		CLSCTX_INPROC_SERVER, IID_IDropTargetHelper, (void**)&dropTargetHelper));

	Register(this);
}

CExpose::~CExpose()
{
	RemoveAll();
	delete iconDib;
	delete dib;
	delete monitors;
	DwmApi::Finalize();
}

void CExpose::OnClose()
{
}

DROPEFFECT CExpose::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD, CPoint point)
{
	DROPEFFECT dwEffect = DROPEFFECT_MOVE;
	if(userDropTargetHelper)
	{
		IDataObject *dataObject = pDataObject->GetIDataObject(FALSE);
		dropTargetHelper->DragEnter(pWnd->GetSafeHwnd(), dataObject, &point, dwEffect);
	}
	dragging = true;
	itemOver = NULL;
	ptOver = point;
	return dwEffect;
}
	
DROPEFFECT CExpose::OnDragOver(CWnd*, COleDataObject*, DWORD, CPoint point)
{
	DROPEFFECT dwEffect = DROPEFFECT_MOVE;
	if(userDropTargetHelper)
	{
		dropTargetHelper->DragOver(&point, dwEffect);
	}
	if(readyToWork)
	{
		if(ptOver != point)
		{
			KillTimer(1);
			itemOver = ItemAt(point);
			SetTimer(1, 900, NULL);
		}
	}
	else
	{
		KillTimer(1);
	}
	ptOver = point;
	return dwEffect;
}

BOOL CExpose::OnDrop(CWnd*, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	dropEffect = DROPEFFECT_NONE;
	if(userDropTargetHelper)
	{
		IDataObject *dataObject = pDataObject->GetIDataObject(FALSE);
		dropTargetHelper->Drop(dataObject, &point, dropEffect);
	}
	dragging = false;
	Hide((GetKeyState(VK_SHIFT) & 0x8000) == 0x8000);
	RemoveAll();
	return TRUE;
}

void CExpose::OnDragLeave(CWnd*)
{
	if(userDropTargetHelper)
	{
		dropTargetHelper->DragLeave();
	}
	dragging = false;
	Hide((GetKeyState(VK_SHIFT) & 0x8000) == 0x8000);
	RemoveAll();
}

void CExpose::RemoveAll()
{
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		delete items.GetAt(p);
		items.GetNext(p);
	}
	items.RemoveAll();
}

void CExpose::UpdateLayer(CDIB *dib)
{
	if(!dib)
	{
		dib = CExpose::dib;
	}
	if(!dib->Ready())
	{
		return;
	}
	CRect rect;
	GetWindowRect(&rect);

	CWnd *wndDst = GetDesktopWindow();
	CDC *hdcDst = wndDst->GetDC();
	CDC *dc = new CDC();
	dc->Attach(dib->dc);
	
	BLENDFUNCTION blend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
	CPoint zp(0, 0);
	CPoint pt(rect.left, rect.top);
	CSize size(rect.Width(), rect.Height());

	UpdateLayeredWindow(hdcDst, &pt, &size, dc, &zp, NULL, &blend, ULW_ALPHA);

	dc->Detach();
	delete dc;
	wndDst->ReleaseDC(hdcDst);
}

void CExpose::DrawLayer(CDIB *dib)
{
	if(!dib)
	{
		dib = CExpose::dib;
	}

	CRect rect;
	GetWindowRect(&rect);

	dib->Resize(rect.Width(), rect.Height());
	if(!dib->Ready())
	{
		return;
	}
	
	Graphics g(dib->dc);
	g.SetCompositingMode(CompositingModeSourceCopy);
	
	SolidBrush brush(Color(255 * 70 / 100, 0x0, 0x0, 0x0));

	RectF rf(0, 0, (REAL)dib->Width(), (REAL)dib->Height());
	g.FillRectangle(&brush, rf);



	CRect r = iconRect;
	r.top += (int)(iconRect.Height() * 0.7f);
	r.bottom += (int)(iconRect.Height() * 0.2f);
	r.left -= (int)(iconRect.Width() * 0.15f);
	r.right += (int)(iconRect.Width() * 0.15f);

	GraphicsPath *p = new GraphicsPath();
	p->AddEllipse(r.left, r.top, r.Width(), r.Height());

	PathGradientBrush *pbr = new PathGradientBrush(p);
	pbr->SetCenterPoint(Point(r.left + r.Width() / 2, r.top + r.Height() / 2));
	pbr->SetCenterColor(Color(1, 0, 0, 0));

	Color colors[] = { Color(255 * 70 / 100, 0, 0, 0) };
	int colorsCount = 1;
	pbr->SetSurroundColors(colors, &colorsCount);

	g.FillPath(pbr, p);

	delete pbr;
	delete p;



	int h = min(iconRect.Height(), iconDib->Height());
	int w = min(iconRect.Width(), iconDib->Width());
	for(int y = 0; y < h; y++)
	{
		DIB_ARGB *pd = (DIB_ARGB*)((int)dib->scan0 + ((dib->Height() - 1 - y - iconRect.top) * dib->Width() + iconRect.left) * 4);
		DIB_ARGB *ps = (DIB_ARGB*)((int)iconDib->scan0 + (iconDib->Height() - 1 - y) * iconDib->Width() * 4);
		for(int x = 0; x < w; x++, ps++, pd++)
		{
			if (ps->a > 0)
			{
				pd->a = max(1, pd->a - ps->a);
			}
		}
	}
}

void CExpose::DrawStep(CDIB *dst, CDIB *src, float step)
{
	dst->Fill();
	BLENDFUNCTION fblend = {AC_SRC_OVER, 0, (unsigned char)(255 * step), AC_SRC_ALPHA};
	AlphaBlend(dst->dc, 0, 0, dst->Width(), dst->Height(), src->dc, 0, 0, src->Width(), src->Height(), fblend);
}

void CExpose::OnTimer(UINT_PTR nIDEvent)
{
	CFrameWnd::OnTimer(nIDEvent);

	if(nIDEvent == 1) // drag over is ready
	{
		Hide((GetKeyState(VK_SHIFT) & 0x8000) == 0x8000);
		if(itemOver)
		{
			if(itemOver->isIconic)
			{
				::ShowWindow(itemOver->hWnd, SW_RESTORE);
			}
			::SetForegroundWindow(itemOver->hWnd);
			::BringWindowToTop(itemOver->hWnd);
		}
		RemoveAll();
	}
}

void CExpose::OnLButtonDown(UINT nFlags, CPoint point)
{
	CFrameWnd::OnLButtonDown(nFlags, point);

	if(dragging)
	{
		return;
	}
	Hide((GetKeyState(VK_SHIFT) & 0x8000) == 0x8000);

	CExposeThumbnail *item = ItemAt(point);

	POSITION p = items.GetHeadPosition();
	while(p)
	{
		CExposeThumbnail *pItem = items.GetAt(p);
		if(pItem->isZoomed)
		{
			if(pItem->zoomRect.PtInRect(point))
			{
				item = pItem;
			}
			break;
		}
		items.GetNext(p);
	}

	if(item)
	{
		if(item->isIconic)
		{
			::ShowWindow(item->hWnd, SW_RESTORE);
		}
		::SetForegroundWindow(item->hWnd);
		::BringWindowToTop(item->hWnd);
	}
	RemoveAll();
}

void CExpose::OnRButtonDown(UINT nFlags, CPoint point)
{
	CFrameWnd::OnRButtonDown(nFlags, point);

	if(dragging)
	{
		return;
	}

	CExposeThumbnail *item = NULL;
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		if(items.GetAt(p)->isZoomed)
		{
			item = items.GetAt(p);
			item->isZoomed = false;
			CRect r;
			DWM_THUMBNAIL_PROPERTIES prop;
			prop.dwFlags = DWM_TNP_OPACITY;
			DWORD delay = ((GetKeyState(VK_SHIFT) & 0x8000) == 0x8000) ? 5000 : 150;
			DWORD startAt = GetTickCount();
			for(;;)
			{
				float k = 1 - (float)(GetTickCount() - startAt) / delay;
				if(k < 0)
				{
					k = 0;
				}
				k = sin(k * PI / 2);

				r.left = (int)(item->dstRect.left * (1 - k) + item->zoomRect.left * k);
				r.top = (int)(item->dstRect.top * (1 - k) + item->zoomRect.top * k);
				r.right = r.left + (int)(item->dstRect.Width() * (1 - k) + item->zoomRect.Width() * k);
				r.bottom = r.top + (int)(item->dstRect.Height() * (1 - k) + item->zoomRect.Height() * k);
				item->SetRect(r);

				POSITION p = items.GetHeadPosition();
				while(p)
				{
					CExposeThumbnail *thumb = items.GetAt(p);
					if(thumb != item)
					{
						thumb->opacity = (unsigned char)(255 * (1 - k));
						prop.opacity = thumb->opacity;
						DwmApi::DwmUpdateThumbnailProperties(thumb->handle, &prop);
					}
					items.GetNext(p);
				}

				if(k == 0)
				{
					break;
				}
			}
			if(item->zoomRect.PtInRect(point) || item->dstRect.PtInRect(point))
			{
				return;
			}
			break;
		}
		items.GetNext(p);
	}

	item = ItemAt(point);
	if(item)
	{
		CRect r;
		DWM_THUMBNAIL_PROPERTIES prop;
		prop.dwFlags = DWM_TNP_OPACITY;
		item->isZoomed = true;
		DWORD delay = ((GetKeyState(VK_SHIFT) & 0x8000) == 0x8000) ? 5000 : 200;
		DWORD startAt = GetTickCount();
		for(;;)
		{
			float k = (float)(GetTickCount() - startAt) / delay;
			if(k > 1)
			{
				k = 1;
			}
			k = sin(k * PI / 2);

			r.left = (int)(item->dstRect.left * (1 - k) + item->zoomRect.left * k);
			r.top = (int)(item->dstRect.top * (1 - k) + item->zoomRect.top * k);
			r.right = r.left + (int)(item->dstRect.Width() * (1 - k) + item->zoomRect.Width() * k);
			r.bottom = r.top + (int)(item->dstRect.Height() * (1 - k) + item->zoomRect.Height() * k);
			item->SetRect(r);

			POSITION p = items.GetHeadPosition();
			while(p)
			{
				CExposeThumbnail *thumb = items.GetAt(p);
				if(thumb != item)
				{
					thumb->opacity = (unsigned char)(255 * (1 - k));
					prop.opacity = thumb->opacity;
					DwmApi::DwmUpdateThumbnailProperties(thumb->handle, &prop);
				}
				items.GetNext(p);
			}

			if(k == 1)
			{
				break;
			}
		}
	}
}

CExposeThumbnail* CExpose::ItemAt(CPoint point)
{
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		CExposeThumbnail *item = items.GetAt(p);
		if(ItemRect(item).PtInRect(point))
		{
			return item;
		}
		items.GetNext(p);
	}
	return NULL;
}

CRect CExpose::ItemRect(CExposeThumbnail *item)
{
	CRect rect(0, 0, 0, 0);

	int c, r, s, w, h, wh;

	wh = min(clientRect.Width(), clientRect.Height());
	if(items.GetCount() == 1)
	{
		c = 1;
	}
	else
	{
		c = (int)sqrt((float)items.GetCount()) + 1;
	}
	r = items.GetCount() / c;
	if(items.GetCount() % c > 0)
	{
		r++;
	}
	s = (int)(wh * 0.9f / c);

	float k = min(s * 0.9f / item->thumbSize.cx, (s - (textHeight + 4)) * 0.9f / item->thumbSize.cy);
	if(k > 1)
	{
		k = 1;
	}
	
	w = (int)(item->thumbSize.cx * k);
	h = (int)(item->thumbSize.cy * k);

	rect.left = (int)(clientRect.left + item->index % c * s + (clientRect.Width() - s * c) / 2 + (s - w) / 2);
	rect.top = (int)(clientRect.top + item->index / c * s + (clientRect.Height() - s * r) / 2 + (s - (textHeight + 4) - h) / 2);
	rect.right = rect.left + w;
	rect.bottom = rect.top + h;

	return rect;
}

void CExpose::ItemDrawText(CExposeThumbnail *item)
{
	CRect rect(0, 0, 0, 0);

	int c, r, s, wh;

	wh = min(clientRect.Width(), clientRect.Height());
	if(items.GetCount() == 1)
	{
		c = 1;
	}
	else
	{
		c = (int)sqrt((float)items.GetCount()) + 1;
	}
	r = items.GetCount() / c;
	if(items.GetCount() % c > 0)
	{
		r++;
	}
	s = (int)(wh * 0.9f / c);

	rect.left = (int)(clientRect.left + item->index % c * s + (clientRect.Width() - s * c) / 2) + 2;
	rect.top = (int)(clientRect.top + item->index / c * s + (clientRect.Height() - s * r) / 2 + s - (textHeight + 4));
	rect.right = rect.left + s - 4;
	rect.bottom = rect.top + textHeight + 4;



	Graphics g(dib->dc);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	RectF rf;
	rf.X = (REAL)rect.left;
	rf.Y = (REAL)rect.top + 1;
	rf.Width = (REAL)rect.Width();
	rf.Height = (REAL)rect.Height() - 2;

	const float radius = rf.Height;

	Font font(L"Arial", 11.0f);
	StringFormat *stringFormat = new StringFormat();
	stringFormat->SetAlignment(StringAlignmentCenter);
	stringFormat->SetLineAlignment(StringAlignmentCenter);
	stringFormat->SetFormatFlags(StringFormatFlagsLineLimit);
	stringFormat->SetTrimming(StringTrimmingEllipsisCharacter);

	RectF rff;
	rff.X = 0;
	rff.Y = 0;
	rff.Width = 0;
	rff.Height = 0;
	g.MeasureString(item->text.GetBuffer(), item->text.GetLength(), &font, rff, stringFormat, &rff);

	float w = min(rf.Width, rff.Width + radius);
	rf.X += (rf.Width - w) / 2;
	rf.Width = w;

	GraphicsPath *path = new GraphicsPath();
	path->AddArc(rf.X, rf.Y + rf.Height - radius, radius - 1, radius - 1, 90, 90);
	path->AddArc(rf.X, rf.Y, radius - 1, radius - 1, 180, 90);
	path->AddArc(rf.X + rf.Width - radius, rf.Y, radius - 1, radius - 1, 270, 90);
	path->AddArc(rf.X + rf.Width - radius, rf.Y + rf.Height - radius, radius - 1, radius - 1, 0, 90);
	path->CloseFigure();

	SolidBrush brush(Color(255 * 40 / 100, 0x0, 0x0, 0x0));
	g.FillPath(&brush, path);

	rf.X += radius / 2;
	rf.Width -= radius;

	brush.SetColor(0xffffffff);
	g.DrawString(item->text.GetBuffer(), item->text.GetLength(), &font, rf, stringFormat, &brush);

	delete path;
	delete stringFormat;
}

BOOL CALLBACK CExpose::EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	DWORD pid = NULL;
	GetWindowThreadProcessId(hWnd, &pid);
	CExpose *expose = (CExpose*)lParam;
	if((expose->pid == pid) && ::IsWindowVisible(hWnd))
	{
		DWORD style = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
		if((style & WS_EX_TOOLWINDOW) || ((::GetParent(hWnd) != NULL) && ((style & WS_EX_APPWINDOW) == 0)))
		{
			return TRUE;
		}
		wchar_t buff[MAX_PATH];
		GetClassName(hWnd, buff, MAX_PATH);
		if((wcscmp(buff, L"Progman") == 0) || (wcscmp(buff, L"WorkerW") == 0) || 
			(wcscmp(buff, L"Shell_TrayWnd") == 0) || (wcscmp(buff, L"XWindowsDockClass") == 0))
		{
			return TRUE;
		}
		CExposeThumbnail *thumb = new CExposeThumbnail(expose->CFrameWnd::m_hWnd, hWnd);
		thumb->index = expose->items.GetCount();
		expose->items.AddHead(thumb);
	}
	return TRUE;
}

void CExpose::SetIconRect(CRect rect, CDIB *icon)
{
	iconRect = rect;
	iconDib->Assign(icon);

	DrawLayer();
	UpdateLayer();
}

bool CExpose::Expose(HWND hParent, CRect rect, CDIB *icon, CString exePath, DockPosition position, bool slow)
{
	if(!DwmApi::Ready())
	{
		return false;
	}

	monitor = monitors->GetMonitor(hParent);
	if(!monitor)
	{
		return false;
	}

	RemoveAll();

	// find out all exe and all their windows
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if(hSnapshot)
	{
		PROCESSENTRY32 processEntry = {0};
		processEntry.dwSize = sizeof(PROCESSENTRY32);
		if(Process32First(hSnapshot, &processEntry))
		do
		{
			if(GetPathByPID(processEntry.th32ProcessID).CompareNoCase(exePath) == 0)
			{
				pid = processEntry.th32ProcessID;
				EnumWindows(EnumWindowsProc, (LPARAM)this);
			}
		}
		while(Process32Next(hSnapshot, &processEntry));
		CloseHandle(hSnapshot);
	}

	if(items.IsEmpty())
	{
		return false;
	}

	iconRect = rect;
	iconDib->Assign(icon);

	CRect monitorRect = monitor->Rect();
	clientRect = monitorRect;
	switch(position)
	{
	case DockPositionBottom:
		{
			clientRect.bottom = iconRect.top - 20;
		}
		break;

	case DockPositionTop:
		{
			clientRect.top = iconRect.bottom + 20;
		}
		break;

	case DockPositionLeft:
		{
			clientRect.left = iconRect.right + 20;
		}
		break;

	case DockPositionRight:
		{
			clientRect.right = iconRect.left - 20;
		}
		break;
	}

	SetWindowPos(&wndTopMost, monitorRect.left, monitorRect.top, monitorRect.Width(), monitorRect.Height(), 0);
	DrawLayer();

	CDIB tmp;
	tmp.Resize(dib->Width(), dib->Height());

	UpdateLayer(&tmp);
	ShowWindow(SW_SHOW);

	POSITION p = items.GetHeadPosition();
	while(p)
	{
		CExposeThumbnail *thumb = items.GetAt(p);

		::GetWindowRect(thumb->hWnd, &thumb->srcRect);
		if(thumb->isIconic)
		{
			thumb->srcRect.left = monitorRect.left + monitorRect.Width() / 2;
			thumb->srcRect.top = monitorRect.top + monitorRect.Height() / 2;
			thumb->srcRect.right = thumb->srcRect.left;
			thumb->srcRect.bottom = thumb->srcRect.top;
		}

		thumb->Initialize();
		thumb->dstRect = ItemRect(thumb);

		float k = min((float)min(clientRect.Width() * 0.8f, thumb->thumbSize.cx) / thumb->dstRect.Width(), 
			(float)min(clientRect.Height() * 0.8f, thumb->thumbSize.cy) / thumb->dstRect.Height());

		thumb->zoomRect.left = clientRect.left + (int)(clientRect.Width() - thumb->dstRect.Width() * k) / 2;
		thumb->zoomRect.top = clientRect.top + (int)(clientRect.Height() - thumb->dstRect.Height() * k) / 2;
		thumb->zoomRect.right = (int)(thumb->zoomRect.left + thumb->dstRect.Width() * k);
		thumb->zoomRect.bottom = (int)(thumb->zoomRect.top + thumb->dstRect.Height() * k);

		thumb->Animate(0);
		thumb->SetVisible();

		ItemDrawText(thumb);
		items.GetNext(p);
	}

	p = items.GetHeadPosition();
	while(p)
	{
		CExposeThumbnail *thumb = items.GetAt(p);
		if(!thumb->isIconic)
		{
			::SetWindowPos(thumb->hWnd, NULL, monitorRect.left - thumb->srcRect.Width(), 
				monitorRect.top - thumb->srcRect.Height(), thumb->srcRect.Width(), thumb->srcRect.Height(),
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
		items.GetNext(p);
	}

	DWORD delay = slow ? 5000 : 250;
	DWORD startAt = GetTickCount();
	for(;;)
	{
		float k = (float)(GetTickCount() - startAt) / delay;
		if(k > 1)
		{
			k = 1;
		}
		k = sin(k * PI / 2);

		DrawStep(&tmp, dib, k);
		UpdateLayer(&tmp);

		p = items.GetHeadPosition();
		while(p)
		{
			items.GetAt(p)->Animate(k);
			items.GetNext(p);
		}

		if(k == 1)
		{
			break;
		}
	}

	UpdateLayer();
	BringWindowToTop();

	readyToWork = true;
	KillTimer(1);
	return true;
}

void CExpose::Hide(bool slow)
{
	if(!IsWindowVisible())
	{
		return;
	}

	KillTimer(1);
	readyToWork = false;

	CDIB tmp;
	tmp.Resize(dib->Width(), dib->Height());

	DWORD delay = slow ? 5000 : 250;
	DWORD startAt = GetTickCount();
	for(;;)
	{
		float k = 1 - (float)(GetTickCount() - startAt) / delay;
		if(k < 0)
		{
			k = 0;
		}
		k = sin(k * PI / 2);

		DrawStep(&tmp, dib, k);
		UpdateLayer(&tmp);

		POSITION p = items.GetHeadPosition();
		while(p)
		{
			items.GetAt(p)->Animate(k);
			items.GetNext(p);
		}

		if(k == 0)
		{
			break;
		}
	}

	POSITION p = items.GetHeadPosition();
	while(p)
	{
		CExposeThumbnail *thumb = items.GetAt(p);
		if(!thumb->isIconic)
		{
			::SetWindowPos(thumb->hWnd, NULL, thumb->srcRect.left, thumb->srcRect.top, 
				thumb->srcRect.Width(), thumb->srcRect.Height(),
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
		items.GetNext(p);
	}

	p = items.GetHeadPosition();
	while(p)
	{
		items.GetAt(p)->SetVisible(false);
		items.GetNext(p);
	}

	ShowWindow(SW_HIDE);
	dib->FreeImage();
	iconDib->FreeImage();

	// notify
	HWND hWnd = ::FindWindow(L"XWindowsDockClass", NULL);
	if(hWnd)
	{
		::SendMessage(hWnd, WM_EXPOSENOTIFY, 0, 0);
	}
}
