#include "editicon.h"

BEGIN_MESSAGE_MAP(CEditIcon, CFrameWnd)
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
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()

	ON_MESSAGE(WM_SETTEXT, OnSetText)

	ON_REGISTERED_MESSAGE(WM_CONTROLNOTIFY, OnControlNotify)
END_MESSAGE_MAP()

using namespace Gdiplus;
using namespace ShellIO;

#define CAPTION_HEIGHT 24
#define WINDOW_BORDER_LEFT 1
#define WINDOW_BORDER_TOP 1
#define WINDOW_BORDER_RIGHT 1
#define WINDOW_BORDER_BOTTOM 1

enum
{
	idButtonClose = 1
};

#define LANGUAGE_USEDRAGDROP L"Use Drag&Drop to replace/delete icon"
#define LANGUAGE_DESCRIPTION L"Description"
#define LANGUAGE_ARGUMENTS L"Arguments"
#define LANGUAGE_WORKDIRECTORY L"Work Directory"

CEditIcon::CEditIcon()
{
	item = NULL;
	notifer = NULL;
	nID = 0;
//	logo = new CDIB();
	icon = new CDIB();
	dib = new CDIB();
	dropRoot = NULL;
	dropFile = NULL;
	dropIsIcon = false;
	hideArgWorkDir = false;
	mouseDown = false;
	drawIcon = true;

	CreateEx(NULL, NULL, L"XWindows Dock - Icon\'s Properties", WS_OVERLAPPED | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, CRect(0, 0, 0, 0), NULL, NULL);
}

CEditIcon::~CEditIcon()
{
	if(dropRoot)
	{
		ILFree(dropRoot);
	}
	if(dropFile)
	{
		ILFree(dropFile);
	}
	delete dib;
//	delete logo;
	delete icon;
}

LRESULT CEditIcon::OnNcHitTest(CPoint point)
{
	ScreenToClient(&point);
	if(point.y < CAPTION_HEIGHT)
	{
		return HTCAPTION;
	}
	return HTCLIENT;
}

int CEditIcon::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CFrameWnd::OnCreate(lpCreateStruct);
	ModifyStyle(WS_CAPTION | WS_BORDER, NULL);
	SetClassLong(CFrameWnd::m_hWnd, GCL_HICON, (LONG)LoadIcon(AfxGetInstanceHandle(), IDI_APPLICATION));

	userDropTargetHelper = SUCCEEDED(CoCreateInstance(CLSID_DragDropHelper, NULL, 
		CLSCTX_INPROC_SERVER, IID_IDropTargetHelper, (void**)&dropTargetHelper));

	Register(this);

	btnClose = new CImageButton();
	btnClose->color1 = 0xff6b6b6b;
	btnClose->color2 = 0xff474747;
	btnClose->CreateEx(NULL, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 14, 14), this, idButtonClose);

	editDesc = new CEdit();
	editDesc->CreateEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, CRect(0, 0, 0, 0), this, NULL);
	editDesc->SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));

	editArguments = new CEdit();
	editArguments->CreateEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, CRect(0, 0, 0, 0), this, NULL);
	editArguments->SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));

	editWorkDir = new CEdit();
	editWorkDir->CreateEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, CRect(0, 0, 0, 0), this, NULL);
	editWorkDir->SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));

	return 0;
}

void CEditIcon::OnDestroy()
{
	delete editDesc;
	delete editArguments;
	delete editWorkDir;

	CFrameWnd::OnDestroy();
}

void CEditIcon::OnWindowPosChanged(WINDOWPOS *lpwndpos)
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

	editDesc->SetWindowPos(&wndTop, r.right - 20 - 240, r.top + CAPTION_HEIGHT + 70, 240, 24, SWP_NOZORDER | SWP_NOACTIVATE);
	editArguments->SetWindowPos(&wndTop, r.right - 20 - 240, r.top + CAPTION_HEIGHT + 140, 240, 24, SWP_NOZORDER | SWP_NOACTIVATE);
	editWorkDir->SetWindowPos(&wndTop, r.right - 20 - 240, r.top + CAPTION_HEIGHT + 170, 240, 24, SWP_NOZORDER | SWP_NOACTIVATE);
}

BOOL CEditIcon::PreCreateWindow(CREATESTRUCT &cs)
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

