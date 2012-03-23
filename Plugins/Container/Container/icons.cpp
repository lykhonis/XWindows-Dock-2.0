#include "icons.h"

using namespace Gdiplus;

int Icons::GetIconSize(CString fileName)
{
	int iconSize = GetSystemMetrics(SM_CXICON);
	FILE *f = NULL;
	_wfopen_s(&f, fileName.GetBuffer(), L"r+b");
	if(f)
	{
		ICONDIR iconDir = {0};
		fread(&iconDir, 1, sizeof(ICONDIR), f);
		if((iconDir.idReserved == 0) && (iconDir.idType == 1))
		{
			void *iconEntry = malloc(iconDir.idCount * sizeof(ICONDIRENTRY));
			fread(iconEntry, sizeof(ICONDIRENTRY), iconDir.idCount, f); 
			ICONDIRENTRY *p = (ICONDIRENTRY*)iconEntry;
			for(int i = 0; i < iconDir.idCount; i++, p++)
			{
				if(iconSize < p->bWidth)
				{
					iconSize = p->bWidth;
				}
			}
			free(iconEntry);
		}
		fclose(f);
	}
	return iconSize;
}

typedef struct {
	CString name;
	int index;
	int counter;
} ENUMDATA;

BOOL WINAPI EnumResNameProc(HMODULE, LPCTSTR, LPTSTR lpszName, LONG_PTR lParam)
{
	ENUMDATA *data = (ENUMDATA*)lParam;
	if(data->counter == data->index)
	{
		if(IS_INTRESOURCE(lpszName))
		{
			data->name.Format(L"#%d", (ULONG_PTR)lpszName);
		}
		else
		{
			data->name = lpszName;
		}
		return FALSE;
	}
	else
	{
		data->counter++;
		return TRUE;
	}
}

bool AnalizeIcon256(HICON icon)
{
	CDIB dib;
	Icons::GetDIBFromIcon(icon, &dib);
	if(!dib.Ready())
	{
		return false;
	}
	DIB_ARGB *p = (DIB_ARGB*)((int)dib.scan0 + (48 * dib.Width() + 48) * 4);
	int size = (dib.Width() - 48) * (dib.Height() - 48);
	for(int i = 0; i < size; i++, p++)
	{
		if(p->c != 0)
		{
			return true;
		}
	}
	return false;
}

HICON Icons::GetIcon(CString fileName)
{
	if(fileName.IsEmpty())
	{
		return NULL;
	}

	HICON icon = NULL;
	SHFILEINFO sfi = {0};
	LPITEMIDLIST pidl = SHSimpleIDListFromPath(fileName.GetBuffer());

	if(pidl && (fileName.Mid(0, 3) == L"::{"))
	{
		SHGetFileInfo((LPCWSTR)pidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICONLOCATION | SHGFI_PIDL);
		if(wcscmp(sfi.szDisplayName, L"\0") != 0)
		{
			fileName = sfi.szDisplayName;
		}
	}

	CString ext;
	int i = fileName.ReverseFind(L'.');
	if(i >= 0)
	{
		ext = fileName.Mid(i).MakeLower();
	}
	if((ext == L".exe") || (ext == L".ocx") || (ext == L".dll") || 
		(ext == L".scr") || (ext == L".bin") || (ext == L".cpl"))
	{
		HMODULE hModule = LoadLibraryEx(fileName.GetBuffer(), 0, LOAD_LIBRARY_AS_DATAFILE);
		if(hModule)
		{
			ENUMDATA enumData;
			if(sfi.iIcon >= 0)
			{
				enumData.index = sfi.iIcon;
				enumData.counter = 0;
				EnumResourceNames(hModule, MAKEINTRESOURCE(3 + DIFFERENCE), EnumResNameProc, (LONG_PTR)&enumData);
			}
			else
			{
				enumData.name.Format(L"#%d", -sfi.iIcon);
			}
			icon = GetIcon(hModule, enumData.name);
			FreeLibrary(hModule);
		}
	}
	else
	if((ext == L".ico") || (ext == L".icon"))
	{
		int iconSize = GetIconSize(fileName);
		icon = (HICON)LoadImage(0, fileName.GetBuffer(), IMAGE_ICON, iconSize, iconSize, LR_LOADFROMFILE | LR_COLOR);
	}

	if(!icon)
	{
		IImageList* imageList;
		OSVERSIONINFO osinf = {0};
		osinf.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osinf);
		
		if((osinf.dwMajorVersion >= 6) && SUCCEEDED(SHGetImageList(SHIL_JUMBO, IID_IImageList, (void**)&imageList)))
		{
			SHGetFileInfo(fileName.GetBuffer(), 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX); 
			imageList->GetIcon(sfi.iIcon, ILD_IMAGE | ILD_TRANSPARENT, &icon);
			if(!AnalizeIcon256(icon))
			{
				DestroyIcon(icon);
				icon = NULL;
			}
		}
		if(!icon && SUCCEEDED(SHGetImageList(SHIL_EXTRALARGE, IID_IImageList, (void**)&imageList)))
		{
			SHGetFileInfo(fileName.GetBuffer(), 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX); 
			imageList->GetIcon(sfi.iIcon, ILD_IMAGE | ILD_TRANSPARENT, &icon);
		}
	}

	if(!icon && pidl && SHGetFileInfo((LPCWSTR)pidl, 0, &sfi, sizeof(SHFILEINFO), 
		SHGFI_ICON | SHGFI_LARGEICON | SHGFI_PIDL))
	{
		icon = sfi.hIcon;
	}

	if(pidl)
	{
		ILFree(pidl);
	}

	return icon;
}

