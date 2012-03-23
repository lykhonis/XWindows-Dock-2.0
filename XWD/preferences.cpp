#include "preferences.h"

BEGIN_MESSAGE_MAP(CPreferences, CFrameWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_NCHITTEST()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_NCACTIVATE()
	ON_WM_CLOSE()

	ON_MESSAGE(WM_SETTEXT, OnSetText)

	ON_REGISTERED_MESSAGE(WM_CONTROLNOTIFY, OnControlNotify)
END_MESSAGE_MAP()

using namespace Gdiplus;
using namespace ShellIO;

#define CAPTION_HEIGHT 24
#define TOOLBAR_HEIGHT 76
#define WINDOW_BORDER_LEFT 1
#define WINDOW_BORDER_TOP 1
#define WINDOW_BORDER_RIGHT 1
#define WINDOW_BORDER_BOTTOM 1
#define MONITORS_OFFSET 15

#define LANGUAGE_ICONSIZE L"Icons\' Size:"
#define LANGUAGE_DOCKMODE L"Dock Mode:"
#define LANGUAGE_POSITION L"Position on Screen:"
#define LANGUAGE_MONITOR L"Monitor:"
#define LANGUAGE_LOCKITEMS L"Lock Items"
#define LANGUAGE_RUNWITHWINDOWS L"Run with Windows"
#define LANGUAGE_DOCKTOPMOST L"Keep Dock over all Windows"
#define LANGUAGE_RESERVESCREEN L"Reserve Screen"
#define LANGUAGE_ENABLEICONSSHADOW L"Enable Icons Shadow"
#define LANGUAGE_ENABLEWINDOWSREFLECTION L"Enable Windows Reflection"
#define LANGUAGE_CHECKFORUPDATES L"Check for Updates"
#define LANGUAGE_SHOWALLRUNNINGAPPSINDOCK L"Show all running apps in Dock"
#define LANGUAGE_TRACKBAR_SMALLER L"Smaller"
#define LANGUAGE_TRACKBAR_LARGER L"Larger"
#define LANGUAGE_2D L"2D"
#define LANGUAGE_3D L"3D"
#define LANGUAGE_LEFT L"Left"
#define LANGUAGE_TOP L"Top"
#define LANGUAGE_RIGHT L"Right"
#define LANGUAGE_BOTTOM L"Bottom"

#define LANGUAGE_PLUGINS_DRAGDROP L"Use Drag&Drop to add the plugin to the dock"

#define LANGUAGE_SKINS L"Use navigations buttons to apply skin to the dock"
#define LANGUAGE_SKINS_NEXT L"Next"
#define LANGUAGE_SKINS_PREV L"Previous"
#define LANGUAGE_SKINS_APPLY L"Apply"

enum
{
	idButtonClose = 1,
	idTabs,
	idMode2D,
	idMode3D,
	idPositionLeft,
	idPositionRight,
	idPositionTop,
	idPositionBottom,
	idLockItems,
	idRunWithWindows,
	idDockTopMost,
	idReserveScreen,
	idIconsShadow,
	idWindowsReflection,
	idCheckForUpdates,
	idShowAllRunningAppsInDock,
	idIconSize,
	idMonitors,
	idSkinNext,
	idSkinPrev,
	idSkinApply
};

CPreferences::CPreferences()
{
//	logo = new CDIB();
	dib = new CDIB();
	anim = new CDIB();
	animating = false;
	notifer = NULL;
	nID = 0;
	lastTab = NULL;

	CreateEx(NULL, NULL, L"XWindows Dock - Preferences", WS_OVERLAPPED | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, CRect(0, 0, 0, 0), NULL, NULL);
}

CPreferences::~CPreferences()
{
	delete anim;
	delete dib;
//	delete logo;
}

LRESULT CPreferences::OnNcHitTest(CPoint point)
{
	ScreenToClient(&point);
	if(point.y < CAPTION_HEIGHT)
	{
		return HTCAPTION;
	}
	return HTCLIENT;
}

int CPreferences::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CFrameWnd::OnCreate(lpCreateStruct);
	ModifyStyle(WS_CAPTION | WS_BORDER, NULL);
	SetClassLong(CFrameWnd::m_hWnd, GCL_HICON, (LONG)LoadIcon(AfxGetInstanceHandle(), IDI_APPLICATION));

	btnClose = new CImageButton();
	btnClose->color1 = 0xff757575;
	btnClose->color2 = 0xff6d6d6d;
	btnClose->CreateEx(NULL, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 14, 14), this, idButtonClose);

	tabs = new CTabsControl();
	tabs->color1 = 0xff696969;
	tabs->color2 = 0xff373737;
	tabs->CreateEx(NULL, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, idTabs);

	tabs->selected = tabs->Add(L"General", L"", idTabsGeneral);
	tabs->selected->selected = true;

	tabs->Add(L"Skins", L"", idTabsSkins);
	tabs->Add(L"Plugins", L"", idTabsPlugins);

	mode2d = new CCheckBox();
	mode2d->radio = true;
	mode2d->CreateEx(NULL, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 13, 13), this, idMode2D);

	mode3d = new CCheckBox();
	mode3d->radio = true;
	mode3d->CreateEx(NULL, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 13, 13), this, idMode3D);

	positionLeft = new CCheckBox();
	positionLeft->radio = true;
	positionLeft->CreateEx(NULL, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 13, 13), this, idPositionLeft);

	positionRight = new CCheckBox();
	positionRight->radio = true;
	positionRight->CreateEx(NULL, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 13, 13), this, idPositionRight);

	positionTop = new CCheckBox();
	positionTop->radio = true;
	positionTop->CreateEx(NULL, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 13, 13), this, idPositionTop);

	positionBottom = new CCheckBox();
	positionBottom->radio = true;
	positionBottom->CreateEx(NULL, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 13, 13), this, idPositionBottom);

	lockItems = new CCheckBox();
	lockItems->CreateEx(NULL, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 13, 13), this, idLockItems);

	runWithWindows = new CCheckBox();
	runWithWindows->CreateEx(NULL, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 13, 13), this, idRunWithWindows);

	dockTopMost = new CCheckBox();
	dockTopMost->CreateEx(NULL, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 13, 13), this, idDockTopMost);

	reserveScreen = new CCheckBox();
	reserveScreen->CreateEx(NULL, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 13, 13), this, idReserveScreen);

	enableIconsShadow = new CCheckBox();
	enableIconsShadow->CreateEx(NULL, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 13, 13), this, idIconsShadow);

	enableWindowsReflection = new CCheckBox();
	enableWindowsReflection->CreateEx(NULL, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 13, 13), this, idWindowsReflection);

	checkForUpdates = new CCheckBox();
	checkForUpdates->CreateEx(NULL, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 13, 13), this, idCheckForUpdates);

	showAllRunningAppsInDock = new CCheckBox();
	showAllRunningAppsInDock->CreateEx(NULL, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 13, 13), this, idShowAllRunningAppsInDock);

	iconSize = new CTrackBar();
	iconSize->CreateEx(NULL, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 200, 10), this, idIconSize);

	monitors = new CGComboBox();
	monitors->CreateEx(NULL, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 110, 20), this, idMonitors);

	pluginsList = new CPluginList();
	pluginsList->CreateEx(NULL, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, NULL);

	skinsList = new CSkinList();
	skinsList->CreateEx(NULL, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, NULL);

	btnSkinPrev = new CImageButton();
	btnSkinPrev->color1 = 0xffebebeb;
	btnSkinPrev->color2 = 0xffebebeb;
	btnSkinPrev->CreateEx(NULL, NULL, LANGUAGE_SKINS_PREV, WS_CHILD | WS_VISIBLE, CRect(0, 0, 75, 20), this, idSkinPrev);

	btnSkinNext = new CImageButton();
	btnSkinNext->color1 = 0xffebebeb;
	btnSkinNext->color2 = 0xffebebeb;
	btnSkinNext->CreateEx(NULL, NULL, LANGUAGE_SKINS_NEXT, WS_CHILD | WS_VISIBLE, CRect(0, 0, 75, 20), this, idSkinNext);

	btnSkinApply = new CImageButton();
	btnSkinApply->color1 = 0xffebebeb;
	btnSkinApply->color2 = 0xffebebeb;
	btnSkinApply->CreateEx(NULL, NULL, LANGUAGE_SKINS_APPLY, WS_CHILD | WS_VISIBLE, CRect(0, 0, 75, 20), this, idSkinApply);

	return 0;
}