BOOL CEditIcon::OnEraseBkgnd(CDC*)
{
	return TRUE;
}

BOOL CEditIcon::OnNcActivate(BOOL)
{
	return TRUE;
}

void CEditIcon::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS *lpncsp)
{
	/*if(lpncsp->lppos)
	{
		int cxfr = GetSystemMetrics(SM_CXSIZEFRAME);
		int cyfr = GetSystemMetrics(SM_CYSIZEFRAME);
		InflateRect(lpncsp->rgrc, cxfr - 1, cyfr - 1);
	}*/
	CFrameWnd::OnNcCalcSize(bCalcValidRects, lpncsp);
}

void CEditIcon::OnNcPaint()
{
	CFrameWnd::OnNcPaint();
}

void CEditIcon::OnPaint()
{
	CPaintDC dc(this);
	if(dib->Ready())
	{
		CRect r;
		GetClientRect(&r);
		BitBlt(dc.m_hDC, r.left, r.top, r.Width(), r.Height(), dib->dc, 0, 0, SRCCOPY);
	}
}

void CEditIcon::Draw()
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
	rx.Height = CAPTION_HEIGHT;
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


	// Painting content
	if(drawIcon)
	{
		if(icon->Ready())
		{
			const int shadow = 6;

			float k = min(200.0f / icon->Width(), 200.0f / icon->Height());

			rx = rf;
			rx.X += 20 + (200 - icon->Width() * k) / 2;
			rx.Y += CAPTION_HEIGHT + 30 + (200 - icon->Height() * k) / 2;
			rx.Width = icon->Width() * k;
			rx.Height = icon->Height() * k;

			tmp.Resize((int)rx.Width + shadow * 2, (int)rx.Height + shadow * 2);
			if(tmp.Ready())
			{
				Graphics g2(tmp.bmp);
				g2.DrawImage(icon->bmp, RectF((REAL)shadow, (REAL)shadow, rx.Width, rx.Height));

				DIB_ARGB *p = (DIB_ARGB*)tmp.scan0;
				int size = tmp.Width() * tmp.Height();
				for(int y = 0; y < size; y++, p++)
				{
					p->r = 0;
					p->g = 0;
					p->b = 0;
					p->a = (unsigned char)(p->a * 0.5f);
				}

				tmp.Blur(tmp.Rect(), CRect(0, 0, 0, 0), shadow);

				g.DrawImage(tmp.bmp, RectF(rx.X - shadow, rx.Y - shadow, rx.Width + shadow * 2, rx.Height + shadow * 2));
			}
			g.DrawImage(icon->bmp, rx);
		}
	}

	Font font2(L"Arial", 9.0f);
	stringFormat->SetAlignment(StringAlignmentCenter);
	stringFormat->SetLineAlignment(StringAlignmentNear);

	rx = rf;
	rx.X += 10;
	rx.Y += CAPTION_HEIGHT + 30 + 216;
	rx.Height = 20;
	rx.Width = 220;

	rx.Y++;
	brush.SetColor(0xfff0f0f0);
	g.DrawString(LANGUAGE_USEDRAGDROP, wcslen(LANGUAGE_USEDRAGDROP), &font2, rx, stringFormat, &brush);

	rx.Y--;
	brush.SetColor(0xff000000);
	g.DrawString(LANGUAGE_USEDRAGDROP, wcslen(LANGUAGE_USEDRAGDROP), &font2, rx, stringFormat, &brush);


	Font font3(L"Arial", 10.0f, FontStyleBold);
	stringFormat->SetAlignment(StringAlignmentFar);
	stringFormat->SetLineAlignment(StringAlignmentCenter);

	rx = rf;
	rx.X += 20 + 200 + 10;
	rx.Y += CAPTION_HEIGHT + 70;
	rx.Width -= (rx.X + 240 + 20 + 4);
	rx.Height = 24;

	rx.Y++;
	brush.SetColor(0xfff0f0f0);
	g.DrawString(LANGUAGE_DESCRIPTION, wcslen(LANGUAGE_DESCRIPTION), &font3, rx, stringFormat, &brush);

	rx.Y--;
	brush.SetColor(0xff000000);
	g.DrawString(LANGUAGE_DESCRIPTION, wcslen(LANGUAGE_DESCRIPTION), &font3, rx, stringFormat, &brush);

	if(!hideArgWorkDir)
	{
		rx.Y = rf.Y + CAPTION_HEIGHT + 140;

		rx.Y++;
		brush.SetColor(0xfff0f0f0);
		g.DrawString(LANGUAGE_ARGUMENTS, wcslen(LANGUAGE_ARGUMENTS), &font2, rx, stringFormat, &brush);

		rx.Y--;
		brush.SetColor(0xff000000);
		g.DrawString(LANGUAGE_ARGUMENTS, wcslen(LANGUAGE_ARGUMENTS), &font2, rx, stringFormat, &brush);

		rx.Y = rf.Y + CAPTION_HEIGHT + 170;

		rx.Y++;
		brush.SetColor(0xfff0f0f0);
		g.DrawString(LANGUAGE_WORKDIRECTORY, wcslen(LANGUAGE_WORKDIRECTORY), &font2, rx, stringFormat, &brush);

		rx.Y--;
		brush.SetColor(0xff000000);
		g.DrawString(LANGUAGE_WORKDIRECTORY, wcslen(LANGUAGE_WORKDIRECTORY), &font2, rx, stringFormat, &brush);
	}

	delete stringFormat;

	// shadow for caption
	Pen pen(0xff000000);
	g.DrawLine(&pen, rf.X, rf.Y + CAPTION_HEIGHT - 1, rf.X + rf.Width, rf.Y + CAPTION_HEIGHT - 1);

	for(float y = 0; y < 8; y++)
	{
		float k = 1 - (1 - y / 8) * 0.3f;
		DIB_ARGB *p = (DIB_ARGB*)dib->Pixels((int)rf.X, (int)(rf.Y + CAPTION_HEIGHT + y));
		for(float x = 0; x < rf.Width; x++, p++)
		{
			p->r = (unsigned char)(p->r * k);
			p->g = (unsigned char)(p->g * k);
			p->b = (unsigned char)(p->b * k);
		}
	}
}

