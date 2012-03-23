#ifndef SHELLIO_H
#define SHELLIO_H

#include <afxwin.h>
#include <objbase.h>
#include <shlobj.h>

namespace ShellIO
{
	enum {
		LI_DESCRIPTION		= 1 << 0,
		LI_PATH				= 1 << 1,
		LI_ARGUMENTS		= 1 << 2,
		LI_WORKDIRECTORY	= 1 << 3,
		LI_ICONLOCATION		= 1 << 4
	};

	struct LinkInfo
	{
		CString description;
		CString path;
		CString arguments;
		CString workDirectory;
		CString iconLocation;
		int iconIndex;
	};
	typedef struct LinkInfo LinkInfo;

	LPITEMIDLIST StringToPIDL(CString path);
	CString PIDLToString(LPITEMIDLIST pIdl, LPITEMIDLIST pParentIdl, SHGDNF flags);
	bool CreateLink(CString fileName, LinkInfo *linkInfo, int flags);
	bool GetLinkInfo(CString fileName, LinkInfo *linkInfo, int flags);
	bool SelectDirectory(HWND hWnd, CString text, CString &dir, bool editBox, bool showFiles, bool allowCreateDirs);
	HRESULT GetUIObjectOfFile(HWND hwnd, LPCWSTR pszPath, REFIID riid, void **ppv);
};

#endif /* SHELLIO_H */