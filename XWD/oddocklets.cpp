#include "oddocklets.h"

ODDockletIsVisible odDockletIsVisible = NULL;
ODDockletGetRect odDockletGetRect = NULL;
ODDockletGetLabel odDockletGetLabel = NULL;
ODDockletSetLabel odDockletSetLabel = NULL;
ODDockletLoadGDIPlusImage odDockletLoadGDIPlusImage = NULL;
ODDockletSetImage odDockletSetImage = NULL;
ODDockletSetImageFile odDockletSetImageFile = NULL;
ODDockletSetImageOverlay odDockletSetImageOverlay = NULL;
ODDockletBrowseForImage odDockletBrowseForImage = NULL;
ODDockletLockMouseEffect odDockletLockMouseEffect = NULL;
ODDockletDoAttentionAnimation odDockletDoAttentionAnimation = NULL;
ODDockletGetRelativeFolder odDockletGetRelativeFolder = NULL;
ODDockletGetRootFolder odDockletGetRootFolder = NULL;
ODDockletDefaultConfigDialog odDockletDefaultConfigDialog = NULL;
ODDockletQueryDockEdge odDockletQueryDockEdge = NULL;
ODDockletSetDockEdge odDockletSetDockEdge = NULL;
ODDockletQueryDockAlign odDockletQueryDockAlign = NULL;
ODDockletSetDockAlign odDockletSetDockAlign = NULL;

BOOL CALLBACK DockletIsVisible(HWND hwndDocklet)
{
	if(odDockletIsVisible)
	{
		return odDockletIsVisible(hwndDocklet);
	}
	return FALSE;
}

BOOL CALLBACK DockletGetRect(HWND hwndDocklet, RECT *rcDocklet)
{
	if(odDockletGetRect)
	{
		return odDockletGetRect(hwndDocklet, rcDocklet);
	}
	return FALSE;
}

int CALLBACK DockletGetLabel(HWND hwndDocklet, char *szLabel)
{
	if(odDockletGetLabel)
	{
		return odDockletGetLabel(hwndDocklet, szLabel);
	}
	return 0;
}

int CALLBACK DockletSetLabel(HWND hwndDocklet, char *szLabel)
{
	if(odDockletSetLabel)
	{
		return odDockletSetLabel(hwndDocklet, szLabel);
	}
	return 0;
}

Gdiplus::Bitmap* CALLBACK DockletLoadGDIPlusImage(char *szImage)
{
	if(odDockletLoadGDIPlusImage)
	{
		return odDockletLoadGDIPlusImage(szImage);
	}
	return NULL;
}

void CALLBACK DockletSetImage(HWND hwndDocklet, Gdiplus::Image *lpImageNew, BOOL bAutomaticallyDeleteImage)
{
	if(odDockletSetImage)
	{
		odDockletSetImage(hwndDocklet, lpImageNew, bAutomaticallyDeleteImage);
	}
}

void CALLBACK DockletSetImageFile(HWND hwndDocklet, char *szImage)
{
	if(odDockletSetImageFile)
	{
		odDockletSetImageFile(hwndDocklet, szImage);
	}
}

void CALLBACK DockletSetImageOverlay(HWND hwndDocklet, Gdiplus::Image *imageOverlay, BOOL bAutomaticallyDeleteImage)
{
	if(odDockletSetImageOverlay)
	{
		odDockletSetImageOverlay(hwndDocklet, imageOverlay, bAutomaticallyDeleteImage);
	}
}

BOOL CALLBACK DockletBrowseForImage(HWND hwndParent, char *szImage, char *szAlternateRelativeRoot)
{
	if(odDockletBrowseForImage)
	{
		return odDockletBrowseForImage(hwndParent, szImage, szAlternateRelativeRoot);
	}
	return FALSE;
}

void CALLBACK DockletLockMouseEffect(HWND hwndDocklet, BOOL bLock)
{
	if(odDockletLockMouseEffect)
	{
		odDockletLockMouseEffect(hwndDocklet, bLock);
	}
}

void CALLBACK DockletDoAttentionAnimation(HWND hwndDocklet)
{
	if(odDockletDoAttentionAnimation)
	{
		odDockletDoAttentionAnimation(hwndDocklet);
	}
}

void CALLBACK DockletGetRelativeFolder(HWND hwndDocklet, char *szFolder)
{
	if(odDockletGetRelativeFolder)
	{
		odDockletGetRelativeFolder(hwndDocklet, szFolder);
	}
}

void CALLBACK DockletGetRootFolder(HWND hwndDocklet, char *szFolder)
{
	if(odDockletGetRootFolder)
	{
		odDockletGetRootFolder(hwndDocklet, szFolder);
	}
}

void CALLBACK DockletDefaultConfigDialog(HWND hwndDocklet)
{
	if(odDockletDefaultConfigDialog)
	{
		odDockletDefaultConfigDialog(hwndDocklet);
	}
}