LRESULT CEditIcon::OnSetText(WPARAM wParam, LPARAM lParam)
{
	LRESULT res = DefWindowProc(WM_SETTEXT, wParam, lParam);
	RedrawWindow();
	return res;
}

LRESULT CEditIcon::OnControlNotify(WPARAM wParam, LPARAM)
{
	switch(LOWORD(wParam))
	{
	case idButtonClose:
		{
			OnClose();
		}
		break;
	}
	return 0;
}

void CEditIcon::OnClose()
{
	if(notifer)
	{
		notifer->PostMessage(WM_CONTROLNOTIFY, MAKEWPARAM(nID, 1));
	}
}

BOOL CEditIcon::PreTranslateMessage(MSG *pMSG)
{
	switch(pMSG->message)
	{
	case WM_KEYDOWN:
		{
			switch(pMSG->wParam)
			{
			case VK_RETURN:
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

			case 0x4f: // 'O'
				{
					SHORT lctrl = GetKeyState(VK_LCONTROL);
					SHORT rctrl = GetKeyState(VK_RCONTROL);
					if((lctrl & 0x8000) || (rctrl & 0x8000))
					{
						OpenFileDialog();
					}
				}
				break;
			}
		}
		break;
	}
	return CFrameWnd::PreTranslateMessage(pMSG);
}

void CEditIcon::OpenFileDialog()
{
	wchar_t buff[1024] = {0};

	CFileDialog dlg(TRUE);
	dlg.GetOFN().hwndOwner = CFrameWnd::m_hWnd;
	dlg.GetOFN().lpstrFile = buff;
	dlg.GetOFN().nMaxFile = 1024;

	EnableWindow(FALSE);

	INT_PTR nResult = dlg.DoModal();
	if(nResult == IDOK)
	{
		CString fileName = buff;
		CString ext = fileName.Mid(fileName.ReverseFind(L'.')).MakeLower();

		// first check whether it is an icon
		CDIB tmp;
		if((ext == L".ico") || (ext == L".icon"))
		{
			Icons::GetIcon(fileName, &tmp);
		}
		else
		{
			tmp.Load(fileName);
		}

		// install new icon
		if(tmp.Ready())
		{
			iconPath = fileName;
			icon->Assign(&tmp);
			Draw();
			RedrawWindow();
		}
	}

	EnableWindow();
}

