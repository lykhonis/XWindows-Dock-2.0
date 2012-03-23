#include "menu.h"

BEGIN_MESSAGE_MAP(CLMenu, CFrameWnd)
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

#define SHADOW_SIZE 9
#define BORDER_SIZE 2
#define ARROW_WIDTH 20
#define ARROW_HEIGHT 10
#define ARROW_HEIGHT_CHECK(a) (a ? 0 : ARROW_HEIGHT)
#define BCKG_OPACITY 90
#define ITEM_ADDITION 24
#define ITEM_HEIGHT 22
#define RADIUS 10
#define FONT_SIZE 10.0f
#define TIMER_SHOW 2
#define TIMER_SHOW_DELAY 150

using namespace Gdiplus;

CLMenuItem::CLMenuItem()
{
	dib = new CDIB();
	icon = new CDIB();
	nID = NULL;
	visible = TRUE;
	enabled = TRUE;
	selected = false;
	checkbox = false;
	checked = false;
	isLine = false;
	hWnd = NULL;
	AliasId = NULL;
	AliasParentId = NULL;
	menu = new CLMenu();
}

CLMenuItem::~CLMenuItem()
{
	delete dib;
	delete icon;
}

CLMenu::CLMenu()
{
	dib = new CDIB();
	bckg = new CDIB();
	parent = NULL;
	selected = NULL;
	childMenu = NULL;
	wndParent = NULL;
	general = this;
	tmp = NULL;
}

CLMenu::~CLMenu()
{
	RemoveAll();
	delete dib;
	delete bckg;
}

void CLMenu::EnableItem(CLMenuItem *item, bool bEnabled)
{
	if(item->enabled != bEnabled)
	{
		item->enabled = bEnabled;
	}
}

void CLMenu::VisibleItem(CLMenuItem *item, bool bVisible)
{
	if(item->visible != bVisible)
	{
		item->visible = bVisible;
	}
}

void CLMenu::CheckedItem(CLMenuItem *item, bool bChecked)
{
	if(item->checked != bChecked)
	{
		item->checked = bChecked;
	}
}

CLMenuItem *CLMenu::GetItem(int nID)
{
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		CLMenuItem *item = items.GetAt(p);
		if(!item->isLine && (item->nID == nID))
		{
			return item;
		}
		item = item->menu->GetItem(nID);
		if(item)
		{
			return item;
		}
		items.GetNext(p);
	}
	return NULL;
}

CLMenuItem *CLMenu::GetItemByAlias(int AliasId)
{
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		CLMenuItem *item = items.GetAt(p);
		if(!item->isLine && (item->AliasId == AliasId))
		{
			return item;
		}
		item = item->menu->GetItemByAlias(AliasId);
		if(item)
		{
			return item;
		}
		items.GetNext(p);
	}
	return NULL;
}

void CLMenu::EnableItem(int nID, bool bEnabled)
{
	CLMenuItem *item = general->GetItem(nID);
	if(item)
	{
		EnableItem(item, bEnabled);
	}
}

void CLMenu::VisibleItem(int nID, bool bVisible)
{
	CLMenuItem *item = general->GetItem(nID);
	if(item)
	{
		VisibleItem(item, bVisible);
	}
}

void CLMenu::CheckedItem(int nID, bool bChecked)
{
	CLMenuItem *item = general->GetItem(nID);
	if(item)
	{
		CheckedItem(item, bChecked);
	}
}

bool CLMenu::IsEnableItem(CLMenuItem *item)
{
	return item->enabled;
}
	
bool CLMenu::IsVisibleItem(CLMenuItem *item)
{
	return item->visible;
}

bool CLMenu::IsCheckboxItem(CLMenuItem *item)
{
	return item->checkbox;
}
	
bool CLMenu::IsCheckedItem(CLMenuItem *item)
{
	return item->checked;
}

bool CLMenu::IsEnableItem(int nID)
{
	CLMenuItem *item = general->GetItem(nID);
	if(item)
	{
		return IsEnableItem(item);
	}
	return FALSE;
}
	
bool CLMenu::IsVisibleItem(int nID)
{
	CLMenuItem *item = general->GetItem(nID);
	if(item)
	{
		return IsVisibleItem(item);
	}
	return FALSE;
}


bool CLMenu::IsCheckboxItem(int nID)
{
	CLMenuItem *item = general->GetItem(nID);
	if(item)
	{
		return IsCheckboxItem(item);
	}
	return FALSE;
}

bool CLMenu::IsCheckedItem(int nID)
{
	CLMenuItem *item = general->GetItem(nID);
	if(item)
	{
		return IsCheckedItem(item);
	}
	return FALSE;
}

void CLMenu::SetSelected(CLMenuItem *item)
{
	if(selected != item)
	{
		if(items.Find(selected))
		{
			selected->selected = false;
			DrawItem(selected);
		}
		if(childMenu)
		{
			childMenu->Hide();
			childMenu = NULL;
		}
		if(item && item->isLine)
		{
			if(selected)
			{
				selected = NULL;
				DrawLayer();
				UpdateLayer();
			}
			return;
		}
		selected = item;
		if(selected)
		{
			selected->selected = true;
			DrawItem(selected);

			if(selected->menu->ItemCount(TRUE))
			{
				CRect rw;
				GetWindowRect(&rw);
				CRect r = ItemRect(selected);

				r.left = rw.left;
				r.right = rw.right - SHADOW_SIZE;
				r.OffsetRect(0, rw.top);

				childMenu = selected->menu;
				selected->menu->Popup(this, general->position, r.right, r.top);
			}
		}
		DrawLayer();
		UpdateLayer();
	}
}

