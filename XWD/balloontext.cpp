#include "balloontext.h"

using namespace Gdiplus;

BEGIN_MESSAGE_MAP(CBalloonText, CFrameWnd)
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_WM_TIMER()
END_MESSAGE_MAP()

#define ArrowWidth 12
#define ArrowHeight 6
#define SkipSize 2
#define ShadowSize 0

enum
{
	baloonTextTimerHide = 1
};

#define BALOONTEXT_HIDE_DELAY 150

CBalloonText::CBalloonText()
{
	dib = new CDIB();
	tmp = new CDIB();
	position = DockPositionBottom;
	hiding = false;
	prepearing = false;
	fontSize = 11.0f;

	CreateEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST, NULL, NULL, WS_POPUP, CRect(0, 0, 0, 0), NULL, NULL);
}

CBalloonText::~CBalloonText()
{
	delete tmp;
	delete dib;
}

LRESULT CBalloonText::OnSetText(WPARAM wParam, LPARAM lParam)
{
	LRESULT res = DefWindowProc(WM_SETTEXT, wParam, lParam);
	if(IsWindowVisible() && !prepearing)
	{
		if (GetWindowTextLength() > 0)
		{
			Prepare();
		}
		else
		{
			KillTimer(baloonTextTimerHide);
			hiding = false;
			ShowWindow(SW_HIDE);
		}
	}
	return res;
}

void CBalloonText::OnTimer(UINT_PTR nIDEvent)
{
	CFrameWnd::OnTimer(nIDEvent);

	switch(nIDEvent)
	{
	case baloonTextTimerHide:
		{
			float k = (float)(GetTickCount() - hideStartAt) / BALOONTEXT_HIDE_DELAY;
			if(k >= 1)
			{
				k = 1;
				KillTimer(nIDEvent);
				hiding = false;
				ShowWindow(SW_HIDE);
			}
			else
			{
				tmp->Assign(dib);
				if(tmp->Ready())
				{
					tmp->AlphaBlend(tmp->Rect(), (unsigned char)(255 * (1 - k)), DrawFlagsPaint);
					LayerUpdate(tmp);
				}
			}
		}
		break;
	}
}

void CBalloonText::Prepare(CPoint point, DockPosition position)
{
	CBalloonText::point = point;
	CBalloonText::position = position;

	Prepare();
}

void CBalloonText::Prepare()
{
	CMonitors monitors;
	CMonitor *monitor = monitors.GetMonitor(point);
	if(!monitor)
	{
		return;
	}

	CDIB dib;
	dib.Resize(1, 1);
	if(!dib.Ready())
	{
		return;
	}

	Graphics g(dib.dc);

	Font font(L"Arial", fontSize);

	StringFormat *stringFormat = new StringFormat();
	stringFormat->SetAlignment(StringAlignmentCenter);
	stringFormat->SetLineAlignment(StringAlignmentCenter);
	stringFormat->SetFormatFlags(StringFormatFlagsLineLimit);
	stringFormat->SetTrimming(StringTrimmingEllipsisCharacter);

	CString s;
	GetWindowText(s);

	RectF rf;
	g.MeasureString(s.GetBuffer(), s.GetLength(), &font, rf, stringFormat, &rf);
	rf.Height += 2 + ShadowSize; // to be sure that we'll see the whole text in the height
	rf.Width += 2;

	delete stringFormat;

	radius = rf.Height + SkipSize;

	CRect rect;
	switch(position)
	{
	case DockPositionBottom:
		{
			rect.left = point.x - (int)((rf.Width + radius) / 2);
			rect.right = rect.left + (int)(rf.Width + radius);
			rect.top = point.y - (int)(rf.Height + SkipSize * 2 + ArrowHeight);
			rect.bottom = point.y;
		}
		break;

	case DockPositionTop:
		{
			rect.left = point.x - (int)((rf.Width + radius) / 2);
			rect.right = rect.left + (int)(rf.Width + radius);
			rect.top = point.y;
			rect.bottom = rect.top + (int)(rf.Height + SkipSize * 2 + ArrowHeight);
		}
		break;

	case DockPositionLeft:
		{
			rect.left = point.x;
			rect.right = rect.left + (int)(rf.Width + radius);
			rect.top = point.y - (int)((rf.Height + SkipSize * 2) / 2);
			rect.bottom = rect.top + (int)(rf.Height + SkipSize * 2);
		}
		break;

	//case DockPositionRight:
	default:
		{
			rect.left = point.x - (int)(rf.Width + radius);
			rect.right = point.x;
			rect.top = point.y - (int)((rf.Height + SkipSize * 2) / 2);
			rect.bottom = rect.top + (int)(rf.Height + SkipSize * 2);
		}
		break;
	}

	CRect monitorRect = monitor->Rect();
	arrowOffset = 0;
	if(rect.left < monitorRect.left)
	{
		arrowOffset = rect.left - monitorRect.left;
		rect.MoveToX(monitorRect.left);
	}
	if(rect.right > monitorRect.right)
	{
		arrowOffset = rect.right - monitorRect.right;
		rect.MoveToX(monitorRect.right - rect.Width());
	}

	SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

	LayerDraw();
	LayerUpdate();
}