DROPEFFECT CEditIcon::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD, CPoint point)
{
	DROPEFFECT dwEffect = DROPEFFECT_NONE;
	if(pDataObject->IsDataAvailable(CF_SHELLIDLIST))
	{
		FORMATETC fmtetc = {CF_SHELLIDLIST, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		HGLOBAL hMem = pDataObject->GetGlobalData(CF_SHELLIDLIST, &fmtetc);
		if(hMem)
		{
			LPIDA ida = (LPIDA)GlobalLock(hMem);
			if(!ida)
			{
				return TRUE;
			}

			if(ida->cidl == 0)
			{
				GlobalUnlock(hMem);
				return TRUE;
			}

			if(dropRoot)
			{
				ILFree(dropRoot);
			}
			if(dropFile)
			{
				ILFree(dropFile);
			}

			dropRoot = ILClone((LPITEMIDLIST)((UINT)ida + ida->aoffset[0]));
			dropFile = ILClone((LPITEMIDLIST)((UINT)ida + ida->aoffset[1]));

			GlobalUnlock(hMem);

			CString path = PIDLToString(dropFile, dropRoot, SHGDN_FORPARSING);
			CString ext = path.Mid(path.ReverseFind(L'.')).MakeLower();

			CDIB icon;
			if((ext == L".ico") || (ext == L".icon"))
			{
				Icons::GetIcon(path, &icon);
			}
			else
			{
				icon.Load(path);
			}
			dropIsIcon = icon.Ready();
			if(dropIsIcon)
			{
				dwEffect = DROPEFFECT_MOVE;
			}
		}
	}
	if(userDropTargetHelper)
	{
		IDataObject *dataObject = pDataObject->GetIDataObject(FALSE);
		dropTargetHelper->DragEnter(pWnd->GetSafeHwnd(), dataObject, &point, dwEffect);
	}
	return dwEffect;
}
	
DROPEFFECT CEditIcon::OnDragOver(CWnd*, COleDataObject*, DWORD, CPoint point)
{
	DROPEFFECT dwEffect = DROPEFFECT_NONE;
	if(dropIsIcon)
	{
		CRect r;
		GetClientRect(&r);

		r.left += WINDOW_BORDER_LEFT + 20;
		r.top += WINDOW_BORDER_TOP + CAPTION_HEIGHT + 30;
		r.right = r.left + 200;
		r.bottom = r.top + 200;

		if(r.PtInRect(point))
		{
			dwEffect = DROPEFFECT_MOVE;
		}
	}
	if(userDropTargetHelper)
	{
		dropTargetHelper->DragOver(&point, dwEffect);
	}
	return dwEffect;
}

BOOL CEditIcon::OnDrop(CWnd*, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	dropEffect = DROPEFFECT_MOVE;
	
	icon->FreeImage();

	iconPath = PIDLToString(dropFile, dropRoot, SHGDN_FORPARSING);
	CString ext = iconPath.Mid(iconPath.ReverseFind(L'.')).MakeLower();
	if((ext == L".ico") || (ext == L".icon"))
	{
		Icons::GetIcon(iconPath, icon);
	}
	else
	{
		icon->Load(iconPath);
	}

	Draw();
	RedrawWindow();

	if(userDropTargetHelper)
	{
		IDataObject *dataObject = pDataObject->GetIDataObject(FALSE);
		dropTargetHelper->Drop(dataObject, &point, dropEffect);
	}
	return TRUE;
}
	
void CEditIcon::OnDragLeave(CWnd*)
{
	if(userDropTargetHelper)
	{
		dropTargetHelper->DragLeave();
	}
}

void CEditIcon::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CFrameWnd::OnLButtonDblClk(nFlags, point);

	if(icon->Ready())
	{
		float k = min(200.0f / icon->Width(), 200.0f / icon->Height());

		CRect rect;
		rect.left = (int)(20 + (200 - icon->Width() * k) / 2);
		rect.top = (int)(CAPTION_HEIGHT + 30 + (200 - icon->Height() * k) / 2);
		rect.right = rect.left + (int)(icon->Width() * k);
		rect.bottom = rect.top + (int)(icon->Height() * k);

		if(rect.PtInRect(point))
		{
			OpenFileDialog();
		}
	}
}

