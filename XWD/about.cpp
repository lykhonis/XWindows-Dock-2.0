#include "about.h"

BEGIN_MESSAGE_MAP(CAbout, CFrameWnd)
	ON_WM_CLOSE()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

using namespace Gdiplus;

CAbout::CAbout()
{
	dib = new CDIB();
	imageClose = new CDIB();
	notifer = NULL;
	nID = 0;

	CreateEx(WS_EX_LAYERED, NULL, L"XWindows Dock - About", WS_POPUP, CRect(0, 0, 0, 0), NULL, NULL);
	SetClassLong(m_hWnd, GCL_HICON, (LONG)LoadIcon(AfxGetInstanceHandle(), IDI_APPLICATION));
}

CAbout::~CAbout()
{
	delete imageClose;
	delete dib;
}

void CAbout::OnClose()
{
	if(notifer)
	{
		notifer->PostMessage(WM_CONTROLNOTIFY, MAKEWPARAM(nID, NULL));
	}
}

void CAbout::OnLButtonDown(UINT nFlags, CPoint point)
{
	CFrameWnd::OnLButtonDown(nFlags, point);
	OnClose();
}

void CAbout::LayerDraw()
{
	CRect rect;
	GetWindowRect(&rect);
	
	dib->Resize(rect.Width(), rect.Height());
	if(!dib->Ready())
	{
		return;
	}

	Graphics g(dib->dc);
	g.SetCompositingMode(CompositingModeSourceCopy);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

	RectF rf(0, 0, (REAL)dib->Width(), (REAL)dib->Height());

	const float radius = 20;
	const int shadow = 10;

	GraphicsPath *path = new GraphicsPath();

	path->AddArc(rf.X + rf.Width - radius - shadow, rf.Y + rf.Height - radius - shadow, radius - 1, radius - 1, 0, 90);
	path->AddArc(rf.X + shadow, rf.Y + rf.Height - radius - shadow, radius - 1, radius - 1, 90, 90);
	path->AddArc(rf.X + shadow, rf.Y + shadow, radius - 1, radius - 1, 180, 90);
	path->AddArc(rf.X + rf.Width - radius - shadow, rf.Y + shadow, radius - 1, radius - 1, 270, 90);
	path->CloseFigure();

	SolidBrush brush(0xff000000);
	g.FillPath(&brush, path);

	dib->Blur(dib->Rect(), CRect(shadow * 2, shadow * 2, dib->Width() - shadow * 2, dib->Height() - shadow * 2), shadow);

	brush.SetColor(0x0);
	g.FillPath(&brush, path);

	brush.SetColor(0xf4202020);
	g.FillPath(&brush, path);

	Pen pen(0xf4202020);
	g.SetCompositingMode(CompositingModeSourceOver);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.DrawPath(&pen, path);

	delete path;

	CDIB icon;
	HICON hIcon = Icons::GetIcon(AfxGetInstanceHandle(), L"#32512"); // IDI_APPLICATION
	Icons::GetDIBFromIcon(hIcon, &icon);
	DeleteObject(hIcon);

	if(icon.Ready())
	{
		RectF ri;
		ri.X = shadow + radius / 2;
		ri.Y = shadow + radius / 2 + 4;	
		ri.Width = 120;
		ri.Height = ri.Width;

		g.DrawImage(icon.bmp, ri);
	}

	if(imageClose->Ready())
	{
		RectF ri;
		ri.X = rf.Width - 14 - (shadow + radius) / 2;
		ri.Y = (shadow + radius) / 2;	
		ri.Width = 14;
		ri.Height = 14;

		g.DrawImage(imageClose->bmp, ri);
	}

	Font fontTitle(L"Arial", 19.0f, FontStyleBold);
	Font fontText(L"Arial", 10.0f);
	Font fontCopyright(L"Arial", 8.5f);

	StringFormat *stringFormat = new StringFormat();
	stringFormat->SetAlignment(StringAlignmentNear);
	stringFormat->SetLineAlignment(StringAlignmentNear);
	stringFormat->SetFormatFlags(StringFormatFlagsLineLimit);
	stringFormat->SetTrimming(StringTrimmingEllipsisCharacter);

	brush.SetColor(0xffffffff);

	CString s;
	RectF rt;
	rt.X = shadow + radius / 2 + 128 + 6;
	rt.Y = shadow + radius / 2 + 4;
	rt.Width = rf.Width - rt.X - 14 - shadow - radius / 2;
	rt.Height = 30;
	s = L"XWindows Dock ";
	s.Append(_T(XWDVERSION));
	g.DrawString(s.GetBuffer(), s.GetLength(), &fontTitle, rt, stringFormat, &brush);

	rt.Y += 34;
	rt.Height = 16;
	s = L"http://xwdock.aqua-soft.org";
	g.DrawString(s.GetBuffer(), s.GetLength(), &fontText, rt, stringFormat, &brush);

	rt.Y += 50;
	s = L"Developer: Lichonos Vladimir";
	g.DrawString(s.GetBuffer(), s.GetLength(), &fontText, rt, stringFormat, &brush);

	rt.Y += 20;
	rt.Height = 48;
	s = L"Thanks: Plotnikov Alexey, Norrapol Paiboonsil";
	g.DrawString(s.GetBuffer(), s.GetLength(), &fontText, rt, stringFormat, &brush);
 
	brush.SetColor(0xffb0b0b0);

	rt.Y += 52;
	rt.Height = 100;
	s = L"XWindows Dock unrelated to Dock.app, Mac OS X Leopard or other software company Apple Inc. "
		L"All trademarks are the property of their respective owners.\n\nLichonos Vladimir assumes no responsibility for "
		L"any negative actions resulting from the work of XWindows Dock.";
	g.DrawString(s.GetBuffer(), s.GetLength(), &fontCopyright, rt, stringFormat, &brush);
	
	rt.Y += 136;
	rt.Height = 16;
	s = L"© 2008-2010 Lichonos Vladimir. All rights reserved.";
	g.DrawString(s.GetBuffer(), s.GetLength(), &fontCopyright, rt, stringFormat, &brush);

	delete stringFormat;
}

void CAbout::LayerUpdate()
{
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
