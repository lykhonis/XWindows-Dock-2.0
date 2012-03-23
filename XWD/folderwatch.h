#ifndef H_FOLDERWATCH
#define H_FOLDERWATCH

#include <afxwin.h>
#include <afxtempl.h>

const DWORD WM_FILECHANGED = RegisterWindowMessage(L"WM_FILECHANGED");
/*
	wParam: id
	lParam: state
*/

namespace FolderWatcher
{
	enum 
	{
		FileNone			= 0, 
		FileAdded			= 1 << 0,
		FileRemoved			= 1 << 1,
		FileModified		= 1 << 2,
		FileRenamedOldName	= 1 << 3,
		FileRenamedNewName	= 1 << 4,
		FolderAdded			= 1 << 5,
		FolderRemoved		= 1 << 6,
		FolderModified		= 1 << 7,
		FolderRenamedOldName= 1 << 8,
		FolderRenamedNewName= 1 << 9
	};

	class FolderInfo
	{
	public:
		CWinThread *thread;
		HWND callback;
		WORD id;
		WORD actions;
		CString folder;
		bool active;
		HANDLE hFile;
		void *state;
		int uid;

	public:
		FolderInfo(int uid, HWND callback, WORD id, WORD actions, CString folder, void *state);
		~FolderInfo();
	};

	FolderInfo* AddFolder(int uid, HWND callback, WORD id, WORD actions, CString folder, void *state);
	void RemoveFolder(FolderInfo *folderInfo);
	FolderInfo* FindFolderInfo(int uid, WORD folderId);
	void CleanUp();
};

#endif /* H_FOLDERWATCH */