#include "pluginlist.h"

BEGIN_MESSAGE_MAP(CPluginList, CFrameWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_SHOWWINDOW()
	ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()

using namespace Gdiplus;

#define TIMER_SCROLL 1
#define ITEM_WIDTH 130
#define ITEM_HEIGHT 100
#define COL_COUNT 4
#define SEPARATOR_SIZE 10

CPluginList::CPluginList()
{
	mouseDown = false;
	scrollState = 0;
	scroll = 0;
	scrollMax = 0;
	scrollVisible = FALSE;
	scrolling = FALSE;
	scrollTop = new CDIB();
	scrollMiddle = new CDIB();
	scrollBottom = new CDIB();
	scrollButton = new CDIB();
	bckg = new CDIB();
	selected = NULL;
	hover = NULL;
	draging = false;
}

CPluginList::~CPluginList()
{
	RemoveAll();
	delete bckg;
	delete scrollTop;
	delete scrollMiddle;
	delete scrollBottom;
	delete scrollButton;
}

BOOL CPluginList::PreCreateWindow(CREATESTRUCT& cs)
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

CPluginListItem* CPluginList::Add()
{
	CPluginListItem *item = new CPluginListItem();
	items.AddTail(item);
	return item;
}
	
void CPluginList::RemoveAll()
{
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		delete items.GetAt(p);
		items.GetNext(p);
	}
	items.RemoveAll();
}

bool CPluginList::IsItemVisible(CPluginListItem *item)
{
	CRect rw;
	GetClientRect(&rw);
	CRect r = GetItemRect(item);
	if(scrollVisible)
	{
		rw.right -= scrollTop->Width();
	}
	return ((r.left < rw.right) && (r.top < rw.bottom) &&
		(r.right > rw.left) && (r.bottom > rw.top));
}

CRect CPluginList::GetItemRect(CPluginListItem *item)
{
	CRect r, rw;
	GetClientRect(&rw);

	int i = 0;
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		if(items.GetAt(p) == item)
		{
			break;
		}
		i++;
		items.GetNext(p);
	}

	r.left = rw.left + (i % COL_COUNT) * (SEPARATOR_SIZE + ITEM_WIDTH);
	r.top = rw.top + SEPARATOR_SIZE + (i / COL_COUNT) * (SEPARATOR_SIZE + ITEM_HEIGHT);
	r.right = r.left + ITEM_WIDTH;
	r.bottom = r.top + ITEM_HEIGHT;

	r.OffsetRect((rw.Width() - COL_COUNT * (SEPARATOR_SIZE + ITEM_WIDTH)) / 2, 0);

	if(scrollVisible)
	{
		r.OffsetRect(0, -scroll);
	}
	return r;
}

CPluginListItem* CPluginList::GetItemAt(CPoint point)
{
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		CPluginListItem *item = items.GetAt(p);
		if(IsItemVisible(item) && GetItemRect(item).PtInRect(point))
		{
			return item;
		}
		items.GetNext(p);
	}
	return NULL;
}

void CPluginList::AdjustSize(bool drawItems)
{
	CRect rc;
	GetClientRect(&rc);

	int count = items.GetCount() / COL_COUNT;
	if(items.GetCount() % COL_COUNT)
	{
		count++;
	}

	scrollMax = 0;
	scrollVisible = SEPARATOR_SIZE + count * (ITEM_HEIGHT + SEPARATOR_SIZE) > rc.Height();

	if(scrollVisible)
	{
		scrollMax = max(1, SEPARATOR_SIZE + count * (ITEM_HEIGHT + SEPARATOR_SIZE) - rc.Height());
	}
	ScrollTo(scroll);

	if(drawItems)
	{
		DrawItems();
	}
	RedrawWindow();
}

CRect CPluginList::ScrollBarRect()
{
	CRect r;
	CRect rw;
	GetClientRect(&rw);
	r.SetRect(rw.right - scrollTop->Width(), rw.top, rw.right, rw.bottom);
	return r;
}

