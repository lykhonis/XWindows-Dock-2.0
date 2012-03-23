#ifndef UTILS_H
#define UTILS_H

#include <afxwin.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <gdiplus.h>
#include "consts.h"

#define FreeNull(p) \
	if(p) \
	{ \
		delete p; \
		p = NULL; \
	}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

bool IsRecycleBin(CString path);
bool IsMyComputer(CString path);
CString GetSpeacialFolderLocation(int csidl);

bool ExtractParam(CString data, CString &param, int index);
CString ExtractParam(CString data, int index);
bool ExtractParams(CString data, CStringList &params);
bool ExtractParams(CString data, RectI &rect);

enum
{
	EnumFilesFlagDirectoryBegin = 0,
	EnumFilesFlagDirectoryEnd,
	EnumFilesFlagFile
};

typedef void(*EnumFilesProc)(CString path, int flags, void *param);

bool EnumFiles(CString path, EnumFilesProc enumProc, void *param = NULL, DWORD flags = NULL);

DWORD GetPIDByPath(CString path);
CString GetPathByPID(DWORD pid);
CString GetPathByHWND(HWND hWnd);
int GetApplicationCopiesCount(CString path);
bool IsApplicationRunning(CString path);
int GetApplicationVisibleWindows(CString path);
bool DeleteDirectory(CString path);
HWND GetApplicationMainWindow(CString path);

#endif /* UTILS_H */