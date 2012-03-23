#include "items.h"

using namespace Gdiplus;
using namespace ShellIO;

BEGIN_MESSAGE_MAP(CDockItemLayer, CFrameWnd)
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

CDockItemLayer::CDockItemLayer()
{
	state = 0;

	dib = new CDIB();
	image = new CDIB();

	CreateEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW, NULL, L"XWindows Dock", WS_POPUP, CRect(0, 0, 0, 0), NULL, NULL);
}

CDockItemLayer::~CDockItemLayer()
{
	delete dib;
	delete image;
}

void CDockItemLayer::LayerDraw(CDIB *dib)
{
	if(!dib)
	{
		dib = CDockItemLayer::dib;
	}
	if(!dib->Ready())
	{
		return;
	}
	if(state & StateFlagDropOver)
	{
		DIB_ARGB *p = (DIB_ARGB*)dib->scan0;
		int size = dib->Width() * dib->Height();
		for(int y = 0; y < size; y++, p++)
		{
			p->r = (unsigned char)(p->r * 0.4f);
			p->g = (unsigned char)(p->g * 0.4f);
			p->b = (unsigned char)(p->b * 0.4f);
		}
	}
	else
	if(state & StateFlagPressed)
	{
		DIB_ARGB *p = (DIB_ARGB*)dib->scan0;
		int size = dib->Width() * dib->Height();
		for(int y = 0; y < size; y++, p++)
		{
			p->r = (unsigned char)(p->r * 0.4f);
			p->g = (unsigned char)(p->g * 0.4f);
			p->b = (unsigned char)(p->b * 0.4f);
		}
	}
}
	
void CDockItemLayer::LayerUpdate(CDIB *dib)
{
	if(!dib)
	{
		dib = CDockItemLayer::dib;
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

void CDockItemLayer::Event(unsigned short dockItemEvent)
{
	HWND hWnd = ::FindWindow(L"XWindowsDockClass", NULL);
	if(hWnd)
	{
		HANDLE dockItem = GetProp(m_hWnd, L"DockItem");
		if(dockItem)
		{
			::SendMessage(hWnd, WM_DOCKITEMNOTIFY, MAKEWPARAM(dockItemEvent, 0), (LPARAM)dockItem);
		}
	}
}

void CDockItemLayer::OnWindowPosChanged(WINDOWPOS *lpwndpos)
{
	CFrameWnd::OnWindowPosChanged(lpwndpos);

	if((lpwndpos->flags & SWP_NOSIZE) == SWP_NOSIZE)
	{
		return;
	}

	LayerDraw();
	LayerUpdate();
}

void CDockItemLayer::OnClose()
{
}

BEGIN_MESSAGE_MAP(CDockItemImage, CDockItemLayer)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_DESTROY()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

CDockItemImage::CDockItemImage()
{
	mouseDown = false;
	imageOverlay = new CDIB();
}

CDockItemImage::~CDockItemImage()
{
	delete imageOverlay;
}

void CDockItemImage::OnDestroy()
{
	CDockItem *item = (CDockItem*)GetProp(m_hWnd, L"DockItem");
	if(item)
	{
		switch(item->type)
		{
		case DockItemTypeODDocklet:
			{
				item->docklet->OnDestroy(m_hWnd);
			}
			break;

		case DockItemTypeDocklet:
			{
				item->plugin->PluginEvent((XWDId)item, item->pluginData, XWDEventDestroy);
				item->plugin->PluginDeinitialize((XWDId)item, item->pluginData);
			}
			break;
		}
	}
	CDockItemLayer::OnDestroy();
}

LRESULT CDockItemImage::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	CDockItem *item = (CDockItem*)GetProp(m_hWnd, L"DockItem");
	if(item && (item->type == DockItemTypeODDocklet) && item->docklet &&
		!(item->animationFlags & animationFlagPoof))
	{
		item->docklet->OnProcessMessage(m_hWnd, message, wParam, lParam);
	}
	return CDockItemLayer::WindowProc(message, wParam, lParam);
}

BOOL CDockItemImage::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CDockItem *item = (CDockItem*)GetProp(m_hWnd, L"DockItem");
	if(item && (item->type == DockItemTypeSeparator))
	{
		switch(item->dockPosition)
		{
		case DockPositionTop:
		case DockPositionBottom:
			{
				::SetCursor(AfxGetApp()->LoadCursor(MAKEINTRESOURCE(32764)));
			}
			break;

		case DockPositionLeft:
		case DockPositionRight:
			{
				::SetCursor(AfxGetApp()->LoadCursor(MAKEINTRESOURCE(32765)));
			}
			break;
		}
		return TRUE;
	}
	return CDockItemLayer::OnSetCursor(pWnd, nHitTest, message);
}