void CPluginList::DrawScrollBar(Gdiplus::Graphics *g)
{
	if(!scrollVisible || !scrollTop->Ready() || !scrollMiddle->Ready() || 
		!scrollBottom->Ready() || !scrollButton->Ready())
	{
		return;
	}

	g->SetSmoothingMode(SmoothingModeNone);
	g->SetCompositingMode(CompositingModeSourceOver);
	//g->SetInterpolationMode(InterpolationModeHighQualityBicubic);
	
	CRect r = ScrollBarRect();
	CDIB tmp;

	int y = r.top;
	while(y < r.bottom)
	{
		g->DrawImage(scrollMiddle->bmp, RectF((REAL)r.left, (REAL)y, 
			(REAL)r.Width(), (REAL)scrollMiddle->Height()));
		y += scrollMiddle->Height();
	}

	int n = r.bottom - y;
	if(n > 0)
	{
		g->DrawImage(scrollMiddle->bmp, RectF((REAL)r.left, (REAL)y, 
			(REAL)r.Width(), (REAL)n),
			(REAL)0, (REAL)0, (REAL)scrollMiddle->Width(), (REAL)n,
			UnitPixel, NULL, NULL, NULL);
	}

	tmp.Assign(scrollTop);
	if((scrollState & ScrollBarButtonUpPressed) == ScrollBarButtonUpPressed)
	{
		DIB_ARGB *p = tmp.scan0;
		for(int i = 0; i < tmp.Height() * tmp.Width(); i++)
		{
			p->r = (unsigned char)(p->r * 0.8);
			p->g = (unsigned char)(p->g * 0.8);
			p->b = (unsigned char)(p->b * 0.8);
			p++;
		}
	}
	g->DrawImage(tmp.bmp, RectF((REAL)r.left, (REAL)r.top, 
		(REAL)r.Width(), (REAL)scrollTop->Height()));

	tmp.Assign(scrollBottom);
	if((scrollState & ScrollBarButtonDownPressed) == ScrollBarButtonDownPressed)
	{
		DIB_ARGB *p = tmp.scan0;
		for(int i = 0; i < tmp.Height() * tmp.Width(); i++)
		{
			p->r = (unsigned char)(p->r * 0.8);
			p->g = (unsigned char)(p->g * 0.8);
			p->b = (unsigned char)(p->b * 0.8);
			p++;
		}
	}
	g->DrawImage(tmp.bmp, RectF((REAL)r.left, (REAL)(r.bottom - scrollBottom->Height()),
		(REAL)r.Width(), (REAL)scrollBottom->Height()));

	r.OffsetRect(0, scrollTop->Height() + (r.Height() - scrollButton->Height() - scrollTop->Height() -
		scrollBottom->Height()) * scroll / scrollMax);
	r.bottom = r.top + scrollButton->Height();

	tmp.Assign(scrollButton);
	if((scrollState & ScrollBarButtonPressed) == ScrollBarButtonPressed)
	{
		DIB_ARGB *p = tmp.scan0;
		for(int i = 0; i < tmp.Height() * tmp.Width(); i++)
		{
			p->r = (unsigned char)(p->r * 0.8);
			p->g = (unsigned char)(p->g * 0.8);
			p->b = (unsigned char)(p->b * 0.8);
			p++;
		}
	}
	g->DrawImage(tmp.bmp, RectF((REAL)r.left, (REAL)r.top,
		(REAL)r.Width(), (REAL)r.Height()));
}

bool CPluginList::ScrollTo(int value)
{
	if(value < 0)
	{
		value = 0;
	}
	if(value > scrollMax)
	{
		value = scrollMax;
	}
	if(scroll != value)
	{
		scroll = value;
		RedrawWindow();
		return TRUE;
	}
	return FALSE;
}

bool CPluginList::ScrollUp()
{
	return ScrollTo(scroll - ITEM_HEIGHT / 2);
}

bool CPluginList::ScrollDown()
{
	return ScrollTo(scroll + ITEM_HEIGHT / 2);
}

