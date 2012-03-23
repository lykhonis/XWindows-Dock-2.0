#include "skinlist.h"

BEGIN_MESSAGE_MAP(CSkinList, CFrameWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_WINDOWPOSCHANGED()
END_MESSAGE_MAP()

using namespace Gdiplus;

CSkinList::CSkinList()
{
	bckg = new CDIB();
	dib = new CDIB();
	skin = NULL;
	loader = new CSkinLoader();
	preferMode = DockMode3D;
}

CSkinList::~CSkinList()
{
	delete dib;
	delete bckg;
	delete loader;
}

BOOL CSkinList::PreCreateWindow(CREATESTRUCT& cs)
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

BOOL CSkinList::OnEraseBkgnd(CDC*)
{
	return FALSE;
}

void CSkinList::Draw()
{
	if(!loader->items.Find(skin) || !skin->Load())
	{
		return;
	}
	CRect r;
	GetClientRect(&r);

	dib->Resize(r.Width(), (int)(r.Height() * 0.64f));
	if(!dib->Ready())
	{
		return;
	}

	if((skin->mode & DockMode3D) && !((preferMode != DockMode3D) && (skin->mode & DockMode2D)))
	{
		CRect rect;
		rect.left = 0;
		rect.top = skin->bckgEdge3d->Height();
		rect.right = dib->Width();
		rect.bottom = r.Height();
		rect.bottom -= (int)(rect.Height() * sin(PI * (30 + 10.0f * skin->iconPosition3d / 100) / 180));

		Dock3D::DrawImage(dib, rect, (int)(rect.Height() * sin(PI * skin->bckgEdgeAngle3d / 180)), skin->bckg3d);

		dib->ReflectVertical();
			
		if(skin->bckgEdge3d->Ready())
		{
			dib->DrawTail(CRect(1, dib->Height() - skin->bckgEdge3d->Height(), dib->Width() - 1, dib->Height()), 
				skin->bckgEdge3d->Rect(), skin->bckgEdge3d, DrawFlagsPaint);
		}

		dib->AlphaBlend(dib->Rect(), (unsigned char)(255 * skin->bckgOpacity3d / 100), DrawFlagsPaint);
	}
	else
	if(skin->mode & DockMode2D)
	{
		dib->Draw(CRect(
			0,
			0,
			skin->bckgLeftTop2d->Width(),
			skin->bckgLeftTop2d->Height()),
			0, 0, skin->bckgLeftTop2d, DrawFlagsPaint);

		dib->DrawTail(CRect(
			skin->bckgLeftTop2d->Width(), 
			0,
			dib->Width() - skin->bckgRightTop2d->Width(),
			skin->bckgTop2d->Height()),
			skin->bckgTop2d->Rect(), skin->bckgTop2d, DrawFlagsPaint);

		dib->Draw(CRect(
			dib->Width() - skin->bckgLeftTop2d->Width(),
			0,
			dib->Width(),
			skin->bckgRightTop2d->Height()),
			0, 0, skin->bckgRightTop2d, DrawFlagsPaint);

		dib->DrawTail(CRect(
			dib->Width() - skin->bckgRight2d->Width(), 
			skin->bckgRightTop2d->Height(),
			dib->Width(),
			dib->Height() - skin->bckgRightBottom2d->Height()),
			skin->bckgRight2d->Rect(), skin->bckgRight2d, DrawFlagsPaint);

		dib->Draw(CRect(
			dib->Width() - skin->bckgRightBottom2d->Width(),
			dib->Height() - skin->bckgRightTop2d->Height(),
			dib->Width(),
			dib->Height()),
			0, 0, skin->bckgRightBottom2d, DrawFlagsPaint);

		dib->DrawTail(CRect(
			skin->bckgLeftBottom2d->Width(), 
			dib->Height() - skin->bckgBottom2d->Height(),
			dib->Width() - skin->bckgRightBottom2d->Width(),
			dib->Height()),
			skin->bckgBottom2d->Rect(), skin->bckgBottom2d, DrawFlagsPaint);

		dib->Draw(CRect(
			0,
			dib->Height() - skin->bckgLeftBottom2d->Height(),
			skin->bckgLeftBottom2d->Width(),
			dib->Height()),
			0, 0, skin->bckgLeftBottom2d, DrawFlagsPaint);

		dib->DrawTail(CRect(
			0, 
			skin->bckgLeftTop2d->Height(),
			skin->bckgLeft2d->Width(),
			dib->Height() - skin->bckgLeftBottom2d->Height()),
			skin->bckgLeft2d->Rect(), skin->bckgLeft2d, DrawFlagsPaint);

		dib->DrawTail(CRect(
			skin->bckgLeft2d->Width(), 
			skin->bckgTop2d->Height(),
			dib->Width() - skin->bckgRight2d->Width(),
			dib->Height() - skin->bckgBottom2d->Height()),
			skin->bckgMiddle2d->Rect(), skin->bckgMiddle2d, DrawFlagsPaint);

		dib->AlphaBlend(dib->Rect(), (unsigned char)(255 * skin->bckgOpacity2d / 100), DrawFlagsPaint);
	}
}

void CSkinList::Draw(CDIB &tmp)
{
	CRect r;
	GetClientRect(&r);

	tmp.Resize(r.Width(), r.Height());
	if(!tmp.Ready())
	{
		return;
	}

	RectF rf(0, 0, (REAL)tmp.Width(), (REAL)tmp.Height());
	RectF rx;

	Graphics g(tmp.dc);
	g.SetCompositingMode(CompositingModeSourceOver);

	if(bckg->Ready())
	{
		tmp.Draw(tmp.Rect(), 0, 0, bckg, DrawFlagsReflectDest);
	}

	if(dib->Ready())
	{
		g.DrawImage(dib->bmp, 0.0f, r.Height() * 0.36f);

		Font font(L"Arial", 9.0f);
		StringFormat *stringFormat = new StringFormat();
		stringFormat->SetAlignment(StringAlignmentCenter);
		stringFormat->SetLineAlignment(StringAlignmentCenter);
		stringFormat->SetTrimming(StringTrimmingEllipsisCharacter);

		CString s;
		s.Format(L"%s\n%s", skin->name.GetBuffer(), skin->description.GetBuffer());
		
		RectF rx(0, 0, (REAL)dib->Width(), r.Height() * 0.36f);

		rx.Y++;
		SolidBrush brush(0xfff0f0f0);
		g.DrawString(s.GetBuffer(), s.GetLength(), &font, rx, stringFormat, &brush);

		rx.Y--;
		brush.SetColor(0xff000000);
		g.DrawString(s.GetBuffer(), s.GetLength(), &font, rx, stringFormat, &brush);

		delete stringFormat;
	}
}

void CSkinList::OnPaint()
{	
	CPaintDC dc(this);
	CDIB tmp;
	Draw(tmp);
	if(!tmp.Ready())
	{
		return;
	}
	BitBlt(dc.m_hDC, 0, 0, tmp.Width(), tmp.Height(), tmp.dc, 0, 0, SRCCOPY);
}

void CSkinList::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CFrameWnd::OnShowWindow(bShow, nStatus);
	Draw();
	RedrawWindow();
}

void CSkinList::OnWindowPosChanged(WINDOWPOS *lpwndpos)
{
	CFrameWnd::OnWindowPosChanged(lpwndpos);
	if(!IsWindowVisible() || ((lpwndpos->flags & SWP_NOSIZE) == SWP_NOSIZE))
	{
		return;
	}
	Draw();
	RedrawWindow();
}
