#include "checkbox.h"

BEGIN_MESSAGE_MAP(CCheckBox, CFrameWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_LBUTTONDOWN()
	ON_WM_ENABLE()
END_MESSAGE_MAP()

using namespace Gdiplus;

CCheckBox::CCheckBox()
{
	bckg = new CDIB();
	radio = false;
	checked = false;
}

CCheckBox::~CCheckBox()
{
	delete bckg;
}

void CCheckBox::Checked(bool value)
{
	if(value != checked)
	{
		checked = value;
		RedrawWindow();
	}
}

BOOL CCheckBox::PreCreateWindow(CREATESTRUCT& cs)
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

BOOL CCheckBox::OnEraseBkgnd(CDC*)
{
	return TRUE;
}

void CCheckBox::OnWindowPosChanged(WINDOWPOS *lpwndpos)
{
	CFrameWnd::OnWindowPosChanged(lpwndpos);
}

void CCheckBox::Draw(CDIB &dib)
{
	CRect r;
	GetClientRect(&r);

	dib.Resize(r.Width(), r.Height());
	if(!dib.Ready())
	{
		return;
	}

	if(bckg->Ready())
	{
		dib.Draw(dib.Rect(), 0, 0, bckg, DrawFlagsPaint | DrawFlagsReflectSrc);
	}

	RectF rf(0.0f, 0.0f, (REAL)dib.Width(), (REAL)dib.Height());
	float radius = rf.Width;

	Graphics g(dib.dc);
	g.SetCompositingMode(CompositingModeSourceOver);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	GraphicsPath *path = new GraphicsPath();

	if(radio)
	{
		path->AddArc(rf.X + rf.Width - radius, rf.Y + rf.Height - radius, radius - 1, radius - 1, 0, 90);
		path->AddArc(rf.X, rf.Y + rf.Height - radius, radius - 1, radius - 1, 90, 90);
		path->AddArc(rf.X, rf.Y, radius - 1, radius - 1, 180, 90);
		path->AddArc(rf.X + rf.Width - radius, rf.Y, radius - 1, radius - 1, 270, 90);
		path->CloseFigure();
	}

	LinearGradientBrush lb(rf, 0xfffefefe, 0xffa9a9a9, LinearGradientModeVertical);
	if(!IsWindowEnabled())
	{
		lb.SetLinearColors(0xffcfcfcf, 0xff898989);
	}
	
	Pen pen(0xff4f4f4f);

	if(radio)
	{
		g.FillPath(&lb, path);
		g.DrawPath(&pen, path);
	}
	else
	{
		RectF rc = rf;
		rc.Width--;
		rc.Height--;
		g.FillRectangle(&lb, rc);
		g.DrawRectangle(&pen, rc);
	}

	if(checked)
	{
		if(radio)
		{
			rf.X += rf.Width * 0.26f;
			rf.Y += rf.Height * 0.26f;
			rf.Width *= 0.48f;
			rf.Height *= 0.48f;

			radius = rf.Width;

			path->Reset();
			path->AddArc(rf.X + rf.Width - radius, rf.Y + rf.Height - radius, radius - 1, radius - 1, 0, 90);
			path->AddArc(rf.X, rf.Y + rf.Height - radius, radius - 1, radius - 1, 90, 90);
			path->AddArc(rf.X, rf.Y, radius - 1, radius - 1, 180, 90);
			path->AddArc(rf.X + rf.Width - radius, rf.Y, radius - 1, radius - 1, 270, 90);
			path->CloseFigure();

			SolidBrush brush(0xff000000);
			g.FillPath(&brush, path);
		}
		else
		{
			path->Reset();
			
			/*PointF p[3];
			p[0].X = rf.X + rf.Width * 0.22f;
			p[0].Y = rf.Y + rf.Height * 0.48f;
			p[1].X = rf.X + rf.Width * 0.45f;
			p[1].Y = rf.Y + rf.Height * 0.66f;
			p[2].X = rf.X + rf.Width * 0.7f;
			p[2].Y = rf.Y + rf.Height * 0.26f;

			path->AddCurve(p, 3);*/

			#define scaleFactor 0.16f
			rf.X += rf.Width * scaleFactor * 1.1f;
			rf.Y += rf.Height * scaleFactor * 0.45f;
			rf.Width *= (1 - 2 * scaleFactor);
			rf.Height *= (1 - 2 * scaleFactor);

			path->AddLine(rf.X + rf.Width * 0.4f, rf.Y + rf.Height, rf.X + rf.Width * 0.9f, rf.Y);
			path->AddLine(rf.X + rf.Width * 0.9f, rf.Y, rf.X + rf.Width * 0.4f, rf.Y + rf.Height * 0.8f);
			path->AddLine(rf.X + rf.Width * 0.4f, rf.Y + rf.Height * 0.8f, rf.X, rf.Y + rf.Height * 0.6f);
			path->CloseFigure();

			Pen pen(0xff000000);
			SolidBrush brush(0xff000000);
			g.FillPath(&brush, path);
			g.DrawPath(&pen, path);
		}
	}
	delete path;
}

void CCheckBox::OnPaint()
{	
	CPaintDC dc(this);
	CDIB dib;
	Draw(dib);
	if(!dib.Ready())
	{
		return;
	}
	BitBlt(dc.m_hDC, 0, 0, dib.Width(), dib.Height(), dib.dc, 0, 0, SRCCOPY);
}

void CCheckBox::OnLButtonDown(UINT nFlags, CPoint point)
{
	CFrameWnd::OnLButtonDown(nFlags, point);

	CWnd *parent = GetParent();
	if(parent)
	{
		parent->SendMessage(WM_CONTROLNOTIFY, MAKEWPARAM(GetDlgCtrlID(), NULL));
	}
}

void CCheckBox::OnEnable(BOOL bEnable)
{
	CFrameWnd::OnEnable(bEnable);
	RedrawWindow();
}