void CPluginList::DrawItem(CPluginListItem *item)
{
	CRect r = GetItemRect(item);
	item->dib->Resize(r.Width(), r.Height());
	if(!item->dib->Ready())
	{
		return;
	}

	RectF rf((REAL)0, (REAL)0, (REAL)item->dib->Width(), (REAL)item->dib->Height());
	RectF rx;
	
	Graphics g(item->dib->bmp);
	g.SetCompositingMode(CompositingModeSourceOver);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	SolidBrush brush(0xffffffff);
	//g.FillRectangle(&brush, rf);

	if(item->icon->Ready())
	{
		rx = rf;
		rx.Height *= 0.66f;
		rx.Width = rx.Height;
		rx.X = (rf.Width - rx.Width) / 2;

		float k = min(rx.Width / item->icon->Width(), rx.Height / item->icon->Height());

		RectF ri;
		ri.Width = item->icon->Width() * k;
		ri.Height = item->icon->Height() * k;
		ri.X = rx.X + (rx.Width - ri.Width) / 2;
		ri.Y = rx.Y + (rx.Height - ri.Height) / 2;

		g.DrawImage(item->icon->bmp, ri);
	}

	rx = rf;
	rx.Y += rx.Height * 0.68f;
	rx.Height *= 0.32f;

	if(item->description.IsEmpty())
	{
		rx.Height *= 0.5f;
	}

	const float radius = rx.Height * 0.5f;

	GraphicsPath *path = new GraphicsPath();
	path->AddArc(rx.X + rx.Width - radius, rx.Y, radius - 1, radius - 1, 270, 90);
	path->AddArc(rx.X + rx.Width - radius, rx.Y + rx.Height - radius, radius - 1, radius - 1, 0, 90);
	path->AddArc(rx.X, rx.Y + rx.Height - radius, radius - 1, radius - 1, 90, 90);
	path->AddArc(rx.X, rx.Y, radius - 1, radius - 1, 180, 90);
	path->CloseFigure();

	LinearGradientBrush lb(rx, 0xff6fa6de, 0xff1e6cbb, LinearGradientModeVertical);
	g.FillPath(&lb, path);

	delete path;

	rx.Height = rf.Height * 0.16f;

	Font fontText(L"Arial", 9.0f);
	Font fontDescription(L"Arial", 8.0f);
	StringFormat *stringFormat = new StringFormat();
	stringFormat->SetAlignment(StringAlignmentCenter);
	stringFormat->SetLineAlignment(StringAlignmentCenter);
	stringFormat->SetTrimming(StringTrimmingEllipsisCharacter);
	stringFormat->SetFormatFlags(StringFormatFlagsLineLimit);

	brush.SetColor(0xff1e6cbb);
	rx.Y++;
	g.DrawString(item->text.GetBuffer(), item->text.GetLength(), &fontText, rx, stringFormat, &brush);
		
	brush.SetColor(0xffffffff);
	rx.Y--;
	g.DrawString(item->text.GetBuffer(), item->text.GetLength(), &fontText, rx, stringFormat, &brush);

	rx.Y += rx.Height;
	rx.Height = rf.Height * 0.14f;

	brush.SetColor(0xff1e6cbb);
	rx.Y++;
	g.DrawString(item->description.GetBuffer(), item->description.GetLength(), &fontDescription, rx, stringFormat, &brush);

	brush.SetColor(0xffffffff);
	rx.Y--;
	g.DrawString(item->description.GetBuffer(), item->description.GetLength(), &fontDescription, rx, stringFormat, &brush);

	delete stringFormat;
}
	
void CPluginList::DrawItems()
{
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		CPluginListItem *item = items.GetAt(p);
		DrawItem(item);
		items.GetNext(p);
	}
}

void CPluginList::UpdateItem(Gdiplus::Graphics *g, CPluginListItem *item)
{
	if(item->dib->Ready() && IsItemVisible(item))
	{
		CRect r = GetItemRect(item);
		g->DrawImage(item->dib->bmp, (REAL)r.left, (REAL)r.top, (REAL)r.Width(), (REAL)r.Height());
	}
}

void CPluginList::UpdateItems(Gdiplus::Graphics *g)
{
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		CPluginListItem *item = items.GetAt(p);
		UpdateItem(g, item);
		items.GetNext(p);
	}
}

int CPluginList::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CFrameWnd::OnCreate(lpCreateStruct);

	userDropTargetHelper = SUCCEEDED(CoCreateInstance(CLSID_DragDropHelper, NULL, 
		CLSCTX_INPROC_SERVER, IID_IDropTargetHelper, (void**)&dropTargetHelper));

	Register(this);

	return 0;
}

BOOL CPluginList::OnEraseBkgnd(CDC*)
{
	return TRUE;
}

void CPluginList::Draw(CDIB &dib)
{
	CRect r;
	GetClientRect(&r);

	dib.Resize(r.Width(), r.Height());
	if(!dib.Ready())
	{
		return;
	}

	RectF rf(0, 0, (REAL)dib.Width(), (REAL)dib.Height());
	RectF rx;

	Graphics g(dib.dc);
	g.SetCompositingMode(CompositingModeSourceOver);

	if(bckg->Ready())
	{
		dib.Draw(dib.Rect(), 0, 0, bckg, DrawFlagsReflectDest);
	}

	UpdateItems(&g);

	if(bckg->Ready())
	{
		for(int y = 0; y < 40; y++)
		{
			DIB_AARGB *pd = (DIB_AARGB*)dib.Pixels(0, y);
			DIB_AARGB *ps = (DIB_AARGB*)bckg->Pixels(0, bckg->Height() - 1 - y);
			float k = (float)y / 40;
			for(int x = 0; x < dib.Width(); x++)
			{
				pd[x]->r = (unsigned char)(pd[x]->r * k + ps[x]->r * (1 - k));
				pd[x]->g = (unsigned char)(pd[x]->g * k + ps[x]->g * (1 - k));
				pd[x]->b = (unsigned char)(pd[x]->b * k + ps[x]->b * (1 - k));
			}
			pd = (DIB_AARGB*)dib.Pixels(0, dib.Height() - 1 - y);
			ps = (DIB_AARGB*)bckg->Pixels(0, y);
			for(int x = 0; x < dib.Width(); x++)
			{
				pd[x]->r = (unsigned char)(pd[x]->r * k + ps[x]->r * (1 - k));
				pd[x]->g = (unsigned char)(pd[x]->g * k + ps[x]->g * (1 - k));
				pd[x]->b = (unsigned char)(pd[x]->b * k + ps[x]->b * (1 - k));
			}
		}
	}

	DrawScrollBar(&g);
}

