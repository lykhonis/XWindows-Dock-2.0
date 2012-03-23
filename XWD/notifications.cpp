#include "notifications.h"

using namespace Gdiplus;

BEGIN_MESSAGE_MAP(CNotification, CFrameWnd)
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

#define SHADOW_SIZE 2

CNotification::CNotification()
{
	dib = new CDIB();
	position = NotificationPositionLeftTop;
}

CNotification::~CNotification()
{
	delete dib;
}

int CNotification::CalculateWidth(int defaultSize)
{
	CString s;
	GetWindowText(s);
	
	CDIB dib;
	dib.Resize(1, 1);
	if(!dib.Ready())
	{
		return defaultSize;
	}
	Graphics g(dib.dc);
	g.SetCompositingMode(CompositingModeSourceOver);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	RectF rf(0, 0, 0, 0);

	Font font(L"Arial", defaultSize * 0.6f, FontStyleRegular, UnitPixel);
	StringFormat *stringFormat = new StringFormat();
	stringFormat->SetAlignment(StringAlignmentCenter);
	stringFormat->SetLineAlignment(StringAlignmentCenter);
	stringFormat->SetFormatFlags(StringFormatFlagsLineLimit);
	stringFormat->SetTrimming(StringTrimmingEllipsisCharacter);

	g.MeasureString(s.GetBuffer(), s.GetLength(), &font, rf, stringFormat, &rf);

	delete stringFormat;

	defaultSize += (int)rf.Width - defaultSize / 2;
	return defaultSize;
}

void CNotification::LayerDraw(CDIB *dib)
{
	if(!dib)
	{
		dib = CNotification::dib;
	}
	CRect rect;
	GetWindowRect(&rect);
	dib->Resize(rect.Width(), rect.Height());
	if(!dib->Ready())
	{
		return;
	}

	RectF rf(0.0f, 0.0f, (REAL)dib->Width(), (REAL)dib->Height());
	rf.Width -= SHADOW_SIZE;
	rf.Height -= SHADOW_SIZE;

	Graphics g(dib->dc);
	g.SetCompositingMode(CompositingModeSourceOver);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	GraphicsPath *path = new GraphicsPath();

	float radius = rf.Height;

	path->AddArc(rf.X + rf.Width - radius, rf.Y + rf.Height - radius, radius - 1, radius - 1, 0, 90);
	path->AddArc(rf.X, rf.Y + rf.Height - radius, radius - 1, radius - 1, 90, 90);
	path->AddArc(rf.X, rf.Y, radius - 1, radius - 1, 180, 90);
	path->AddArc(rf.X + rf.Width - radius, rf.Y, radius - 1, radius - 1, 270, 90);
	path->CloseFigure();

	g.TranslateTransform(SHADOW_SIZE, SHADOW_SIZE);
	g.ScaleTransform((rf.Width - SHADOW_SIZE) / rf.Width, (rf.Height - SHADOW_SIZE) / rf.Height);

	SolidBrush brush2(0xf0000000);
	g.FillPath(&brush2, path);

	g.ResetTransform();

	dib->Blur(dib->Rect(), CRect(0, 0, 0, 0), SHADOW_SIZE);

	brush2.SetColor(0xffffffff);
	g.FillPath(&brush2, path);

	rf.X += rf.Height * 0.1f;
	rf.Y += rf.Height * 0.1f;
	rf.Width -= rf.Height * 0.2f;
	rf.Height -= rf.Height * 0.2f;

	radius = rf.Height;

	path->Reset();
	path->AddArc(rf.X + rf.Width - radius, rf.Y + rf.Height - radius, radius - 1, radius - 1, 0, 90);
	path->AddArc(rf.X, rf.Y + rf.Height - radius, radius - 1, radius - 1, 90, 90);
	path->AddArc(rf.X, rf.Y, radius - 1, radius - 1, 180, 90);
	path->AddArc(rf.X + rf.Width - radius, rf.Y, radius - 1, radius - 1, 270, 90);
	path->CloseFigure();

	LinearGradientBrush brush(rf, 0xff6fa6de, 0xff1e6cbb, LinearGradientModeVertical);
	g.FillPath(&brush, path);

	delete path;

	Font font(L"Arial", rect.Height() * 0.6f, FontStyleRegular, UnitPixel);
	StringFormat *stringFormat = new StringFormat();
	stringFormat->SetAlignment(StringAlignmentCenter);
	stringFormat->SetLineAlignment(StringAlignmentCenter);
	stringFormat->SetFormatFlags(StringFormatFlagsLineLimit);
	stringFormat->SetTrimming(StringTrimmingEllipsisCharacter);

	CString s;
	GetWindowText(s);

	g.DrawString(s.GetBuffer(), s.GetLength(), &font, rf, stringFormat, &brush2);

	delete stringFormat;
}

void CNotification::LayerUpdate(CDIB *dib)
{
	if(!dib)
	{
		dib = CNotification::dib;
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

void CNotification::OnWindowPosChanged(WINDOWPOS *lpwndpos)
{
	lpwndpos->cx += SHADOW_SIZE;
	lpwndpos->cy += SHADOW_SIZE;

	CFrameWnd::OnWindowPosChanged(lpwndpos);

	if((lpwndpos->flags & SWP_NOSIZE) == SWP_NOSIZE)
	{
		return;
	}
	LayerDraw();
	LayerUpdate();
}

void CNotification::OnClose()
{
}