CLMenuItem *CLMenu::Insert(CLMenuItem *prevItem, CString text, int nID, bool checkBox, bool addTail)
{
	CLMenuItem *item = new CLMenuItem();
	item->text = text;
	item->nID = nID;
	item->checkbox = checkBox;
	item->menu->Create(this);
	item->menu->general = general;
	POSITION p = items.Find(prevItem);
	if(p)
	{
		items.InsertAfter(p, item);
	}
	else
	if(addTail)
	{
		items.AddTail(item);
	}
	else
	{
		items.AddHead(item);
	}
	return item;
}

CLMenuItem *CLMenu::Add(CString text, int nID, bool checkBox)
{
	return Insert(NULL, text, nID, checkBox);
}

CLMenuItem *CLMenu::AddLine()
{
	return InsertLine(NULL);
}

CLMenuItem *CLMenu::InsertLine(CLMenuItem *prevItem, bool addTail)
{
	CLMenuItem *item = new CLMenuItem();
	item->isLine = true;
	item->menu->Create(this);
	item->menu->general = general;
	POSITION p = items.Find(prevItem);
	if(p)
	{
		items.InsertAfter(p, item);
	}
	else
	if(addTail)
	{
		items.AddTail(item);
	}
	else
	{
		items.AddHead(item);
	}
	return item;
}

bool CLMenu::Remove(CLMenuItem *item)
{
	if(!item)
	{
		return false;
	}
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		CLMenuItem *pitem = items.GetAt(p);
		if(pitem == item)
		{
			items.RemoveAt(p);
			delete item;
			return true;
		}
		if(pitem->menu->Remove(item))
		{
			return true;
		}
		items.GetNext(p);
	}
	return false;
}

void CLMenu::RemoveAll()
{
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		delete items.GetAt(p);
		items.GetNext(p);
	}
	items.RemoveAll();
}

int CLMenu::GetMaxItemWidth()
{
	int w = 0;

	HDC dc = CreateCompatibleDC(0);
	Graphics g(dc);
	RectF r;
	Font font(L"Arial", FONT_SIZE);
	StringFormat *stringFormat = new StringFormat();
	stringFormat->SetAlignment(StringAlignmentNear);
	stringFormat->SetLineAlignment(StringAlignmentCenter);
	stringFormat->SetTrimming(StringTrimmingEllipsisCharacter);
	stringFormat->SetFormatFlags(StringFormatFlagsLineLimit);

	POSITION p = items.GetHeadPosition();
	while(p)
	{
		r.X = 0;
		r.Y = 0;
		r.Width = 0;
		r.Height = 0;

		CLMenuItem *item = items.GetAt(p);
		if(item->visible)
		{
			g.MeasureString(item->text.GetBuffer(), item->text.GetLength(), &font, r, &r);

			if(r.Width > w)
			{
				w = (int)r.Width;
			}
		}
		items.GetNext(p);
	}

	delete stringFormat;
	DeleteObject(dc);

	return w;
}

int CLMenu::ItemCount(bool onlyVisible, bool onlyEnabled)
{
	int n = 0;
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		CLMenuItem *item = items.GetAt(p);
		if(!((onlyVisible && !item->visible) || (onlyEnabled && !item->enabled)))
		{
			n++;
		}
		items.GetNext(p);
	}
	return n;
}

CRect CLMenu::ItemRect(CLMenuItem *item)
{
	CRect r;
	CRect rw;
	GetWindowRect(&rw);
	r.left = SHADOW_SIZE + BORDER_SIZE;
	r.right = r.left + rw.Width() - SHADOW_SIZE * 2 - BORDER_SIZE * 2;

	switch(position)
	{
	case DockPositionTop:
		{
			r.top = SHADOW_SIZE + BORDER_SIZE + ARROW_HEIGHT_CHECK(parent) + RADIUS / 2;
		}
		break;

	default:
		{
			r.top = SHADOW_SIZE + BORDER_SIZE + RADIUS / 2;
		}
		break;
	}

	POSITION p = items.GetHeadPosition();
	while(p && (items.GetAt(p) != item))
	{
		if(items.GetAt(p)->visible)
		{
		r.top += ITEM_HEIGHT;
		}
		items.GetNext(p);
	}
	r.bottom = r.top + ITEM_HEIGHT;

	return r;
}

CLMenuItem *CLMenu::ItemAt(int x, int y)
{
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		CLMenuItem *item = items.GetAt(p);
		if(item->visible && ItemRect(item).PtInRect(CPoint(x, y)))
		{
			return item;
		}
		items.GetNext(p);
	}
	return NULL;
}