void CEditIcon::OnLButtonDown(UINT nFlags, CPoint point)
{
	CFrameWnd::OnLButtonDown(nFlags, point);
	SetCapture();
	if(icon->Ready())
	{
		float k = min(200.0f / icon->Width(), 200.0f / icon->Height());

		CRect rect;
		rect.left = (int)(20 + (200 - icon->Width() * k) / 2);
		rect.top = (int)(CAPTION_HEIGHT + 30 + (200 - icon->Height() * k) / 2);
		rect.right = rect.left + (int)(icon->Width() * k);
		rect.bottom = rect.top + (int)(icon->Height() * k);

		mouseDown = (rect.PtInRect(point) == TRUE);
		mousePt = point;
	}
}

void CEditIcon::OnLButtonUp(UINT nFlags, CPoint point)
{
	CFrameWnd::OnLButtonUp(nFlags, point);
	ReleaseCapture();
	mouseDown = false;
}

void CEditIcon::OnMouseMove(UINT nFlags, CPoint point)
{
	CFrameWnd::OnMouseMove(nFlags, point);

	if(mouseDown && ((abs(point.x - mousePt.x) > 10) || (abs(point.y - mousePt.y) > 10)))
	{
		ReleaseCapture();
		mouseDown = false;

		drawIcon = false;
		Draw();
		RedrawWindow();

		DragIcon(point);

		if(notifer)
		{
			GetCursorPos(&point);
			if(WindowFromPoint(point) != this)
			{
				notifer->SendMessage(WM_CONTROLNOTIFY, MAKEWPARAM(nID, 2));
			}
		}

		drawIcon = true;
		Draw();
		RedrawWindow();
	}
}

void CEditIcon::DragIcon(CPoint point)
{
	HGLOBAL hMem = GlobalAlloc(GHND | GMEM_DDESHARE, 4);
	if(hMem == 0)
	{
		return;
	}

	COleDataSourceEx dropData;
	dropData.CacheGlobalData((CLIPFORMAT)RegisterClipboardFormat(L"UNKNOWN"), hMem);
	CComPtr<IDragSourceHelper> dragSourceHelper;

	HRESULT hRes = CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_ALL, IID_IDragSourceHelper, (void**)&dragSourceHelper);
	if(SUCCEEDED(hRes))
	{
		float k = min(200.0f / icon->Width(), 200.0f / icon->Height());

		RectF rx;
		rx.X = 0;
		rx.Y = 0;
		rx.Width = icon->Width() * k;
		rx.Height = icon->Height() * k;

		CDIB tmp;
		tmp.Resize((int)rx.Width, (int)rx.Height);

		Graphics g(tmp.dc);
		g.SetSmoothingMode(SmoothingModeNone);
		g.SetCompositingMode(CompositingModeSourceCopy);
		g.DrawImage(icon->bmp, rx);

		HBITMAP bmp = CreateCompatibleBitmap(tmp.dc, tmp.Width(), tmp.Height());
		HDC dc = CreateCompatibleDC(tmp.dc);
		HBITMAP oldBmp = (HBITMAP)SelectObject(dc, bmp);
		BitBlt(dc, 0, 0, tmp.Width(), tmp.Height(), tmp.dc, 0, 0, SRCCOPY);
		SelectObject(dc, oldBmp);

		point.x -= (20 + (200 - tmp.Width()) / 2);
		point.y -= (CAPTION_HEIGHT + 30 + (200 - tmp.Height()) / 2);

		SHDRAGIMAGE info;
		info.sizeDragImage.cx = tmp.Width();
		info.sizeDragImage.cy = tmp.Height();
		info.ptOffset = point;
		info.hbmpDragImage = bmp;
		info.crColorKey = 0xa9b8c7;

		hRes = dragSourceHelper->InitializeFromBitmap(&info, (IDataObject*)dropData.GetInterface(&IID_IDataObject));
		if(FAILED(hRes))
		{
			DeleteObject(info.hbmpDragImage);
		}
	}
	dropData.DoDragDrop(DROPEFFECT_MOVE);
	GlobalFree(hMem);
}
