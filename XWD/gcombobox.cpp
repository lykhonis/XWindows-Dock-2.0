#include "gcombobox.h"

BEGIN_MESSAGE_MAP(CGComboBox, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_ENABLE()

	ON_REGISTERED_MESSAGE(WM_MENUNOTIFY, OnMenuNotify)
END_MESSAGE_MAP()

using namespace Gdiplus;

CGComboBoxItem::CGComboBoxItem()
{
	id = 0;
}

CGComboBoxItem::~CGComboBoxItem()
{
}

CGComboBox::CGComboBox()
{
	bckg = new CDIB();
	selected = NULL;
}

CGComboBox::~CGComboBox()
{
	RemoveAll();
	delete bckg;
}

BOOL CGComboBox::PreCreateWindow(CREATESTRUCT& cs)
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

CGComboBoxItem* CGComboBox::Add(CString text, int id)
{
	CGComboBoxItem *item = new CGComboBoxItem();
	item->text = text;
	item->id = id;
	items.AddTail(item);
	return item;
}
	
void CGComboBox::RemoveAll()
{
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		delete items.GetAt(p);
		items.GetNext(p);
	}
	items.RemoveAll();
}

int CGComboBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CFrameWnd::OnCreate(lpCreateStruct);

	menu = new CLMenu();
	menu->Create(this);

	return 0;
}

void CGComboBox::OnDestroy()
{
	CFrameWnd::OnDestroy();
}

BOOL CGComboBox::OnEraseBkgnd(CDC*)
{
	return TRUE;
}

void CGComboBox::Draw(CDIB &dib)
{
	CRect rc;
	GetClientRect(&rc);

	dib.Resize(rc.Width(), rc.Height());
	if(!dib.Ready())
	{
		return;
	}

	Graphics g(dib.dc);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	RectF rf(0.0f, 0.0f, (REAL)rc.Width(), (REAL)rc.Height());

	if(bckg->Ready())
	{
		dib.Draw(dib.Rect(), 0, 0, bckg, DrawFlagsPaint | DrawFlagsReflectSrc);
	}

	GraphicsPath *path = new GraphicsPath();

	float radius = 10;

	path->AddArc(rf.X + rf.Width - radius, rf.Y + rf.Height - radius, radius - 1, radius - 1, 0, 90);
	path->AddArc(rf.X, rf.Y + rf.Height - radius, radius - 1, radius - 1, 90, 90);
	path->AddArc(rf.X, rf.Y, radius - 1, radius - 1, 180, 90);
	path->AddArc(rf.X + rf.Width - radius, rf.Y, radius - 1, radius - 1, 270, 90);
	path->CloseFigure();

	LinearGradientBrush lb(rf, 0xfff9f9f9, 0xffe8e8e8, LinearGradientModeVertical);
	Pen pen(0xff4f4f4f);

	g.FillPath(&lb, path);
	g.DrawPath(&pen, path);

	path->Reset();
	path->AddArc(rf.X + rf.Width - radius, rf.Y, radius - 1, radius - 1, 270, 90);
	path->AddArc(rf.X + rf.Width - radius, rf.Y + rf.Height - radius, radius - 1, radius - 1, 0, 90);
	path->AddLine(rf.X + rf.Width - radius, rf.Y + rf.Height - 1, rf.X + rf.Width - rf.Height, rf.Y + rf.Height - 1);
	path->AddLine(rf.X + rf.Width - rf.Height, rf.Y + rf.Height - 1, rf.X + rf.Width - rf.Height, rf.Y);
	path->CloseFigure();

	if(IsWindowEnabled())
	{
		lb.SetLinearColors(0xfffefefe, 0xffa9a9a9);
	}
	else
	{
		lb.SetLinearColors(0xffafafaf, 0xff696969);
	}
	g.FillPath(&lb, path);
	g.DrawPath(&pen, path);

	path->Reset();
	path->AddLine(rf.X + rf.Width - rf.Height * 0.65f, rf.Y + rf.Height * 0.35f, rf.X + rf.Width - rf.Height * 0.35f, rf.Y + rf.Height * 0.35f);
	path->AddLine(rf.X + rf.Width - rf.Height * 0.35f, rf.Y + rf.Height * 0.35f, rf.X + rf.Width - rf.Height / 2, rf.Y + rf.Height * 0.65f);
	path->CloseFigure();

	SolidBrush brush(0xff000000);
	g.FillPath(&brush, path);
	
	if(selected)
	{
		RectF rx = rf;
		rx.X += radius / 2;
		rx.Width -= (rf.Height + radius / 2);

		Font font(L"Arial", 9.0f);
		StringFormat *stringFormat = new StringFormat();
		stringFormat->SetAlignment(StringAlignmentNear);
		stringFormat->SetLineAlignment(StringAlignmentCenter);
		stringFormat->SetTrimming(StringTrimmingEllipsisCharacter);
		stringFormat->SetFormatFlags(StringFormatFlagsLineLimit);

		g.DrawString(selected->text.GetBuffer(), selected->text.GetLength(), &font, rx, stringFormat, &brush);

		delete stringFormat;
	}

	delete path;
}

void CGComboBox::OnPaint()
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

LRESULT CGComboBox::OnMenuNotify(WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam))
	{
	case MenuSelect:
		{
			CGComboBoxItem *item = FindItem(lParam);
			if(item)
			{
				selected = item;
				RedrawWindow();

				CWnd *parent = GetParent();
				if(parent)
				{
					parent->SendMessage(WM_CONTROLNOTIFY, MAKEWPARAM(GetDlgCtrlID(), NULL));
				}
			}
		}
		break;
	}
	return 0;
}

void CGComboBox::OnLButtonDown(UINT nFlags, CPoint point)
{
	CFrameWnd::OnLButtonDown(nFlags, point);
	SetFocus();

	menu->RemoveAll();
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		CGComboBoxItem *item = items.GetAt(p);
		menu->Add(item->text, item->id, true)->checked = (item == selected);
		items.GetNext(p);
	}

	CRect rect;
	GetWindowRect(&rect);

	menu->Popup(NULL, DockPositionNone, rect.left, rect.top);
}

CGComboBoxItem* CGComboBox::FindItem(int id)
{
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		CGComboBoxItem *item = items.GetAt(p);
		if(item->id == id)
		{
			return item;
		}
		items.GetNext(p);
	}
	return NULL;
}

void CGComboBox::OnEnable(BOOL bEnable)
{
	CFrameWnd::OnEnable(bEnable);
	RedrawWindow();
}