void CLMenu::DrawItem(CLMenuItem *item)
{
	CRect r = ItemRect(item);

	item->dib->Resize(r.Width(), r.Height());
	if(!item->dib->Ready())
	{
		return;
	}

	Graphics g(item->dib->bmp);
	g.SetSmoothingMode(SmoothingModeNone);
	g.SetCompositingMode(CompositingModeSourceCopy);

	RectF rf(0, 0, (REAL)item->dib->Width(), (REAL)item->dib->Height());

	SolidBrush brush(Color(255 * BCKG_OPACITY / 100, 0x10, 0x10, 0x10));
	g.FillRectangle(&brush, rf);

	g.SetCompositingMode(CompositingModeSourceOver);

	if(item->selected)
	{
		//LinearGradientBrush brush(rf, 0xff6fa6de, 0xff1e6cbb, LinearGradientModeVertical);
		//g.FillRectangle(&brush, rf);

		RectF rx = rf;
		rx.Height /= 2;
		//rx.X++;
		//rx.Width -= 2;

		LinearGradientBrush brush(rx, 0xff5b9de1, 0xff3d7ebf, LinearGradientModeVertical);
		g.FillRectangle(&brush, rx);

		rx.Y += rx.Height;

		brush.SetLinearColors(0xff3076bc, 0xff4988c8);
		g.FillRectangle(&brush, rx);

		Pen pen(0xff3d7ebf);
		g.DrawLine(&pen, rx.X /*+ 1*/, rx.Y, rx.X + rx.Width /*- 1*/, rx.Y);
	}

	g.SetSmoothingMode(SmoothingModeAntiAlias);

	if(item->isLine)
	{
		Pen pen(0xff909090);
		g.DrawLine(&pen, rf.X + 2, rf.Y + rf.Height / 2, rf.X + rf.Width - 3, rf.Y + rf.Height / 2);
	}
	else
	{
		rf.X += 4;
		rf.Width -= 14;

		if(item->icon->Ready())
		{
			RectF ri = rf;
			ri.Width = rf.Height * 0.9f;
			ri.Height = ri.Width;
			ri.Y += (rf.Height - ri.Height) / 2;

			rf.X += rf.Height;
			rf.Width -= rf.Height;

			float k = min(ri.Width / item->icon->Width(), ri.Height / item->icon->Height());
			
			g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

			ri.X += (ri.Width - item->icon->Width() * k) / 2;
			ri.Y += (ri.Height - item->icon->Height() * k) / 2;
			ri.Width = item->icon->Width() * k;
			ri.Height = item->icon->Height() * k;

			g.DrawImage(item->icon->bmp, ri);
		}

		RectF rc = rf;
		rc.Width = rf.Height * 0.42f;
		rc.Height = rc.Width;
		rc.Y += (rf.Height - rc.Height) / 2;

		if(item->checkbox)
		{
			if(item->checked)
			{
				Pen pen(0xffffffff);
				SolidBrush brush(0xffffffff);
				if(!item->enabled)
				{
					pen.SetColor(0xffb0b0b0);
					brush.SetColor(0xffb0b0b0);
				}

				GraphicsPath *path = new GraphicsPath();
				path->AddLine(rc.X + rc.Width * 0.4f, rc.Y + rc.Height, rc.X + rc.Width * 0.9f, rc.Y);
				path->AddLine(rc.X + rc.Width * 0.9f, rc.Y, rc.X + rc.Width * 0.4f, rc.Y + rc.Height * 0.8f);
				path->AddLine(rc.X + rc.Width * 0.4f, rc.Y + rc.Height * 0.8f, rc.X, rc.Y + rc.Height * 0.6f);
				path->CloseFigure();

				g.FillPath(&brush, path);
				g.DrawPath(&pen, path);

				delete path;
			}
		}

		if(!item->icon->Ready())
		{
			rf.X += (rc.Height + 2);
			rf.Width -= (rc.Height + 2);
		}

		SolidBrush brush(0xff000000);

		Font font(L"Arial", FONT_SIZE);
		StringFormat *stringFormat = new StringFormat();
		stringFormat->SetAlignment(StringAlignmentNear);
		stringFormat->SetLineAlignment(StringAlignmentCenter);
		stringFormat->SetTrimming(StringTrimmingEllipsisCharacter);
		stringFormat->SetFormatFlags(StringFormatFlagsLineLimit);

		//rf.Y++;
		//g.DrawString(item->text.GetBuffer(), item->text.GetLength(), &font, rf, stringFormat, &brush);

		brush.SetColor(0xffffffff);
		if(!item->enabled)
		{
			brush.SetColor(0xffb0b0b0);
		}

		//rf.Y--;
		g.DrawString(item->text.GetBuffer(), item->text.GetLength(), &font, rf, stringFormat, &brush);

		if(item->menu->ItemCount(TRUE))
		{
			GraphicsPath *path = new GraphicsPath();

			path->AddLine((REAL)(item->dib->Width() - 12), (REAL)(item->dib->Height() * 0.33), (REAL)(item->dib->Width() - 6), (REAL)(item->dib->Height() * 0.5));
			path->AddLine((REAL)(item->dib->Width() - 6), (REAL)(item->dib->Height() * 0.5), (REAL)(item->dib->Width() - 12), (REAL)(item->dib->Height() * 0.67));
			path->CloseFigure();

			g.FillPath(&brush, path);

			delete path;
		}

		delete stringFormat;
	}
}

