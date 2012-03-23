#include "utils.h"

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;
   UINT  size = 0;

   Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

   Gdiplus::GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;

   pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;

   Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if(wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;
      }    
   }

   free(pImageCodecInfo);
   return -1;
}

bool DeleteDirectory(CString path)
{
	HANDLE hFind;
    WIN32_FIND_DATA FindFileData;

	CString DirPath = path + L"\\*";
	CString FileName = path + L"\\";

    hFind = FindFirstFile(DirPath, &FindFileData);
    if(hFind == INVALID_HANDLE_VALUE) 
	{
		return false;
	}
	DirPath = FileName;

    bool bSearch = true;
    while (bSearch) 
	{
        if (FindNextFile(hFind, &FindFileData))
		{
			if ((wcscmp(FindFileData.cFileName, L"..") == 0) || (wcscmp(FindFileData.cFileName, L".") == 0)) 
			{
				continue;
			}
			FileName += FindFileData.cFileName;

            if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
			{
                if (!DeleteDirectory(FileName)) 
				{ 
                    FindClose(hFind); 
                    return false;
                }
                RemoveDirectory(FileName);
				FileName = DirPath;
            }
            else 
			{
                if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
				{
					DWORD mode = SetErrorMode(SEM_FAILCRITICALERRORS);
					SetFileAttributes(FileName.GetBuffer(), FILE_ATTRIBUTE_NORMAL);
					SetErrorMode(mode);
				}
                if (!DeleteFile(FileName)) 
				{
                    FindClose(hFind); 
                    return false; 
                }
				FileName = DirPath;
            }
        }
        else 
		{
            if (GetLastError() == ERROR_NO_MORE_FILES)
			{
				bSearch = false;
			}
            else 
			{
                FindClose(hFind); 
                return false;
            }

        }

    }
    FindClose(hFind); 
 
    return RemoveDirectory(path) == TRUE;
}

bool IsRecycleBin(CString path)
{
	return (path.GetLength() && (path == GetSpeacialFolderLocation(CSIDL_BITBUCKET)));
}

bool IsMyComputer(CString path)
{
	return (path.GetLength() && (path == GetSpeacialFolderLocation(CSIDL_DRIVES)));
}

CString GetSpeacialFolderLocation(int csidl)
{
	IShellFolder *desktop;
	LPITEMIDLIST pidl;
	STRRET strRet;
	if(FAILED(SHGetDesktopFolder(&desktop)))
	{
		return L"";
	}
	if(FAILED(SHGetSpecialFolderLocation(NULL, csidl, &pidl)))
	{
		return L"";
	}
	if(FAILED(desktop->GetDisplayNameOf(pidl, SHGDN_FORPARSING, &strRet)))
	{
		return L"";
	}
	if(pidl)
	{
		ILFree(pidl);
	}
	return strRet.pOleStr;
}

bool ExtractParam(CString data, CString &param, int index)
{
	wchar_t *p = data.GetBuffer();
	int i = 0;
	int n = 0;
	int d = 0;
	while(*p)
	{
		if(*p == L',')
		{
			if(d == index)
			{
				param = data.Mid(n, i - n);
				param.MakeLower();
				return true;
			}
			d++;
			n = i + 1;
		}
		i++;
		p++;
	}
	if((n < i) && (d == index))
	{
		param = data.Mid(n, i - n);
		param.MakeLower();
		return true;
	}
	return false;
}

CString ExtractParam(CString data, int index)
{
	CString param;
	ExtractParam(data, param, index);
	return param;
}

bool ExtractParams(CString data, CStringList &params)
{
	CString param;
	params.RemoveAll();
	for(int i = 0; ; i++)
	{
		if(ExtractParam(data, param, i))
		{
			params.AddTail(param);
		}
		else
		{
			break;
		}
	}
	return !params.IsEmpty();
}

bool ExtractParams(CString data, RectI &rect)
{
	CString param;
	for(int i = 0; i < 4; i++)
	{
		if(ExtractParam(data, param, i))
		{
			int n  = _wtoi(param.GetBuffer());
			switch(i)
			{
			case 0:
				{
					rect.X = n;
				}
				break;

			case 1:
				{
					rect.Y = n;
				}
				break;

			case 2:
				{
					rect.Width = n;
				}
				break;

			case 3:
				{
					rect.Height = n;
				}
				break;
			}
		}
		else
		{
			return false;
		}
	}
	return true;
}