int CALLBACK DockletQueryDockEdge(HWND hwndDocklet)
{
	if(odDockletQueryDockEdge)
	{
		return odDockletQueryDockEdge(hwndDocklet);
	}
	return 0;
}

int CALLBACK DockletSetDockEdge(HWND hwndDocklet, int iNewEdge)
{
	if(odDockletSetDockEdge)
	{
		return odDockletSetDockEdge(hwndDocklet, iNewEdge);
	}
	return 0;
}

int CALLBACK DockletQueryDockAlign(HWND hwndDocklet)
{
	if(odDockletQueryDockAlign)
	{
		return odDockletQueryDockAlign(hwndDocklet);
	}
	return 0;
}

int CALLBACK DockletSetDockAlign(HWND hwndDocklet, int iNewAlign)
{
	if(odDockletSetDockAlign)
	{
		return odDockletSetDockAlign(hwndDocklet, iNewAlign);
	}
	return 0;
}

CODDocklet::CODDocklet()
{
	hModule = NULL;
	count = 0;
}

CODDocklet::~CODDocklet()
{
	Unload();
}

bool CODDocklet::Check(CString fileName)
{
	HMODULE hModule = LoadLibrary(fileName.GetBuffer());
	if(!hModule)
	{
		return false;
	}
	ODGetInfomation odGetInfomation = (ODGetInfomation)GetProcAddress(hModule, "OnGetInformation");
	FreeLibrary(hModule);
	return (odGetInfomation != NULL);
}

bool CODDocklet::Load(CString fileName)
{
	if(hModule)
	{
		return (fileName == path) || fileName.IsEmpty();
	}
	if(!fileName.IsEmpty())
	{
		path = fileName;
	}
	if(path.IsEmpty())
	{
		return false;
	}
	hModule = LoadLibrary(path.GetBuffer());
	if(!hModule)
	{
		return false;
	}
	odGetInformation = (ODGetInfomation)GetProcAddress(hModule, "OnGetInformation");
	if(!odGetInformation)
	{
		FreeLibrary(hModule);
		hModule = NULL;
		return false;
	}

	odOnCreate = (ODOnCreate)GetProcAddress(hModule, "OnCreate");
	odOnSave = (ODOnSave)GetProcAddress(hModule, "OnSave");
	odOnDestroy = (ODOnDestroy)GetProcAddress(hModule, "OnDestroy");
	odOnExportFiles = (ODOnExportFiles)GetProcAddress(hModule, "OnExportFiles");
	odOnLeftButtonClick = (ODOnLeftButtonClick)GetProcAddress(hModule, "OnLeftButtonClick");
	odOnDoubleClick = (ODOnDoubleClick)GetProcAddress(hModule, "OnDoubleClick");
	odOnLeftButtonHeld = (ODOnLeftButtonHeld)GetProcAddress(hModule, "OnLeftButtonHeld");
	odOnRightButtonClick = (ODOnRightButtonClick)GetProcAddress(hModule, "OnRightButtonClick");
	odOnConfigure = (ODOnConfigure)GetProcAddress(hModule, "OnConfigure");
	odOnAcceptDropFiles = (ODOnAcceptDropFiles)GetProcAddress(hModule, "OnAcceptDropFiles");
	odOnDropFiles = (ODOnDropFiles)GetProcAddress(hModule, "OnDropFiles");
	odOnProcessMessage = (ODOnProcessMessage)GetProcAddress(hModule, "OnProcessMessage");

	return true;
}

void CODDocklet::Unload()
{
	if(hModule)
	{
		FreeLibrary(hModule);
		hModule = NULL;
	}
}

void CODDocklet::OnGetInfomation(char *szName, char *szAuthor, int *lpiVersion, char *szNotes)
{
	if(hModule && odGetInformation)
	{
		odGetInformation(szName, szAuthor, lpiVersion, szNotes);
	}
}

void CODDocklet::OnCreate(HWND hwndDocklet, HINSTANCE hInstance, char *szIni, char *szIniGroup)
{
	if(count == 0)
	{
		Load();
	}
	if(hModule && odOnCreate)
	{
		void *data = odOnCreate(hwndDocklet, hInstance, szIni, szIniGroup);
		SetProp(hwndDocklet, L"DockletData", (HANDLE)data);
		count++;
	}
}

void CODDocklet::OnSave(HWND hwndDocklet, char *szIni, char *szIniGroup, BOOL bIsForExport)
{
	if(hModule && odOnSave)
	{
		void *data = (void*)GetProp(hwndDocklet, L"DockletData");
		if(data)
		{
			odOnSave(data, szIni, szIniGroup, bIsForExport);
		}
	}
}

