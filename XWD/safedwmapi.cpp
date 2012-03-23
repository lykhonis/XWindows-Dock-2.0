#include "safedwmapi.h"

typedef HRESULT(STDAPICALLTYPE *__DwmQueryThumbnailSourceSize)(HTHUMBNAIL hThumbnail, __out PSIZE pSize);
typedef HRESULT(STDAPICALLTYPE *__DwmRegisterThumbnail)(HWND hwndDestination, HWND hwndSource, __out PHTHUMBNAIL phThumbnailId);
typedef HRESULT(STDAPICALLTYPE *__DwmUnregisterThumbnail)(HTHUMBNAIL hThumbnailId);
typedef HRESULT(STDAPICALLTYPE *__DwmUpdateThumbnailProperties)(HTHUMBNAIL hThumbnailId, __in const DWM_THUMBNAIL_PROPERTIES* ptnProperties);
typedef HRESULT(STDAPICALLTYPE *__DwmIsCompositionEnabled)(BOOL *pfEnabled);
typedef HRESULT(STDAPICALLTYPE *__DwmEnableBlurBehindWindow)(HWND hWnd, __in const DWM_BLURBEHIND* pBlurBehind);

__DwmQueryThumbnailSourceSize _DwmQueryThumbnailSourceSize = NULL;
__DwmRegisterThumbnail _DwmRegisterThumbnail = NULL;
__DwmUnregisterThumbnail _DwmUnregisterThumbnail = NULL;
__DwmUpdateThumbnailProperties _DwmUpdateThumbnailProperties = NULL;
__DwmIsCompositionEnabled _DwmIsCompositionEnabled = NULL;
__DwmEnableBlurBehindWindow _DwmEnableBlurBehindWindow = NULL;

HMODULE dwmApiDll = NULL;
int dwmApiCounter = 0;

void DwmApi::Initialize()
{
	if(dwmApiCounter == 0)
	{
		OSVERSIONINFO osi = {0};
		osi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osi);
		if(osi.dwMajorVersion >= 6)
		{
			dwmApiDll = LoadLibrary(L"dwmapi.dll");
			if(dwmApiDll)
			{
				_DwmQueryThumbnailSourceSize = (__DwmQueryThumbnailSourceSize)GetProcAddress(dwmApiDll, "DwmQueryThumbnailSourceSize");
				_DwmRegisterThumbnail = (__DwmRegisterThumbnail)GetProcAddress(dwmApiDll, "DwmRegisterThumbnail");
				_DwmUnregisterThumbnail = (__DwmUnregisterThumbnail)GetProcAddress(dwmApiDll, "DwmUnregisterThumbnail");
				_DwmUpdateThumbnailProperties = (__DwmUpdateThumbnailProperties)GetProcAddress(dwmApiDll, "DwmUpdateThumbnailProperties");
				_DwmIsCompositionEnabled = (__DwmIsCompositionEnabled)GetProcAddress(dwmApiDll, "DwmIsCompositionEnabled");
				_DwmEnableBlurBehindWindow = (__DwmEnableBlurBehindWindow)GetProcAddress(dwmApiDll, "DwmEnableBlurBehindWindow");
			}
		}
	}
	dwmApiCounter++;
}
	
void DwmApi::Finalize()
{
	if(dwmApiCounter > 0)
	{
		dwmApiCounter--;
	}
	if((dwmApiCounter == 0) && Ready())
	{
		FreeLibrary(dwmApiDll);
		dwmApiDll = NULL;
	}
}
	
bool DwmApi::Ready()
{
	if(dwmApiDll)
	{
		BOOL enabled = FALSE;
		_DwmIsCompositionEnabled(&enabled);
		return (enabled == TRUE);
	}
	return false;
}

HRESULT DwmApi::DwmQueryThumbnailSourceSize(HTHUMBNAIL hThumbnail, __out PSIZE pSize)
{
	if(Ready())
	{
		return _DwmQueryThumbnailSourceSize(hThumbnail, pSize);
	}
	return S_FALSE;
}

HRESULT DwmApi::DwmRegisterThumbnail(HWND hwndDestination, HWND hwndSource, __out PHTHUMBNAIL phThumbnailId)
{
	if(Ready())
	{
		return _DwmRegisterThumbnail(hwndDestination, hwndSource, phThumbnailId);
	}
	return S_FALSE;
}

HRESULT DwmApi::DwmUnregisterThumbnail(HTHUMBNAIL hThumbnailId)
{
	if(Ready())
	{
		return _DwmUnregisterThumbnail(hThumbnailId);
	}
	return S_FALSE;
}

HRESULT DwmApi::DwmUpdateThumbnailProperties(HTHUMBNAIL hThumbnailId, __in const DWM_THUMBNAIL_PROPERTIES* ptnProperties)
{
	if(Ready())
	{
		return _DwmUpdateThumbnailProperties(hThumbnailId, ptnProperties);
	}
	return S_FALSE;
}

HRESULT DwmApi::DwmEnableBlurBehindWindow(HWND hWnd, __in const DWM_BLURBEHIND* pBlurBehind)
{
	if(Ready())
	{
		return _DwmEnableBlurBehindWindow(hWnd, pBlurBehind);
	}
	return S_FALSE;
}