void CBalloonText::UpdatePosition(CPoint point, DockPosition position)
{
	CMonitors monitors;
	CMonitor *monitor = monitors.GetMonitor(point);
	if(!monitor)
	{
		return;
	}

	CRect rect;
	GetWindowRect(&rect);
	switch(position)
	{
	case DockPositionBottom:
		{
			rect.MoveToXY(point.x - rect.Width() / 2, point.y - rect.Height());
		}
		break;

	case DockPositionTop:
		{
			rect.MoveToXY(point.x - rect.Width() / 2, point.y);
		}
		break;

	case DockPositionLeft:
		{
			rect.MoveToXY(point.x, point.y - rect.Height() / 2);
		}
		break;

	//case DockPositionRight:
	default:
		{
			rect.MoveToXY(point.x - rect.Width(), point.y - rect.Height() / 2);
		}
		break;
	}

	CRect monitorRect = monitor->Rect();
	int a = arrowOffset;
	arrowOffset = 0;
	if(rect.left < monitorRect.left)
	{
		arrowOffset = rect.left - monitorRect.left;
		rect.MoveToX(monitorRect.left);
	}
	if(rect.right > monitorRect.right)
	{
		arrowOffset = rect.right - monitorRect.right;
		rect.MoveToX(monitorRect.right - rect.Width());
	}

	SetWindowPos(&wndTop, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	if(a != arrowOffset)
	{
		LayerDraw();
		LayerUpdate();
	}
}

void CBalloonText::Popup()
{
	if(IsWindowVisible() && !hiding)
	{
		return;
	}
	if(hiding)
	{
		KillTimer(baloonTextTimerHide);
		hiding = false;
	}
	ShowWindow(SW_SHOWNOACTIVATE);
}

void CBalloonText::Hide()
{
	if(!IsWindowVisible() || hiding)
	{
		return;
	}
	hiding = true;
	hideStartAt = GetTickCount();
	SetTimer(baloonTextTimerHide, 10, NULL);
}

void CBalloonText::LayerDraw(CDIB *dib)
{
	if(!dib)
	{
		dib = CBalloonText::dib;
	}
	
	CRect rect;
	GetWindowRect(&rect);
	
	dib->Resize(rect.Width(), rect.Height());
	if(!dib->Ready())
	{
		return;
	}
	RectF rf(0, 0, (REAL)dib->Width(), (REAL)dib->Height() - ShadowSize);
	RectF rt;

	/*DIB_ARGB *p = dib->scan0;
	for(int i = 0; i< dib->Width() * dib->Height(); i++, p++)
	{
		p->c = 0xffff0000;
	}*/

	Graphics g(dib->dc);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	GraphicsPath *path = new GraphicsPath();

	switch(position)
	{
	case DockPositionBottom:
		{
			path->AddArc(rf.X, rf.Y + rf.Height - radius - ArrowHeight, radius - 1, radius - 1, 90, 90);
			path->AddArc(rf.X, rf.Y, radius - 1, radius - 1, 180, 90);
			path->AddArc(rf.X + rf.Width - radius, rf.Y, radius - 1, radius - 1, 270, 90);
			path->AddArc(rf.X + rf.Width - radius, rf.Y + rf.Height - radius - ArrowHeight, radius - 1, radius - 1, 0, 90);

			path->AddLine(rf.X + (rf.Width - ArrowWidth) / 2 + ArrowWidth + arrowOffset, rf.Y + rf.Height - 1 - ArrowHeight,
				rf.X + (rf.Width - ArrowWidth) / 2 + ArrowWidth / 2 + arrowOffset, rf.Y + rf.Height - 1);

			path->AddLine(rf.X + (rf.Width - ArrowWidth) / 2 + ArrowWidth / 2 + arrowOffset, rf.Y + rf.Height - 1,
				rf.X + (rf.Width - ArrowWidth) / 2 + arrowOffset, rf.Y + rf.Height - 1 - ArrowHeight);

			rt.X = rf.X + radius / 2;
			rt.Y = rf.Y + SkipSize;
			rt.Width = rf.Width - radius;
			rt.Height = rf.Height - SkipSize * 2 - ArrowHeight;
		}
		break;

	case DockPositionTop:
		{
			path->AddArc(rf.X + rf.Width - radius, rf.Y + ArrowHeight, radius - 1, radius - 1, 270, 90);
			path->AddArc(rf.X + rf.Width - radius, rf.Y + rf.Height - radius, radius - 1, radius - 1, 0, 90);
			path->AddArc(rf.X, rf.Y + rf.Height - radius, radius - 1, radius - 1, 90, 90);
			path->AddArc(rf.X, rf.Y + ArrowHeight, radius - 1, radius - 1, 180, 90);
			
			path->AddLine(rf.X + (rf.Width - ArrowWidth) / 2 + ArrowWidth + arrowOffset, rf.Y + ArrowHeight,
				rf.X + (rf.Width - ArrowWidth) / 2 + ArrowWidth / 2 + arrowOffset, rf.Y);

			path->AddLine(rf.X + (rf.Width - ArrowWidth) / 2 + ArrowWidth / 2 + arrowOffset, rf.Y,
				rf.X + (rf.Width - ArrowWidth) / 2 + arrowOffset, rf.Y + ArrowHeight);

			rt.X = rf.X + radius / 2;
			rt.Y = rf.Y + ArrowHeight + SkipSize;
			rt.Width = rf.Width - radius;
			rt.Height = rf.Height - SkipSize * 2 - ArrowHeight;
		}
		break;

	//case DockPositionLeft:
	//case DockPositionRight:
	default:
		{
			path->AddArc(rf.X + rf.Width - radius, rf.Y + rf.Height - radius, radius - 1, radius - 1, 0, 90);
			path->AddArc(rf.X, rf.Y + rf.Height - radius, radius - 1, radius - 1, 90, 90);
			path->AddArc(rf.X, rf.Y, radius - 1, radius - 1, 180, 90);
			path->AddArc(rf.X + rf.Width - radius, rf.Y, radius - 1, radius - 1, 270, 90);

			rt.X = rf.X + radius / 2;
			rt.Y = rf.Y + SkipSize;
			rt.Width = rf.Width - radius;
			rt.Height = rf.Height - SkipSize * 2;
		}
		break;
	}

	path->CloseFigure();

	SolidBrush brush(0xff000000);

	/*CDIB tmp;
	tmp.Resize(dib->Width(), dib->Height());
	if(tmp.Ready())
	{
		Graphics gt(tmp.bmp);
		gt.FillPath(&brush, path);

		tmp.Blur(tmp.Rect(), CRect(0, 0, 0, 0), ShadowSize);

		g.DrawImage(tmp.bmp, RectF(rf.X + ShadowSize / 2, rf.Y + ShadowSize, rf.Width - ShadowSize, rf.Height),
			0, (REAL)ShadowSize, rf.Width, rf.Height, UnitPixel);
	}

	g.SetCompositingMode(CompositingModeSourceCopy);
	brush.SetColor(0x0);
	g.FillPath(&brush, path);
	*/
	g.SetCompositingMode(CompositingModeSourceOver);

	LinearGradientBrush lb(rf, Color(255 * 90 / 100, 0x44, 0x44, 0x44), Color(255 * 90 / 100, 0x10, 0x10, 0x10), LinearGradientModeVertical);
	g.FillPath(&lb, path);

	//brush.SetColor(Color(255 * 90 / 100, 0x10, 0x10, 0x10));
	//g.FillPath(&brush, path);

	Font font(L"Arial", fontSize);
	StringFormat *stringFormat = new StringFormat();
	stringFormat->SetAlignment(StringAlignmentCenter);
	stringFormat->SetLineAlignment(StringAlignmentCenter);
	stringFormat->SetFormatFlags(StringFormatFlagsLineLimit);
	stringFormat->SetTrimming(StringTrimmingEllipsisCharacter);

	CString s;
	GetWindowText(s);
	
	//brush.SetColor(0xff000000);
	//rt.Y++;
	//g.DrawString(s.GetBuffer(), s.GetLength(), &font, rt, stringFormat, &brush);

	//g.SetTextRenderingHint(TextRenderingHintAntiAlias);
	
	brush.SetColor(0xffffffff);
	//rt.Y--;
	g.DrawString(s.GetBuffer(), s.GetLength(), &font, rt, stringFormat, &brush);

	delete stringFormat;
	delete path;
}

void CBalloonText::LayerUpdate(CDIB *dib)
{
	if(!dib)
	{
		dib = CBalloonText::dib;
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
	CPoint dstPt(rect.left, rect.top);
	CSize dstSize(rect.Width(), rect.Height());

	UpdateLayeredWindow(hdcDst, &dstPt, &dstSize, dc, &zp, NULL, &blend, ULW_ALPHA);

	dc->Detach();
	delete dc;
	wndDst->ReleaseDC(hdcDst);
}