bool EnumFiles(CString path, EnumFilesProc enumProc, void *param, DWORD flags)
{
	DWORD mode = SetErrorMode(SEM_FAILCRITICALERRORS);
	DWORD attr = GetFileAttributes(path.GetBuffer());
	SetErrorMode(mode);

	if((attr & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		enumProc(path, EnumFilesFlagFile, param);
		return true;
	}

	if(path.Mid(path.GetLength() - 1) != L'\\')
	{
		path += L'\\';
	}
	WIN32_FIND_DATA data;
	HANDLE find = FindFirstFile(CString(path + L'*').GetBuffer(), &data);
	if(find == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	enumProc(path, EnumFilesFlagDirectoryBegin, param);
	do
	{
		if(((data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0) &&
			((data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == 0))
		{
			if((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			{
				if((wcscmp(data.cFileName, L".") != 0) && (wcscmp(data.cFileName, L"..") != 0))
				{
					EnumFiles(path + data.cFileName, enumProc, param, flags);
				}
			}
			else
			{
				enumProc(path + data.cFileName, EnumFilesFlagFile, param);
			}
		}
	}
	while(FindNextFile(find, &data));
	FindClose(find);

	enumProc(path, EnumFilesFlagDirectoryEnd, param);

	return true;
}

CString GetPathByHWND(HWND hWnd)
{
	DWORD pid;
	GetWindowThreadProcessId(hWnd, &pid);
	return GetPathByPID(pid);
}

DWORD GetPIDByPath(CString path)
{
	DWORD pid = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if(hSnapshot)
	{
		PROCESSENTRY32 processEntry = {0};
		processEntry.dwSize = sizeof(PROCESSENTRY32);
		if(Process32First(hSnapshot, &processEntry))
		do
		{
			if(GetPathByPID(processEntry.th32ProcessID).CompareNoCase(path) == 0)
			{
				pid = processEntry.th32ProcessID;
			}
		}
		while(!pid && Process32Next(hSnapshot, &processEntry));
		CloseHandle(hSnapshot);
	}
	return pid;
}

typedef BOOL (WINAPI *__QueryFullProcessImageNameW)(__in HANDLE hProcess, __in DWORD dwFlags, __out LPTSTR lpExeName, __inout PDWORD lpdwSize);
CString GetPathByPID(DWORD pid)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if(hProcess)
	{
		DWORD size = MAX_PATH;
		wchar_t buff[MAX_PATH] = {0};

		OSVERSIONINFO osi = {0};
		osi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osi);

		if(osi.dwMajorVersion >= 6)
		{
			HMODULE h = LoadLibrary(L"kernel32.dll");
			if(h)
			{
				__QueryFullProcessImageNameW _QueryFullProcessImageNameW = (__QueryFullProcessImageNameW)GetProcAddress(h, "QueryFullProcessImageNameW");
				if (_QueryFullProcessImageNameW)
				{
					_QueryFullProcessImageNameW(hProcess, NULL, buff, &size);
					GetLongPathName(buff, buff, MAX_PATH);
					FreeLibrary(h);
					CloseHandle(hProcess);
					return buff;
				}
				FreeLibrary(h);
			}
		}

		GetModuleFileNameEx((HMODULE)hProcess, NULL, buff, MAX_PATH);
		CloseHandle(hProcess);
		return buff;
	}
	return L"";
}

int GetApplicationCopiesCount(CString path)
{
	int i = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if(hSnapshot)
	{
		PROCESSENTRY32 processEntry = {0};
		processEntry.dwSize = sizeof(PROCESSENTRY32);

		if(Process32First(hSnapshot, &processEntry))
		do
		{
			if(GetPathByPID(processEntry.th32ProcessID).CompareNoCase(path) == 0)
			{
				i++;
			}
		}
		while(Process32Next(hSnapshot, &processEntry));
		CloseHandle(hSnapshot);
	}
	return i;
}

bool IsApplicationRunning(CString path)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if(hSnapshot)
	{
		PROCESSENTRY32 processEntry = {0};
		processEntry.dwSize = sizeof(PROCESSENTRY32);
		if(Process32First(hSnapshot, &processEntry))
		do
		{
			if(GetPathByPID(processEntry.th32ProcessID).CompareNoCase(path) == 0)
			{
				CloseHandle(hSnapshot);
				return true;
			}
		}
		while(Process32Next(hSnapshot, &processEntry));
		CloseHandle(hSnapshot);
	}
	return false;
}

struct FindAppWindowData
{
	int counter;
	DWORD pid;
	HWND firstWnd;
};
typedef struct FindAppWindowData FindAppWindowData;

BOOL CALLBACK FindAppEnumWindows(HWND hWnd, LPARAM lParam)
{
	DWORD pid = NULL;
	GetWindowThreadProcessId(hWnd, &pid);
	FindAppWindowData *data = (FindAppWindowData*)lParam;
	if((data->pid == pid) && ::IsWindowVisible(hWnd))
	{
		DWORD style = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
		if((style & WS_EX_TOOLWINDOW) || ((::GetParent(hWnd) != NULL) && ((style & WS_EX_APPWINDOW) == 0)))
		{
			return TRUE;
		}
		wchar_t buff[MAX_PATH];
		GetClassName(hWnd, buff, MAX_PATH);
		if((wcscmp(buff, L"Progman") == 0) || (wcscmp(buff, L"WorkerW") == 0) || 
			(wcscmp(buff, L"Shell_TrayWnd") == 0) || (wcscmp(buff, L"XWindowsDockClass") == 0))
		{
			return TRUE;
		}
		if (data->counter == 0)
		{
			data->firstWnd = hWnd;
		}
		data->counter++;
	}
	return TRUE;
}

BOOL CALLBACK FindAppEnumWindows2(HWND hWnd, LPARAM lParam)
{
	DWORD pid = NULL;
	GetWindowThreadProcessId(hWnd, &pid);
	FindAppWindowData *data = (FindAppWindowData*)lParam;
	if((data->pid == pid) && ::IsWindowVisible(hWnd))
	{
		//DWORD style = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
		//if((style & WS_EX_TOOLWINDOW) || ((::GetParent(hWnd) != NULL) && ((style & WS_EX_APPWINDOW) == 0)))
		//{
			//return TRUE;
		//}
		wchar_t buff[MAX_PATH] = {0};
		GetClassName(hWnd, buff, MAX_PATH);
		if((wcscmp(buff, L"Progman") == 0) || (wcscmp(buff, L"WorkerW") == 0) || 
			(wcscmp(buff, L"Shell_TrayWnd") == 0) || (wcscmp(buff, L"XWindowsDockClass") == 0))
		{
			return TRUE;
		}
		if (data->counter == 0)
		{
			data->firstWnd = hWnd;
		}
		data->counter++;
	}
	return TRUE;
}

int GetApplicationVisibleWindows(CString path)
{
	FindAppWindowData data = {0};
	data.counter = 0;

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if(hSnapshot)
	{
		PROCESSENTRY32 processEntry = {0};
		processEntry.dwSize = sizeof(PROCESSENTRY32);
		if(Process32First(hSnapshot, &processEntry))
		do
		{
			if(GetPathByPID(processEntry.th32ProcessID).CompareNoCase(path) == 0)
			{
				data.pid = processEntry.th32ProcessID;
				EnumWindows(FindAppEnumWindows2, (LPARAM)&data);
			}
		}
		while(Process32Next(hSnapshot, &processEntry));
		CloseHandle(hSnapshot);
	}

	return data.counter;
}

HWND GetApplicationMainWindow(CString path)
{
	FindAppWindowData data = {0};
	data.counter = 0;

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if(hSnapshot)
	{
		PROCESSENTRY32 processEntry = {0};
		processEntry.dwSize = sizeof(PROCESSENTRY32);
		if(Process32First(hSnapshot, &processEntry))
		do
		{
			if(GetPathByPID(processEntry.th32ProcessID).CompareNoCase(path) == 0)
			{
				data.pid = processEntry.th32ProcessID;
				EnumWindows(FindAppEnumWindows, (LPARAM)&data);
			}
		}
		while(Process32Next(hSnapshot, &processEntry));
		CloseHandle(hSnapshot);
	}

	return data.firstWnd;
}