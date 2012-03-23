#ifndef APP_H
#define APP_H

#include <afxwin.h>
#include <afxole.h>
#include <gdiplus.h>
#include "stdafx.h"
#include "xwd.h"

class CApp: public CWinApp
{
public:
	CApp();
	~CApp();

	virtual BOOL InitInstance();
	virtual int ExitInstance();

public:
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	HANDLE mutex;
};

#endif /* APP_H */