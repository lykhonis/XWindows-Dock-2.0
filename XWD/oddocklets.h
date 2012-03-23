#ifndef ODDOCKLETS_H
#define ODDOCKLETS_H

#include <afxwin.h>
#include <afxtempl.h>
#include <gdiplus.h>
#include "utils.h"

typedef void(CALLBACK *ODGetInfomation)(char *szName, char *szAuthor, int *lpiVersion, char *szNotes);
typedef void*(CALLBACK *ODOnCreate)(HWND hwndDocklet, HINSTANCE hInstance, char *szIni, char *szIniGroup);
typedef void(CALLBACK *ODOnSave)(void *lpData, char *szIni, char *szIniGroup, BOOL bIsForExport);
typedef void(CALLBACK *ODOnDestroy)(void *lpData, HWND hwndDocklet);
typedef void(CALLBACK *ODOnExportFiles)(void *lpData, char *szFileRelativeOut, int iteration);
typedef BOOL(CALLBACK *ODOnLeftButtonClick)(void *lpData, POINT *ptCursor, SIZE *sizeDocklet);
typedef BOOL(CALLBACK *ODOnDoubleClick)(void *lpData, POINT *ptCursor, SIZE *sizeDocklet);
typedef BOOL(CALLBACK *ODOnLeftButtonHeld)(void *lpData, POINT *ptCursor, SIZE *sizeDocklet);
typedef BOOL(CALLBACK *ODOnRightButtonClick)(void *lpData, POINT *ptCursor, SIZE *sizeDocklet);
typedef void(CALLBACK *ODOnConfigure)(void *lpData);
typedef BOOL(CALLBACK *ODOnAcceptDropFiles)(void *lpData);
typedef BOOL(CALLBACK *ODOnDropFiles)(void *lpData, HDROP hDrop);
typedef void(CALLBACK *ODOnProcessMessage)(void *lpData, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef BOOL(*ODDockletIsVisible)(HWND hwndDocklet);
typedef BOOL(*ODDockletGetRect)(HWND hwndDocklet, RECT *rcDocklet);
typedef int(*ODDockletGetLabel)(HWND hwndDocklet, char *szLabel);
typedef int(*ODDockletSetLabel)(HWND hwndDocklet, char *szLabel);
typedef Gdiplus::Bitmap* (*ODDockletLoadGDIPlusImage)(char *szImage);
typedef void(*ODDockletSetImage)(HWND hwndDocklet, Gdiplus::Image *lpImageNew, BOOL bAutomaticallyDeleteImage);
typedef void(*ODDockletSetImageFile)(HWND hwndDocklet, char *szImage);
typedef void(*ODDockletSetImageOverlay)(HWND hwndDocklet, Gdiplus::Image *imageOverlay, BOOL bAutomaticallyDeleteImage);
typedef BOOL(*ODDockletBrowseForImage)(HWND hwndParent, char *szImage, char *szAlternateRelativeRoot);
typedef void(*ODDockletLockMouseEffect)(HWND hwndDocklet, BOOL bLock);
typedef void(*ODDockletDoAttentionAnimation)(HWND hwndDocklet);
typedef void(*ODDockletGetRelativeFolder)(HWND hwndDocklet, char *szFolder);
typedef void(*ODDockletGetRootFolder)(HWND hwndDocklet, char *szFolder);
typedef void(*ODDockletDefaultConfigDialog)(HWND hwndDocklet);
typedef int(*ODDockletQueryDockEdge)(HWND hwndDocklet);
typedef int(*ODDockletSetDockEdge)(HWND hwndDocklet, int iNewEdge);
typedef int(*ODDockletQueryDockAlign)(HWND hwndDocklet);
typedef int(*ODDockletSetDockAlign)(HWND hwndDocklet, int iNewAlign);

class CODDocklet
{
public:
	CODDocklet();
	~CODDocklet();

public:
	int count;

public:
	ODGetInfomation odGetInformation;
	ODOnCreate odOnCreate;
	ODOnSave odOnSave;
	ODOnDestroy odOnDestroy;
	ODOnExportFiles odOnExportFiles;
	ODOnLeftButtonClick odOnLeftButtonClick;
	ODOnDoubleClick odOnDoubleClick;
	ODOnLeftButtonHeld odOnLeftButtonHeld;
	ODOnRightButtonClick odOnRightButtonClick;
	ODOnConfigure odOnConfigure;
	ODOnAcceptDropFiles odOnAcceptDropFiles;
	ODOnDropFiles odOnDropFiles;
	ODOnProcessMessage odOnProcessMessage;

public:
	void OnGetInfomation(char *szName, char *szAuthor, int *lpiVersion, char *szNotes);
	void OnCreate(HWND hwndDocklet, HINSTANCE hInstance, char *szIni, char *szIniGroup);
	void OnSave(HWND hwndDocklet, char *szIni, char *szIniGroup, BOOL bIsForExport);
	void OnDestroy(HWND hwndDocklet);
	void OnExportFiles(HWND hwndDocklet, char *szFileRelativeOut, int iteration);
	BOOL OnLeftButtonClick(HWND hwndDocklet, POINT *ptCursor, SIZE *sizeDocklet);
	BOOL OnDoubleClick(HWND hwndDocklet, POINT *ptCursor, SIZE *sizeDocklet);
	BOOL OnLeftButtonHeld(HWND hwndDocklet, POINT *ptCursor, SIZE *sizeDocklet);
	BOOL OnRightButtonClick(HWND hwndDocklet, POINT *ptCursor, SIZE *sizeDocklet);
	void OnConfigure(HWND hwndDocklet);
	BOOL OnAcceptDropFiles(HWND hwndDocklet);
	BOOL OnDropFiles(HWND hwndDocklet, HDROP hDrop);
	void OnProcessMessage(HWND hwndDocklet, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	HMODULE hModule;
	CString path;

public:
	bool Load(CString fileName = L"");
	void Unload();

	static bool Check(CString fileName);
};

class CODDocklets
{
public:
	CODDocklets();
	~CODDocklets();

public:
	CList<CODDocklet*> items;

public:
	void RemoveAll();

	CODDocklet* Find(CString path);

	static void EnumFile(CString path, int flags, void *param);
	void ScanDirectory(CString path);
};

#endif /* ODDOCKLETS_H */