HICON Icons::GetIcon(HMODULE hModule, CString resName)
{
	HICON icon = NULL;
	HRSRC hResource = FindResource(hModule, resName.GetBuffer(), MAKEINTRESOURCE(3 + DIFFERENCE));
	if(hResource)
	{
		HGLOBAL hMem = LoadResource(hModule, hResource);
		LPVOID lpResource = LockResource(hMem);

		if(hMem && lpResource)
		{
			int nId = LookupIconIdFromDirectoryEx((PBYTE)lpResource, TRUE, 0x200, 0x200, LR_DEFAULTCOLOR);
			hResource = FindResource(hModule, MAKEINTRESOURCE(nId), MAKEINTRESOURCE(3));
			if(hResource)
			{
				hMem = LoadResource(hModule, hResource);
				lpResource = LockResource(hMem);
				if(hMem && lpResource)
				{
					icon = CreateIconFromResourceEx((PBYTE)lpResource, SizeofResource(hModule, hResource),
						TRUE, 0x00030000, 0, 0, LR_DEFAULTCOLOR);
				}
			}
		}
	}
	return icon;
}

HICON Icons::GetIcon(CString fileName, CString resName)
{
	if(fileName.IsEmpty() || resName.IsEmpty())
	{
		return NULL;
	}
	HICON icon = NULL;
	HMODULE hModule = (HMODULE)LoadLibraryEx(fileName.GetBuffer(), NULL, LOAD_LIBRARY_AS_DATAFILE);
	if(hModule)
	{
		icon = GetIcon(hModule, resName);
		FreeLibrary(hModule);
	}
	return icon;
}

void Icons::GetDIBFromIcon(HICON icon, CDIB *dib)
{
	if(!icon || !dib)
	{
		return;
	}
	ICONINFO ii;
	GetIconInfo(icon, &ii);

	BITMAP bitmap;
	GetObject(ii.hbmColor, sizeof(BITMAP), &bitmap);

	HDC mainDC = CreateCompatibleDC(0);
	HDC maskDC = CreateCompatibleDC(0);
	HGDIOBJ mainOld = SelectObject(mainDC, ii.hbmColor);
	HGDIOBJ maskOld = SelectObject(maskDC, ii.hbmMask);

	dib->Resize(bitmap.bmWidth, bitmap.bmHeight);

	if(bitmap.bmBitsPixel < 32)
	{
		BitBlt(dib->dc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, maskDC, 0, 0, SRCCOPY);
		BitBlt(dib->dc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, maskDC, 0, 0, DSTINVERT);
		BitBlt(dib->dc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, mainDC, 0, 0, SRCAND);
	}
	else
	{
		BitBlt(dib->dc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, mainDC, 0, 0, SRCCOPY);
	}

	dib->ReflectVertical();

	// Check old icons >> 32 bits
	bool check = true;
	DIB_ARGB *p = dib->scan0;
	for(int i = 0; i < dib->Width() * dib->Height(); i++, p++)
	{
		if(p->a)
		{
			check = false;
			break;
		}
	}
	if(check)
	{
		p = dib->scan0;
		for(int i = 0; i < dib->Width() * dib->Height(); i++, p++)
		{
			if((p->r != 0x0) && (p->g != 0x0) && (p->b != 0x0))
			{
				p->a = 255;
			}
		}
	}
	SelectObject(maskDC, maskOld);
	SelectObject(mainDC, mainOld);
	DeleteDC(maskDC);
	DeleteDC(mainDC);
	DeleteObject(ii.hbmMask);
	DeleteObject(ii.hbmColor); 
}

bool Icons::GetIcon(CString fileName, CDIB *dib)
{
	if(!fileName.IsEmpty())
	{
		HICON icon = GetIcon(fileName);
		if(icon)
		{
			GetDIBFromIcon(icon, dib);
			DestroyIcon(icon);
			return true;
		}
	}
	return false;
}

HICON Icons::GetSystemIcon(UINT uType)
{
	CString resName;
	if((uType & MB_ICONEXCLAMATION) == MB_ICONEXCLAMATION)
	{
		resName = L"#101";
	}
	else
	if((uType & MB_ICONQUESTION) == MB_ICONQUESTION)
	{
		resName = L"#102";
	}
	else
	if((uType & MB_ICONERROR) == MB_ICONERROR)
	{
		resName = L"#103";
	}
	else
	if((uType & MB_ICONINFORMATION) == MB_ICONINFORMATION)
	{
		resName = L"#104";
	}
	else
	{
		return NULL;
	}
	return GetIcon(L"user32.dll", resName);
}