void CPreferences::OnDestroy()
{
	CFrameWnd::OnDestroy();
}

void CPreferences::OnWindowPosChanged(WINDOWPOS *lpwndpos)
{
	CFrameWnd::OnWindowPosChanged(lpwndpos);

	if((lpwndpos->flags & SWP_NOSIZE) == SWP_NOSIZE)
	{
		return;
	}
	CRect r;
	GetClientRect(&r);

	r.left += WINDOW_BORDER_LEFT;
	r.top += WINDOW_BORDER_TOP;
	r.right -= WINDOW_BORDER_RIGHT;
	r.bottom -= WINDOW_BORDER_BOTTOM;

	btnClose->SetWindowPos(&wndTop, r.right - (CAPTION_HEIGHT - 14) / 2 - 14, r.top + (CAPTION_HEIGHT - 14) / 2 - 1, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);

	tabs->SetWindowPos(&wndTop, r.left, r.top + CAPTION_HEIGHT, r.Width(), TOOLBAR_HEIGHT - 1, SWP_NOZORDER | SWP_NOACTIVATE);

	int monitorOffset = (monitors->items.GetCount() > 1 ? 0 : MONITORS_OFFSET);

	mode2d->SetWindowPos(&wndTop, r.left + (int)(r.Width() * 0.38f) + 4, r.top + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 40 + 50 + monitorOffset + (20 - 13) / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	mode3d->SetWindowPos(&wndTop, r.left + (int)(r.Width() * 0.38f) + 4 + 60, r.top + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 40 + 50 + monitorOffset + (20 - 13) / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	positionLeft->SetWindowPos(&wndTop, r.left + (int)(r.Width() * 0.38f) + 4, r.top + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 40 + 50 + monitorOffset + 20 + (20 - 13) / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	positionRight->SetWindowPos(&wndTop, r.left + (int)(r.Width() * 0.38f) + 4 + 60, r.top + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 40 + 50 + monitorOffset + 20 + (20 - 13) / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	positionTop->SetWindowPos(&wndTop, r.left + (int)(r.Width() * 0.38f) + 4 + 60 * 2, r.top + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 40 + 50 + monitorOffset + 20 + (20 - 13) / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	positionBottom->SetWindowPos(&wndTop, r.left + (int)(r.Width() * 0.38f) + 4 + 60 * 3, r.top + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 40 + 50 + monitorOffset + 20 + (20 - 13) / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	lockItems->SetWindowPos(&wndTop, r.left + (int)(r.Width() * 0.38f) + 4, 
		r.top + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 40 + 30 + monitorOffset * 2 + 30 + 40 + (20 - 13) / 2, 
		0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	dockTopMost->SetWindowPos(&wndTop, r.left + (int)(r.Width() * 0.38f) + 4, 
		r.top + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 40 + 30 + monitorOffset * 2 + 30 + 40 + 20 + (20 - 13) / 2, 
		0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	reserveScreen->SetWindowPos(&wndTop, r.left + (int)(r.Width() * 0.38f) + 4, 
		r.top + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 40 + 30 + monitorOffset * 2 + 30 + 40 + 20 * 2 + (20 - 13) / 2, 
		0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	runWithWindows->SetWindowPos(&wndTop, r.left + (int)(r.Width() * 0.38f) + 4, 
		r.top + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 40 + 30 + monitorOffset * 2 + 30 + 40 + 20 * 3 + (20 - 13) / 2, 
		0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	enableIconsShadow->SetWindowPos(&wndTop, r.left + (int)(r.Width() * 0.38f) + 4, 
		r.top + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 40 + 30 + monitorOffset * 2 + 30 + 40 + 20 * 4 + (20 - 13) / 2, 
		0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	enableWindowsReflection->SetWindowPos(&wndTop, r.left + (int)(r.Width() * 0.38f) + 4, 
		r.top + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 40 + 30 + monitorOffset * 2 + 30 + 40 + 20 * 5 + (20 - 13) / 2, 
		0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	checkForUpdates->SetWindowPos(&wndTop, r.left + (int)(r.Width() * 0.38f) + 4, 
		r.top + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 40 + 30 + monitorOffset * 2 + 30 + 40 + 20 * 6 + (20 - 13) / 2, 
		0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	showAllRunningAppsInDock->SetWindowPos(&wndTop, r.left + (int)(r.Width() * 0.38f) + 4,
		r.top + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 40 + 30 + monitorOffset * 2 + 30 + 40 + 20 * 7 + (20 - 13) / 2,
		0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	iconSize->SetWindowPos(&wndTop, r.left + (int)(r.Width() * 0.38f) + 10, r.top + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 44, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	monitors->SetWindowPos(&wndTop, r.left + (int)(r.Width() * 0.38f) + 4, r.top + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 40 + 50 + 30 + 40 + 60 + 70, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	pluginsList->SetWindowPos(&wndTop, r.left, r.top + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 50, r.Width(), r.Height() - 50 - CAPTION_HEIGHT - TOOLBAR_HEIGHT, SWP_NOZORDER | SWP_NOACTIVATE);
	
	skinsList->SetWindowPos(&wndTop, r.left + (int)(r.Width() * 0.1f), r.top + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 100, (int)(r.Width() * 0.8f), 100, SWP_NOZORDER | SWP_NOACTIVATE);

	btnSkinPrev->SetWindowPos(&wndTop, r.left + (r.Width() - 3 * (75 + 2)) / 2, r.top + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 100 + 110, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	btnSkinNext->SetWindowPos(&wndTop, r.left + (r.Width() - 3 * (75 + 2)) / 2 + 1 * (75 + 2), r.top + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 100 + 110, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	btnSkinApply->SetWindowPos(&wndTop, r.left + (r.Width() - 3 * (75 + 2)) / 2 + 2 * (75 + 2), r.top + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 100 + 110, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

BOOL CPreferences::PreCreateWindow(CREATESTRUCT &cs)
{
    if(CFrameWnd::PreCreateWindow(cs))
    {
        cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
        return TRUE;
    }
    else
	{
        return FALSE;
	}
}

BOOL CPreferences::OnEraseBkgnd(CDC*)
{
	return TRUE;
}

BOOL CPreferences::OnNcActivate(BOOL)
{
	return TRUE;
}

void CPreferences::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS *lpncsp)
{
	/*if(lpncsp->lppos)
	{
		int cxfr = GetSystemMetrics(SM_CXSIZEFRAME);
		int cyfr = GetSystemMetrics(SM_CYSIZEFRAME);
		InflateRect(lpncsp->rgrc, cxfr - 1, cyfr - 1);
	}*/
	CFrameWnd::OnNcCalcSize(bCalcValidRects, lpncsp);
}

void CPreferences::OnNcPaint()
{
	CFrameWnd::OnNcPaint();
}

void CPreferences::OnPaint()
{
	CPaintDC dc(this);

	CRect r;
	GetClientRect(&r);

	if(animating)
	{
		BitBlt(dc.m_hDC, r.left, r.top + (WINDOW_BORDER_TOP + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 8), 
			r.Width(), r.Height(), anim->dc, 0, 0, SRCCOPY);
		return;
	}

	if(dib->Ready())
	{
		BitBlt(dc.m_hDC, r.left, r.top, r.Width(), r.Height(), dib->dc, 0, 0, SRCCOPY);
	}
}

void CPreferences::Draw(CDIB *dib, CTabControl *tab)
{
	CRect r;
	GetClientRect(&r);

	dib->Resize(r.Width(), r.Height());
	if(!dib->Ready())
	{
		return;
	}

	RectF rf((REAL)r.left, (REAL)r.top, (REAL)r.Width(), (REAL)r.Height());
	RectF rx;

	Graphics g(dib->dc);
	g.SetSmoothingMode(SmoothingModeNone);
	g.SetCompositingMode(CompositingModeSourceCopy);

	SolidBrush brush(0xff000000);
	g.FillRectangle(&brush, rf);

	rf.X += WINDOW_BORDER_LEFT;
	rf.Y += WINDOW_BORDER_TOP;
	rf.Width -= (WINDOW_BORDER_LEFT + WINDOW_BORDER_RIGHT);
	rf.Height -= (WINDOW_BORDER_TOP + WINDOW_BORDER_BOTTOM);

	brush.SetColor(0xffebebeb);
	g.FillRectangle(&brush, rf);

	g.SetCompositingMode(CompositingModeSourceOver);
	g.SetClip(rf);

	// background
//	if(logo->Ready())
//	{
//		g.DrawImage(logo->bmp, rf.X + (rf.Width - logo->Width()) / 2, rf.Y + (rf.Height - logo->Height()) / 2);
//	}

	rx = rf;
	rx.Height = CAPTION_HEIGHT + TOOLBAR_HEIGHT;
	LinearGradientBrush lb1(rx, 0xff787878, 0xff363636, LinearGradientModeVertical);
	g.FillRectangle(&lb1, rx);

	g.SetSmoothingMode(SmoothingModeAntiAlias);
	Font font(L"Arial", 9.0f, FontStyleBold);
	StringFormat *stringFormat = new StringFormat();
	stringFormat->SetAlignment(StringAlignmentNear);
	stringFormat->SetLineAlignment(StringAlignmentCenter);
	stringFormat->SetTrimming(StringTrimmingEllipsisCharacter);
	stringFormat->SetFormatFlags(StringFormatFlagsLineLimit);

	rx = rf;
	rx.Height = CAPTION_HEIGHT - 1;

	CString s;
	GetWindowText(s);

	RectF rt;
	g.MeasureString(s.GetBuffer(), s.GetLength(), &font, rx, stringFormat, &rt);

	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

	HICON hIcon = (HICON)GetClassLong(CFrameWnd::m_hWnd, GCL_HICON);
	CDIB tmp;
	Icons::GetDIBFromIcon(hIcon, &tmp);
	if(tmp.Ready())
	{
		rx.X = (rx.Width - rt.Width - 18) / 2;
		rx.Width = rt.Width + 16;

		RectF ri = rx;
		ri.Y += (ri.Height - 16) / 2;
		ri.Width = 16;
		ri.Height = 16;

		rx.X += 18;

		g.DrawImage(tmp.bmp, ri);
	}
	else
	{
		rx.X = (rx.Width - rt.Width) / 2;
		rx.Width = rt.Width;
	}

	rx.Y++;

	brush.SetColor(0xff000000);
	g.DrawString(s.GetBuffer(), s.GetLength(), &font, rx, stringFormat, &brush);

	rx.Y--;

	brush.SetColor(0xfff0f0f0);
	g.DrawString(s.GetBuffer(), s.GetLength(), &font, rx, stringFormat, &brush);

	// Paint content
#define DrawString(s, f) \
	rx.Y++; \
	brush.SetColor(0xfff0f0f0); \
	g.DrawString(s, wcslen(s), &f, rx, stringFormat, &brush); \
	rx.Y--; \
	brush.SetColor(0xff000000); \
	g.DrawString(s, wcslen(s), &f, rx, stringFormat, &brush);

	Font font2(L"Arial", 9.0f);
	Font font3(L"Arial", 7.0f);

	if(tab)
	{
		switch(tab->id)
		{
		case idTabsPlugins:
			{
				rx = rf;
				rx.Y += CAPTION_HEIGHT + TOOLBAR_HEIGHT + 10;
				rx.Height = 40;

				stringFormat->SetAlignment(StringAlignmentCenter);
				stringFormat->SetLineAlignment(StringAlignmentCenter);

				DrawString(LANGUAGE_PLUGINS_DRAGDROP, font2);
			}
			break;

		case idTabsSkins:
			{
				rx = rf;
				rx.Y += CAPTION_HEIGHT + TOOLBAR_HEIGHT + 10;
				rx.Height = 40;

				stringFormat->SetAlignment(StringAlignmentCenter);
				stringFormat->SetLineAlignment(StringAlignmentCenter);

				DrawString(LANGUAGE_SKINS, font2);
			}
			break;

		case idTabsGeneral:
			{
				stringFormat->SetAlignment(StringAlignmentFar);

				rx = rf;
				rx.Width *= 0.38f;
				rx.Y += CAPTION_HEIGHT + TOOLBAR_HEIGHT + 40;
				rx.Height = 20;

				DrawString(LANGUAGE_ICONSIZE, font2);

				if(monitors->items.GetCount() <= 1)
				{
					rx.Y += MONITORS_OFFSET;
				}

				rx.Y += 50;
				DrawString(LANGUAGE_DOCKMODE, font2);

				rx.Y += 20;
				DrawString(LANGUAGE_POSITION, font2);

				if(monitors->items.GetCount() <= 1)
				{
					rx.Y += MONITORS_OFFSET;
				}

				rx.Y += 30;
				DrawString(LANGUAGE_LOCKITEMS, font2);

				rx.Y += 20;
				DrawString(LANGUAGE_DOCKTOPMOST, font2);

				rx.Y += 20;
				DrawString(LANGUAGE_RESERVESCREEN, font2);

				rx.Y += 20;
				DrawString(LANGUAGE_RUNWITHWINDOWS, font2);

				rx.Y += 20;
				DrawString(LANGUAGE_ENABLEICONSSHADOW, font2);

				rx.Y += 20;
				DrawString(LANGUAGE_ENABLEWINDOWSREFLECTION, font2);

				rx.Y += 20;
				DrawString(LANGUAGE_CHECKFORUPDATES, font2);

				rx.Y += 20;
				DrawString(LANGUAGE_SHOWALLRUNNINGAPPSINDOCK, font2);

				if(monitors->items.GetCount() > 1)
				{
					rx.Y += 30;
					DrawString(LANGUAGE_MONITOR, font2);
				}

				stringFormat->SetAlignment(StringAlignmentNear);

				rx = rf;
				rx.X += rx.Width * 0.38f + 20;
				rx.Y += CAPTION_HEIGHT + TOOLBAR_HEIGHT + 40 + 50;
				rx.Height = 20;
				rx.Width = 50;

				if(monitors->items.GetCount() <= 1)
				{
					rx.Y += MONITORS_OFFSET;
				}

				DrawString(LANGUAGE_2D, font2);

				rx.X += 60;
				DrawString(LANGUAGE_3D, font2);

				rx = rf;
				rx.X += rx.Width * 0.38f + 20;
				rx.Y += CAPTION_HEIGHT + TOOLBAR_HEIGHT + 40 + 50 + 20;
				rx.Height = 20;
				rx.Width = 50;

				if(monitors->items.GetCount() <= 1)
				{
					rx.Y += MONITORS_OFFSET;
				}

				DrawString(LANGUAGE_LEFT, font2);

				rx.X += 60;
				DrawString(LANGUAGE_RIGHT, font2);

				rx.X += 60;
				DrawString(LANGUAGE_TOP, font2);

				rx.X += 60;
				DrawString(LANGUAGE_BOTTOM, font2);

				stringFormat->SetLineAlignment(StringAlignmentNear);

				rx = rf;
				rx.X += rx.Width * 0.38f + 10;
				rx.Y += CAPTION_HEIGHT + TOOLBAR_HEIGHT + 40 + 16;
				rx.Height = 20;
				rx.Width = 200;

				DrawString(LANGUAGE_TRACKBAR_SMALLER, font3);

				stringFormat->SetAlignment(StringAlignmentFar);
				DrawString(LANGUAGE_TRACKBAR_LARGER, font3);
			}
			break;
		}
	}
	delete stringFormat;

	// shadow for caption
	Pen pen(0xff000000);
	g.DrawLine(&pen, rf.X, rf.Y + CAPTION_HEIGHT + TOOLBAR_HEIGHT - 1, rf.X + rf.Width, rf.Y + CAPTION_HEIGHT + TOOLBAR_HEIGHT - 1);

	for(float y = 0; y < 8; y++)
	{
		float k = 1 - (1 - y / 8) * 0.3f;
		DIB_ARGB *p = (DIB_ARGB*)dib->Pixels((int)rf.X, (int)(rf.Y + CAPTION_HEIGHT + TOOLBAR_HEIGHT + y));
		for(float x = 0; x < rf.Width; x++, p++)
		{
			p->r = (unsigned char)(p->r * k);
			p->g = (unsigned char)(p->g * k);
			p->b = (unsigned char)(p->b * k);
		}
	}

	// update controls' background
#define ControlBckg(c) \
	c->GetWindowRect(&r); \
	ScreenToClient(&r); \
	c->bckg->Resize(r.Width(), r.Height()); \
	c->bckg->Draw(c->bckg->Rect(), r.left, r.top, dib, DrawFlagsReflectSrc);

	ControlBckg(mode2d);
	ControlBckg(mode3d);
	ControlBckg(positionLeft);
	ControlBckg(positionRight);
	ControlBckg(positionTop);
	ControlBckg(positionBottom);
	ControlBckg(lockItems);
	ControlBckg(runWithWindows);
	ControlBckg(dockTopMost);
	ControlBckg(reserveScreen);
	ControlBckg(enableIconsShadow);
	ControlBckg(enableWindowsReflection);
	ControlBckg(showAllRunningAppsInDock);
	ControlBckg(checkForUpdates);
	ControlBckg(iconSize);
	ControlBckg(monitors);
	ControlBckg(pluginsList);
	ControlBckg(skinsList);
}

void CPreferences::DrawAnim(CDIB *tmp, CTabControl *tab)
{
#define offsetY (WINDOW_BORDER_TOP + CAPTION_HEIGHT + TOOLBAR_HEIGHT + 8)

	CDIB tmp2;
	Draw(&tmp2, tab);
	if(!tmp2.Ready())
	{
		return;
	}
	tmp->Resize(tmp2.Width(), tmp2.Height() - offsetY);
	if(!tmp->Ready())
	{
		return;
	}
	tmp->Draw(tmp->Rect(), 0, offsetY, &tmp2, DrawFlagsReflectSrc);

	CRect r;
#define DrawControl(control) \
	control->Draw(tmp2); \
	if(tmp2.Ready()) \
	{ \
		control->GetWindowRect(&r); \
		ScreenToClient(&r); \
		r.OffsetRect(0, -offsetY); \
		tmp->Draw(r, 0, 0, &tmp2, DrawFlagsReflectSrc); \
	}

	switch(tab->id)
	{
	case idTabsGeneral:
		{
			DrawControl(iconSize);
			DrawControl(mode2d);
			DrawControl(mode3d);
			DrawControl(positionLeft);
			DrawControl(positionRight);
			DrawControl(positionTop);
			DrawControl(positionBottom);
			DrawControl(runWithWindows);
			DrawControl(lockItems);
			DrawControl(dockTopMost);
			DrawControl(reserveScreen);
			DrawControl(enableIconsShadow);
			DrawControl(enableWindowsReflection);
			DrawControl(showAllRunningAppsInDock);
			DrawControl(checkForUpdates);
			if(monitors->items.GetCount() > 1)
			{
				DrawControl(monitors);
			}
		}
		break;

	case idTabsSkins:
		{
			DrawControl(skinsList);
			DrawControl(btnSkinNext);
			DrawControl(btnSkinPrev);
			DrawControl(btnSkinApply);
		}
		break;

	case idTabsPlugins:
		{
			DrawControl(pluginsList);
		}
		break;
	}
}

void CPreferences::OpenTab(CTabControl *tab)
{
	if(lastTab)
	{
		CDIB tmp0, tmp1;
		DrawAnim(&tmp0, lastTab);
		DrawAnim(&tmp1, tab);

		anim->Assign(&tmp0);
		anim->ReflectVertical();
		animating = true;

		iconSize->ShowWindow(SW_HIDE);
		mode2d->ShowWindow(SW_HIDE);
		mode3d->ShowWindow(SW_HIDE);
		positionLeft->ShowWindow(SW_HIDE);
		positionRight->ShowWindow(SW_HIDE);
		positionTop->ShowWindow(SW_HIDE);
		positionBottom->ShowWindow(SW_HIDE);
		lockItems->ShowWindow(SW_HIDE);
		runWithWindows->ShowWindow(SW_HIDE);
		dockTopMost->ShowWindow(SW_HIDE);
		reserveScreen->ShowWindow(SW_HIDE);
		enableIconsShadow->ShowWindow(SW_HIDE);
		enableWindowsReflection->ShowWindow(SW_HIDE);
		showAllRunningAppsInDock->ShowWindow(SW_HIDE);
		checkForUpdates->ShowWindow(SW_HIDE);
		pluginsList->ShowWindow(SW_HIDE);
		skinsList->ShowWindow(SW_HIDE);
		btnSkinPrev->ShowWindow(SW_HIDE);
		btnSkinNext->ShowWindow(SW_HIDE);
		btnSkinApply->ShowWindow(SW_HIDE);
		monitors->ShowWindow(SW_HIDE);

		RedrawWindow();

		DWORD startAt = GetTickCount();
		for(;;)
		{
			float k = (float)(GetTickCount() - startAt) / 170; // 7000;
			if(k >= 1)
			{
				k = 1;
				break;
			}
			k = cos((1 - k) * PI / 2);
			for(int y = 0; y < tmp0.Height(); y++)
			{
				DIB_ARGB *pd = (DIB_ARGB*)anim->Pixels(0, tmp0.Height() - 1 - y);
				DIB_ARGB *ps0 = (DIB_ARGB*)tmp0.Pixels(0, y);
				DIB_ARGB *ps1 = (DIB_ARGB*)tmp1.Pixels(0, y);
				for(int x = 0; x < tmp0.Width(); x++, pd++, ps0++, ps1++)
				{
					pd->a = 255;
					pd->r = (unsigned char)(ps0->r * (1 - k) + ps1->r * k);
					pd->g = (unsigned char)(ps0->g * (1 - k) + ps1->g * k);
					pd->b = (unsigned char)(ps0->b * (1 - k) + ps1->b * k);
				}
			}
			RedrawWindow();
		}

		animating = false;
		anim->FreeImage();
	}
	lastTab = tab;
	Draw(dib, tab);

	iconSize->ShowWindow(tab->id == idTabsGeneral ? SW_SHOW : SW_HIDE);
	mode2d->ShowWindow(tab->id == idTabsGeneral ? SW_SHOW : SW_HIDE);
	mode3d->ShowWindow(tab->id == idTabsGeneral ? SW_SHOW : SW_HIDE);
	positionLeft->ShowWindow(tab->id == idTabsGeneral ? SW_SHOW : SW_HIDE);
	positionRight->ShowWindow(tab->id == idTabsGeneral ? SW_SHOW : SW_HIDE);
	positionTop->ShowWindow(tab->id == idTabsGeneral ? SW_SHOW : SW_HIDE);
	positionBottom->ShowWindow(tab->id == idTabsGeneral ? SW_SHOW : SW_HIDE);
	lockItems->ShowWindow(tab->id == idTabsGeneral ? SW_SHOW : SW_HIDE);
	runWithWindows->ShowWindow(tab->id == idTabsGeneral ? SW_SHOW : SW_HIDE);
	dockTopMost->ShowWindow(tab->id == idTabsGeneral ? SW_SHOW : SW_HIDE);
	reserveScreen->ShowWindow(tab->id == idTabsGeneral ? SW_SHOW : SW_HIDE);
	enableIconsShadow->ShowWindow(tab->id == idTabsGeneral ? SW_SHOW : SW_HIDE);
	enableWindowsReflection->ShowWindow(tab->id == idTabsGeneral ? SW_SHOW : SW_HIDE);
	showAllRunningAppsInDock->ShowWindow(tab->id == idTabsGeneral ? SW_SHOW : SW_HIDE);
	checkForUpdates->ShowWindow(tab->id == idTabsGeneral ? SW_SHOW : SW_HIDE);
	pluginsList->ShowWindow(tab->id == idTabsPlugins ? SW_SHOW : SW_HIDE);
	skinsList->ShowWindow(tab->id == idTabsSkins ? SW_SHOW : SW_HIDE);
	btnSkinPrev->ShowWindow(tab->id == idTabsSkins ? SW_SHOW : SW_HIDE);
	btnSkinNext->ShowWindow(tab->id == idTabsSkins ? SW_SHOW : SW_HIDE);
	btnSkinApply->ShowWindow(tab->id == idTabsSkins ? SW_SHOW : SW_HIDE);
	monitors->ShowWindow((tab->id == idTabsGeneral) && (monitors->items.GetCount() > 1) ? SW_SHOW : SW_HIDE);

	switch(tab->id)
	{
	case idTabsSkins:
		{
			btnSkinApply->EnableWindow(FALSE);
			btnSkinPrev->EnableWindow(FALSE);
			btnSkinNext->EnableWindow(FALSE);
			POSITION p1 = skinsList->loader->items.Find(skinsList->skin);
			if(p1)
			{
				btnSkinApply->EnableWindow();
				POSITION p = p1;
				skinsList->loader->items.GetPrev(p);
				if(p)
				{
					btnSkinPrev->EnableWindow();
				}
				p = p1;
				skinsList->loader->items.GetNext(p);
				if(p)
				{
					btnSkinNext->EnableWindow();
				}
			}
		}
		break;
	}

	RedrawWindow();
}

LRESULT CPreferences::OnSetText(WPARAM wParam, LPARAM lParam)
{
	LRESULT res = DefWindowProc(WM_SETTEXT, wParam, lParam);
	RedrawWindow();
	return res;
}

LRESULT CPreferences::OnControlNotify(WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam))
	{
	case idSkinPrev:
		{
			POSITION p = skinsList->loader->items.Find(skinsList->skin);
			if(p)
			{
				skinsList->loader->items.GetPrev(p);
				if(p)
				{
					skinsList->skin = skinsList->loader->items.GetAt(p);
					skinsList->Draw();
					skinsList->RedrawWindow();

					btnSkinNext->EnableWindow();
					skinsList->loader->items.GetPrev(p);
					btnSkinPrev->EnableWindow(p != NULL);
				}
			}
		}
		break;

	case idSkinNext:
		{
			POSITION p = skinsList->loader->items.Find(skinsList->skin);
			if(p)
			{
				skinsList->loader->items.GetNext(p);
				if(p)
				{
					skinsList->skin = skinsList->loader->items.GetAt(p);
					skinsList->Draw();
					skinsList->RedrawWindow();

					btnSkinPrev->EnableWindow();
					skinsList->loader->items.GetNext(p);
					btnSkinNext->EnableWindow(p != NULL);
				}
			}
		}
		break;

	case idSkinApply:
		{
			AfxGetMainWnd()->PostMessage(WM_SETUPDOCK, MAKEWPARAM(SetupDockSkin, NULL), (LPARAM)skinsList->skin->name.GetBuffer());
		}
		break;

	case idTabs:
		{
			OpenTab((CTabControl*)lParam);
		}
		break;

	case idButtonClose:
		{
			OnClose();
		}
		break;

	case idMonitors:
		{
			AfxGetMainWnd()->PostMessage(WM_SETUPDOCK, MAKEWPARAM(SetupDockMonitor, monitors->selected->id));
		}
		break;

	case idMode2D:
		{
			if(AfxGetMainWnd()->SendMessage(WM_SETUPDOCK, MAKEWPARAM(SetupDockMode, DockMode2D)))
			{
				mode3d->Checked(false);
				mode2d->Checked();
				skinsList->preferMode = DockMode2D;
				skinsList->Draw();
				skinsList->RedrawWindow();
			}
		}
		break;

	case idMode3D:
		{
			if(AfxGetMainWnd()->SendMessage(WM_SETUPDOCK, MAKEWPARAM(SetupDockMode, DockMode3D)))
			{
				mode3d->Checked();
				mode2d->Checked(false);
				positionLeft->Checked(false);
				positionRight->Checked(false);
				positionTop->Checked(false);
				positionBottom->Checked();
				skinsList->preferMode = DockMode3D;
				skinsList->Draw();
				skinsList->RedrawWindow();
			}
		}
		break;

	case idPositionLeft:
		{
			if(AfxGetMainWnd()->SendMessage(WM_SETUPDOCK, MAKEWPARAM(SetupDockPosition, DockPositionLeft)))
			{
				positionLeft->Checked();
				positionRight->Checked(false);
				positionTop->Checked(false);
				positionBottom->Checked(false);
				mode3d->Checked(false);
				mode2d->Checked();
				skinsList->preferMode = DockMode2D;
				skinsList->Draw();
				skinsList->RedrawWindow();
			}
		}
		break;

	case idPositionRight:
		{
			if(AfxGetMainWnd()->SendMessage(WM_SETUPDOCK, MAKEWPARAM(SetupDockPosition, DockPositionRight)))
			{
				positionLeft->Checked(false);
				positionRight->Checked();
				positionTop->Checked(false);
				positionBottom->Checked(false);
				mode3d->Checked(false);
				mode2d->Checked();
				skinsList->preferMode = DockMode2D;
				skinsList->Draw();
				skinsList->RedrawWindow();
			}
		}
		break;

	case idPositionTop:
		{
			if(AfxGetMainWnd()->SendMessage(WM_SETUPDOCK, MAKEWPARAM(SetupDockPosition, DockPositionTop)))
			{
				positionLeft->Checked(false);
				positionRight->Checked(false);
				positionTop->Checked();
				positionBottom->Checked(false);
				mode3d->Checked(false);
				mode2d->Checked();
				skinsList->preferMode = DockMode2D;
				skinsList->Draw();
				skinsList->RedrawWindow();
			}
		}
		break;

	case idPositionBottom:
		{
			if(AfxGetMainWnd()->SendMessage(WM_SETUPDOCK, MAKEWPARAM(SetupDockPosition, DockPositionBottom)))
			{
				positionLeft->Checked(false);
				positionRight->Checked(false);
				positionTop->Checked(false);
				positionBottom->Checked();
			}
		}
		break;

	case idIconSize:
		{
			int size = AfxGetMainWnd()->SendMessage(WM_SETUPDOCK, 
				MAKEWPARAM(SetupDockIconSize, 20 + (256 - 20) * lParam / iconSize->max));
			if(size)
			{
				iconSize->SetPosition((size - 20) * iconSize->max / (256 - 20));
			}
		}
		break;

	case idLockItems:
		{
			lockItems->Checked(!lockItems->checked);
		}
		break;

	case idShowAllRunningAppsInDock:
		{
			showAllRunningAppsInDock->Checked(!showAllRunningAppsInDock->checked);
		}
		break;

	case idCheckForUpdates:
		{
			checkForUpdates->Checked(!checkForUpdates->checked);
		}
		break;

	case idRunWithWindows:
		{
			runWithWindows->Checked(!runWithWindows->checked);
		}
		break;

	case idDockTopMost:
		{
			dockTopMost->Checked(!dockTopMost->checked);
		}
		break;

	case idReserveScreen:
		{
			reserveScreen->Checked(!reserveScreen->checked);
		}
		break;

	case idIconsShadow:
		{
			enableIconsShadow->Checked(!enableIconsShadow->checked);
		}
		break;

	case idWindowsReflection:
		{
			enableWindowsReflection->Checked(!enableWindowsReflection->checked);
		}
		break;
	}
	return 0;
}

void CPreferences::OnClose()
{
	if(notifer)
	{
		notifer->PostMessage(WM_CONTROLNOTIFY, MAKEWPARAM(nID, NULL));
	}
}

BOOL CPreferences::PreTranslateMessage(MSG *pMSG)
{
	switch(pMSG->message)
	{
	case WM_KEYDOWN:
		{
			switch(pMSG->wParam)
			{
			case VK_ESCAPE:
				{
					OnClose();
					return TRUE;
				}
				break;

			case VK_TAB:
				{
					CWnd *wnd = GetNextDlgTabItem(GetFocus());
					if(wnd)
					{
						wnd->SetFocus();
					}
					return TRUE;
				}
				break;
			}
		}
		break;
	}
	return CFrameWnd::PreTranslateMessage(pMSG);
}
