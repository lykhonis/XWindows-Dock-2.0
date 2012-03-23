#include "TrackBar.h"

BEGIN_MESSAGE_MAP(CTrackBar, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
END_MESSAGE_MAP()

using namespace Gdiplus;

CTrackBar::CTrackBar()
{
	min = 0;
	max = 100;
	position = 0;
	tracking = FALSE;
	state = 0;
	button = new CDIB();
	bckg = new CDIB();
	bckgTrack = new CDIB();
}

CTrackBar::~CTrackBar()
{
	delete button;
	delete bckg;
	delete bckgTrack;
}

BOOL CTrackBar::PreCreateWindow(CREATESTRUCT& cs)
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

int CTrackBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	return CFrameWnd::OnCreate(lpCreateStruct);
}

void CTrackBar::OnDestroy()
{
	CFrameWnd::OnDestroy();
}

bool CTrackBar::SetPosition(int value)
{
	if(position != value)
	{
		if(value < min)
		{
			value = min;
		}
		if(value > max)
		{
			value = max;
		}
		if(position != value)
		{
			position = value;
			RedrawWindow();
			return true;
		}
	}
	return false;
}

BOOL CTrackBar::OnEraseBkgnd(CDC*)
{
	return TRUE;
}

CRect CTrackBar::GetTrackRect()
{
	CRect r;
	GetClientRect(&r);
	r.left += (r.Width() - r.Height()) * (position - min) / (max == min ? 1 : max - min);
	r.right = r.left + r.Height();
	return r;
}

void CTrackBar::Draw(CDIB &dib)
{
	CRect rc;
	GetClientRect(&rc);

	dib.Resize(rc.Width(), rc.Height());
	if(!dib.Ready())
	{
		return;
	}

	Graphics g(dib.dc);
	g.SetSmoothingMode(SmoothingModeNone);
	g.SetCompositingMode(CompositingModeSourceCopy);

	RectF rf(0.0f, 0.0f, (REAL)rc.Width(), (REAL)rc.Height());

	if(bckg->Ready())
	{
		dib.Draw(dib.Rect(), 0, 0, bckg, DrawFlagsPaint | DrawFlagsReflectSrc);
	}

	g.SetCompositingMode(CompositingModeSourceOver);

	if(bckgTrack->Ready())
	{
		RectF rt(rf.X,
			rf.Y + (rf.Height - bckgTrack->Height()) / 2,
			rf.Width,
			(REAL)bckgTrack->Height());

		g.DrawImage(bckgTrack->bmp, rt);
	}

	if(button->Ready() && (position >= min) && (position <= max) && (min < max))
	{
		CDIB tmp;
		tmp.Assign(button);

		if(!IsWindowEnabled())
		{
			DIB_ARGB *p = tmp.scan0;
			int size = tmp.Height() * tmp.Width();
			for(int i = 0; i < size; i++)
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
			int size = tmp.Height() * tmp.Width();
			for(int i = 0; i < size; i++)
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
			int size = tmp.Height() * tmp.Width();
			for(int i = 0; i < size; i++)
			{
				p->r = (unsigned char)(min(255, p->r * 1.1f));
				p->g = (unsigned char)(min(255, p->g * 1.1f));
				p->b = (unsigned char)(min(255, p->b * 1.1f));
				p++;
			}
		}

		RectF rt(rf.X + (rf.Width - tmp.Height()) * (position - min) / (max - min),
			rf.Y + (rf.Height - tmp.Height()) / 2,
			(REAL)tmp.Width(), 
			(REAL)tmp.Height());

		g.DrawImage(tmp.bmp, rt);
	}
}

void CTrackBar::OnPaint()
{
	CPaintDC dc(this);
	CDIB dib;
	Draw(dib);
	if(!dib.Ready())
	{
		return;
	}
	::BitBlt(dc.m_hDC, 0, 0, dib.Width(), dib.Height(), dib.dc, 0, 0, SRCCOPY);
}

void CTrackBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	CFrameWnd::OnLButtonDown(nFlags, point);
	SetCapture();
	SetFocus();

	CRect r;
	GetClientRect(&r);
	if(r.Width() == r.Height())
	{
		return;
	}

	int x = point.x - r.Height() / 2;
	if(x < 0)
	{
		x = 0;
	}

	tracking = TRUE;

	state |= ButtonPressed;
	RedrawWindow();

	CWnd *parent = GetParent();
	if(parent)
	{
		parent->PostMessage(WM_CONTROLNOTIFY, MAKEWPARAM(GetDlgCtrlID(), 0), 
			(LPARAM)(x * (max - min) / (r.Width() - r.Height()) + min));
	}

	/*if(SetPosition(x * (max - min) / (r.Width() - r.Height()) + min))
	{
		KillTimer(1);
		SetTimer(1, 100, NULL);
	}
	else
	{
		RedrawWindow();
	}*/
}

void CTrackBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	CFrameWnd::OnLButtonUp(nFlags, point);
	ReleaseCapture();

	tracking = false;
	state &= ~ButtonPressed;
	RedrawWindow();
}

void CTrackBar::OnTimer(UINT_PTR nIDEvent)
{
	CFrameWnd::OnTimer(nIDEvent);

	/*if(nIDEvent == 1)
	{
		KillTimer(1);

		CWnd *parent = GetParent();
		if(parent)
		{
			parent->PostMessage(WM_CONTROLNOTIFY, MAKEWPARAM(GetDlgCtrlID(), 0));
		}
	}*/
}

void CTrackBar::OnMouseMove(UINT nFlags, CPoint point)
{
	CFrameWnd::OnMouseMove(nFlags, point);

	if(tracking)
	{
		CRect r;
		GetClientRect(&r);
		if(r.Width() == r.Height())
		{
			return;
		}

		int x = point.x - r.Height() / 2;
		if(x < 0)
		{
			x = 0;
		}

		/*if(SetPosition(x * (max - min) / (r.Width() - r.Height()) + min))
		{
			//KillTimer(1);
			//SetTimer(1, 100, NULL);
			
			CWnd *parent = GetParent();
			if(parent)
			{
				parent->PostMessage(WM_CONTROLNOTIFY, MAKEWPARAM(GetDlgCtrlID(), 0));
			}
		}*/

		CWnd *parent = GetParent();
		if(parent)
		{
			parent->PostMessage(WM_CONTROLNOTIFY, MAKEWPARAM(GetDlgCtrlID(), 0), 
				(LPARAM)(x * (max - min) / (r.Width() - r.Height()) + min));
		}
	}
}