void CLMenu::UpdateItem(Gdiplus::Graphics *g, CLMenuItem *item)
{
	if(item->visible && item->dib->Ready())
	{
		CRect r = ItemRect(item);
		g->DrawImage(item->dib->bmp, (REAL)r.left, (REAL)r.top, (REAL)r.Width(), (REAL)r.Height());
	}
}

void CLMenu::DrawBckg()
{
	CRect r;
	GetWindowRect(&r);

	bckg->Resize(r.Width(), r.Height());
	if(!bckg->Ready())
	{
		return;
	}

	Graphics g(bckg->dc);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.SetCompositingMode(CompositingModeSourceCopy);

	SolidBrush brush(Color(255 * 70 / 100, 0, 0, 0));

	GraphicsPath *path = new GraphicsPath();

	if(parent)
	{
		path->AddArc(bckg->Width() - RADIUS - SHADOW_SIZE, SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 270, 90);
		path->AddArc(bckg->Width() - RADIUS - SHADOW_SIZE, bckg->Height() - RADIUS - SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 0, 90);
		path->AddArc(SHADOW_SIZE, bckg->Height() - RADIUS - SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 90, 90);
		path->AddArc(SHADOW_SIZE, SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 180, 90);
	}
	else
	{
		switch(position)
		{
		case DockPositionTop:
			{
				path->AddArc(bckg->Width() - RADIUS - SHADOW_SIZE, SHADOW_SIZE + ARROW_HEIGHT, RADIUS - 1, RADIUS - 1, 270, 90);
				path->AddArc(bckg->Width() - RADIUS - SHADOW_SIZE, bckg->Height() - RADIUS - SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 0, 90);
				path->AddArc(SHADOW_SIZE, bckg->Height() - RADIUS - SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 90, 90);
				path->AddArc(SHADOW_SIZE, SHADOW_SIZE + ARROW_HEIGHT, RADIUS - 1, RADIUS - 1, 180, 90);
					
				path->AddLine((REAL)SHADOW_SIZE + RADIUS / 2 + arrowOffset, (REAL)SHADOW_SIZE + ARROW_HEIGHT + arrowOffset, (REAL)SHADOW_SIZE + RADIUS / 2 + ARROW_WIDTH / 2 + arrowOffset, (REAL)SHADOW_SIZE);
				path->AddLine((REAL)SHADOW_SIZE + RADIUS / 2 + ARROW_WIDTH / 2 + arrowOffset, (REAL)SHADOW_SIZE, (REAL)SHADOW_SIZE + RADIUS / 2 + ARROW_WIDTH + arrowOffset, (REAL)SHADOW_SIZE + ARROW_HEIGHT);
			}
			break;

		case DockPositionBottom:
			{
				path->AddArc(SHADOW_SIZE, bckg->Height() - RADIUS - SHADOW_SIZE - ARROW_HEIGHT, RADIUS - 1, RADIUS - 1, 90, 90);
				path->AddArc(SHADOW_SIZE, SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 180, 90);
				path->AddArc(bckg->Width() - RADIUS - SHADOW_SIZE, SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 270, 90);
				path->AddArc(bckg->Width() - RADIUS - SHADOW_SIZE, bckg->Height() - RADIUS - SHADOW_SIZE - ARROW_HEIGHT, RADIUS - 1, RADIUS - 1, 0, 90);
				
				path->AddLine((REAL)SHADOW_SIZE + RADIUS / 2 + ARROW_WIDTH + arrowOffset, (REAL)bckg->Height() - 1 - SHADOW_SIZE - ARROW_HEIGHT, (REAL)SHADOW_SIZE + RADIUS / 2 + ARROW_WIDTH / 2 + arrowOffset, (REAL)bckg->Height() - 1 - SHADOW_SIZE);
				path->AddLine((REAL)SHADOW_SIZE + RADIUS / 2 + ARROW_WIDTH / 2 + arrowOffset, (REAL)bckg->Height() - 1 - SHADOW_SIZE, (REAL)SHADOW_SIZE + RADIUS / 2 + arrowOffset, (REAL)bckg->Height() - 1 - SHADOW_SIZE - ARROW_HEIGHT);
			}
			break;

		default:
			{
				path->AddArc(bckg->Width() - RADIUS - SHADOW_SIZE, SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 270, 90);
				path->AddArc(bckg->Width() - RADIUS - SHADOW_SIZE, bckg->Height() - RADIUS - SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 0, 90);
				path->AddArc(SHADOW_SIZE, bckg->Height() - RADIUS - SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 90, 90);
				path->AddArc(SHADOW_SIZE, SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 180, 90);
			}
			break;
		}
	}

	path->CloseFigure();
	g.FillPath(&brush, path);
	delete path;

	if(parent)
	{
		bckg->Blur(CRect(0, 0, bckg->Width(), bckg->Height()), 
			CRect(SHADOW_SIZE * 2, SHADOW_SIZE * 2, bckg->Width() - SHADOW_SIZE * 2, bckg->Height() - SHADOW_SIZE * 2), 
			SHADOW_SIZE);
	}
	else
	{
		switch(position)
		{
		case DockPositionTop:
			{
				bckg->Blur(CRect(0, 0, bckg->Width(), bckg->Height()), 
					CRect(SHADOW_SIZE * 2, SHADOW_SIZE * 2 + ARROW_HEIGHT, bckg->Width() - SHADOW_SIZE * 2, bckg->Height() - SHADOW_SIZE * 2), 
					SHADOW_SIZE);
			}
			break;

		case DockPositionBottom:
			{
				bckg->Blur(CRect(0, 0, bckg->Width(), bckg->Height()), 
					CRect(SHADOW_SIZE * 2, SHADOW_SIZE * 2, bckg->Width() - SHADOW_SIZE * 2, bckg->Height() - SHADOW_SIZE * 2 - ARROW_HEIGHT), 
					SHADOW_SIZE);
			}
			break;

		default:
			{
				bckg->Blur(CRect(0, 0, bckg->Width(), bckg->Height()), 
					CRect(SHADOW_SIZE * 2, SHADOW_SIZE * 2, bckg->Width() - SHADOW_SIZE * 2, bckg->Height() - SHADOW_SIZE * 2), 
					SHADOW_SIZE);
			}
			break;
		}
	}
}

