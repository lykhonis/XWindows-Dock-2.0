#include "ContainerLib.h"

using namespace Gdiplus;

CContainerItem::CContainerItem()
{
	dib = new CDIB();
	tmp = new CDIB();
}

CContainerItem::~CContainerItem()
{
	delete tmp;
	delete dib;
}

void CContainerItem::DrawLayer(CDIB*)
{
}

void CContainerItem::DrawStep(CDIB*, CDIB*, float, bool)
{
}

void CContainerItem::UpdateLayer(CDIB *dib)
{
	if(!dib)
	{
		dib = CContainerItem::dib;
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

#define ArrowWidth 24
#define ArrowHeight 12
#define ShadowSize 12
#define BorderSize 12
#define CaptionSize 40
#define ButtonWidth 70
#define ButtonHeight 24

BEGIN_MESSAGE_MAP(CContainerBckg, CContainerItem)
END_MESSAGE_MAP()

void CContainerBckg::DrawStep(CDIB *dst, CDIB *src, float step, bool)
{
	dst->Fill();
	CRect r;
	switch(viewMode)
	{
	case ContainerViewModeFan:
		{
			if(step < 0.6f)
			{
				return;
			}

			r.left = (int)(dst->Width() * 0.4f * (1 - step));
			r.top = (int)(dst->Height() * 0.3f * (1 - step));
			r.right = r.left + (int)(dst->Width() * (0.6f + 0.4f * step));
			r.bottom = r.top + (int)(dst->Height() * (0.7f + 0.3f * step));

			BLENDFUNCTION fblend = {AC_SRC_OVER, 0, (unsigned char)(255 * (step - 0.6f) / 0.4f), AC_SRC_ALPHA};
			AlphaBlend(dst->dc, r.left, r.top, r.Width(), r.Height(), src->dc, 0, 0, src->Width(), src->Height(), fblend);
		}
		break;

	case ContainerViewModeGrid:
		{
			if(step < 0.2f)
			{
				return;
			}
			switch(position)
			{
			case ContainerPositionBottom:
				{
					r.left = (int)((dst->Width() / 2 + ArrowOffset) * (1 - step));
					r.top = (int)(dst->Height() * (1 - step));
					r.right = r.left + (int)(dst->Width() * step);
					r.bottom = r.top + (int)(dst->Height() * step);
				}
				break;

			case ContainerPositionLeft:
				{
					r.left = 0;
					r.top = (int)(dst->Height() * (1 - step) - (dst->Height() / 2 - ArrowOffset) * (1 - step));
					r.right = r.left + (int)(dst->Width() * step);
					r.bottom = r.top + (int)(dst->Height() * step);
				}
				break;

			case ContainerPositionTop:
				{
					r.left = (int)((dst->Width() / 2 + ArrowOffset) * (1 - step));
					r.top = 0;
					r.right = r.left + (int)(dst->Width() * step);
					r.bottom = r.top + (int)(dst->Height() * step);
				}
				break;

			case ContainerPositionRight:
				{
					r.left = (int)(dst->Width() * (1 - step));
					r.top = (int)(dst->Height() * (1 - step) - (dst->Height() / 2 - ArrowOffset) * (1 - step));
					r.right = r.left + (int)(dst->Width() * step);
					r.bottom = r.top + (int)(dst->Height() * step);
				}
				break;
			}
			BLENDFUNCTION fblend = {AC_SRC_OVER, 0, (unsigned char)(255 * (step - 0.2f) / 0.8f), AC_SRC_ALPHA};
			AlphaBlend(dst->dc, r.left, r.top, r.Width(), r.Height(), src->dc, 0, 0, src->Width(), src->Height(), fblend);
		}
		break;
	}
}

void CContainerBckg::DrawLayer(CDIB *dib)
{
#define FillDibPath() \
	switch(position) \
	{ \
	case ContainerPositionBottom: \
		{ \
			path->AddArc(rf.X, rf.Y + rf.Height - radius - ArrowHeight, radius - 1, radius - 1, 90, 90); \
			path->AddArc(rf.X, rf.Y, radius - 1, radius - 1, 180, 90); \
			path->AddArc(rf.X + rf.Width - radius, rf.Y, radius - 1, radius - 1, 270, 90); \
			path->AddArc(rf.X + rf.Width - radius, rf.Y + rf.Height - radius - ArrowHeight, radius - 1, radius - 1, 0, 90); \
 \
			path->AddLine(rf.X + (rf.Width - ArrowWidth) / 2 + ArrowWidth + ArrowOffset, rf.Y + rf.Height - 1 - ArrowHeight, \
				rf.X + (rf.Width - ArrowWidth) / 2 + ArrowWidth / 2 + ArrowOffset, rf.Y + rf.Height - 1); \
 \
			path->AddLine(rf.X + (rf.Width - ArrowWidth) / 2 + ArrowWidth / 2 + ArrowOffset, rf.Y + rf.Height - 1, \
				rf.X + (rf.Width - ArrowWidth) / 2 + ArrowOffset, rf.Y + rf.Height - 1 - ArrowHeight); \
		} \
		break; \
 \
	case ContainerPositionLeft: \
		{ \
			path->AddArc(rf.X + ArrowHeight, rf.Y, radius - 1, radius - 1, 180, 90); \
			path->AddArc(rf.X + rf.Width - radius, rf.Y, radius - 1, radius - 1, 270, 90); \
			path->AddArc(rf.X + rf.Width - radius, rf.Y + rf.Height - radius, radius - 1, radius - 1, 0, 90); \
			path->AddArc(rf.X + ArrowHeight, rf.Y + rf.Height - radius, radius - 1, radius - 1, 90, 90); \
 \
			path->AddLine(rf.X + ArrowHeight, rf.Y + (rf.Height - ArrowWidth) / 2 + ArrowWidth + ArrowOffset, \
				rf.X, rf.Y + (rf.Height - ArrowWidth) / 2 + ArrowWidth / 2 + ArrowOffset); \
 \
			path->AddLine(rf.X, rf.Y + (rf.Height - ArrowWidth) / 2 + ArrowWidth / 2 + ArrowOffset, \
				rf.X + ArrowHeight, rf.Y + (rf.Height - ArrowWidth) / 2 + ArrowOffset); \
		} \
		break; \
 \
	case ContainerPositionRight: \
		{ \
			path->AddArc(rf.X + rf.Width - ArrowHeight - radius, rf.Y + rf.Height - radius, radius - 1, radius - 1, 0, 90); \
			path->AddArc(rf.X, rf.Y + rf.Height - radius, radius - 1, radius - 1, 90, 90); \
			path->AddArc(rf.X, rf.Y, radius - 1, radius - 1, 180, 90); \
			path->AddArc(rf.X + rf.Width - ArrowHeight - radius, rf.Y, radius - 1, radius - 1, 270, 90); \
 \
			path->AddLine(rf.X + rf.Width - 1 - ArrowHeight, rf.Y + (rf.Height - ArrowWidth) / 2 + ArrowOffset, \
				rf.X + rf.Width - 1, rf.Y + (rf.Height - ArrowWidth) / 2 + ArrowWidth / 2 + ArrowOffset); \
 \
			path->AddLine(rf.X + rf.Width - 1, rf.Y + (rf.Height - ArrowWidth) / 2 + ArrowWidth / 2 + ArrowOffset, \
				rf.X + rf.Width - 1 - ArrowHeight, rf.Y + (rf.Height - ArrowWidth) / 2 + ArrowWidth + ArrowOffset); \
		} \
		break; \
 \
	case ContainerPositionTop: \
		{ \
			path->AddArc(rf.X + rf.Width - radius, rf.Y + ArrowHeight, radius - 1, radius - 1, 270, 90); \
			path->AddArc(rf.X + rf.Width - radius, rf.Y + rf.Height - radius, radius - 1, radius - 1, 0, 90); \
			path->AddArc(rf.X, rf.Y + rf.Height - radius, radius - 1, radius - 1, 90, 90); \
			path->AddArc(rf.X, rf.Y + ArrowHeight, radius - 1, radius - 1, 180, 90); \
 \
			path->AddLine(rf.X + (rf.Width - ArrowWidth) / 2 + ArrowOffset, rf.Y + ArrowHeight, \
				rf.X + (rf.Width - ArrowWidth) / 2 + ArrowWidth / 2 + ArrowOffset, rf.Y); \
 \
			path->AddLine(rf.X + (rf.Width - ArrowWidth) / 2 + ArrowWidth / 2 + ArrowOffset, rf.Y, \
				rf.X + (rf.Width - ArrowWidth) / 2 + ArrowWidth + ArrowOffset, rf.Y + ArrowHeight); \
		} \
		break; \
	}

	if(!dib)
	{
		dib = CContainerItem::dib;
	}

	CRect rect;
	GetWindowRect(&rect);
	
	dib->Resize(rect.Width(), rect.Height());
	if(!dib->Ready())
	{
		return;
	}
	
	switch(viewMode)
	{
	case ContainerViewModeFan:
		{
			/*DIB_ARGB *p = dib->scan0;
			for(int i = 0; i < dib->Width() * dib->Height(); i++, p++)
			{
				p->c = 0xffff0000;
			}*/
		}
		break;

	case ContainerViewModeGrid:
		{
			RectF rf(ShadowSize, ShadowSize, (REAL)dib->Width() - ShadowSize * 2, (REAL)dib->Height() - ShadowSize * 2);

			Graphics g(dib->dc);

			GraphicsPath *path = new GraphicsPath();

			const float radius = 20;

			FillDibPath();
			path->CloseFigure();

			SolidBrush brush(Color(255 * 80 / 100, 0, 0, 0));
			g.FillPath(&brush, path);

			dib->Blur(dib->Rect(), CRect(ShadowSize * 2, 0, dib->Width() - ShadowSize * 2, dib->Height() - ShadowSize * 2), ShadowSize);

			g.SetCompositingMode(CompositingModeSourceCopy);
			g.SetSmoothingMode(SmoothingModeNone);

			rf.X = ShadowSize;
			rf.Y = 0;
			rf.Width = (REAL)dib->Width() - ShadowSize * 2;
			rf.Height = (REAL)dib->Height() - ShadowSize;

			path->Reset();
			FillDibPath();
			path->CloseFigure();

			g.FillPath(&brush, path);

			g.SetCompositingMode(CompositingModeSourceOver);
			g.SetSmoothingMode(SmoothingModeAntiAlias);

			Pen pen(Color(0xd0, 0xd0, 0xd0), 2);
			g.DrawPath(&pen, path);

			delete path;
		}
		break;
	}
}

BEGIN_MESSAGE_MAP(CContainerPopupText, CContainerItem)
	ON_WM_TIMER()
END_MESSAGE_MAP()

#define ShadowSizePopupText 6

void CContainerPopupText::DrawLayer(CDIB *dib)
{
	if(!dib)
	{
		dib = CContainerItem::dib;
	}

	CRect rect;
	GetWindowRect(&rect);
	
	dib->Resize(rect.Width(), rect.Height());
	if(!dib->Ready())
	{
		return;
	}
	RectF rf(0, 0, (REAL)dib->Width(), (REAL)dib->Height() - ShadowSizePopupText);

	Graphics g(dib->dc);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	GraphicsPath *path = new GraphicsPath();

	RectF rs = rf;
	rs.X += radius / 2;
	rs.Width -= radius;

	path->AddArc(rs.X, rs.Y + rs.Height - radius, radius - 1, radius - 1, 90, 90);
	path->AddArc(rs.X, rs.Y, radius - 1, radius - 1, 180, 90);
	path->AddArc(rs.X + rs.Width - radius, rs.Y, radius - 1, radius - 1, 270, 90);
	path->AddArc(rs.X + rs.Width - radius, rs.Y + rs.Height - radius, radius - 1, radius - 1, 0, 90);
	path->CloseFigure();

	SolidBrush brush(0xff000000);

	g.FillPath(&brush, path);

	dib->Blur(dib->Rect(), CRect(0, 0, 0, 0), ShadowSizePopupText);

	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.SetCompositingMode(CompositingModeSourceCopy);

	path->Reset();
	path->AddArc(rf.X, rf.Y + rf.Height - radius, radius - 1, radius - 1, 90, 90);
	path->AddArc(rf.X, rf.Y, radius - 1, radius - 1, 180, 90);
	path->AddArc(rf.X + rf.Width - radius, rf.Y, radius - 1, radius - 1, 270, 90);
	path->AddArc(rf.X + rf.Width - radius, rf.Y + rf.Height - radius, radius - 1, radius - 1, 0, 90);
	path->CloseFigure();

	brush.SetColor(0xffb0b0b0);
	g.FillPath(&brush, path);

	g.SetCompositingMode(CompositingModeSourceOver);

	Font font(L"Arial", 9.0f);
	StringFormat *stringFormat = new StringFormat();
	stringFormat->SetAlignment(StringAlignmentCenter);
	stringFormat->SetLineAlignment(StringAlignmentCenter);
	stringFormat->SetFormatFlags(StringFormatFlagsLineLimit);

	brush.SetColor(0xff000000);
	g.DrawString(text.GetBuffer(), text.GetLength(), &font, rf, stringFormat, &brush);

	delete stringFormat;
	delete path;
}

void CContainerPopupText::DrawStep(CDIB *dst, CDIB *src, float step, bool blend)
{
	DIB_ARGB *ps = src->scan0;
	DIB_ARGB *pd = dst->scan0;
	int size = src->Width() * src->Height();
	for(int i = 0; i < size; i++, ps++, pd++)
	{
		pd->a = (unsigned char)(ps->a * step);
		pd->r = (unsigned char)(ps->r * pd->a / 255);
		pd->g = (unsigned char)(ps->g * pd->a / 255);
		pd->b = (unsigned char)(ps->b * pd->a / 255);
	}
}

void CContainerPopupText::OnTimer(UINT_PTR nIDEvent)
{
	CContainerItem::OnTimer(nIDEvent);
	switch(nIDEvent)
	{
	case 1:
		{
			float k = 1 - (float)(GetTickCount() - startAt) / 120;
			if(k < 0)
			{
				k = 0;
			}
			DrawStep(tmp, dib, k, true);
			UpdateLayer(tmp);
			if(k == 0)
			{
				KillTimer(1);
				DestroyWindow();
			}
		}
		break;
	}
}

void CContainerPopupText::Delete()
{
	if(GetWindowLong(m_hWnd, GWL_HWNDPARENT))
	{
		SetWindowLong(m_hWnd, GWL_HWNDPARENT, NULL);
		tmp->Resize(dib->Width(), dib->Height());
		startAt = GetTickCount();
		SetTimer(1, 10, NULL);
	}
}

void CContainerPopupText::Popup(CRect rect, CPoint pt)
{
	CRect r;
	GetWindowRect(&r);
	r.MoveToXY(pt);

	if(r.left < rect.left) r.MoveToX(rect.left);
	if(r.right > rect.right) r.MoveToX(rect.right - r.Width());
	if(r.top < rect.top) r.MoveToY(rect.top);
	if(r.bottom > rect.bottom) r.MoveToY(rect.bottom - r.Height());

	SetWindowPos(&wndTop, r.left, r.top, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);

	DrawLayer();
	UpdateLayer();

	ShowWindow(SW_SHOWNOACTIVATE);
}

void CContainerPopupText::Prepare()
{
	HDC dc = CreateCompatibleDC(0);
	Graphics g(dc);

	Font font(L"Arial", 9.0f);
	RectF r;
	StringFormat *stringFormat = new StringFormat();
	stringFormat->SetAlignment(StringAlignmentCenter);
	stringFormat->SetLineAlignment(StringAlignmentCenter);
	stringFormat->SetFormatFlags(StringFormatFlagsLineLimit);

	r.X = 0;
	r.Y = 0;
	r.Width = 0;
	r.Height = 0;
	g.MeasureString(text.GetBuffer(), text.GetLength(), &font, r, stringFormat, &r);

	delete stringFormat;
	DeleteObject(dc);

	radius = r.Height + 6;

	SetWindowPos(&wndTop, 0, 0,
		(int)(r.Width + radius),
		(int)radius + ShadowSizePopupText, 
		SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
}

BEGIN_MESSAGE_MAP(CContainerIcon, CContainerItem)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()

CContainerIcon::CContainerIcon()
{
	pressed = false;
	dragging = false;
	thumbReady = false;
	thumbWorking = false;
	animating = false;
	icon = new CDIB();
	isVideo = false;
	rendered = false;
	videoPosition = 0.5f;
	mouseIsOver = false;
	shortText = false;
	popupText = NULL;
	shutingDown = false;
	thumbThread = NULL;
}

CContainerIcon::~CContainerIcon()
{
	shutingDown = true;
	if(thumbThread)
	{
		WaitForSingleObject(thumbThread->m_hThread, INFINITE);
		delete thumbThread;
	}
	delete icon;
}

void CContainerIcon::Exec()
{
	if(path.IsEmpty())
	{
		return;
	}
	DWORD mode = SetErrorMode(SEM_FAILCRITICALERRORS);
	DWORD attr = GetFileAttributes(path.GetBuffer());
	SetErrorMode(mode);

	if(attr == INVALID_FILE_ATTRIBUTES)
	{
		return;
	}

	if(attr & FILE_ATTRIBUTE_DIRECTORY)
	{
		ShellExecute(NULL, L"open", path.GetBuffer(), NULL, NULL, SW_SHOWNORMAL);
		return;
	}

	LPITEMIDLIST pidl = ShellIO::StringToPIDL(path);
	if(!pidl)
	{
		return;
	}

	SHELLEXECUTEINFO info = {0};
	info.cbSize = sizeof(SHELLEXECUTEINFO);
	info.lpVerb = L"open";
	info.fMask = SEE_MASK_IDLIST;
	info.lpParameters = NULL;
	info.lpDirectory = NULL;
	info.nShow = SW_SHOWNORMAL;
	info.lpIDList = pidl;

	ShellExecuteEx(&info);

	ILFree(pidl);
}

void CContainerIcon::OnMouseLeave()
{
	CContainerItem::OnMouseLeave();

	if(shortText && popupText)
	{
		popupText->Delete();
		popupText = NULL;
	}
	mouseIsOver = false;
}

void CContainerIcon::OnMouseMove(UINT nFlags, CPoint point)
{
	CContainerItem::OnMouseMove(nFlags, point);

	TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, m_hWnd, HOVER_DEFAULT};
	TrackMouseEvent(&tme);

	if(!dragging && pressed && ((abs(pressedPt.x - point.x) > 6) || (abs(pressedPt.y - point.y) > 6)))
	{
		ReleaseCapture();
		pressed = false;
		dragging = true;

		if(container && container->callBack)
		{
			container->callBack->PostMessage(WM_CONTAINEREVENT, (WPARAM)ContainerEventItemBeginDrag, (LPARAM)container->param);
		}

		BeginDrag(point);
		dragging = false;
	}
	else
	if(!mouseIsOver)
	{
		mouseIsOver = true;

		if(shortText && container)
		{
			popupText = new CContainerPopupText();
			popupText->CreateEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST, NULL, NULL, WS_POPUP, 0, 0, 0, 0, m_hWnd, NULL);
			popupText->text = text;
			popupText->Prepare();

			CRect r, rp;
			GetWindowRect(&r);
			r.OffsetRect(0, r.Height() + container->separatorTextSize);

			popupText->GetWindowRect(&rp);
			r.left += (r.Width() - rp.Width()) / 2;
			r.top -= 3;

			container->bckg->GetWindowRect(&rp);
			rp.left += (ShadowSize + 2);
			rp.bottom -= (ShadowSize + 2);
			rp.right -= (ShadowSize + 2);

			popupText->Popup(rp, CPoint(r.left, r.top));
		}
	}
}

void CContainerIcon::OnLButtonDown(UINT nFlags, CPoint point)
{
	CContainerItem::OnLButtonDown(nFlags, point);

	if(shortText && popupText)
	{
		popupText->Delete();
		popupText = NULL;
	}

	SetFocus();
	SetCapture();

	pressedPt = point;
	pressed = true;
	DWORD startAt = GetTickCount();
	for(;;)
	{
		pressedAlpha = 1 - 0.5f * (GetTickCount() - startAt) / 120;
		if(pressedAlpha < 0.5f)
		{
			pressedAlpha = 0.5f;
		}
		DrawLayer();
		UpdateLayer();
		if(pressedAlpha == 0.5f)
		{
			break;
		}
		Sleep(10);
	}
}

void CContainerIcon::OnLButtonUp(UINT nFlags, CPoint point)
{
	CContainerItem::OnLButtonDown(nFlags, point);
	if(pressed)
	{
		ReleaseCapture();

		pressed = false;
		DrawLayer();
		UpdateLayer();

		ClientToScreen(&point);
		if(WindowFromPoint(point) == this)
		{
			if(container && container->callBack)
			{
				container->callBack->SendMessage(WM_CONTAINEREVENT, (WPARAM)ContainerEventItemExec, (LPARAM)container->param);
			}
			Exec();
		}
	}
}

void CContainerIcon::DrawLayer(CDIB *dib)
{
	if(!container)
	{
		return;
	}
	if(!dib)
	{
		dib = CContainerItem::dib;
	}

	switch(container->viewMode)
	{
	case ContainerViewModeGrid:
		{
			dib->Assign(icon);
		}
		break;

	case ContainerViewModeFan:
		{
			dib->Fill();

			Graphics g(dib->bmp);
			g.SetSmoothingMode(SmoothingModeAntiAlias);

			RectF rf;
			rf.Width = (REAL)container->iconSize;
			rf.Height = (REAL)container->iconSize;
			rf.X = (REAL)(dib->Width() - container->iconSize) / 2;
			rf.Y = (REAL)(dib->Height() - container->iconSize) / 2;

			PointF center((REAL)dib->Width() / 2, (REAL)dib->Height() / 2);

			Matrix m(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
			m.RotateAt(index * 1.1f, center);
			m.Invert();
			g.SetTransform(&m);

			g.DrawImage(icon->bmp, rf);
		}
		break;
	}

	if(pressed)
	{
		DIB_ARGB *p = dib->scan0;
		int size = dib->Width() * dib->Height();
		for(int i = 0; i < size; i++, p++)
		{
			p->r = (unsigned char)(p->r * pressedAlpha);
			p->g = (unsigned char)(p->g * pressedAlpha);
			p->b = (unsigned char)(p->b * pressedAlpha);
		}
	}
}

void CContainerIcon::DrawStep(CDIB *dst, CDIB *src, float step, bool blend)
{
	dst->Fill();
	if(container)
	{
		switch(container->viewMode)
		{
		case ContainerViewModeFan:
			{
				Graphics g(dst->bmp);
				g.SetCompositingMode(CompositingModeSourceCopy);
				g.SetSmoothingMode(SmoothingModeAntiAlias);
				
				if(step == 1)
				{
					g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
				}

				RectF rf;
				rf.Width = srcRect.Width() * (1 - step) + container->iconSize * step;
				rf.Height = srcRect.Height() * (1 - step) + container->iconSize * step;
				rf.X = (REAL)(dstRect.Width() - container->iconSize) / 2;
				rf.Y = (REAL)(dstRect.Height() - container->iconSize) / 2 +
					(container->iconSize - srcRect.Height()) * (1 - step);

				PointF center((REAL)dstRect.Width() / 2, (REAL)dstRect.Height() / 2);

				Matrix m(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
				m.RotateAt(index * 1.1f * step, center);
				m.Invert();
				g.SetTransform(&m);

				g.DrawImage(src->bmp, rf);
			}
			break;

		case ContainerViewModeGrid:
			{
				BLENDFUNCTION fblend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
				if(blend && container)
				{
					fblend.SourceConstantAlpha = (unsigned char)(255 * 
						(1 - (1 - step) * index / container->icons.GetCount()));
				}
				AlphaBlend(dst->dc, 0, 0, 
					(int)(srcRect.Width() * (1 - step) + container->iconSize * step), 
					(int)(srcRect.Height() * (1 - step) + container->iconSize * step), 
					src->dc, 0, 0, src->Width(), src->Height(), fblend);
			}
			break;
		}
	}
}

int loadThumbThreadCounter = 0;
UINT AFX_CDECL LoadThumbThread(LPVOID param)
{
	CContainerIcon *icon = (CContainerIcon*)param;

	if(icon->thumbReady || icon->thumbWorking)
	{
		if(icon->shutingDown)
		{
			icon->thumbThread->m_bAutoDelete = false;
		}
		else
		{
			icon->thumbThread = NULL;
		}
		return 0;
	}
	icon->thumbWorking = true;

	while(icon->animating)
	{
		Sleep(10);
	}

	if(!icon->icon->Ready())
	{
		CRect rect;
		icon->GetWindowRect(&rect);
		icon->icon->Resize(rect.Width(), rect.Height());
		if(!icon->icon->Ready())
		{
			icon->thumbWorking = false;
			if(icon->shutingDown)
			{
				icon->thumbThread->m_bAutoDelete = false;
			}
			else
			{
				icon->thumbThread = NULL;
			}
			return 0;
		}
	}

	CString ext = icon->path.Mid(icon->path.ReverseFind(L'.')).MakeLower();
	if((ext == L".ico") || (ext == L".icon"))
	{
		//icon->thumbReady = true;
		icon->thumbWorking = false;
		if(icon->shutingDown)
		{
			icon->thumbThread->m_bAutoDelete = false;
		}
		else
		{
			icon->thumbThread = NULL;
		}
		return 0;
	}

	while(loadThumbThreadCounter >= 5)
	{
		Sleep(10);
	}
	loadThumbThreadCounter++;

	CDIB tmp, preIcon;
	enum {srcNone = 0, srcImage, srcVideo, srcPdf} source = srcNone;

	if(ext == L".pdf")
	{
		source = srcPdf;
	}
	else
	{
		preIcon.Assign(icon->icon);

		// try load as Gdi+ image's format
		tmp.Load(icon->path);
		if(tmp.Ready())
		{
			source = srcImage;
		}

		// may be it's a video file
		if(!tmp.Ready())
		{
			Video::GetFrame(icon->path, &tmp, icon->videoPosition);
			if(tmp.Ready())
			{
				source = srcVideo;
				icon->isVideo = true;
			}
		}
	}

	if(!tmp.Ready())
	{
		//icon->thumbReady = true;
		icon->thumbWorking = false;
		loadThumbThreadCounter--;
		if(icon->shutingDown)
		{
			icon->thumbThread->m_bAutoDelete = false;
		}
		else
		{
			icon->thumbThread = NULL;
		}
		return 0;
	}

	while(icon->animating)
	{
		Sleep(10);
	}

	DIB_ARGB *p = icon->icon->scan0;
	int size = icon->icon->Width() * icon->icon->Height();
	for(int i = 0; i < size; i++, p++)
	{
		p->c = 0x01000000;
		//p->c = 0xffff0000;
	}

	Graphics g(icon->icon->dc);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

	float k = 1;
	RectF rf;

	switch(source)
	{
	case srcImage:
		{
			const int border = 3;

			k = min((float)(icon->icon->Width() - border * 2) / tmp.Width(), 
				(float)(icon->icon->Height() - border * 2) / tmp.Height());

			rf.Width = tmp.Width() * k;
			rf.Height = tmp.Height() * k;
			rf.X = (icon->icon->Width() - rf.Width) / 2;
			//rf.Y = (icon->icon->Height() - rf.Height) / 2;
			rf.Y = icon->icon->Height() - rf.Height - border;

			RectF rb = rf;
			rb.X -= border;
			rb.Y -= border;
			rb.Width += border * 2;
			rb.Height += border * 2;

			/*SolidBrush brush2(Color(255 * 40 / 100, 0x0, 0x0, 0x0));
			g.FillRectangle(&brush2, rb);

			icon->icon->Blur(icon->icon->Rect(), CRect(0, 0, 0, 0), shadow);*/

			g.SetSmoothingMode(SmoothingModeAntiAlias);

			SolidBrush brush(0xff000000);
			g.FillRectangle(&brush, rb);

			rb.Inflate(-1.0f, -1.0f);
			brush.SetColor(0xfff0f0f0);
			g.FillRectangle(&brush, rb);

			g.DrawImage(tmp.bmp, rf);
		}
		break;

	case srcVideo:
		{
			const int border = 8;
			const int shadow = 4;

			k = min((float)(icon->icon->Width() - (/*border + */shadow) * 2) / tmp.Width(), 
				(float)(icon->icon->Height() - (border + shadow) * 2) / tmp.Height());
			
			rf.Width = tmp.Width() * k;
			rf.Height = tmp.Height() * k;
			rf.X = (icon->icon->Width() - rf.Width) / 2;
			rf.Y = (icon->icon->Height() - rf.Height) / 2;
			//rf.Y = icon->icon->Height() - rf.Height - (border + shadow);

			RectF rb = rf;
			rb.Y -= border;
			rb.Height += border * 2;

			SolidBrush brush(Color(255 * 60 / 100, 0x0, 0x0, 0x0));
			g.FillRectangle(&brush, rb);

			icon->icon->Blur(icon->icon->Rect(), CRect(0, 0, 0, 0), shadow);

			brush.SetColor(0xff0f0f0f);
			g.FillRectangle(&brush, rb);

			g.DrawImage(tmp.bmp, rf);

			GraphicsPath *path = new GraphicsPath(); 
			path->AddLine(rf.X, rf.Y - border, rf.X + rf.Width * 0.9f, rf.Y - border);
			path->AddLine(rf.X + rf.Width * 0.9f, rf.Y - border, rf.X, rf.Y + rf.Height * 0.9f);
			path->AddLine(rf.X, rf.Y + rf.Height * 0.9f, rf.X, rf.Y - border);
			path->CloseFigure();

			LinearGradientBrush brush2(rf, 
				Color(255 * 40 / 100, 0x0, 0x0, 0x0),
				Color(255 * 40 / 100, 0xd1, 0xd1, 0xd1),
				LinearGradientModeVertical);

			brush2.RotateTransform(112.0f);

			g.SetSmoothingMode(SmoothingModeAntiAlias);
			g.FillPath(&brush2, path);

			delete path;

			/*if(Icons::GetIcon(icon->path, &tmp))
			{
				const float iconSize = 0.5f;
				k = min((float)rf.Width * iconSize / tmp.Width(), 
					(float)rf.Height * iconSize / tmp.Height());

				rb = rf;
				rb.Width = tmp.Width() * k;
				rb.Height = tmp.Height() * k;
				rb.X += rf.Width - rb.Width;
				rb.Y += border + rf.Height - rb.Height;

				g.DrawImage(tmp.bmp, rb);
			}*/
		}
		break;
	}

	tmp.FreeImage();

	icon->thumbReady = true;
	loadThumbThreadCounter--;

	if(icon->IsWindowVisible())
	{
		switch(icon->container->viewMode)
		{
		case ContainerViewModeFan:
			{
				CDIB tmp0;
				tmp0.Resize(icon->dstRect.Width(), icon->dstRect.Height());
				Graphics g(tmp0.bmp);
				g.SetSmoothingMode(SmoothingModeAntiAlias);
				g.SetCompositingMode(CompositingModeSourceCopy);

				RectF rf;
				rf.Width = (REAL)icon->container->iconSize;
				rf.Height = (REAL)icon->container->iconSize;
				rf.X = (REAL)(icon->dib->Width() - icon->container->iconSize) / 2;
				rf.Y = (REAL)(icon->dib->Height() - icon->container->iconSize) / 2;

				PointF center((REAL)icon->dstRect.Width() / 2, (REAL)icon->dstRect.Height() / 2);

				Matrix m(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
				m.RotateAt(icon->index * 1.1f, center);
				m.Invert();
				g.SetTransform(&m);

				tmp.Resize(icon->icon->Width(), icon->icon->Height());
				if(tmp.Ready())
				{
					DWORD startAt = GetTickCount();
					while(!icon->animating)
					{
						float k = (float)(GetTickCount() - startAt) / 200;
						if(k > 1)
						{
							k = 1;
						}

						DIB_ARGB *ps0 = icon->icon->scan0;
						DIB_ARGB *ps1 = preIcon.scan0;
						DIB_ARGB *pd = tmp.scan0;
						int size = tmp.Width() * tmp.Height();
						for(int i = 0; i < size; i++, ps0++, ps1++, pd++)
						{
							pd->a = (unsigned char)(ps0->a * k + ps1->a * (1 - k));
							pd->r = (unsigned char)(ps0->r * k + ps1->r * (1 - k));
							pd->g = (unsigned char)(ps0->g * k + ps1->g * (1 - k));
							pd->b = (unsigned char)(ps0->b * k + ps1->b * (1 - k));
						}

						g.DrawImage(tmp.bmp, rf);
						icon->UpdateLayer(&tmp0);

						if(k == 1)
						{
							break;
						}
						Sleep(10);
					}
				}
			}
			break;

		case ContainerViewModeGrid:
			{
				tmp.Resize(icon->icon->Width(), icon->icon->Height());
				if(tmp.Ready())
				{
					DWORD startAt = GetTickCount();
					while(!icon->animating)
					{
						float k = (float)(GetTickCount() - startAt) / 200;
						if(k > 1)
						{
							k = 1;
						}

						DIB_ARGB *ps0 = icon->icon->scan0;
						DIB_ARGB *ps1 = preIcon.scan0;
						DIB_ARGB *pd = tmp.scan0;
						int size = tmp.Width() * tmp.Height();
						for(int i = 0; i < size; i++, ps0++, ps1++, pd++)
						{
							pd->a = (unsigned char)(ps0->a * k + ps1->a * (1 - k));
							pd->r = (unsigned char)(ps0->r * k + ps1->r * (1 - k));
							pd->g = (unsigned char)(ps0->g * k + ps1->g * (1 - k));
							pd->b = (unsigned char)(ps0->b * k + ps1->b * (1 - k));
						}

						icon->UpdateLayer(&tmp);

						if(k == 1)
						{
							break;
						}
						Sleep(10);
					}
				}
			}
			break;
		}

		if(!icon->animating)
		{
			icon->DrawLayer();
			icon->UpdateLayer();
		}
	}

	icon->thumbWorking = false;

	if(icon->shutingDown)
	{
		icon->thumbThread->m_bAutoDelete = false;
	}
	else
	{
		icon->thumbThread = NULL;
	}
	return 0;
}

void CContainerIcon::LoadThumb()
{
	thumbThread = AfxBeginThread(LoadThumbThread, (LPVOID)this, THREAD_PRIORITY_IDLE);
}

void CContainerIcon::LoadIcon()
{
	icon->FreeImage();
	if(!container)
	{
		return;
	}

	CDIB tmp;
	Icons::GetIcon(path, &tmp);
	if(!tmp.Ready())
	{
		return;
	}

	icon->Resize(container->iconSize, container->iconSize);
	if(!icon->Ready())
	{
		return;
	}

	DIB_ARGB *p = icon->scan0;
	int size = icon->Width() * icon->Height();
	for(int i = 0; i < size; i++, p++)
	{
		p->a = 1;
	}

	Graphics g(icon->dc);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

	float k = min((float)icon->Width() / tmp.Width(), (float)icon->Height() / tmp.Height());

	RectF rf;
	rf.Width = tmp.Width() * k;
	rf.Height = tmp.Height() * k;
	rf.X = (icon->Width() - rf.Width) / 2;
	rf.Y = (icon->Height() - rf.Height) / 2;

	g.DrawImage(tmp.bmp, rf);
}

void CContainerIcon::BeginDrag(CPoint point)
{
	CComPtr<IDataObject> dataObj;
	if(FAILED(ShellIO::GetUIObjectOfFile(NULL, path.GetBuffer(), IID_IDataObject, (void**)&dataObj)))
	{
		return;
	}

	CComPtr<IDragSourceHelper> dragSourceHelper;
	if(SUCCEEDED(CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_ALL, IID_IDragSourceHelper, (void**)&dragSourceHelper)))
	{
		HBITMAP bmp = CreateCompatibleBitmap(icon->dc, icon->Width(), icon->Height());
		HDC dc = CreateCompatibleDC(icon->dc);
		HBITMAP oldBmp = (HBITMAP)SelectObject(dc, bmp);
		BitBlt(dc, 0, 0, icon->Width(), icon->Height(), icon->dc, 0, 0, SRCCOPY);
		SelectObject(dc, oldBmp);
		DeleteObject(dc);

		if(container)
		{
			switch(container->viewMode)
			{
			case ContainerViewModeFan:
				{
					point.Offset(-dstRect.Width() / 4, -dstRect.Height() / 4);
				}
				break;
			}
		}

		SHDRAGIMAGE info = {0};
		info.sizeDragImage.cx = icon->Width();
		info.sizeDragImage.cy = icon->Height();
		info.ptOffset = point;
		info.hbmpDragImage = bmp;
		info.crColorKey = 0xa9b8c7;

		if(FAILED(dragSourceHelper->InitializeFromBitmap(&info, dataObj)))
		{
			DeleteObject(info.hbmpDragImage);
		}
	}

	DROPEFFECT dropEffect;
	SHDoDragDrop(NULL, dataObj, NULL, DROPEFFECT_LINK | DROPEFFECT_COPY | DROPEFFECT_MOVE, &dropEffect);
}

CContainer::CContainer(HWND hParent)
{
	callBack = NULL;
	param = NULL;
	position = ContainerPositionBottom;
	viewMode = ContainerViewModeGrid;

	bckg = new CContainerBckg();
	bckg->CreateEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST, NULL, NULL, WS_POPUP, 0, 0, 0, 0, hParent, NULL);
	bckg->container = this;

	// calculate size
	HMONITOR monitor = MonitorFromWindow(hParent, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi = {0};
	mi.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &mi);

	int monitorSize = min(mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top);

	iconSize = (int)(monitorSize * 0.09f);
	cellSize = (int)(iconSize * 1.2f);

	limitIconsFan = (int)(monitorSize * 0.8f / iconSize);

	limitIconsGrid = (int)(monitorSize * 0.8f / cellSize);
	limitIconsGrid *= limitIconsGrid;
	int w = (int)(sqrt((float)limitIconsGrid) + 1);
	int h = limitIconsGrid / w;
	if(w * h < limitIconsGrid)
	{
		w++;
	}
	if(w * h < limitIconsGrid)
	{
		h++;
	}
	limitIconsGrid = w * h;

	separatorTextSize = (int)(iconSize * 0.04f);
	separatorSize = (int)(iconSize * 0.16f);

	// calculate text height
	HDC dc = CreateCompatibleDC(0);
	Graphics g(dc);

	Font font(L"Arial", 9.0f);
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
}

CContainer::~CContainer()
{
	RemoveIconAll();
	bckg->DestroyWindow();
}

CContainerIcon* CContainer::AddIcon()
{
	if(((viewMode == ContainerViewModeGrid) && (icons.GetCount() == limitIconsGrid)) ||
		((viewMode == ContainerViewModeFan) && (icons.GetCount() == limitIconsFan)))
	{
		return NULL;
	}

	int size = iconSize;
	if(viewMode == ContainerViewModeFan)
	{
		size = iconSize * 2;
	}

	CContainerIcon *icon = new CContainerIcon();
	icon->CreateEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST, NULL, NULL, WS_POPUP, 0, 0, size, size, bckg->m_hWnd, NULL);
	icon->index = icons.GetCount();
	icon->container = this;
	SetProp(icon->m_hWnd, L"IsContainerIcon", (HANDLE)TRUE);
	SetProp(icon->m_hWnd, L"ContainerIcon", (HANDLE)icon);
	icons.AddTail(icon);
	return icon;
}

void CContainer::RemoveIcon(CContainerIcon *icon)
{
	POSITION p = icons.Find(icon);
	if(p)
	{
		icons.RemoveAt(p);
		icon->DestroyWindow();
	}
}

void CContainer::RemoveIconAll()
{
	POSITION p = icons.GetHeadPosition();
	while(p)
	{
		icons.GetAt(p)->DestroyWindow();
		icons.GetNext(p);
	}
	icons.RemoveAll();
}

CRect CContainer::IconRect(CContainerIcon *icon)
{
	CRect r(0, 0, 0, 0);
	switch(viewMode)
	{
	case ContainerViewModeFan:
		{
			if(icon->index < limitIconsFan)
			{
				r.left = (int)(bckg->dib->Width() - iconSize * 5 / 2 + 5 + iconSize * 0.8f * 
					(1 - cos((float)icon->index / limitIconsFan * PI / 2)));
				r.top = (min(icons.GetCount(), limitIconsFan) - icon->index) * iconSize - iconSize / 2;
				r.right = r.left + iconSize * 2;
				r.bottom = r.top + iconSize * 2;
			}
		}
		break;

	case ContainerViewModeGrid:
		{
			int c = icons.GetCount();
			int w = (int)(sqrt((float)c) + 1);
			int h = c / w;
			if(w * h < c)
			{
				w++;
			}
			if(w * h < c)
			{
				h++;
			}

			r.left = ShadowSize + BorderSize + (icon->index % w) * (cellSize + separatorSize) + (cellSize - iconSize) / 2;
			r.top = CaptionSize + (icon->index / w) * (iconSize + separatorSize + textHeight + separatorTextSize);
			r.right = r.left + iconSize;
			r.bottom = r.top + iconSize;

			switch(position)
			{
			case ContainerPositionLeft:
				{
					r.OffsetRect(ArrowHeight, 0);
				}
				break;

			case ContainerPositionTop:
				{
					r.OffsetRect(0, ArrowHeight);
				}
				break;
			}
		}
		break;
	}

	return r;
}
	
CContainerIcon* CContainer::IconAt(CPoint pt)
{
	POSITION p = icons.GetHeadPosition();
	while(p)
	{
		CContainerIcon *icon = icons.GetAt(p);
		if(IconRect(icon).PtInRect(pt))
		{
			return icon;
		}
		icons.GetNext(p);
	}
	return NULL;
}

#define AnimateItem(item) \
	switch(viewMode) \
	{ \
	case ContainerViewModeGrid: \
		{ \
			switch(position) \
			{ \
				case ContainerPositionTop: \
				case ContainerPositionBottom: \
					{ \
						item->SetWindowPos(&CWnd::wndTop, \
							(int)(item->srcRect.left - (item->srcRect.left - item->dstRect.left) * k), \
							(int)(item->srcRect.top - (item->srcRect.top - item->dstRect.top) * k), \
							0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE); \
					} \
					break; \
		 \
				case ContainerPositionRight: \
				case ContainerPositionLeft: \
					{ \
						item->SetWindowPos(&CWnd::wndTop, \
							(int)(item->srcRect.left - (item->srcRect.left - item->dstRect.left) * k), \
							(int)(item->srcRect.top - (item->srcRect.top - item->dstRect.top) * k), \
							0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE); \
					} \
					break; \
			} \
		} \
		break; \
 \
	case ContainerViewModeFan: \
		{ \
			item->SetWindowPos(&CWnd::wndTop, \
				(int)(item->srcRect.left - (item->srcRect.left - item->dstRect.left) * k), \
				(int)(item->srcRect.top - (item->srcRect.top - item->dstRect.top) * sin(k * PI / 2)), \
				0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE); \
		} \
		break; \
	}

void CContainer::DrawCaption()
{
	switch(viewMode)
	{
	case ContainerViewModeGrid:
		{
			CRect r;
			bckg->GetWindowRect(&r);

			r.right = r.Width() - (ShadowSize + BorderSize * 2 + ButtonWidth) * 2;
			r.left = ShadowSize + BorderSize + ButtonWidth + BorderSize;
			r.top = 0;
			r.right += r.left;
			r.bottom = r.top + CaptionSize;

			switch(position)
			{
			case ContainerPositionLeft:
				{
					r.OffsetRect(ArrowHeight, 0);
				}
				break;

			case ContainerPositionTop:
				{
					r.OffsetRect(0, ArrowHeight);
				}
				break;
			}

			Graphics g(bckg->dib->dc);
			g.SetTextRenderingHint(TextRenderingHintAntiAlias);

			Font font(L"Arial", 10.0f, FontStyleBold);

			StringFormat *stringFormat = new StringFormat();
			stringFormat->SetAlignment(StringAlignmentCenter);
			stringFormat->SetLineAlignment(StringAlignmentCenter);
			stringFormat->SetFormatFlags(StringFormatFlagsLineLimit);
			stringFormat->SetTrimming(StringTrimmingEllipsisCharacter);

			RectF rf((REAL)r.left, (REAL)r.top, (REAL)r.Width(), (REAL)r.Height());

			rf.Offset(0, -1);
			SolidBrush brush(0xff000000);
			g.DrawString(caption.GetBuffer(), caption.GetLength(), &font, rf, stringFormat, &brush);

			rf.Offset(0, 1);
			brush.SetColor(0xffb0b0b0);
			g.DrawString(caption.GetBuffer(), caption.GetLength(), &font, rf, stringFormat, &brush);

			delete stringFormat;
		}
		break;
	}
}

void CContainer::DrawIcon(CContainerIcon *icon)
{
	switch(viewMode)
	{
	case ContainerViewModeFan:
		{
			const float radius = (float)textHeight;
			const float angle = icon->index * 1.15f;

			if(icon->index >= limitIconsFan)
			{
				return;
			}

			CRect r;
			r.left = textHeight;
			r.top = (min(icons.GetCount(), limitIconsFan) - icon->index) * iconSize;
			r.right = bckg->dib->Width() - 5 - iconSize * 2;
			r.bottom = r.top + iconSize;

			Graphics g(bckg->dib->dc);
			g.SetSmoothingMode(SmoothingModeAntiAlias);

			int tw = TextWidth(icon->text);

			RectF rs;
			rs.X = (REAL)(r.left + r.Width() - min(tw, r.Width()) - textHeight);
			rs.Height = (REAL)(textHeight + 2);
			rs.Y = (REAL)(r.top + (r.Height() - rs.Height) / 2);
			rs.Width = (REAL)(min(tw, r.Width()) + textHeight);

			GraphicsPath *path = new GraphicsPath();
			path->AddArc(rs.X, rs.Y + rs.Height - radius, radius - 1, radius - 1, 90, 90);
			path->AddArc(rs.X, rs.Y, radius - 1, radius - 1, 180, 90);
			path->AddArc(rs.X + rs.Width - radius, rs.Y, radius - 1, radius - 1, 270, 90);
			path->AddArc(rs.X + rs.Width - radius, rs.Y + rs.Height - radius, radius - 1, radius - 1, 0, 90);
			path->CloseFigure();

			Matrix m(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
			m.Translate(rs.X + rs.Width + iconSize * 0.8f * 
				(1 - cos((float)icon->index / limitIconsFan * PI / 2)), rs.Y + rs.Height / 2);
			m.Rotate(angle);
			m.Translate(-(rs.X + rs.Width), -(rs.Y + rs.Height / 2));
			g.SetTransform(&m);

			SolidBrush brush(Color(255 * 80 / 100, 0, 0, 0));
			g.FillPath(&brush, path);

			Font font(L"Arial", 9.0f);
			StringFormat *stringFormat = new StringFormat();
			stringFormat->SetAlignment(StringAlignmentCenter);
			stringFormat->SetLineAlignment(StringAlignmentCenter);
			stringFormat->SetFormatFlags(StringFormatFlagsLineLimit);
			stringFormat->SetTrimming(StringTrimmingEllipsisCharacter);

			rs.X += textHeight / 2;
			rs.Width -= textHeight;

			brush.SetColor(0xffffffff);
			g.SetTextRenderingHint(TextRenderingHintAntiAlias);
			g.DrawString(icon->text.GetBuffer(), icon->text.GetLength(), &font, rs, stringFormat, &brush);

			delete stringFormat;
			delete path;

			const int blur = 12;

			CDIB tmp;
			tmp.Resize(iconSize + blur * 2, iconSize + blur * 2);
			tmp.Draw(CRect(blur, blur, blur + iconSize, blur + iconSize), 0, 0, icon->icon, DrawFlagsReflectDest);

			DIB_ARGB *p = tmp.scan0;
			int size = tmp.Width() * tmp.Height();
			for(int n = 0; n < size; n++, p++)
			{
				p->r = 0;
				p->g = 0;
				p->b = 0;
				p->a = (unsigned char)(p->a * 0.3f);
			}

			tmp.Blur(tmp.Rect(), CRect(0, 0, 0, 0), blur);

			const float k = 1;

			RectF rf;
			rf.X = (REAL)bckg->dib->Width() - iconSize * 2 + iconSize * (1 - k) / 2;
			rf.Y = (REAL)r.top + iconSize * (1 - k);
			rf.Width = k * iconSize;
			rf.Height = k * iconSize;

			rf.Y += iconSize * 0.2f;

			g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
			g.DrawImage(tmp.bmp, rf);
		}
		break;

	case ContainerViewModeGrid:
		{
			int c = icons.GetCount();
			int w = (int)(sqrt((float)c) + 1);
			int h = c / w;
			if(w * h < c)
			{
				w++;
			}
			if(w * h < c)
			{
				h++;
			}

			CRect r;
			r.left = ShadowSize + BorderSize + (icon->index % w) * (cellSize + separatorSize);
			r.top = CaptionSize + (icon->index / w) * (iconSize + separatorSize + textHeight + separatorTextSize) + iconSize + separatorTextSize;
			r.right = r.left + cellSize;
			r.bottom = r.top + textHeight;

			switch(position)
			{
			case ContainerPositionLeft:
				{
					r.OffsetRect(ArrowHeight, 0);
				}
				break;

			case ContainerPositionTop:
				{
					r.OffsetRect(0, ArrowHeight);
				}
				break;
			}

			Graphics g(bckg->dib->dc);
			g.SetSmoothingMode(SmoothingModeAntiAlias);

			Font font(L"Arial", 9.0f);
			StringFormat *stringFormat = new StringFormat();
			stringFormat->SetAlignment(StringAlignmentCenter);
			stringFormat->SetLineAlignment(StringAlignmentCenter);
			stringFormat->SetFormatFlags(StringFormatFlagsLineLimit);
			stringFormat->SetTrimming(StringTrimmingEllipsisCharacter);

			RectF rf((REAL)r.left, (REAL)r.top, (REAL)r.Width(), (REAL)r.Height());
			SolidBrush brush(0xffffffff);

			g.DrawString(icon->text.GetBuffer(), icon->text.GetLength(), &font, rf, stringFormat, &brush);

			RectF rb(0, 0, 0, 0);
			g.MeasureString(icon->text.GetBuffer(), icon->text.GetLength(), &font, rb, stringFormat, &rb);
			icon->shortText = rf.Width < rb.Width;

			delete stringFormat;
		}
		break;
	}
}

int CContainer::TextWidth(CString s)
{
	HDC dc = CreateCompatibleDC(0);
	Graphics g(dc);

	Font font(L"Arial", 9.0f);
	StringFormat *stringFormat = new StringFormat();
	stringFormat->SetAlignment(StringAlignmentCenter);
	stringFormat->SetLineAlignment(StringAlignmentCenter);
	stringFormat->SetFormatFlags(StringFormatFlagsLineLimit);
	stringFormat->SetTrimming(StringTrimmingEllipsisCharacter);

	RectF r;
	r.X = 0;
	r.Y = 0;
	r.Width = 0;
	r.Height = 0;

	g.SetTextRenderingHint(TextRenderingHintAntiAlias);
	g.MeasureString(s.GetBuffer(), s.GetLength(), &font, r, stringFormat, &r);

	delete stringFormat;
	DeleteObject(dc);

	return (int)(r.Width + 2);
}

void CContainer::Show(CPoint point, CRect rect, bool slow)
{
	if(bckg->IsWindowVisible())
	{
		return;
	}
	int width, height, x, y;
	
	HMONITOR monitor = MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi = {0};
	mi.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &mi);

	switch(viewMode)
	{
	case ContainerViewModeFan:
		{
			int maxWidth = 0;
			int n = 0;
			POSITION p = icons.GetHeadPosition();
			while(p && (n < limitIconsFan))
			{
				int w = TextWidth(icons.GetAt(p)->text);
				if(w > maxWidth)
				{
					maxWidth = w;
				}
				icons.GetNext(p);
			}

			width = textHeight + (int)min(maxWidth, (mi.rcMonitor.right - mi.rcMonitor.left) * 0.3f) + 5 + iconSize * 2;
			height = (int)(iconSize * 1.2f + iconSize * min(icons.GetCount(), limitIconsFan));

			x = rect.right - width + iconSize - (rect.Width() - iconSize) / 2;
			y = point.y - height;

			// for icons
			rect.OffsetRect(-iconSize / 2, -iconSize / 2);
		}
		break;

	case ContainerViewModeGrid:
		{
			int c = icons.GetCount();
			int w = c == 1 ? 1 : (int)(sqrt((float)c) + 1);
			int h = c / w;
			if(w * h < c)
			{
				w++;
			}
			if(w * h < c)
			{
				h++;
			}
			switch(position)
			{
			case ContainerPositionBottom:
				{
					width = ShadowSize * 2 + BorderSize * 2 + w * (cellSize + separatorSize) - separatorSize;
					height = CaptionSize + ShadowSize + BorderSize + ArrowHeight + h * (iconSize + separatorSize + textHeight + separatorTextSize) - separatorSize;

					x = point.x - width / 2;
					y = point.y - height;

					bckg->ArrowOffset = 0;
					if(mi.rcMonitor.left > x)
					{
						bckg->ArrowOffset = x - mi.rcMonitor.left;
						x = mi.rcMonitor.left;
					}
					if(mi.rcMonitor.right < x + width)
					{
						bckg->ArrowOffset = x + width - mi.rcMonitor.right;
						x = mi.rcMonitor.right - width;
					}
				}
				break;

			case ContainerPositionLeft:
				{
					width = ShadowSize * 2 + BorderSize * 2 + ArrowHeight + w * (cellSize + separatorSize) - separatorSize;
					height = CaptionSize + ShadowSize + BorderSize + h * (iconSize + separatorSize + textHeight + separatorTextSize) - separatorSize;

					x = point.x;
					y = point.y - height / 2;

					bckg->ArrowOffset = 0;
					if(mi.rcMonitor.top > y)
					{
						bckg->ArrowOffset = y - mi.rcMonitor.top;
						y = mi.rcMonitor.top;
					}
					if(mi.rcMonitor.bottom < y + height)
					{
						bckg->ArrowOffset = y + height - mi.rcMonitor.bottom;
						y = mi.rcMonitor.bottom - height;
					}
				}
				break;

			case ContainerPositionTop:
				{
					width = ShadowSize * 2 + BorderSize * 2 + w * (cellSize + separatorSize) - separatorSize;
					height = CaptionSize + ShadowSize + BorderSize + ArrowHeight + h * (iconSize + separatorSize + textHeight + separatorTextSize) - separatorSize;

					x = point.x - width / 2;
					y = point.y + ArrowHeight;

					bckg->ArrowOffset = 0;
					if(mi.rcMonitor.left > x)
					{
						bckg->ArrowOffset = x - mi.rcMonitor.left;
						x = mi.rcMonitor.left;
					}
					if(mi.rcMonitor.right < x + width)
					{
						bckg->ArrowOffset = x + width - mi.rcMonitor.right;
						x = mi.rcMonitor.right - width;
					}
				}
				break;

			case ContainerPositionRight:
				{
					width = ShadowSize * 2 + BorderSize * 2 + ArrowHeight + w * (cellSize + separatorSize) - separatorSize;
					height = CaptionSize + ShadowSize + BorderSize + h * (iconSize + separatorSize + textHeight + separatorTextSize) - separatorSize;

					x = point.x - width;
					y = point.y - height / 2;

					bckg->ArrowOffset = 0;
					if(mi.rcMonitor.top > y)
					{
						bckg->ArrowOffset = y - mi.rcMonitor.top;
						y = mi.rcMonitor.top;
					}
					if(mi.rcMonitor.bottom < y + height)
					{
						bckg->ArrowOffset = y + height - mi.rcMonitor.bottom;
						y = mi.rcMonitor.bottom - height;
					}
				}
				break;
			}
		}
		break;
	}

	bckg->SetWindowPos(&CWnd::wndTop, x, y, width, height, SWP_NOZORDER);
	bckg->position = position;
	bckg->viewMode = viewMode;

	bckg->DrawLayer();
	DrawCaption();
	POSITION p = icons.GetHeadPosition();
	while(p)
	{
		DrawIcon(icons.GetAt(p));
		icons.GetNext(p);
	}

	bckg->tmp->Resize(bckg->dib->Width(), bckg->dib->Height());
	bckg->ShowWindow(SW_SHOW);

	p = icons.GetTailPosition();
	while(p)
	{
		CContainerIcon *icon = icons.GetAt(p);
		icon->SetWindowPos(&CWnd::wndTop, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		icon->position = position;
		icon->dstRect = IconRect(icon);
		icon->dstRect.OffsetRect(x, y);
		icon->srcRect = rect;
		icon->dib->Resize(icon->dstRect.Width(), icon->dstRect.Height());
		icon->UpdateLayer();
		icon->ShowWindow(SW_SHOW);
		icons.GetPrev(p);
	}

	if(callBack)
	{
		callBack->SendMessage(WM_CONTAINEREVENT, (WPARAM)ContainerEventPopupAnimation, (LPARAM)param);
	}

	DWORD delay = slow ? 8000 : 300;
	DWORD startAt = GetTickCount();
	for(;;)
	{
		float k = (float)(GetTickCount() - startAt) / delay;
		if(k > 1)
		{
			k = 1;
		}
		k = sin(k * PI / 2);

		bckg->DrawStep(bckg->tmp, bckg->dib, k, true);
		bckg->UpdateLayer(bckg->tmp);

		p = icons.GetTailPosition();
		while(p)
		{
			CContainerIcon *icon = icons.GetAt(p);
			icon->DrawStep(icon->dib, icon->icon, k, true);
			AnimateItem(icon);
			icon->UpdateLayer();
			icons.GetPrev(p);
		}

		if(k == 1)
		{
			break;
		}
	}

	p = icons.GetHeadPosition();
	while(p)
	{
		icons.GetAt(p)->LoadThumb();
		icons.GetNext(p);
	}

	bckg->UpdateLayer();
}

void CContainer::Hide(bool slow)
{
	if(!bckg->IsWindowVisible())
	{
		return;
	}

	bckg->tmp->Resize(bckg->dib->Width(), bckg->dib->Height());

	POSITION p = icons.GetHeadPosition();
	while(p)
	{
		icons.GetAt(p)->animating = true;
		icons.GetNext(p);
	}

	p = icons.FindIndex(min(icons.GetCount() - 1, 4));
	while(p)
	{
		icons.GetAt(p)->BringWindowToTop();
		icons.GetPrev(p);
	}

	DWORD delay = slow ? 8000 : 200;
	DWORD startAt = GetTickCount();
	for(;;)
	{
		float k = 1 - (float)(GetTickCount() - startAt) / delay;
		if(k < 0)
		{
			k = 0;
		}
		k = sin(k * PI / 2);
		
		bckg->DrawStep(bckg->tmp, bckg->dib, k, true);
		bckg->UpdateLayer(bckg->tmp);

		p = icons.GetTailPosition();
		while(p)
		{
			CContainerIcon *icon = icons.GetAt(p);
			icon->DrawStep(icon->dib, icon->icon, k, true);
			AnimateItem(icon);
			icon->UpdateLayer();
			icons.GetPrev(p);
		}

		if(k == 0)
		{
			break;
		}
	}

	if(callBack)
	{
		callBack->SendMessage(WM_CONTAINEREVENT, (WPARAM)ContainerEventHideIcons, (LPARAM)param);
	}

	p = icons.GetTailPosition();
	while(p)
	{
		CContainerIcon *icon = icons.GetAt(p);
		icon->ShowWindow(SW_HIDE);
		icon->dib->FreeImage();
		icon->animating = false;
		icons.GetPrev(p);
	}

	bckg->ShowWindow(SW_HIDE);
	bckg->dib->FreeImage();
	bckg->tmp->FreeImage();
}
