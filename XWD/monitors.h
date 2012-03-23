#ifndef MONITORS_H
#define MONITORS_H

#include <afxwin.h>
#include <afxtempl.h>

class CMonitor
{
public:
	CMonitor(): handle(NULL) {}

	CRect WorkRect();
	CRect Rect();
	bool IsPrimary();

public:
	HMONITOR handle;
	CRect workRect;
	CRect rect;
};

class CMonitors
{
public:
	CMonitors();
	~CMonitors();

	CMonitor* GetMonitor(HMONITOR hMonitor);
	CMonitor* GetMonitor(HWND hWnd);
	CMonitor* GetMonitor(CPoint point);
	CMonitor* GetMonitor(CRect rect);

	CMonitor* Primary();

public:
	CList<CMonitor*> items;

private:
	static BOOL CALLBACK EnumMonitors(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
};

#endif /* MONITORS_H */