void CDockItemImage::LayerDraw(CDIB *dib)
{
	if(!dib)
	{
		dib = CDockItemLayer::dib;
	}

	CRect rect;
	GetWindowRect(&rect);

	dib->Resize(rect.Width(), rect.Height());
	if(!dib->Ready())
	{
		return;
	}

	DIB_ARGB *p = dib->scan0;
	int size = dib->Width() * dib->Height();
	for(int i = 0; i < size; i++, p++)
	{
		p->a = 1;
	}

	if(image->Ready())
	{
		Graphics g(dib->dc);
		g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

		CDockItem *item = (CDockItem*)GetProp(m_hWnd, L"DockItem");
		if(item && (item->type == DockItemTypeSeparator))
		{
			Bitmap tmp(image->Width(), image->Height(), image->Width() * 4, PixelFormat32bppARGB, (BYTE*)image->scan0);
			switch(item->dockPosition)
			{
			case DockPositionLeft:
				{
					tmp.RotateFlip(Rotate90FlipNone);
				}
				break;

			case DockPositionRight:
				{
					tmp.RotateFlip(Rotate270FlipY);
				}
				break;

			case DockPositionTop:
				{
					tmp.RotateFlip(Rotate180FlipX);
				}
				break;
			}
			g.DrawImage(&tmp, RectF(0, 0, (REAL)dib->Width(), (REAL)dib->Height()));
		}
		else
		if(item)
		{
			float k = min((float)dib->Width() / image->Width(), (float)dib->Height() / image->Height());

			RectF rf;
			rf.Width = image->Width() * k;
			rf.Height = image->Height() * k;
			rf.X = (dib->Width() - rf.Width) / 2;
			rf.Y = (dib->Height() - rf.Height) / 2;

			if(item->dockMode == DockMode3D)
			{
				rf.Y = item->imageShadowSize3d + (dib->Height() - item->imageShadowSize3d - rf.Height) / 2;
			}

			/*DIB_ARGB *p = (DIB_ARGB*)dib->scan0;
			int size = dib->Width() * dib->Height();
			for(int y = 0; y < size; y++, p++)
			{
				p->r = 255;
				p->g = 0;
				p->b = 0;
				p->a = 255;
			}*/

			if((item->dockMode == DockMode3D) && item->imageShadowSize3d)
			{
				CDIB tmp;
				tmp.Resize((int)rf.Width, (int)rf.Height);
				if(tmp.Ready())
				{
					Graphics g2(tmp.bmp);
					g2.DrawImage(image->bmp, RectF((REAL)item->imageShadowSize3d, (REAL)item->imageShadowSize3d, 
						rf.Width - item->imageShadowSize3d * 2, rf.Height - item->imageShadowSize3d * 2));

					if(imageOverlay->Ready())
					{
						float k = min((float)dib->Width() / imageOverlay->Width(), (float)dib->Height() / imageOverlay->Height());

						RectF rf;
						rf.Width = imageOverlay->Width() * k;
						rf.Height = imageOverlay->Height() * k;
						rf.X = (dib->Width() - rf.Width) / 2;
						rf.Y = (dib->Height() - rf.Height) / 2;

						if(item->dockMode == DockMode3D)
						{
							rf.Y = item->imageShadowSize3d + (dib->Height() - item->imageShadowSize3d - rf.Height) / 2;
						}

						g2.DrawImage(image->bmp, RectF((REAL)item->imageShadowSize3d, (REAL)item->imageShadowSize3d, 
							rf.Width - item->imageShadowSize3d * 2, rf.Height - item->imageShadowSize3d * 2));
					}

					DIB_ARGB *p = (DIB_ARGB*)tmp.scan0;
					int size = tmp.Width() * tmp.Height();
					for(int y = 0; y < size; y++, p++)
					{
						p->r = 0;
						p->g = 0;
						p->b = 0;
						p->a = (unsigned char)(p->a * 0.6f);
					}

					CDIB tmp2;
					tmp2.Assign(&tmp);

					tmp2.AlphaBlend(tmp.Rect(), 160);
					tmp2.Blur(tmp2.Rect(), CRect(0, 0, 0, 0), 3);

					g.DrawImage(tmp2.bmp, RectF(
						rf.X - item->imageShadowSize3d * 0.6f, 
						rf.Y + rf.Height * 0.5f, 
						rf.Width * 0.9f, rf.Height * 0.5f));

					tmp.Blur(tmp.Rect(), CRect(0, 0, 0, 0), item->imageShadowSize3d);

					g.DrawImage(tmp.bmp, RectF(rf.X, rf.Y - item->imageShadowSize3d, rf.Width, rf.Height));
				}
			}

			g.DrawImage(image->bmp, rf);

			if(imageOverlay->Ready())
			{
				float k = min((float)dib->Width() / imageOverlay->Width(), (float)dib->Height() / imageOverlay->Height());

				RectF rf;
				rf.Width = imageOverlay->Width() * k;
				rf.Height = imageOverlay->Height() * k;
				rf.X = (dib->Width() - rf.Width) / 2;
				rf.Y = (dib->Height() - rf.Height) / 2;

				if(item->dockMode == DockMode3D)
				{
					rf.Y = item->imageShadowSize3d + (dib->Height() - item->imageShadowSize3d - rf.Height) / 2;
				}
				g.DrawImage(imageOverlay->bmp, rf);
			}
		}
	}

	CDockItemLayer::LayerDraw(dib);
}

BOOL CDockItemImage::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	BOOL bReturn = CDockItemLayer::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	if(!bReturn && (nCode == CN_UPDATE_COMMAND_UI))
	{
		CCmdUI *pUI = (CCmdUI*)pExtra;
		pUI->Enable();
		return TRUE;
	}
	return bReturn;
}

void CDockItemImage::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDockItemLayer::OnLButtonDown(nFlags, point);

	CDockItem *item = (CDockItem*)GetProp(m_hWnd, L"DockItem");
	if(!item || (item->animationFlags & animationFlagPoof))
	{
		return;
	}

	SetCapture();
	mouseDown = true;
	Event(DockItemLMouseDown);
}

void CDockItemImage::OnLButtonUp(UINT nFlags, CPoint point)
{
	CDockItemLayer::OnLButtonUp(nFlags, point);

	CDockItem *item = (CDockItem*)GetProp(m_hWnd, L"DockItem");
	if(!item || (item->animationFlags & animationFlagPoof))
	{
		return;
	}

	if(mouseDown)
	{
		ReleaseCapture();
		mouseDown = false;
		Event(DockItemLMouseUp);

		CRect rect;
		GetWindowRect(&rect);
		ClientToScreen(&point);
		if(rect.PtInRect(point))
		{
			Event(DockItemLClick);
		}
	}
}

void CDockItemImage::OnRButtonDown(UINT nFlags, CPoint point)
{
	CDockItemLayer::OnRButtonDown(nFlags, point);

	CDockItem *item = (CDockItem*)GetProp(m_hWnd, L"DockItem");
	if(item && (item->animationFlags & animationFlagPoof))
	{
		return;
	}

	Event(DockItemRMouseDown);
}

void CDockItemImage::OnRButtonUp(UINT nFlags, CPoint point)
{
	CDockItemLayer::OnRButtonUp(nFlags, point);

	CDockItem *item = (CDockItem*)GetProp(m_hWnd, L"DockItem");
	if(item && (item->animationFlags & animationFlagPoof))
	{
		return;
	}

	Event(DockItemRMouseUp);
}

CDockItemReflection::CDockItemReflection()
{
	imageOverlay = new CDIB();
	paint = true;
}

CDockItemReflection::~CDockItemReflection()
{
	delete imageOverlay;
}

void CDockItemReflection::LayerDraw(CDIB *dib)
{
	if(!dib)
	{
		dib = CDockItemLayer::dib;
	}

	CRect rect;
	GetWindowRect(&rect);

	dib->Resize(rect.Width(), rect.Height());
	if(!dib->Ready())
	{
		return;
	}

	if(image->Ready())
	{
		Graphics g(dib->dc);
		g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

		float k = min((float)dib->Width() / image->Width(), (float)dib->Width() / image->Height());

		RectF rf;
		rf.Width = image->Width() * k;
		rf.Height = image->Height() * k;
		rf.X = (dib->Width() - rf.Width) / 2;
		rf.Y = dib->Height() - rf.Height;

		CDockItem *item = (CDockItem*)GetProp(m_hWnd, L"DockItem");

		//g.DrawImage(image->bmp, rf, 0, item == NULL ? 0 : (REAL)item->reflectionSkip, 
		//	(REAL)image->Width(), (REAL)image->Height(), UnitPixel);

		if(item)
		{
			rf.Y += item->reflectionSkipTop;
		}

		g.DrawImage(image->bmp, rf);

		if(imageOverlay->Ready())
		{
			float k = min((float)dib->Width() / imageOverlay->Width(), (float)dib->Width() / imageOverlay->Height());

			RectF rf;
			rf.Width = imageOverlay->Width() * k;
			rf.Height = imageOverlay->Height() * k;
			rf.X = (dib->Width() - rf.Width) / 2;
			rf.Y = dib->Height() - rf.Height;

			g.DrawImage(imageOverlay->bmp, rf);
		}

		if(paint)
		{
			dib->ReflectVertical();
		
			//dib->Blur(dib->Rect(), CRect(0, 0, 0, 0), 2);

			unsigned char alphaOld = item == NULL ? 150 : item->reflectionOpacity;

			for(int y = 0; y < dib->Height(); y++)
			{
				unsigned char alpha = alphaOld;
				if(item)
				{
					alpha -= (unsigned char)(item->reflectionOpacityFactor / 100.0f * alphaOld * y / dib->Height());
				}
				DIB_AARGB *p = (DIB_AARGB*)dib->Pixels(0, y);
				for(int x = 0; x < dib->Width(); x++)
				{
					unsigned char a = p[x]->a * alpha / 255;
					float k = (float)a / max(p[x]->a, 1);
					p[x]->r = (unsigned char)(p[x]->r * k);
					p[x]->g = (unsigned char)(p[x]->g * k);
					p[x]->b = (unsigned char)(p[x]->b * k);
					p[x]->a = a;
				}
			}
		}
	}

	CDockItemLayer::LayerDraw(dib);
}

