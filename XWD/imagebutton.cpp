#include "imagebutton.h"

BEGIN_MESSAGE_MAP(CImageButton, CFrameWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_ENABLE()
END_MESSAGE_MAP()

using namespace Gdiplus;

CImageButton::CImageButton()
{
	state = 0;
	down = FALSE;
	checked = FALSE;
	color1 = 0xffffffff;
	color2 = 0xffffffff;
	textColor = 0xff333333;
	pressed = FALSE;
	image = new CDIB();
}

CImageButton::~CImageButton()
{
	delete image;
}

void CImageButton::SetPressed(bool value)
{
	if(checked && (pressed != value))
	{
		pressed = value;
		if(pressed)
		{
			state |= ButtonPressed;
		}
		else
		{
			state &= ~ButtonPressed;
		}
		RedrawWindow();
	}
}

BOOL CImageButton::PreCreateWindow(CREATESTRUCT& cs)
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

BOOL CImageButton::OnEraseBkgnd(CDC*)
{
	return TRUE;
}

void CImageButton::Draw(CDIB &dib)
{
	CRect r;
	GetClientRect(&r);

	dib.Resize(r.Width(), r.Height());
	if(!dib.Ready())
	{
		return;
	}

	Graphics g(dib.dc);
	g.SetCompositingMode(CompositingModeSourceCopy);

	if(color1 == color2)
	{
		SolidBrush brush(color1);
		g.FillRectangle(&brush, 0.0f, 0.0f, (REAL)dib.Width(), (REAL)dib.Height());
	}
	else
	{
		RectF rf(0.0f, 0.0f, (REAL)dib.Width(), (REAL)dib.Height());
		LinearGradientBrush brush(rf, color1, color2, LinearGradientModeVertical);
		g.FillRectangle(&brush, rf);
	}

	g.SetCompositingMode(CompositingModeSourceOver);
	//g.SetTextRenderingHint(TextRenderingHintAntiAlias);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	if(image->Ready())
	{
		CDIB tmp;
		tmp.Assign(image);

		if(!IsWindowEnabled())
		{
			DIB_ARGB *p = tmp.scan0;
			for(int i = 0; i < tmp.Height() * tmp.Width(); i++)
			{
				p->r = (unsigned char)(p->r * 0.75f);
				p->g = (unsigned char)(p->g * 0.75f);
				p->b = (unsigned char)(p->b * 0.75f);
				p++;
			}
		}
		else
		if(((state & ButtonPressed) == ButtonPressed))
		{
			DIB_ARGB *p = tmp.scan0;
			for(int i = 0; i < tmp.Height() * tmp.Width(); i++)
			{
				p->r = (unsigned char)(p->r * 0.85f);
				p->g = (unsigned char)(p->g * 0.85f);
				p->b = (unsigned char)(p->b * 0.85f);
				p++;
			}
		}
		else
		if((state & ButtonHover) == ButtonHover)
		{
			DIB_ARGB *p = tmp.scan0;
			for(int i = 0; i < tmp.Height() * tmp.Width(); i++)
			{
				p->r = (unsigned char)(min(255, p->r * 1.1f));
				p->g = (unsigned char)(min(255, p->g * 1.1f));
				p->b = (unsigned char)(min(255, p->b * 1.1f));
				p++;
			}
		}
		g.DrawImage(tmp.bmp, 0.0f, 0.0f, (REAL)dib.Width(), (REAL)dib.Height());
	}

	Font font(L"Arial", 9);
	StringFormat *stringFormat = new StringFormat();
	stringFormat->SetAlignment(StringAlignmentCenter);
	stringFormat->SetLineAlignment(StringAlignmentCenter);
	stringFormat->SetTrimming(StringTrimmingEllipsisCharacter);
	stringFormat->SetFormatFlags(StringFormatFlagsLineLimit);

	SolidBrush brush(0xffe0e0e0);

	CString s;
	GetWindowText(s);

	RectF rf(0.0f, 0.0f, (REAL)dib.Width(), (REAL)dib.Height());

	rf.Y++;
	g.DrawString(s.GetBuffer(), s.GetLength(), &font, rf, stringFormat, &brush);

	rf.Y--;
	brush.SetColor(textColor);
	g.DrawString(s.GetBuffer(), s.GetLength(), &font, rf, stringFormat, &brush);

	delete stringFormat;
}

void CImageButton::OnPaint()
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

void CImageButton::OnLButtonDown(UINT nFlags, CPoint point)
{
	CFrameWnd::OnLButtonDown(nFlags, point);

	SetFocus();
	SetCapture();

	down = TRUE;

	if(!(checked && pressed))
	{
		state |= ButtonPressed;
		RedrawWindow();
	}
}
	
void CImageButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	CFrameWnd::OnLButtonUp(nFlags, point);

	ReleaseCapture();

	if(!(checked && pressed))
	{
		state &= ~ButtonPressed;
		RedrawWindow();
	}

	if(down)
	{
		down = FALSE;
		ClientToScreen(&point);
		if(WindowFromPoint(point) == this)
		{
			Click();
		}
	}
}

void CImageButton::OnMouseLeave()
{
	CFrameWnd::OnMouseLeave();

	state &= ~ButtonHover;
	RedrawWindow();
}

void CImageButton::OnMouseMove(UINT nFlags, CPoint point)
{
	CFrameWnd::OnMouseMove(nFlags, point);
	
	if((state & ButtonHover) == 0)
	{
		state |= ButtonHover;
		RedrawWindow();
	}
	
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE;
	tme.hwndTrack = m_hWnd;
	tme.dwHoverTime = HOVER_DEFAULT;
	TrackMouseEvent(&tme);
}

void CImageButton::OnEnable(BOOL bEnable)
{
	CFrameWnd::OnEnable(bEnable);
	RedrawWindow();
}

void CImageButton::Click()
{
	CWnd *parent = GetParent();
	if(parent)
	{
		parent->SendMessage(WM_CONTROLNOTIFY, MAKEWPARAM(GetDlgCtrlID(), NULL));
	}
}