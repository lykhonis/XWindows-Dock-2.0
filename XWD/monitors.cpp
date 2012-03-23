#include "Monitors.h"

CRect CMonitor::WorkRect()
{
	return workRect;
	/*if(handle)
	{
		MONITORINFO mi = {0};
		mi.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(handle, &mi);
		return mi.rcWork;
	}
	return CRect(0, 0, 0, 0);*/
}

CRect CMonitor::Rect()
{
	return rect;
	/*if(handle)
	{
		MONITORINFO mi = {0};
		mi.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(handle, &mi);
		return mi.rcMonitor;
	}
	return CRect(0, 0, 0, 0);*/
}

bool CMonitor::IsPrimary()
{
	if(handle)
	{
		MONITORINFO mi = {0};
		mi.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(handle, &mi);
		return (mi.dwFlags & MONITORINFOF_PRIMARY);
	}
	return false;
}

BOOL WINAPI CMonitors::EnumMonitors(HMONITOR hMonitor, HDC, LPRECT, LPARAM dwData)
{
	CMonitors *monitors = (CMonitors*)dwData;
	CMonitor *monitor = new CMonitor();
	monitor->handle = hMonitor;
	monitors->items.AddTail(monitor);

	MONITORINFO mi = {0};
	mi.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor->handle, &mi);
	monitor->rect = mi.rcMonitor;
	monitor->workRect = mi.rcWork;

	return TRUE;
}

CMonitors::CMonitors()
{
	EnumDisplayMonitors(NULL, NULL, EnumMonitors, (LPARAM)this);
}

CMonitors::~CMonitors()
{
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		delete items.GetAt(p);
		items.GetNext(p);
	}
	items.RemoveAll();
}

CMonitor* CMonitors::GetMonitor(HMONITOR hMonitor)
{
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		CMonitor *monitor = items.GetAt(p);
		if(monitor->handle == hMonitor)
		{
			return monitor;
		}
		items.GetNext(p);
	}
	return NULL;
}

CMonitor* CMonitors::GetMonitor(HWND hWnd)
{
	return GetMonitor(::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST));
}

CMonitor* CMonitors::GetMonitor(CPoint point)
{
	return GetMonitor(::MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST));
}

CMonitor* CMonitors::GetMonitor(CRect rect)
{
	return GetMonitor(::MonitorFromRect(rect, MONITOR_DEFAULTTONEAREST));
}

CMonitor* CMonitors::Primary()
{
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		CMonitor *monitor = items.GetAt(p);
		if(monitor->IsPrimary())
		{
			return monitor;
		}
		items.GetNext(p);
	}
	return NULL;
}