void CPluginList::OnPaint()
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

void CPluginList::OnTimer(UINT_PTR nIDEvent)
{
	CFrameWnd::OnTimer(nIDEvent);

	switch(nIDEvent)
	{
	case TIMER_SCROLL:
		{
			if((scrollState & ScrollBarButtonUpPressed) == ScrollBarButtonUpPressed)
			{
				if(!ScrollUp())
				{
					KillTimer(nIDEvent);
				}
			}
			if((scrollState & ScrollBarButtonDownPressed) == ScrollBarButtonDownPressed)
			{
				if(!ScrollDown())
				{
					KillTimer(nIDEvent);
				}
			}
		}
		break;
	}
}

void CPluginList::OnLButtonDown(UINT nFlags, CPoint point)
{
	CFrameWnd::OnLButtonDown(nFlags, point);
	SetFocus();
	SetCapture();

	mouseDown = true;
	downPt = point;
	scrolling = false;

	if(scrollVisible)
	{
		CRect r = ScrollBarRect();
		if(r.PtInRect(point))
		{
			// Scroll Up
			if(point.y < r.top + scrollTop->Height())
			{
				scrollState |= ScrollBarButtonUpPressed;
				RedrawWindow();
				if(ScrollUp())
				{
					SetTimer(TIMER_SCROLL, 100, NULL);
				}
			}
			else
			// Scroll Down
			if(point.y > r.bottom - scrollBottom->Height())
			{
				scrollState |= ScrollBarButtonDownPressed;
				RedrawWindow();
				if(ScrollDown())
				{
					SetTimer(TIMER_SCROLL, 100, NULL);
				}
			}
			else
			// Scroll Button
			{
				int scrollBarHeight = r.Height();
				r.OffsetRect(0, scrollTop->Height() + (scrollBarHeight - scrollButton->Height() - 
					scrollTop->Height() - scrollBottom->Height()) * scroll / scrollMax);
				r.bottom = r.top + scrollButton->Height();
				
				scrolling = (r.PtInRect(downPt) == TRUE);
				if(scrolling)
				{
					scrollState |= ScrollBarButtonPressed;
					RedrawWindow();

					downPt.y -= (scrollBarHeight - scrollButton->Height() -
						scrollTop->Height() - scrollBottom->Height()) * scroll / scrollMax;
				}
			}
			return;
		}
	}

	if(!scrolling)
	{
		selected = GetItemAt(point);
	}
}

void CPluginList::OnLButtonUp(UINT nFlags, CPoint point)
{
	CFrameWnd::OnLButtonUp(nFlags, point);
	mouseDown = false;
	scrolling = false;
	ReleaseCapture();

	if((scrollState & ScrollBarButtonPressed) == ScrollBarButtonPressed)
	{
		scrollState &= ~ScrollBarButtonPressed;
		RedrawWindow();
	}
	if((scrollState & ScrollBarButtonUpPressed) == ScrollBarButtonUpPressed)
	{
		KillTimer(TIMER_SCROLL);
		scrollState &= ~ScrollBarButtonUpPressed;
		RedrawWindow();
	}
	if((scrollState & ScrollBarButtonDownPressed) == ScrollBarButtonDownPressed)
	{
		KillTimer(TIMER_SCROLL);
		scrollState &= ~ScrollBarButtonDownPressed;
		RedrawWindow();
	}
}

void CPluginList::OnMouseLeave()
{
	CFrameWnd::OnMouseLeave();
}