CDockItemIndicator::CDockItemIndicator()
{
}

CDockItemIndicator::~CDockItemIndicator()
{
}

void CDockItemIndicator::LayerDraw(CDIB *dib)
{
	if(!dib)
	{
		dib = CDockItemLayer::dib;
	}

	CRect rect;
	GetWindowRect(&rect);

	dib->Resize(rect.Width(), rect.Height());
	if(!dib->Ready())
	{
		return;
	}

	if(image->Ready())
	{
		Graphics g(dib->dc);

		CDockItem *item = (CDockItem*)GetProp(m_hWnd, L"DockItem");
		if(item)
		{
			Bitmap tmp(image->Width(), image->Height(), image->Width() * 4, PixelFormat32bppARGB, (BYTE*)image->scan0);
			switch(item->dockPosition)
			{
			case DockPositionLeft:
				{
					tmp.RotateFlip(Rotate90FlipNone);
				}
				break;

			case DockPositionRight:
				{
					tmp.RotateFlip(Rotate270FlipY);
				}
				break;

			case DockPositionTop:
				{
					tmp.RotateFlip(Rotate180FlipX);
				}
				break;
			}
			g.DrawImage(&tmp, RectF(0, 0, (REAL)dib->Width(), (REAL)dib->Height()));
		}
		else
		{
			g.DrawImage(image->bmp, RectF(0, 0, (REAL)dib->Width(), (REAL)dib->Height()));
		}
	}
	CDockItemLayer::LayerDraw(dib);
}

BEGIN_MESSAGE_MAP(CDockItem, CFrameWnd)
	ON_WM_TIMER()
END_MESSAGE_MAP()

#define DOCK_ITEM_TIMER_MOVE_DELAY 300
#define DOCK_ITEM_TIMER_POOF_DELAY 600
#define DOCK_ITEM_TIMER_REFLECTIONSHOW_DELAY 320
#define DOCK_ITEM_TIMER_INDICATORHIDE_DELAY 700
#define DOCK_ITEM_TIMER_BOUNCE_NORMAL_DELAY 700
#define DOCK_ITEM_TIMER_BOUNCE_ATTENTION_DELAY 500

#define DOCK_ITEM_TIMER 1
#define DOCK_ITEM_TIMER_BOUNCE_ATTENTION 2

#define DOCK_ITEM_BOUNCE_HEIGHT 0.9f

CDockItem::CDockItem()
{
	type = DockItemTypeIcon;
	showAs = DockItemShowAsNormal;
	exec = DockItemExecIfNotRun;
	iconIndex = 0;
	pluginCanDrop = false;

	identificator = 0;
	imageShadowSize3d = 0;

	reflectionSize = 0;
	reflectionSkipTop = 0;
	reflectionSkipBottom = 0;
	reflectionOffset = 0;
	reflectionOpacity = 100;
	iconShadowEnabled = false;
	dockMode = DockMode2D;

	bounceCancelOnFocus = false;

	addTmp = new CDIB();

	resizeFlag = 1.0f;
	animationFlags = 0;

	poof = NULL;
	docklet = NULL;

	reflection = new CDockItemReflection();
	image = new CDockItemImage();
	indicator = new CDockItemIndicator();

	SetProp(image->m_hWnd, L"DockItem", (HANDLE)this);
	SetWindowLong(image->m_hWnd, GWL_HWNDPARENT, (LONG)indicator->m_hWnd);

	SetProp(indicator->m_hWnd, L"DockItem", (HANDLE)this);
	SetWindowLong(indicator->m_hWnd, GWL_HWNDPARENT, (LONG)reflection->m_hWnd);

	SetProp(reflection->m_hWnd, L"DockItem", (HANDLE)this);

	CreateEx(WS_EX_TOOLWINDOW, NULL, L"XWindows Dock", WS_POPUP, CRect(0, 0, 0, 0), NULL, NULL);
}

CDockItem::~CDockItem()
{
	POSITION p = folderWatcherList.GetHeadPosition();
	while (p)
	{
		FolderWatcher::RemoveFolder(folderWatcherList.GetAt(p));
		folderWatcherList.GetNext(p);
	}
	folderWatcherList.RemoveAll();
	delete addTmp;
}

void CDockItem::Event(unsigned short dockItemEvent)
{
	image->Event(dockItemEvent);
}

void CDockItem::SetBottomWindow(CWnd *wnd)
{
	SetWindowLong(reflection->m_hWnd, GWL_HWNDPARENT, (LONG)wnd->m_hWnd);
}

void CDockItem::Show(bool visible)
{
	DWORD show = visible ? SW_SHOW/*NOACTIVATE*/ : SW_HIDE;

	reflection->ShowWindow(((dockMode == DockMode3D) && (type != DockItemTypeSeparator)) ? show : SW_HIDE);
	image->ShowWindow(show);
}

