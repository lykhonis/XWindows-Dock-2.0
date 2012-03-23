#ifndef SAFEDWMAPI_H
#define SAFEDWMAPI_H

#include <afxwin.h>
#include <dwmapi.h>

namespace DwmApi
{
	void Initialize();
	void Finalize();
	bool Ready();

	HRESULT DwmQueryThumbnailSourceSize(HTHUMBNAIL hThumbnail, __out PSIZE pSize);
	HRESULT DwmRegisterThumbnail(HWND hwndDestination, HWND hwndSource, __out PHTHUMBNAIL phThumbnailId);
	HRESULT DwmUnregisterThumbnail(HTHUMBNAIL hThumbnailId);
	HRESULT DwmUpdateThumbnailProperties(HTHUMBNAIL hThumbnailId, __in const DWM_THUMBNAIL_PROPERTIES* ptnProperties);
	HRESULT DwmEnableBlurBehindWindow(HWND hWnd, __in const DWM_BLURBEHIND* pBlurBehind);
};

#endif /* SAFEDWMAPI_H */