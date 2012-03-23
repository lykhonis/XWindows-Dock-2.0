#include "minimizer.h"

#include <d2d1.h>
#include <d2d1helper.h>
#pragma comment(lib, "d2d1.lib")

#include "xwd.h"

BEGIN_MESSAGE_MAP(CMinimizer, CFrameWnd)
	ON_WM_CLOSE()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

CMinimizer::CMinimizer()
{
	CreateEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST, NULL, L"XWindows Dock - Minimizer", WS_POPUP, CRect(0, 0, 0, 0), NULL, NULL);

	dib = new CDIB();
	monitors = new CMonitors();

	hideWindow = new CFrameWnd();
	hideWindow->CreateEx(WS_EX_TOOLWINDOW, NULL, NULL, WS_POPUP, CRect(0, 0, 0, 0), NULL, NULL);
}

CMinimizer::~CMinimizer()
{
	hideWindow->DestroyWindow();

	delete dib;
	delete monitors;
}

void CMinimizer::OnClose()
{
}

void CMinimizer::UpdateLayer(CDIB *dib)
{
	if(!dib)
	{
		dib = CMinimizer::dib;
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

void CMinimizer::DrawLayer(CDIB *dib)
{
	if(!dib)
	{
		dib = CMinimizer::dib;
	}

	CRect rect;
	GetWindowRect(&rect);

	dib->Resize(rect.Width(), rect.Height());
	if(!dib->Ready())
	{
		return;
	}
}

extern CXWD *xwd;

void CMinimizer::Minimize(HWND hParent, HWND)
{
	monitor = monitors->GetMonitor(hParent);
	if(!monitor)
	{
		return;
	}
	CRect monitorRect = monitor->Rect();
	SetWindowPos(&wndTopMost, monitorRect.left, monitorRect.top, monitorRect.Width(), monitorRect.Height(), 0);
	DrawLayer();
	UpdateLayer();
	ShowWindow(SW_SHOW);


	const D2D1_PIXEL_FORMAT format = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
	const D2D1_RENDER_TARGET_PROPERTIES properties = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, format);

	CComPtr<ID2D1Factory> factory;
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);

	CComPtr<ID2D1DCRenderTarget> target;
	factory->CreateDCRenderTarget(&properties, &target);

	const RECT rect = {0, 0, this->dib->Width(), this->dib->Height()};
	target->BindDC(this->dib->dc, &rect);

	CComPtr<ID2D1Bitmap> pBmp;
	target->CreateBitmap(D2D1::SizeU(xwd->skin->bckg3d->Width(), xwd->skin->bckg3d->Height()), 
		xwd->skin->bckg3d->scan0, xwd->skin->bckg3d->Width() * 4,
		D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)), &pBmp);

	target->BeginDraw();

	CComPtr<ID2D1PathGeometry> pPath;
	factory->CreatePathGeometry(&pPath);

	CComPtr<ID2D1GeometrySink> pSink;
	pPath->Open(&pSink);

	CComPtr<ID2D1BitmapBrush> pBrush;
	target->CreateBitmapBrush(pBmp, D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_CLAMP, D2D1_EXTEND_MODE_CLAMP), D2D1::BrushProperties(), &pBrush);

	pSink->BeginFigure(D2D1::Point2F(20.f, 0.f), D2D1_FIGURE_BEGIN_FILLED);

	pSink->AddLine(D2D1::Point2F(20.f, 0.f));
	pSink->AddLine(D2D1::Point2F(700.f - 20.f, 0.f));
	pSink->AddLine(D2D1::Point2F(700.f, 50.f));
	pSink->AddLine(D2D1::Point2F(0.f, 50.f));

	pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
	pSink->Close();

	target->SetTransform(D2D1::Matrix3x2F::Translation(300, 300));

	target->FillGeometry(pPath, pBrush);

	target->EndDraw();

	UpdateLayer();


	/*CRect wRect;
	::GetWindowRect(hWindow, &wRect);

	CDIB dib;
	dib.Resize(wRect.Width(), wRect.Height());

	::BringWindowToTop(hWindow);
	::UpdateWindow(hWindow);


	HDC scrDC = ::CreateDC(L"DISPLAY", NULL, NULL, NULL);
	BitBlt(dib.dc, 0, 0, dib.Width(), dib.Height(), scrDC, wRect.left, wRect.top, SRCCOPY | 0x40000000);
	::DeleteDC(scrDC);

	dib.ReflectVertical();*/

	/*HRGN hRgn = ::CreateRectRgn(0, 0, dib.Width(), dib.Height());
	::GetWindowRgn(hWnd, hRgn);
	DWORD dwCount = ::GetRegionData(hRgn, NULL, NULL);
	RGNDATAHEADER *rgnData = (RGNDATAHEADER*)malloc(dwCount);
	::GetRegionData(hRgn, dwCount, (LPRGNDATA)rgnData);
	LPRECT rects = (LPRECT)((DWORD)rgnData + sizeof(RGNDATAHEADER));

	for (int i = 0; i < rgnData->nCount; i++)
	{
		CRect r = rects[i];
		for (int y = r.top; y < r.bottom; y++)
		{
			DIB_ARGB *p = (DIB_ARGB*)dib.Pixels(r.left, y);
			for (int x = r.left; x < r.right; x++, p++)
			{
				p->c = 0;
			}
		}
	}

	free(rgnData);
	::DeleteObject(hRgn);*/

	/*WINDOWPLACEMENT orgWndPlacement = {0};
	::GetWindowPlacement(hWindow, &orgWndPlacement);
	HWND orgWndParent = ::GetParent(hWindow);


	::SetParent(hWindow, hideWindow->m_hWnd);
	WINDOWPLACEMENT wndPlacement = {0};
	wndPlacement.flags = WPF_SETMINPOSITION;
	wndPlacement.showCmd = SW_MINIMIZE;
	::SetWindowPlacement(hWindow, &wndPlacement);




	const D2D1_PIXEL_FORMAT format = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
	const D2D1_RENDER_TARGET_PROPERTIES properties = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, format);

	CComPtr<ID2D1Factory> factory;
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);

	CComPtr<ID2D1DCRenderTarget> target;
	factory->CreateDCRenderTarget(&properties, &target);

	const RECT rect = {0, 0, this->dib->Width(), this->dib->Height()};
	target->BindDC(this->dib->dc, &rect);

	CComPtr<ID2D1Bitmap> pBmp;
	HRESULT hr = target->CreateBitmap(D2D1::SizeU(dib.Width(), dib.Height()), dib.scan0, dib.Width() * 4,
		D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)), &pBmp);

	if (SUCCEEDED(hr))
	{
		target->BeginDraw();

		CComPtr<ID2D1PathGeometry> pPath;
		factory->CreatePathGeometry(&pPath);

		CComPtr<ID2D1GeometrySink> pSink;
		pPath->Open(&pSink);

		CComPtr<ID2D1BitmapBrush> pBrush;
		target->CreateBitmapBrush(pBmp, &D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP), &D2D1::BrushProperties(), &pBrush);

		pSink->BeginFigure(D2D1::Point2F(0.0f, 0.0f), D2D1_FIGURE_BEGIN_FILLED);

		pSink->AddLine(D2D1::Point2F(0.0f, 0.0f));
		pSink->AddLine(D2D1::Point2F(0.0f + dib.Width(), 0.0f));
		pSink->AddLine(D2D1::Point2F(0.0f + dib.Width(), 0.0f + dib.Height()));
		pSink->AddLine(D2D1::Point2F(0.0f, 0.0f + dib.Height()));

		pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
		pSink->Close();

		//target->SetTransform(D2D1::IdentityMatrix());
		target->SetTransform(D2D1::Matrix3x2F::Translation(400, 300));

		target->FillGeometry(pPath, pBrush);

		target->EndDraw();
		UpdateLayer();
	}





	Sleep(5000);

	::SetWindowPlacement(hWindow, &orgWndPlacement);
	::SetParent(hWindow, orgWndParent);*/
}

void CMinimizer::OnRButtonDown(UINT nFlags, CPoint point)
{
	CFrameWnd::OnRButtonDown(nFlags, point);

	dib->FreeImage();
	ShowWindow(SW_HIDE);
}