void CDockItem::TopMost(bool topMost)
{
	const CWnd *wnd = topMost ? &wndTopMost : &wndNoTopMost;
	image->SetWindowPos(wnd, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	indicator->SetWindowPos(wnd, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	reflection->SetWindowPos(wnd, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
}

void CDockItem::NotificationUpdatePositions()
{
	CRect rect;
	image->GetWindowRect(&rect);

	/*if ((dockMode == DockMode3D) && iconShadowEnabled)
	{
		imageShadowSize3d = 6 * rect.Height() / 42;
		rect.top -= imageShadowSize3d;
	}*/

	POSITION p = notifications.GetHeadPosition();
	while(p)
	{
		CNotification *notification = notifications.GetAt(p);

		CRect notifyRect;
		notification->GetWindowRect(&notifyRect);

		switch(notification->position)
		{
		case NotificationPositionLeftTop:
			{
				notifyRect.MoveToXY(rect.left - (int)(notifyRect.Width() * 0.2f), rect.top - (int)(notifyRect.Height() * 0.2f));
			}
			break;

		case NotificationPositionTopMiddle:
			{
				notifyRect.MoveToXY(rect.left + (rect.Width() - notifyRect.Width()) / 2, rect.top - (int)(notifyRect.Height() * 0.2f));
			}
			break;

		case NotificationPositionRightTop:
			{
				notifyRect.MoveToXY(rect.right - (int)(notifyRect.Width() * 0.8f), rect.top - (int)(notifyRect.Height() * 0.2f));
			}
			break;

		case NotificationPositionRightMiddle:
			{
				notifyRect.MoveToXY(rect.right - (int)(notifyRect.Width() * 0.8f), rect.top + (rect.Height() - notifyRect.Height()) / 2);
			}
			break;

		case NotificationPositionRightBottom:
			{
				notifyRect.MoveToXY(rect.right - (int)(notifyRect.Width() * 0.8f), rect.bottom - (int)(notifyRect.Height() * 0.8f));
			}
			break;

		case NotificationPositionBottomMiddle:
			{
				notifyRect.MoveToXY(rect.left + (rect.Width() - notifyRect.Width()) / 2, rect.bottom - (int)(notifyRect.Height() * 0.8f));
			}
			break;

		case NotificationPositionLeftBottom:
			{
				notifyRect.MoveToXY(rect.left - (int)(notifyRect.Width() * 0.2f), rect.bottom - (int)(notifyRect.Height() * 0.8f));
			}
			break;

		case NotificationPositionLeftMiddle:
			{
				notifyRect.MoveToXY(rect.left - (int)(notifyRect.Width() * 0.2f), rect.top + (rect.Height() - notifyRect.Height()) / 2);
			}
			break;
		}

		notification->SetWindowPos(&wndTop, notifyRect.left, notifyRect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);

		notifications.GetNext(p);
	}
}

bool CDockItem::Move(int x, int y, bool animation)
{
	if(animation)
	{
		if(!((animationFlags & animationFlagMoving) && (moveNextPt.x == x) && (moveNextPt.y == y)) &&
			!((rect.left == x) && (rect.top == y)))
		{
			movePt.x = rect.left;
			movePt.y = rect.top + ((dockMode == DockMode3D) && (type != DockItemTypeSeparator) ? imageShadowSize3d : 0);
			moveNextPt.x = x;
			moveNextPt.y = y;
			moveStartAt = GetTickCount();
			animationFlags |= animationFlagMoving;
			SetTimer(DOCK_ITEM_TIMER, 10, NULL);
			return true;
		}
	}
	else
	//if(!((animationFlags & animationFlagMoving) && (rect.left == moveNextPt.x) && (rect.top == moveNextPt.y)))
	{
		rect.MoveToXY(x, y);

		if(dockMode == DockMode3D)
		{
			if(type != DockItemTypeSeparator)
			{
				rect.OffsetRect(0, -imageShadowSize3d);
			}
			if(animationFlags & animationFlagBounce)
			{
				switch(bounceType)
				{
				case DockItemBounceNormal:
					{
						DWORD sb = GetTickCount() - bounceStartAt;
						if(sb < DOCK_ITEM_TIMER_BOUNCE_NORMAL_DELAY / 2)
						{
							float k = (float)sb / (DOCK_ITEM_TIMER_BOUNCE_NORMAL_DELAY / 2);
							if(k > 1) k = 1;
							int d = (int)((bouncePt.y - bounceNextPt.y) * k);
							reflection->SetWindowPos(&wndTop, 
								bouncePt.x, bouncePt.y + rect.Height() - reflectionOffset - imageShadowSize3d + d, 
								bounceReflectionSize.cx,
								max(bounceReflectionSize.cy - d, 0), 
								SWP_NOZORDER | SWP_NOACTIVATE);
						}
						else
						{
							float k = (float)(sb - DOCK_ITEM_TIMER_BOUNCE_NORMAL_DELAY / 2) / (DOCK_ITEM_TIMER_BOUNCE_NORMAL_DELAY / 2);
							if(k > 1) k = 1;
							k = 1 - cos(k * PI / 2);
							int d = (int)((bouncePt.y - bounceNextPt.y) * (1 - k));
							reflection->SetWindowPos(&wndTop, 
								bouncePt.x, bouncePt.y + rect.Height() - reflectionOffset - imageShadowSize3d + d, 
								bounceReflectionSize.cx, 
								max(bounceReflectionSize.cy - d, 0),
								SWP_NOZORDER | SWP_NOACTIVATE);
						}
					}
					break;

				case DockItemBounceAttention:
					{
						DWORD sb = GetTickCount() - bounceStartAt;
						if(sb < bounceDelay / 2)
						{
							float k = (float)sb / (bounceDelay / 2);
							if(k > 1) k = 1;
							int d = (int)((bouncePt.y - bounceNextPt.y) * k);
							reflection->SetWindowPos(&wndTop, 
								bouncePt.x, bouncePt.y + rect.Height() - reflectionOffset - imageShadowSize3d + d, 
								bounceReflectionSize.cx,
								max(bounceReflectionSize.cy - d, 0), 
								SWP_NOZORDER | SWP_NOACTIVATE);
						}
						else
						{
							float k = (float)(sb - bounceDelay / 2) / (bounceDelay / 2);
							if(k > 1) k = 1;
							k = 1 - cos(k * PI / 2);
							int d = (int)((bouncePt.y - bounceNextPt.y) * (1 - k));
							reflection->SetWindowPos(&wndTop, 
								bouncePt.x, bouncePt.y + rect.Height() - reflectionOffset - imageShadowSize3d + d, 
								bounceReflectionSize.cx, 
								max(bounceReflectionSize.cy - d, 0),
								SWP_NOZORDER | SWP_NOACTIVATE);
						}
					}
					break;
				}
			}
			else
			if((animationFlags & animationFlagMoving) && (moveNextPt.y > movePt.y))
			{
				float k = (float)(GetTickCount() - moveStartAt) / DOCK_ITEM_TIMER_MOVE_DELAY;
				reflection->SetWindowPos(&wndTop, 
					rect.left,
					moveNextPt.y + rect.Height() - reflectionOffset - imageShadowSize3d + 
					(int)((moveNextPt.y - movePt.y) * (1 - k)),
					0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
			}
			else
			{
				reflection->SetWindowPos(&wndTop, rect.left, rect.bottom - reflectionOffset, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
			}
		}
		
		image->SetWindowPos(&wndTop, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);

		NotificationUpdatePositions();

		Event(DockItemMoving);	
		return true;
	}
	return false;
}

void CDockItem::Size(int width, int height)
{
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;
	Rect(rect);
}

void CDockItem::Rect(CRect rect)
{
	if((dockMode == DockMode3D) && (type != DockItemTypeSeparator))
	{
		imageShadowSize3d = (iconShadowEnabled ? (6 * rect.Height() / 42) : 0);
		rect.top -= imageShadowSize3d;

		reflection->SetWindowPos(&wndTop, rect.left, rect.bottom - reflectionOffset, rect.Width(), max(0, reflectionSize - reflectionSkipBottom), SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else
	{
		reflection->dib->FreeImage();
		reflection->SetWindowPos(&wndTop, rect.left, rect.bottom - reflectionOffset, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE);
	}
	CDockItem::rect = rect;

	image->SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

	NotificationUpdatePositions();

	Event(DockItemMoving);	
}

void CDockItem::Rect(int x, int y, int width, int height)
{
	Rect(CRect(x, y, x + width, y + height));
}

void CDockItem::OnTimer(UINT_PTR nIDEvent)
{
	CFrameWnd::OnTimer(nIDEvent);

	switch(nIDEvent)
	{
	case DOCK_ITEM_TIMER_BOUNCE_ATTENTION:
		{
			KillTimer(nIDEvent);
			switch(dockPosition)
			{
			case DockPositionBottom:
				{
					bounceNextPt.y = (int)(bouncePt.y - rect.Height());
				}
				break;

			case DockPositionTop:
				{
					bounceNextPt.y = (int)(bouncePt.y + rect.Height());
				}
				break;

			case DockPositionLeft:
				{
					bounceNextPt.x = (int)(bouncePt.x + rect.Width());
				}
				break;

			case DockPositionRight:
				{
					bounceNextPt.x = (int)(bouncePt.x - rect.Width());
				}
				break;
			}
			bounceCounter = 0;
			bounceDelay = DOCK_ITEM_TIMER_BOUNCE_ATTENTION_DELAY;
			bounceStartAt = GetTickCount();
		}
		break;

	case DOCK_ITEM_TIMER:
		{
			if(animationFlags & animationFlagPoof)
			{
				float k = (float)(GetTickCount() - poofStartAt) / DOCK_ITEM_TIMER_POOF_DELAY;
				if(k >= 1)
				{
					k = 1;
					animationFlags &= ~animationFlagPoof;
					if(animationFlags == 0)
					{
						KillTimer(nIDEvent);
					}
					Delete();
					return;
				}
				else
				{
					PoofDraw((int)(15 * k));
				}
			}
			if(animationFlags & animationFlagMoving)
			{
				float k = (float)(GetTickCount() - moveStartAt) / DOCK_ITEM_TIMER_MOVE_DELAY;
				if(k >= 1)
				{
					k = 1;
					animationFlags &= ~animationFlagMoving;
					if(animationFlags == 0)
					{
						KillTimer(nIDEvent);
					}
				}
				Move((int)(movePt.x * (1 - k) + moveNextPt.x * k), (int)(movePt.y * (1 - k) + moveNextPt.y * k));
			}
			if(animationFlags & animationFlagReflectionShow)
			{
				float k = (float)(GetTickCount() - reflectionShowStartAt) / DOCK_ITEM_TIMER_REFLECTIONSHOW_DELAY;
				if(k >= 1)
				{
					animationFlags &= ~animationFlagReflectionShow;
					if(animationFlags == 0)
					{
						KillTimer(nIDEvent);
					}
					reflection->LayerUpdate();
					delete reflectionTmp;
				}
				else
				{
					reflectionTmp->Assign(reflection->dib);
					reflectionTmp->AlphaBlend(reflectionTmp->Rect(), (unsigned char)(255 * k), DrawFlagsPaint);
					reflection->LayerUpdate(reflectionTmp);
				}
			}
			if(animationFlags & animationFlagIndicatorHide)
			{
				float k = 1 - (float)(GetTickCount() - indicatorShowStartAt) / DOCK_ITEM_TIMER_INDICATORHIDE_DELAY;
				if(k <= 0)
				{
					animationFlags &= ~animationFlagIndicatorHide;
					if(animationFlags == 0)
					{
						KillTimer(nIDEvent);
					}
					delete indicatorTmp;
					indicator->dib->FreeImage();
					indicator->ShowWindow(SW_HIDE);
				}
				else
				{
					indicatorTmp->Assign(indicator->dib);
					indicatorTmp->AlphaBlend(indicatorTmp->Rect(), (unsigned char)(255 * k), DrawFlagsPaint);
					indicator->LayerUpdate(indicatorTmp);
				}
			}
			if(animationFlags & animationFlagBounce)
			{
				switch(bounceType)
				{
				case DockItemBounceNormal:
					{
						DWORD sb = GetTickCount() - bounceStartAt;
						if(sb < DOCK_ITEM_TIMER_BOUNCE_NORMAL_DELAY / 2)
						{
							float k = (float)sb / (DOCK_ITEM_TIMER_BOUNCE_NORMAL_DELAY / 2);
							if(k > 1) k = 1;
							Move((int)(bouncePt.x * (1 - k) + bounceNextPt.x * k), (int)(bouncePt.y * (1 - k) + bounceNextPt.y * k));
						}
						else
						{
							float k = (float)(sb - DOCK_ITEM_TIMER_BOUNCE_NORMAL_DELAY / 2) / (DOCK_ITEM_TIMER_BOUNCE_NORMAL_DELAY / 2);
							if(k > 1) k = 1;
							k = 1 - cos(k * PI / 2);
							if(k >= 0.1f)
							{
								Move((int)(bouncePt.x * k + bounceNextPt.x * (1 - k)), (int)(bouncePt.y * k + bounceNextPt.y * (1 - k)));
							}
							if(k == 1)
							{
								animationFlags &= ~animationFlagBounce;
								if(animationFlags == 0)
								{
									KillTimer(nIDEvent);
								}
							}
						}
					}
					break;

				case DockItemBounceAttention:
					{
						if(bounceCounter == 3) break; // waiting for DOCK_ITEM_TIMER_BOUNCE_ATTENTION
						DWORD sb = GetTickCount() - bounceStartAt;
						if(sb < bounceDelay / 2)
						{
							float k = (float)sb / (bounceDelay / 2);
							if(k > 1) k = 1;
							Move((int)(bouncePt.x * (1 - k) + bounceNextPt.x * k), (int)(bouncePt.y * (1 - k) + bounceNextPt.y * k));
						}
						else
						{
							float k = (float)(sb - bounceDelay / 2) / (bounceDelay / 2);
							if(k > 1) k = 1;
							k = 1 - cos(k * PI / 2);
							Move((int)(bouncePt.x * k + bounceNextPt.x * (1 - k)), (int)(bouncePt.y * k + bounceNextPt.y * (1 - k)));
							if(k == 1)
							{
								if(bounceCounter == 2)
								{
									if(animationFlags & animationFlagBounceAttention)
									{
										if(animationFlags & animationFlagBounceAttentionCancel)
										{
											animationFlags &= ~animationFlagBounceAttentionCancel;
											animationFlags &= ~animationFlagBounceAttention;
											animationFlags &= ~animationFlagBounce;
											if(animationFlags == 0)
											{
												KillTimer(nIDEvent);
											}
										}
										else
										{
											bounceCountNow++;
											if((bounceCount > 0) && (bounceCount == bounceCountNow))
											{
												animationFlags &= ~animationFlagBounceAttention;
												animationFlags &= ~animationFlagBounce;
												if(animationFlags == 0)
												{
													KillTimer(nIDEvent);
												}
											}
											else
											{
												bounceCounter++;
												SetTimer(DOCK_ITEM_TIMER_BOUNCE_ATTENTION, 400, NULL);
											}
										}
									}
									else
									{
										animationFlags &= ~animationFlagBounce;
										if(animationFlags == 0)
										{
											KillTimer(nIDEvent);
										}
									}
								}
								else
								{
									bounceCounter++;
									float d = 1 - 0.7f * sin((float)bounceCounter / 2 * PI / 2);
									switch(dockPosition)
									{
									case DockPositionBottom:
										{
											bounceNextPt.y = (int)(bouncePt.y - rect.Height() * DOCK_ITEM_BOUNCE_HEIGHT * d);
										}
										break;

									case DockPositionTop:
										{
											bounceNextPt.y = (int)(bouncePt.y + rect.Height() * DOCK_ITEM_BOUNCE_HEIGHT * d);
										}
										break;

									case DockPositionLeft:
										{
											bounceNextPt.x = (int)(bouncePt.x + rect.Width() * DOCK_ITEM_BOUNCE_HEIGHT * d);
										}
										break;

									case DockPositionRight:
										{
											bounceNextPt.x = (int)(bouncePt.x - rect.Width() * DOCK_ITEM_BOUNCE_HEIGHT * d);
										}
										break;
									}
									bounceDelay = (DWORD)(DOCK_ITEM_TIMER_BOUNCE_ATTENTION_DELAY * d);
									bounceStartAt = GetTickCount();
								}
							}
						}
					}
					break;
				}
			}
		}
		break;

	default:
		{
			switch(type)
			{
			case DockItemTypeDocklet:
				{
					plugin->PluginEvent((XWDId)this, pluginData, XWDEventTimer, nIDEvent);
				}
				break;
			}
		}
		break;
	}
}

void CDockItem::Delete()
{
	XWDPRemovePlugin(publicPluginInfo);

	image->DestroyWindow();
	reflection->DestroyWindow();
	DestroyWindow();
}

bool CDockItem::LoadImage()
{
	image->image->FreeImage();
	if(iconIndex == 0)
	{
		// may Gdiplus can load it as image, but not .ico file, we load it by ourselvs
		CString ext = icon.Mid(icon.ReverseFind(L'.')).MakeLower();
		if((ext != L".ico") && (ext != L".icon"))
		{
			image->image->Load(icon);
		}
		if(!image->image->Ready())
		{
			// check whether file exists
			DWORD mode = SetErrorMode(SEM_FAILCRITICALERRORS);
			DWORD attr = GetFileAttributes(icon.GetBuffer());
			SetErrorMode(mode);
			if(attr != INVALID_FILE_ATTRIBUTES)
			{
				Icons::GetIcon(icon, image->image);
			}
		}
	}
	else
	{
		CString res;
		res.Format(L"#%d", iconIndex);
		HICON hIcon = Icons::GetIcon(icon, res);
		if(hIcon)
		{
			Icons::GetDIBFromIcon(hIcon, image->image);
			DeleteObject(hIcon);
		}
	}
	if(!image->image->Ready())
	{
		// try load icon for path, like for icon
		CString ext = path.Mid(path.ReverseFind(L'.')).MakeLower();
		if((ext != L".ico") && (ext != L".icon"))
		{
			image->image->Load(path);
		}
		// anyway, try to get an icon from path
		if(!image->image->Ready())
		{
			Icons::GetIcon(path, image->image);
		}
	}
	if(type != DockItemTypeSeparator)
	{
		reflection->image->Assign(image->image);
	}
	return image->image->Ready();
}

bool CDockItem::LoadPath(CString path)
{
	/*LinkInfo linkInfo;
	if(GetLinkInfo(path, &linkInfo, LI_DESCRIPTION | LI_PATH | LI_ARGUMENTS | LI_WORKDIRECTORY | LI_ICONLOCATION))
	{
		CDockItem::path = linkInfo.path;
		arguments = linkInfo.arguments;
		workDirectory = linkInfo.workDirectory;
		icon = linkInfo.iconLocation;
		iconIndex = linkInfo.iconIndex;
	}
	else
	{*/
		CDockItem::path = path;
		workDirectory = path.Mid(path.ReverseFind(L'\\') + 1);
		arguments.Empty();
		icon.Empty();
		iconIndex = 0;
	//}
	return LoadImage();
}

void CDockItem::Poof()
{
	if(!poof || !image->dib->Ready())
	{
		DestroyWindow();
		return;
	}
	image->state = 0;
	poofStartAt = GetTickCount();
	animationFlags |= animationFlagPoof;
	SetTimer(DOCK_ITEM_TIMER, 10, NULL);
}

void CDockItem::PoofDraw(int step)
{
	if(!poof || !image->dib->Ready())
	{
		return;
	}
	CRect srcRect = poof->Rect();
	srcRect.top = srcRect.Width() * step;
	srcRect.bottom = srcRect.top + srcRect.Width();

	Graphics g(image->dib->dc);
	g.SetCompositingMode(CompositingModeSourceCopy);
	g.DrawImage(poof->bmp, RectF(0, 0, (REAL)image->dib->Width(), (REAL)image->dib->Height()),
		(REAL)srcRect.left, (REAL)srcRect.top, (REAL)srcRect.Width(), (REAL)srcRect.Height(),
		UnitPixel);
	//image->dib->Draw(image->dib->Rect(), srcRect, poof, DrawFlagsPaint);

	image->LayerUpdate();
}

void CDockItem::AddDraw(float k)
{
	if(!image->dib->Ready())
	{
		return;
	}
	if(k == 1)
	{
		image->LayerDraw();
		image->LayerUpdate();

		if(reflection->IsWindowVisible())
		{
			reflection->LayerDraw();
			reflection->LayerUpdate();
		}
	}
	else
	{
		CDIB dib;
		dib.Assign(addTmp);
		if(!dib.Ready())
		{
			return;
		}
		image->dib->Fill();

		CRect dstRect = image->dib->Rect();
		switch(dockPosition)
		{
		case DockPositionBottom:
		case DockPositionTop:
			{
				dstRect.left += (int)((dstRect.Width() - image->dib->Width() * k) / 2 * resizeFlag);
				dstRect.right = dstRect.left + (int)(image->dib->Width() * k);
				dstRect.top += (int)((dstRect.Height() - image->dib->Height() * k) / 2);
				dstRect.bottom = dstRect.top + (int)(image->dib->Height() * k);
			}
			break;

		case DockPositionLeft:
		case DockPositionRight:
			{
				dstRect.left += (int)((dstRect.Width() - image->dib->Width() * k) / 2);
				dstRect.right = dstRect.left + (int)(image->dib->Width() * k);
				dstRect.top += (int)((dstRect.Height() - image->dib->Height() * k) / 2 * resizeFlag);
				dstRect.bottom = dstRect.top + (int)(image->dib->Height() * k);
			}
			break;
		}

		Graphics g(image->dib->dc);
		g.SetCompositingMode(CompositingModeSourceCopy);
		g.DrawImage(dib.bmp, (REAL)dstRect.left, (REAL)dstRect.top, (REAL)dstRect.Width(), (REAL)dstRect.Height());

		image->dib->AlphaBlend(image->dib->Rect(), (unsigned char)(255 * k), DrawFlagsPaint);
		image->LayerUpdate();

		if(reflection->IsWindowVisible())
		{
			reflection->paint = false;
			reflection->LayerDraw(&dib);
			reflection->paint = true;
			reflection->dib->Fill();

			dstRect = reflection->dib->Rect();
			dstRect.left += (int)((dstRect.Width() - reflection->dib->Width() * k) / 2 * resizeFlag);
			dstRect.right = dstRect.left + (int)(reflection->dib->Width() * k);
			dstRect.top += (int)(dstRect.Height() - reflection->dib->Height() * k) / 2;
			dstRect.bottom = dstRect.top + (int)(reflection->dib->Height() * k);

			Graphics g(reflection->dib->dc);
			g.SetCompositingMode(CompositingModeSourceCopy);
			g.DrawImage(dib.bmp, (REAL)dstRect.left, (REAL)dstRect.top, (REAL)dstRect.Width(), (REAL)dstRect.Height());

			unsigned char alphaOld = reflectionOpacity;

			for(int y = 0; y < reflection->dib->Height(); y++)
			{
				unsigned char alpha = alphaOld - (unsigned char)(reflectionOpacityFactor / 100.0f * alphaOld * y / reflection->dib->Height());
				DIB_AARGB *p = (DIB_AARGB*)reflection->dib->Pixels(0, y);
				for(int x = 0; x < reflection->dib->Width(); x++)
				{
					unsigned char a = p[x]->a * alpha / 255;
					float k = (float)a / max(p[x]->a, 1);
					p[x]->r = (unsigned char)(p[x]->r * k);
					p[x]->g = (unsigned char)(p[x]->g * k);
					p[x]->b = (unsigned char)(p[x]->b * k);
					p[x]->a = a;
				}
			}

			reflection->LayerUpdate();
		}
	}
}

BOOL CALLBACK EnumWindowsToActivate(HWND hWnd, LPARAM lParam)
{
	if(IsWindowVisible(hWnd))
	{
		DWORD pid;
		GetWindowThreadProcessId(hWnd, &pid);
		if(pid == (DWORD)lParam)
		{
			if(IsIconic(hWnd))
			{
				ShowWindow(hWnd, SW_RESTORE);
			}
			BringWindowToTop(hWnd);
			SetForegroundWindow(hWnd);
			return FALSE;
		}
	}
	return TRUE;
}

bool CDockItem::Exec(CString fileName, bool runAs)
{
	if(path.IsEmpty())
	{
		return false;
	}
	LPITEMIDLIST pidl = StringToPIDL(path);

	DWORD mode = SetErrorMode(SEM_FAILCRITICALERRORS);
	DWORD attr = GetFileAttributes(path.GetBuffer());
	SetErrorMode(mode);

	int nShowCmd[3] = {SW_SHOWNORMAL, SW_SHOWMINIMIZED, SW_SHOWMAXIMIZED};

	if(!pidl || (attr == INVALID_FILE_ATTRIBUTES) || (attr & FILE_ATTRIBUTE_DIRECTORY))
	{
		if(pidl)
		{
			ILFree(pidl);
		}
		return false;
	}

	CString params;
	params.Format(L"%s %s", arguments.GetBuffer(), fileName.GetBuffer());

	SHELLEXECUTEINFO info = {0};
	info.cbSize = sizeof(SHELLEXECUTEINFO);
	info.lpVerb = runAs ? L"runas" : L"open";
	info.fMask = SEE_MASK_IDLIST;
	info.lpParameters = params.GetBuffer();
	info.lpDirectory = workDirectory.GetBuffer();
	info.nShow = nShowCmd[showAs];
	info.lpIDList = pidl;

	ShellExecuteEx(&info);

	ILFree(pidl);
	return true;
}

bool CDockItem::Exec(DockItemExec exec, bool runAs, bool bounce)
{
	if(path.IsEmpty())
	{
		return false;
	}
	if(exec == DockItemExecIfNotRun)
	{
		HANDLE hSnapshot;
		PROCESSENTRY32 processEntry;
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if(hSnapshot != INVALID_HANDLE_VALUE)
		{
			processEntry.dwSize = sizeof(PROCESSENTRY32);
			if(Process32First(hSnapshot, &processEntry))
			do
			{
				if(GetPathByPID(processEntry.th32ProcessID).CompareNoCase(path) == 0)
				{
					EnumWindows(EnumWindowsToActivate, (LPARAM)processEntry.th32ProcessID);
					CloseHandle(hSnapshot);
					return true;
				}
			}
			while(Process32Next(hSnapshot, &processEntry));
			CloseHandle(hSnapshot);
		}
	}

	LPITEMIDLIST pidl = StringToPIDL(path);

	DWORD mode = SetErrorMode(SEM_FAILCRITICALERRORS);
	DWORD attr = GetFileAttributes(path.GetBuffer());
	SetErrorMode(mode);

	int nShowCmd[3] = {SW_SHOWNORMAL, SW_SHOWMINIMIZED, SW_SHOWMAXIMIZED};

	if(!pidl || (attr == INVALID_FILE_ATTRIBUTES) || (attr & FILE_ATTRIBUTE_DIRECTORY))
	{
		ShellExecute(NULL, L"open", path.GetBuffer(), arguments.GetBuffer(), workDirectory.GetBuffer(), nShowCmd[showAs]);
	}
	else
	{
		if(bounce)
		{
			Bounce(DockItemBounceNormal);
		}
		SHELLEXECUTEINFO info = {0};
		info.cbSize = sizeof(SHELLEXECUTEINFO);
		info.lpVerb = runAs ? L"runas" : L"open";
		info.fMask = SEE_MASK_IDLIST;
		info.lpParameters = arguments.GetBuffer();
		info.lpDirectory = workDirectory.GetBuffer();
		info.nShow = nShowCmd[showAs];
		info.lpIDList = pidl;
		ShellExecuteEx(&info);
	}
	if(pidl)
	{
		ILFree(pidl);
	}
	return true;
}

BOOL CALLBACK EnumWindowsToClose(HWND hWnd, LPARAM lParam)
{
	if(IsWindowVisible(hWnd))
	{
		DWORD pid;
		GetWindowThreadProcessId(hWnd, &pid);
		if(pid == (DWORD)lParam)
		{
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		}
	}
	return TRUE;
}

void CDockItem::Close()
{
	if(!(identificator & identificatorApplication) || !(identificator & identificatorIndicator))
	{
		return;
	}
	HANDLE hSnapshot;
	PROCESSENTRY32 processEntry;
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(hSnapshot != INVALID_HANDLE_VALUE)
	{
		processEntry.dwSize = sizeof(PROCESSENTRY32);
		if(Process32First(hSnapshot, &processEntry))
		do
		{
			if(GetPathByPID(processEntry.th32ProcessID).CompareNoCase(path) == 0)
			{
				EnumWindows(EnumWindowsToClose, (LPARAM)processEntry.th32ProcessID);
				CloseHandle(hSnapshot);
				return;
			}
		}
		while(Process32Next(hSnapshot, &processEntry));
		CloseHandle(hSnapshot);
	}
}

void CDockItem::ReflectionShow(bool visible, bool animation)
{
	if(visible && (dockMode == DockMode3D) && (type != DockItemTypeSeparator))
	{
		if(animation)
		{
			reflection->LayerDraw();

			reflectionTmp = new CDIB();
			reflectionTmp->Resize(reflection->dib->Width(), reflection->dib->Height());
			reflection->LayerUpdate(reflectionTmp);

			reflectionShowStartAt = GetTickCount();
			animationFlags |= animationFlagReflectionShow;
			SetTimer(DOCK_ITEM_TIMER, 10, NULL);
		}
		reflection->ShowWindow(SW_SHOW/*NOACTIVATE*/);
	}
	else
	if(!visible)
	{
		reflection->ShowWindow(SW_HIDE);
		if(animationFlags & animationFlagReflectionShow)
		{
			animationFlags &= ~animationFlagReflectionShow;
			if(animationFlags == 0)
			{
				KillTimer(DOCK_ITEM_TIMER);
			}
			delete reflectionTmp;
			reflection->dib->FreeImage();
		}
	}
}

void CDockItem::IndicatorShow(bool visible, bool animation)
{
	if(visible && (identificator & identificatorIndicator))
	{
		if(animationFlags & animationFlagIndicatorHide)
		{
			animationFlags &= ~animationFlagIndicatorHide;
			if(animationFlags == 0)
			{
				KillTimer(DOCK_ITEM_TIMER);
			}
			delete indicatorTmp;
		}
		indicator->LayerDraw();
		indicator->LayerUpdate();
		indicator->ShowWindow(SW_SHOW/*NOACTIVATE*/);
	}
	else
	if(!visible)
	{
		if(animation)
		{
			if(!(animationFlags & animationFlagIndicatorHide))
			{
				indicator->LayerDraw();
				indicatorTmp = new CDIB();

				indicatorShowStartAt = GetTickCount();
				animationFlags |= animationFlagIndicatorHide;
				SetTimer(DOCK_ITEM_TIMER, 10, NULL);
			}
		}
		else
		{
			indicator->ShowWindow(SW_HIDE);
			if(animationFlags & animationFlagIndicatorHide)
			{
				animationFlags &= ~animationFlagIndicatorHide;
				if(animationFlags == 0)
				{
					KillTimer(DOCK_ITEM_TIMER);
				}
				delete indicatorTmp;
				indicator->dib->FreeImage();
			}
		}
	}
}

void CDockItem::SetState(unsigned int flags)
{
	bool imagePaint = false;
	bool reflectionPaint = false;

#define SetFlag(f, p, r) \
	if(flags & f) \
	{ \
		if((p->state & f) == 0) \
		{ \
			p->state |= f; \
			r = true; \
		} \
	}

	SetFlag(StateFlagPressed, image, imagePaint);
	SetFlag(StateFlagDropOver, image, imagePaint);

	if(imagePaint)
	{
		image->LayerDraw();
		image->LayerUpdate();
	}

	SetFlag(StateFlagPressed, reflection, reflectionPaint);
	SetFlag(StateFlagDropOver, reflection, reflectionPaint);

	if(reflection->IsWindowVisible())
	{
		if(reflectionPaint)
		{
			reflection->LayerDraw();
			if(!(animationFlags & animationFlagReflectionShow))
			{
				reflection->LayerUpdate();
			}
		}
	}
}
	
void CDockItem::RemoveState(unsigned int flags)
{
	bool imagePaint = false;
	bool reflectionPaint = false;

#define RemoveFlag(f, p, r) \
	if(flags & f) \
	{ \
		if(p->state & f) \
		{ \
			p->state &= ~f; \
			r = true; \
		} \
	}

	RemoveFlag(StateFlagPressed, image, imagePaint);
	RemoveFlag(StateFlagDropOver, image, imagePaint);

	if(imagePaint)
	{
		image->LayerDraw();
		image->LayerUpdate();
	}

	RemoveFlag(StateFlagPressed, reflection, reflectionPaint);
	RemoveFlag(StateFlagDropOver, reflection, reflectionPaint);

	if(reflection->IsWindowVisible())
	{
		if(reflectionPaint)
		{
			reflection->LayerDraw();
			if(!(animationFlags & animationFlagReflectionShow))
			{
				reflection->LayerUpdate();
			}
		}
	}
}

void CDockItem::BounceStop()
{
	if((animationFlags & animationFlagBounce) && !(animationFlags & animationFlagBounceAttentionCancel))
	{
		Move(bouncePt.x, bouncePt.y);
		if(dockMode == DockMode3D)
		{
			reflection->SetWindowPos(&wndTop, 
				bouncePt.x, bouncePt.y + rect.Height() - reflectionOffset - imageShadowSize3d, 
				bounceReflectionSize.cx, 
				bounceReflectionSize.cy,
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
		KillTimer(DOCK_ITEM_TIMER_BOUNCE_ATTENTION);
		animationFlags &= ~animationFlagBounceAttention;
		animationFlags &= ~animationFlagBounce;
		if(animationFlags == 0)
		{
			KillTimer(DOCK_ITEM_TIMER);
		}
	}
}

void CDockItem::BounceCancelAttention()
{
	if(animationFlags & animationFlagBounce)
	{
		animationFlags |= animationFlagBounceAttentionCancel;
	}
}

void CDockItem::Bounce(DockItemBounce bounce, int count)
{
	if(animationFlags & animationFlagBounce)
	{
		switch(bounce)
		{
		case DockItemBounceAttention:
			{
				animationFlags &= ~animationFlagBounceAttentionCancel;
			}
			break;
		}
		return;
	}
	bounceCountNow = 0;
	bounceCount = count;
	bounceType = bounce;
	bounceCounter = 0;

	bouncePt.x = rect.left;
	bouncePt.y = rect.top + ((dockMode == DockMode3D) && (type != DockItemTypeSeparator) ? imageShadowSize3d : 0);

	if(dockMode == DockMode3D)
	{
		CRect r;
		reflection->GetWindowRect(&r);
		bounceReflectionSize.cx = r.Width();
		bounceReflectionSize.cy = r.Height();
	}

	switch(bounceType)
	{
	case DockItemBounceNormal:
		{
			switch(dockPosition)
			{
			case DockPositionBottom:
				{
					bounceNextPt.x = bouncePt.x;
					bounceNextPt.y = (int)(bouncePt.y - rect.Height() * (dockMode == DockMode3D ? 0.4f : 0.5f));
				}
				break;

			case DockPositionTop:
				{
					bounceNextPt.x = bouncePt.x;
					bounceNextPt.y = (int)(bouncePt.y + rect.Height() * 0.5f);
				}
				break;

			case DockPositionLeft:
				{
					bounceNextPt.x = (int)(bouncePt.x + rect.Width() * 0.5f);
					bounceNextPt.y = bouncePt.y;
				}
				break;

			case DockPositionRight:
				{
					bounceNextPt.x = (int)(bouncePt.x - rect.Width() * 0.5f);
					bounceNextPt.y = bouncePt.y;
				}
				break;
			}
		}
		break;

	case DockItemBounceAttention:
		{
			switch(dockPosition)
			{
			case DockPositionBottom:
				{
					bounceNextPt.x = bouncePt.x;
					bounceNextPt.y = (int)(bouncePt.y - rect.Height() * DOCK_ITEM_BOUNCE_HEIGHT);
				}
				break;

			case DockPositionTop:
				{
					bounceNextPt.x = bouncePt.x;
					bounceNextPt.y = (int)(bouncePt.y + rect.Height() * DOCK_ITEM_BOUNCE_HEIGHT);
				}
				break;

			case DockPositionLeft:
				{
					bounceNextPt.x = (int)(bouncePt.x + rect.Width() * DOCK_ITEM_BOUNCE_HEIGHT);
					bounceNextPt.y = bouncePt.y;
				}
				break;

			case DockPositionRight:
				{
					bounceNextPt.x = (int)(bouncePt.x - rect.Width() * DOCK_ITEM_BOUNCE_HEIGHT);
					bounceNextPt.y = bouncePt.y;
				}
				break;
			}
			animationFlags |= animationFlagBounceAttention;
		}
		break;
	}

	bounceDelay = DOCK_ITEM_TIMER_BOUNCE_ATTENTION_DELAY;
	bounceStartAt = GetTickCount();
	animationFlags |= animationFlagBounce;
	SetTimer(DOCK_ITEM_TIMER, 10, NULL);
}
