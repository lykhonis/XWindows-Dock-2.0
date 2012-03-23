#ifndef SCREENCMP_H
#define SCREENCMP_H

#include <afxwin.h>
#include <afxtempl.h>
#include <math.h>
#include "DIB.h"

static const UINT WM_SCREENCMPNOTIFY = RegisterWindowMessage(L"WM_SCREENCMPNOTIFY");

namespace SceenCompare
{
	void Rect(CRect rect);
	void Run(CRect rect, CDIB *buffer, CWnd *notifer);
	void Stop();
	bool Runing();
	void Pause(bool pause = true);
	void Draw(CDIB *buffer);
}

#endif /* SCREENCMP_H */