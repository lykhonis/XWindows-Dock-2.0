#include "app.h"

CApp::CApp()
{
}

CApp::~CApp()
{
}

BOOL CApp::InitInstance() 
{
	mutex = CreateMutex(NULL, FALSE, L"XWindows Dock");
	if((mutex == 0) || (GetLastError() == ERROR_ALREADY_EXISTS))
	{
		exit(0);
	}

	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	AfxOleInit();
	CoInitialize(NULL);

	m_pMainWnd = new CXWD();	
	m_pMainWnd->ShowWindow(SW_SHOW);

	return CWinApp::InitInstance();
}

int CApp::ExitInstance()
{
	Gdiplus::GdiplusShutdown(gdiplusToken);
	CoUninitialize();
	CloseHandle(mutex);

	return CWinApp::ExitInstance();
}