void CODDocklet::OnDestroy(HWND hwndDocklet)
{
	if(hModule && odOnDestroy)
	{
		void *data = (void*)GetProp(hwndDocklet, L"DockletData");
		if(data)
		{
			odOnDestroy(data, hwndDocklet);
		}
		if(count)
		{
			count--;
		}
	}
	if(count == 0)
	{
		Unload();
	}
}

void CODDocklet::OnExportFiles(HWND hwndDocklet, char *szFileRelativeOut, int iteration)
{
	if(hModule && odOnExportFiles)
	{
		void *data = (void*)GetProp(hwndDocklet, L"DockletData");
		if(data)
		{
			odOnExportFiles(data, szFileRelativeOut, iteration);
		}
	}
}
	
BOOL CODDocklet::OnLeftButtonClick(HWND hwndDocklet, POINT *ptCursor, SIZE *sizeDocklet)
{
	if(hModule && odOnLeftButtonClick)
	{
		void *data = (void*)GetProp(hwndDocklet, L"DockletData");
		if(data)
		{
			return odOnLeftButtonClick(data, ptCursor, sizeDocklet);
		}
	}
	return FALSE;
}
	
BOOL CODDocklet::OnDoubleClick(HWND hwndDocklet, POINT *ptCursor, SIZE *sizeDocklet)
{
	if(hModule && odOnDoubleClick)
	{
		void *data = (void*)GetProp(hwndDocklet, L"DockletData");
		if(data)
		{
			return odOnDoubleClick(data, ptCursor, sizeDocklet);
		}
	}
	return FALSE;
}
	
BOOL CODDocklet::OnLeftButtonHeld(HWND hwndDocklet, POINT *ptCursor, SIZE *sizeDocklet)
{
	if(hModule && odOnLeftButtonHeld)
	{
		void *data = (void*)GetProp(hwndDocklet, L"DockletData");
		if(data)
		{
			return odOnLeftButtonHeld(data, ptCursor, sizeDocklet);
		}
	}
	return FALSE;
}
	
BOOL CODDocklet::OnRightButtonClick(HWND hwndDocklet, POINT *ptCursor, SIZE *sizeDocklet)
{
	if(hModule && odOnRightButtonClick)
	{
		void *data = (void*)GetProp(hwndDocklet, L"DockletData");
		if(data)
		{
			return odOnRightButtonClick(data, ptCursor, sizeDocklet);
		}
	}
	return FALSE;
}
	
void CODDocklet::OnConfigure(HWND hwndDocklet)
{
	if(hModule && odOnConfigure)
	{
		void *data = (void*)GetProp(hwndDocklet, L"DockletData");
		if(data)
		{
			odOnConfigure(data);
		}
	}
}
	
BOOL CODDocklet::OnAcceptDropFiles(HWND hwndDocklet)
{
	if(hModule && odOnAcceptDropFiles)
	{
		void *data = (void*)GetProp(hwndDocklet, L"DockletData");
		if(data)
		{
			return odOnAcceptDropFiles(data);
		}
	}
	return FALSE;
}
	
BOOL CODDocklet::OnDropFiles(HWND hwndDocklet, HDROP hDrop)
{
	if(hModule && odOnDropFiles)
	{
		void *data = (void*)GetProp(hwndDocklet, L"DockletData");
		if(data)
		{
			return odOnDropFiles(data, hDrop);
		}
	}
	return FALSE;
}
	
void CODDocklet::OnProcessMessage(HWND hwndDocklet, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(hModule && odOnProcessMessage)
	{
		void *data = (void*)GetProp(hwndDocklet, L"DockletData");
		if(data)
		{
			odOnProcessMessage(data, hwndDocklet, uMsg, wParam, lParam);
		}
	}
}

CODDocklets::CODDocklets()
{
}

CODDocklets::~CODDocklets()
{
	RemoveAll();
}

void CODDocklets::RemoveAll()
{
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		delete items.GetAt(p);
		items.GetNext(p);
	}
	items.RemoveAll();
}

CODDocklet* CODDocklets::Find(CString path)
{
	path.MakeLower();
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		CODDocklet *docklet = items.GetAt(p);
		if(CString(docklet->path).MakeLower() == path)
		{
			return docklet;
		}
		items.GetNext(p);
	}
	return NULL;
}

void CODDocklets::EnumFile(CString path, int flags, void *param)
{
	CODDocklets *docklets = (CODDocklets*)param;
	switch(flags)
	{
	case EnumFilesFlagFile:
		{
			if(!CODDocklet::Check(path))
			{
				break;
			}
			if(docklets->Find(path))
			{
				break;
			}
			CODDocklet *docklet = new CODDocklet();
			docklet->path = path;
			docklets->items.AddTail(docklet);
		}
		break;
	}
}
	
void CODDocklets::ScanDirectory(CString path)
{
	EnumFiles(path, EnumFile, this);
}