void CLMenu::UpdateLayer(CDIB *dib)
{
	if(!dib)
	{
		dib = CLMenu::dib;
	}

	CRect r;
	GetWindowRect(&r);

	CWnd *wndDst = GetDesktopWindow();
	CDC *hdcDst = wndDst->GetDC();
	CDC *dc = new CDC();
	dc->Attach(dib->dc);
	
	BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
	POINT zp = {0, 0};
	POINT sp = {r.left, r.top};
	SIZE sz = {r.Width(), r.Height()};

	UpdateLayeredWindow(hdcDst, &sp, &sz, dc, &zp, NULL, &bf, ULW_ALPHA);

	dc->Detach();
	delete dc;
	wndDst->ReleaseDC(hdcDst);
}

void CLMenu::DrawLayer(CDIB *dib)
{
	if(!dib)
	{
		dib = CLMenu::dib;
	}

	CRect r;
	GetWindowRect(&r);

	dib->Assign(bckg);
	//dib.Resize(bckg.Width(), bckg.Height());
	if(!dib->Ready())
	{
		return;
	}

	Graphics g(dib->dc);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.SetCompositingMode(CompositingModeSourceCopy);

	SolidBrush brush(Color(255 * BCKG_OPACITY / 100, 0x10, 0x10, 0x10));
	Pen pen(Color(0xd0, 0xd0, 0xd0), BORDER_SIZE);
	GraphicsPath *path = new GraphicsPath();

	if(parent)
	{
		path->AddArc(bckg->Width() - RADIUS - SHADOW_SIZE, SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 270, 90);
		path->AddArc(bckg->Width() - RADIUS - SHADOW_SIZE, bckg->Height() - RADIUS - SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 0, 90);
		path->AddArc(SHADOW_SIZE, bckg->Height() - RADIUS - SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 90, 90);
		path->AddArc(SHADOW_SIZE, SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 180, 90);
	}
	else
	{
		switch(position)
		{
		case DockPositionTop:
			{
				path->AddArc(bckg->Width() - RADIUS - SHADOW_SIZE, SHADOW_SIZE + ARROW_HEIGHT, RADIUS - 1, RADIUS - 1, 270, 90);
				path->AddArc(bckg->Width() - RADIUS - SHADOW_SIZE, bckg->Height() - RADIUS - SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 0, 90);
				path->AddArc(SHADOW_SIZE, bckg->Height() - RADIUS - SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 90, 90);
				path->AddArc(SHADOW_SIZE, SHADOW_SIZE + ARROW_HEIGHT, RADIUS - 1, RADIUS - 1, 180, 90);
					
				path->AddLine((REAL)SHADOW_SIZE + RADIUS / 2 + arrowOffset, (REAL)SHADOW_SIZE + ARROW_HEIGHT, (REAL)SHADOW_SIZE + RADIUS / 2 + ARROW_WIDTH / 2 + arrowOffset, (REAL)SHADOW_SIZE);
				path->AddLine((REAL)SHADOW_SIZE + RADIUS / 2 + ARROW_WIDTH / 2 + arrowOffset, (REAL)SHADOW_SIZE, (REAL)SHADOW_SIZE + RADIUS / 2 + ARROW_WIDTH + arrowOffset, (REAL)SHADOW_SIZE + ARROW_HEIGHT);
			}
			break;

		case DockPositionBottom:
			{
				path->AddArc(SHADOW_SIZE, bckg->Height() - RADIUS - SHADOW_SIZE - ARROW_HEIGHT, RADIUS - 1, RADIUS - 1, 90, 90);
				path->AddArc(SHADOW_SIZE, SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 180, 90);
				path->AddArc(bckg->Width() - RADIUS - SHADOW_SIZE, SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 270, 90);
				path->AddArc(bckg->Width() - RADIUS - SHADOW_SIZE, bckg->Height() - RADIUS - SHADOW_SIZE - ARROW_HEIGHT, RADIUS - 1, RADIUS - 1, 0, 90);
				
				path->AddLine((REAL)SHADOW_SIZE + RADIUS / 2 + ARROW_WIDTH + arrowOffset, (REAL)bckg->Height() - 1 - SHADOW_SIZE - ARROW_HEIGHT, (REAL)SHADOW_SIZE + RADIUS / 2 + ARROW_WIDTH / 2 + arrowOffset, (REAL)bckg->Height() - 1 - SHADOW_SIZE);
				path->AddLine((REAL)SHADOW_SIZE + RADIUS / 2 + ARROW_WIDTH / 2 + arrowOffset, (REAL)bckg->Height() - 1 - SHADOW_SIZE, (REAL)SHADOW_SIZE + RADIUS / 2 + arrowOffset, (REAL)bckg->Height() - 1 - SHADOW_SIZE - ARROW_HEIGHT);
			}
			break;

		default:
			{
				path->AddArc(bckg->Width() - RADIUS - SHADOW_SIZE, SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 270, 90);
				path->AddArc(bckg->Width() - RADIUS - SHADOW_SIZE, bckg->Height() - RADIUS - SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 0, 90);
				path->AddArc(SHADOW_SIZE, bckg->Height() - RADIUS - SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 90, 90);
				path->AddArc(SHADOW_SIZE, SHADOW_SIZE, RADIUS - 1, RADIUS - 1, 180, 90);
			}
			break;
		}
	}

	path->CloseFigure();
	g.FillPath(&brush, path);

	g.SetCompositingMode(CompositingModeSourceOver);
	g.DrawPath(&pen, path);

	delete path;

	g.SetCompositingMode(CompositingModeSourceCopy);

	POSITION p = items.GetHeadPosition();
	while(p)
	{
		UpdateItem(&g, items.GetAt(p));
		items.GetNext(p);
	}
}