void CPluginList::OnMouseMove(UINT nFlags, CPoint point)
{
	CFrameWnd::OnMouseMove(nFlags, point);

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE;
	tme.hwndTrack = CFrameWnd::m_hWnd;
	tme.dwHoverTime = HOVER_DEFAULT;
	TrackMouseEvent(&tme);

	if(mouseDown && scrolling)
	{
		CRect r = ScrollBarRect();
		ScrollTo((point.y - downPt.y) * scrollMax / (r.Height() - scrollButton->Height() - 
			scrollTop->Height() - scrollBottom->Height()));
	}
	else
	if(mouseDown && selected)
	{
		if(!draging && (abs(downPt.x - point.x) > 8) || (abs(downPt.y - point.y) > 8))
		{
			mouseDown = false;
			scrolling = false;
			ReleaseCapture();
			draging = true;
			DragItem(selected, point);
			draging = false;
		}
	}
}

BOOL CPluginList::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CFrameWnd::OnMouseWheel(nFlags, zDelta, pt);
	if(scrollVisible)
	{
		if(zDelta > 0)
		{
			return ScrollUp();
		}
		if(zDelta < 0)
		{
			return ScrollDown();
		}
	}
	return FALSE;
}

void CPluginList::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CFrameWnd::OnShowWindow(bShow, nStatus);
	AdjustSize();
}

void CPluginList::OnWindowPosChanged(WINDOWPOS *lpwndpos)
{
	CFrameWnd::OnWindowPosChanged(lpwndpos);
	if(!IsWindowVisible() || ((lpwndpos->flags & SWP_NOSIZE) == SWP_NOSIZE))
	{
		return;
	}
	AdjustSize();
}

void CPluginList::DragItem(CPluginListItem *item, CPoint point)
{
	HGLOBAL hMem = GlobalAlloc(GHND | GMEM_DDESHARE, sizeof(PluginListDrop));
	if(hMem == 0)
	{
		return;
	}

	PluginListDrop *data = (PluginListDrop*)GlobalLock(hMem);
	if(!data)
	{
		GlobalFree(hMem);
		return;
	}

	data->plugin = item->data;

	GlobalUnlock(hMem);

	COleDataSourceEx dropData;
	dropData.CacheGlobalData(CF_PLUGINLIST, hMem);
	CComPtr<IDragSourceHelper> dragSourceHelper;

	HRESULT hRes = CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_ALL, IID_IDragSourceHelper, (void**)&dragSourceHelper);
	if(SUCCEEDED(hRes))
	{
		CDIB dib;
		dib.Assign(item->dib);
		dib.ReflectVertical();

		HBITMAP bmp = CreateCompatibleBitmap(dib.dc, dib.Width(), dib.Height());
		HDC dc = CreateCompatibleDC(dib.dc);
		HBITMAP oldBmp = (HBITMAP)SelectObject(dc, bmp);
		BitBlt(dc, 0, 0, dib.Width(), dib.Height(), dib.dc, 0, 0, SRCCOPY);
		SelectObject(dc, oldBmp);

		CRect rci = GetItemRect(item);
		point.x -= rci.left;
		point.y -= rci.top;

		SHDRAGIMAGE info;
		info.sizeDragImage.cx = dib.Width();
		info.sizeDragImage.cy = dib.Height();
		info.ptOffset = point;
		info.hbmpDragImage = bmp;
		info.crColorKey = 0xa9b8c7;

		hRes = dragSourceHelper->InitializeFromBitmap(&info, (IDataObject*)dropData.GetInterface(&IID_IDataObject));
		if(FAILED(hRes))
		{
			DeleteObject(info.hbmpDragImage);
		}
	}
	DROPEFFECT dropEffect = dropData.DoDragDrop(DROPEFFECT_MOVE);
	switch(dropEffect)
	{
	case DROPEFFECT_NONE:
		{
			GlobalFree(hMem);
		}
		break;
	}
}

DROPEFFECT CPluginList::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD, CPoint point)
{
	DROPEFFECT dwEffect = DROPEFFECT_NONE;
	if(userDropTargetHelper)
	{
		IDataObject *dataObject = pDataObject->GetIDataObject(FALSE);
		dropTargetHelper->DragEnter(pWnd->GetSafeHwnd(), dataObject, &point, dwEffect);
	}
	return dwEffect;
}
	
DROPEFFECT CPluginList::OnDragOver(CWnd*, COleDataObject*, DWORD, CPoint point)
{
	DROPEFFECT dwEffect = DROPEFFECT_NONE;
	if(userDropTargetHelper)
	{
		dropTargetHelper->DragOver(&point, dwEffect);
	}
	return dwEffect;
}

BOOL CPluginList::OnDrop(CWnd*, COleDataObject*, DROPEFFECT, CPoint)
{
	return FALSE;
}
	
void CPluginList::OnDragLeave(CWnd*)
{
	if(userDropTargetHelper)
	{
		dropTargetHelper->DragLeave();
	}
}