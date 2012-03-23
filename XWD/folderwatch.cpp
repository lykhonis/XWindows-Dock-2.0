#include "folderwatch.h"

UINT AFX_CDECL FolderReadChangesThread(LPVOID param);

FolderWatcher::FolderInfo::FolderInfo(int uid, HWND callback, WORD id, WORD actions, CString folder, void *state)
{
	this->callback = callback;
	this->id = id;
	this->actions = actions;
	this->folder = folder;
	this->state = state;
	this->uid = uid;

	hFile = CreateFile(folder.GetBuffer(), GENERIC_READ, 
		FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		active = true;
		thread = AfxBeginThread(FolderReadChangesThread, (LPVOID)this, THREAD_PRIORITY_IDLE, 0, CREATE_SUSPENDED);
		if(thread)
		{
			thread->m_bAutoDelete = FALSE;
			thread->ResumeThread();
		}
	}
}

FolderWatcher::FolderInfo::~FolderInfo()
{
	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		if (thread)
		{
			active = false;
			WaitForSingleObject(thread->m_hThread, INFINITE);
			delete thread;
		}
	}
}

static CList<FolderWatcher::FolderInfo*> folderList;

FolderWatcher::FolderInfo* FolderWatcher::FindFolderInfo(int uid, WORD folderId)
{
	POSITION p = folderList.GetHeadPosition();
	while (p)
	{
		FolderWatcher::FolderInfo *info = folderList.GetAt(p);
		if ((info->uid == uid) && (info->id == folderId))
		{
			return info;
		}
		folderList.GetNext(p);
	}
	return NULL;
}

FolderWatcher::FolderInfo* FolderWatcher::AddFolder(int uid, HWND callback, WORD id, WORD actions, CString folder, void *state)
{
	FolderWatcher::FolderInfo *info = FolderWatcher::FindFolderInfo(uid, id);
	if (!info)
	{
		info = new FolderWatcher::FolderInfo(uid, callback, id, actions, folder, state);
		folderList.AddTail(info);
	}
	info->actions = actions;
	return info;
}

void FolderWatcher::RemoveFolder(FolderWatcher::FolderInfo* folderInfo)
{
	POSITION p = folderList.Find(folderInfo);
	if (p)
	{
		folderList.RemoveAt(p);
		delete folderInfo;
	}
}

void FolderWatcher::CleanUp()
{
	POSITION p = folderList.GetHeadPosition();
	while (p)
	{
		delete folderList.GetAt(p);
		folderList.GetNext(p);
	}
	folderList.RemoveAll();
}

UINT AFX_CDECL FolderReadChangesThread(LPVOID param)
{
	FolderWatcher::FolderInfo *info = (FolderWatcher::FolderInfo*)param;
	const int buffSize = 60 * 1024;
	void *buff = malloc(buffSize);
	DWORD ret;
	while(info->active)
	{
		if(ReadDirectoryChangesW(info->hFile, buff, buffSize, FALSE,
			/*FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
			FILE_NOTIFY_CHANGE_ATTRIBUTES | */FILE_NOTIFY_CHANGE_SIZE | 
			FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION, &ret, NULL, NULL) && ret)
		{
			bool notify = false;
			FILE_NOTIFY_INFORMATION *fni = (FILE_NOTIFY_INFORMATION*)buff;
			while(!notify)
			{
				CString fileName = info->folder;
				fileName.Append(fni->FileName, fni->FileNameLength / sizeof(wchar_t));

				int isFileOrDir = 0;

				DWORD errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
				DWORD attr = GetFileAttributes(fileName.GetBuffer());
				SetErrorMode(errorMode);

				if(attr != INVALID_FILE_ATTRIBUTES)
				{
					if(attr & FILE_ATTRIBUTE_DIRECTORY)
					{
						isFileOrDir = 2;
					}
					else
					{
						HANDLE hFile = CreateFile((LPCWSTR)fileName.GetBuffer(), GENERIC_READ, NULL, NULL, OPEN_EXISTING, NULL, NULL);
						if(hFile != INVALID_HANDLE_VALUE)
						{
							CloseHandle(hFile);
							isFileOrDir = 1;
						}
					}
				}

				switch(fni->Action)
				{
				case FILE_ACTION_ADDED:
					{
						if (((isFileOrDir == 1) && (info->actions & FolderWatcher::FileAdded)) ||
							((isFileOrDir == 2) && (info->actions & FolderWatcher::FolderAdded)))
						{
							notify = true;
						}
					}
					break;

				case FILE_ACTION_MODIFIED:
					{
						if (((isFileOrDir == 1) && (info->actions & FolderWatcher::FileModified)) ||
							((isFileOrDir == 2) && (info->actions & FolderWatcher::FolderModified)))
						{
							notify = true;
						}
					}
					break;

				case FILE_ACTION_REMOVED:
					{
						if (info->actions & FolderWatcher::FileRemoved || info->actions & FolderWatcher::FolderRemoved)
						{
							notify = true;
						}
					}
					break;

				case FILE_ACTION_RENAMED_OLD_NAME:
					{
						if (info->actions & FolderWatcher::FileRemoved || info->actions & FolderWatcher::FolderRemoved)
						{
							notify = true;
						}
					}
					break;

				case FILE_ACTION_RENAMED_NEW_NAME:
					{
						if (((isFileOrDir == 1) && (info->actions & FolderWatcher::FileRenamedNewName)) ||
							((isFileOrDir == 2) && (info->actions & FolderWatcher::FolderRenamedNewName)))
						{
							notify = true;
						}
					}
					break;
				}

				if(!fni->NextEntryOffset)
				{
					break;
				}
				fni = (FILE_NOTIFY_INFORMATION*)((DWORD)fni + fni->NextEntryOffset);
			}
			if(notify)
			{
				PostMessage(info->callback, WM_FILECHANGED, info->id, (LPARAM)info->state);
			}
		}
		Sleep(100);
	}
	free(buff);
	return 0;
}