bool CLMenu::IsVisible()
{
	return (IsWindowVisible() == TRUE);
}

void CLMenu::Create(CWnd *Parent)
{
	if(IsWindow(m_hWnd))
	{
		return;
	}
	wndParent = Parent;
	CreateEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST, NULL, NULL, WS_POPUP, 0, 0, 0, 0, 
		wndParent ? wndParent->m_hWnd : NULL, NULL);
}

void CLMenu::Hide(bool clicked)
{
	if(IsWindowVisible())
	{
		if(childMenu)
		{
			childMenu->Hide();
			childMenu = NULL;
		}

		if (tmp)
		{
			delete tmp;
			tmp = NULL;
		}

		bckg->FreeImage();
		dib->FreeImage();

		ShowWindow(SW_HIDE);

		POSITION p = items.GetHeadPosition();
		while(p)
		{
			items.GetAt(p)->dib->FreeImage();
			items.GetNext(p);
		}

		if((general == this) && general->wndParent)
		{
			general->wndParent->SendMessage(WM_MENUNOTIFY, MAKEWPARAM(MenuHide, clicked ? TRUE : FALSE));
		}
	}
}

void CLMenu::UpdatePosition(CPoint pt)
{
	if(!IsWindow(m_hWnd) || !IsWindowVisible())
	{
		return;
	}

	CMonitors monitors;
	CMonitor *monitor = monitors.GetMonitor(pt);
	if(!monitor)
	{
		return;
	}

	CRect r;
	GetWindowRect(&r);

	if(parent)
	{
		r.MoveToXY(pt.x - SHADOW_SIZE - BORDER_SIZE, pt.y - SHADOW_SIZE - BORDER_SIZE - RADIUS / 2);
	}
	else
	{
		switch(position)
		{
		case DockPositionBottom:
			{
				r.MoveToXY(pt.x - SHADOW_SIZE - BORDER_SIZE - RADIUS / 2 - ARROW_WIDTH / 2, 
					pt.y - SHADOW_SIZE - BORDER_SIZE - RADIUS - ARROW_HEIGHT - ITEM_HEIGHT * ItemCount(TRUE));
			}
			break;

		case DockPositionTop:
			{
				r.MoveToXY(pt.x - SHADOW_SIZE - BORDER_SIZE - RADIUS / 2 - ARROW_WIDTH / 2, pt.y - SHADOW_SIZE - BORDER_SIZE);
			}
			break;

		case DockPositionRight:
			{
				r.MoveToXY(pt.x - SHADOW_SIZE - BORDER_SIZE - RADIUS - ITEM_ADDITION - GetMaxItemWidth(), 
					pt.y - SHADOW_SIZE - BORDER_SIZE - RADIUS / 2 - (ITEM_HEIGHT * ItemCount(TRUE)) / 2);
			}
			break;

		case DockPositionLeft:
			{
				r.MoveToXY(pt.x - SHADOW_SIZE - BORDER_SIZE, 
					pt.y - SHADOW_SIZE - BORDER_SIZE - RADIUS / 2 - (ITEM_HEIGHT * ItemCount(TRUE)) / 2);
			}
			break;

		default:
			{
				r.MoveToXY(pt.x - SHADOW_SIZE - BORDER_SIZE, pt.y - SHADOW_SIZE - BORDER_SIZE - RADIUS / 2);
			}
			break;
		}
	}

	CRect monitorRect = monitor->Rect();
	// update position on the screen
	//int a = arrowOffset;
	//arrowOffset = 0;
	if(r.left < monitorRect.left)
	{
		/*arrowOffset = r.left - monitor->rect.left;
		if(arrowOffset < SHADOW_SIZE + RADIUS / 2)
		{
			arrowOffset = 0;
		}*/
		r.MoveToX(monitorRect.left);
	}
	if(r.top < monitorRect.top)
	{
		r.MoveToY(monitorRect.top);
	}
	if(r.bottom > monitorRect.bottom)
	{
		r.MoveToY(r.top - (r.bottom - monitorRect.bottom));
	}
	//int arrowRight = r.Width() - SHADOW_SIZE * 2 - RADIUS - ARROW_WIDTH;
	if(!parent)
	{
		if(r.right > monitorRect.right)
		{
			/*arrowOffset = r.right - monitor->rect.right;
			if(arrowOffset > arrowRight)
			{
				arrowOffset = arrowRight;
			}*/
			r.MoveToX(r.left - (r.right - monitorRect.right));
		}
	}
	else
	{
		CRect rect;
		parent->GetWindowRect(&rect);

		if(r.right > monitorRect.right)
		{
			/*arrowOffset = r.right - monitor->rect.right;
			if(arrowOffset > arrowRight)
			{
				arrowOffset = arrowRight;
			}*/
			r.MoveToX(rect.left - r.Width() + SHADOW_SIZE * 2);
		}
	}

	SetWindowPos(this, r.left, r.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	/*if(a != arrowOffset)
	{
		DrawBckg();
		Draw();
	}*/
}

bool CLMenu::Popup(CLMenu *Parent, DockPosition position, int X, int Y)
{
	if(!IsWindow(m_hWnd) || IsWindowVisible())
	{
		return FALSE;
	}

	CMonitors monitors;
	CMonitor *monitor = monitors.GetMonitor(CPoint(X, Y));
	if(!monitor)
	{
		return FALSE;
	}

	CLMenu::position = position;
	parent = Parent;
	SetSelected(NULL);

	if(ItemCount(TRUE, TRUE) == 0)
	{
		return FALSE;
	}

	CRect r;

	if(parent)
	{
		r.left = X - SHADOW_SIZE - BORDER_SIZE;
		r.top = Y - SHADOW_SIZE - BORDER_SIZE - RADIUS / 2;
		r.right = r.left + SHADOW_SIZE * 2 + BORDER_SIZE * 2 + RADIUS + ITEM_ADDITION + GetMaxItemWidth();
		r.bottom = r.top + SHADOW_SIZE * 2 + BORDER_SIZE * 2 + RADIUS + ITEM_HEIGHT * ItemCount(TRUE);
	}
	else
	{
		switch(position)
		{
		case DockPositionBottom:
			{
				r.left = X - SHADOW_SIZE - BORDER_SIZE - RADIUS / 2 - ARROW_WIDTH / 2;
				r.top = Y - SHADOW_SIZE - BORDER_SIZE - RADIUS - ARROW_HEIGHT - ITEM_HEIGHT * ItemCount(TRUE);
				r.right = r.left + SHADOW_SIZE * 2 + BORDER_SIZE * 2 + RADIUS + ITEM_ADDITION + GetMaxItemWidth();
				r.bottom = Y + SHADOW_SIZE + BORDER_SIZE;
			}
			break;

		case DockPositionTop:
			{
				r.left = X - SHADOW_SIZE - BORDER_SIZE - RADIUS / 2 - ARROW_WIDTH / 2;
				r.top = Y - SHADOW_SIZE - BORDER_SIZE;
				r.right = r.left + SHADOW_SIZE * 2 + BORDER_SIZE * 2 + RADIUS + ITEM_ADDITION + GetMaxItemWidth();
				r.bottom = Y + SHADOW_SIZE + BORDER_SIZE + ARROW_HEIGHT + RADIUS + ITEM_HEIGHT * ItemCount(TRUE);
			}
			break;

		case DockPositionRight:
			{
				r.left = X - SHADOW_SIZE - BORDER_SIZE - RADIUS - ITEM_ADDITION - GetMaxItemWidth();
				r.top = Y - SHADOW_SIZE - BORDER_SIZE - RADIUS / 2 - (ITEM_HEIGHT * ItemCount(TRUE)) / 2;
				r.right = X + SHADOW_SIZE + BORDER_SIZE;
				r.bottom = r.top + SHADOW_SIZE * 2 + BORDER_SIZE * 2 + RADIUS + ITEM_HEIGHT * ItemCount(TRUE);
			}
			break;

		case DockPositionLeft:
			{
				r.left = X - SHADOW_SIZE - BORDER_SIZE;
				r.top = Y - SHADOW_SIZE - BORDER_SIZE - RADIUS / 2 - (ITEM_HEIGHT * ItemCount(TRUE)) / 2;
				r.right = X + SHADOW_SIZE + BORDER_SIZE + RADIUS + ITEM_ADDITION + GetMaxItemWidth();
				r.bottom = r.top + SHADOW_SIZE * 2 + BORDER_SIZE * 2 + RADIUS + ITEM_HEIGHT * ItemCount(TRUE);
			}
			break;

		default:
			{
				r.left = X - SHADOW_SIZE - BORDER_SIZE;
				r.top = Y - SHADOW_SIZE - BORDER_SIZE - RADIUS / 2;
				r.right = r.left + SHADOW_SIZE * 2 + BORDER_SIZE * 2 + RADIUS + ITEM_ADDITION + GetMaxItemWidth();
				r.bottom = r.top + SHADOW_SIZE * 2 + BORDER_SIZE * 2 + RADIUS + ITEM_HEIGHT * ItemCount(TRUE);
			}
			break;
		}
	}

	// update position on the screen
	CRect monitorRect = monitor->Rect();
	arrowOffset = 3;
	if(r.left < monitorRect.left)
	{
		arrowOffset = r.left - monitorRect.left;
		if(arrowOffset < SHADOW_SIZE + RADIUS / 2)
		{
			arrowOffset = 3;
		}
		r.MoveToX(monitorRect.left);
	}
	if(r.top < monitorRect.top)
	{
		r.MoveToY(monitorRect.top);
	}
	if(r.bottom > monitorRect.bottom)
	{
		r.MoveToY(r.top - (r.bottom - monitorRect.bottom));
	}
	int arrowRight = r.Width() - SHADOW_SIZE * 2 - RADIUS - ARROW_WIDTH;
	if(!parent)
	{
		if(r.right > monitorRect.right)
		{
			arrowOffset = r.right - monitorRect.right;
			if(arrowOffset > arrowRight)
			{
				arrowOffset = arrowRight - 3;
			}
			r.MoveToX(r.left - (r.right - monitorRect.right));
		}
	}
	else
	{
		CRect rect;
		parent->GetWindowRect(&rect);

		if(r.right > monitorRect.right)
		{
			arrowOffset = r.right - monitorRect.right;
			if(arrowOffset > arrowRight)
			{
				arrowOffset = arrowRight - 3;
			}
			r.MoveToX(rect.left - r.Width() + SHADOW_SIZE * 2);
		}
	}

	SetWindowPos(this, r.left, r.top, r.Width(), r.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

	POSITION p = items.GetHeadPosition();
	while(p)
	{
		DrawItem(items.GetAt(p));
		items.GetNext(p);
	}

	DrawBckg();
	DrawLayer();

	tmp = new CDIB();
	tmp->Resize(dib->Width(), dib->Height());

	UpdateLayer(tmp);
	ShowWindow(SW_SHOW);

	SetTimer(TIMER_SHOW, 10, NULL);
	showStartAt = GetTickCount();

	return TRUE;
}

void CLMenu::OnTimer(UINT_PTR nIDEvent)
{
	CFrameWnd::OnTimer(nIDEvent);

	switch(nIDEvent)
	{
	case TIMER_SHOW:
		{
			if (tmp)
			{
				float k = (float)(GetTickCount() - showStartAt) / TIMER_SHOW_DELAY;
				if(k > 1)
				{
					k = 1;
				}
				tmp->Fill();
				BLENDFUNCTION blend = {AC_SRC_OVER, 0, (BYTE)(255 * sin(k * PI / 2)), AC_SRC_ALPHA};
				AlphaBlend(tmp->dc, 0, 0, tmp->Width(), tmp->Height(), dib->dc, 0, 0, dib->Width(), dib->Height(), blend);
				UpdateLayer(tmp);
				if(k == 1)
				{
					UpdateLayer();
					KillTimer(nIDEvent);
					delete tmp;
					tmp = NULL;
				}
			}
			else
			{
				KillTimer(nIDEvent);
			}
		}
		break;
	}
}

void CLMenu::OnMouseMove(UINT nFlags, CPoint point)
{
	CFrameWnd::OnMouseMove(nFlags, point);

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE;
	tme.hwndTrack = m_hWnd;
	tme.dwHoverTime = HOVER_DEFAULT;
	TrackMouseEvent(&tme);

	CLMenuItem *item = ItemAt(point.x, point.y);
	if(item)
	{
		SetSelected(item->enabled ? item : NULL);
	}
	else
	if(!childMenu)
	{
		SetSelected(NULL);
	}
}

void CLMenu::OnMouseLeave()
{
	CFrameWnd::OnMouseLeave();

	if(!childMenu)
	{
		SetSelected(NULL);
	}
}

void CLMenu::OnKillFocus(CWnd *pNewWnd)
{
	CFrameWnd::OnKillFocus(pNewWnd);

	CLMenu *p = general;
	while(p)
	{
		if(p == pNewWnd)
		{
			return;
		}
		p = p->childMenu;
	}
	general->Hide();
}

void CLMenu::OnLButtonDown(UINT nFlags, CPoint point)
{
	CFrameWnd::OnLButtonDown(nFlags, point);
}

void CLMenu::OnLButtonUp(UINT nFlags, CPoint point)
{
	CFrameWnd::OnLButtonUp(nFlags, point);

	if(selected && (selected->menu->ItemCount(TRUE) == 0))
	{
		if(general->wndParent)
		{
			general->wndParent->PostMessage(WM_MENUNOTIFY, MAKEWPARAM(MenuSelect, NULL), selected->nID);
		}
		general->Hide(true);
	}
}