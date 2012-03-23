#include "ShellIO.h"
#include <strsafe.h>

bool ShellIO::GetFileVersionInfo(CString fileName, CString *description)
{
	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	} *lpTranslate;

	DWORD pBlockHandle;
	DWORD pBlockSize = GetFileVersionInfoSize(fileName, &pBlockHandle);

	if (pBlockSize == NULL)
	{
		return false;
	}

	void *pBlock = malloc(pBlockSize);
	if (GetFileVersionInfo(fileName, pBlockHandle, pBlockSize, pBlock))
    {
		UINT cbTranslate;
		if (VerQueryValue(pBlock, L"\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &cbTranslate))
		{
			LPWSTR SubBlock = (LPWSTR)malloc(100);
			for (UINT i = 0; i < (cbTranslate / sizeof(struct LANGANDCODEPAGE)); i++)
			{
				HRESULT hr = StringCchPrintf(SubBlock, 50, L"\\StringFileInfo\\%04x%04x\\FileDescription", lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);
				LPCWCHAR lpBuffer;
				UINT cbBuffer;
				if (SUCCEEDED(hr) && VerQueryValue(pBlock, SubBlock, (LPVOID*)&lpBuffer, &cbBuffer) && description)
				{
					*description = lpBuffer;
					break;
				}
			}
			free(SubBlock);
		}
    }

	free(pBlock);
	return true;
}

LPITEMIDLIST ShellIO::StringToPIDL(CString path)
{
	return ILCreateFromPath(path.GetBuffer());
}

CString ShellIO::PIDLToString(LPITEMIDLIST pIdl, LPITEMIDLIST pParentIdl, SHGDNF flags)
{
	IShellFolder *desktopFolder;
	IShellFolder2 *shellFolder = NULL;
	STRRET strRet;
	HRESULT hRes;

	hRes = SHGetDesktopFolder(&desktopFolder);
	if(FAILED(hRes))
	{
		return L"";
	}

	if(pParentIdl)
	{
		desktopFolder->BindToObject((PCUIDLIST_RELATIVE)pParentIdl, NULL, IID_IShellFolder, (void**)&shellFolder);
	}

	if(shellFolder == NULL)
	{
		shellFolder = (IShellFolder2*)desktopFolder;
	}

	hRes = shellFolder->GetDisplayNameOf((PCUITEMID_CHILD)pIdl, flags, &strRet);
	if(FAILED(hRes))
	{
		return L"";
	}
	
	return strRet.pOleStr;
}

bool ShellIO::CreateLink(CString fileName, LinkInfo *linkInfo, int flags)
{
	IShellLink *psl;
	IPersistFile *ppf;
	HRESULT hRes;
	bool ret = FALSE;

	hRes = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl);
    if(SUCCEEDED(hRes))
    {
        hRes = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
        if(SUCCEEDED(hRes))
        {
			if((flags & LI_DESCRIPTION) == LI_DESCRIPTION)
			{
				psl->SetDescription(linkInfo->description.GetBuffer());
			}

			if((flags & LI_PATH) == LI_PATH)
			{
				psl->SetPath(linkInfo->path.GetBuffer());
			}

			if((flags & LI_ARGUMENTS) == LI_ARGUMENTS)
			{
				psl->SetArguments(linkInfo->arguments.GetBuffer());
			}

			if((flags & LI_WORKDIRECTORY) == LI_WORKDIRECTORY)
			{
				psl->SetWorkingDirectory(linkInfo->workDirectory.GetBuffer());
			}

			if((flags & LI_ICONLOCATION) == LI_ICONLOCATION)
			{
				psl->SetIconLocation(linkInfo->iconLocation.GetBuffer(), linkInfo->iconIndex);
			}

			hRes = ppf->Save(fileName.GetBuffer(), FALSE);
			ret = SUCCEEDED(hRes);
         
            ppf->Release();
        }

        psl->Release();
	}

	return ret;
}

bool ShellIO::GetLinkInfo(CString fileName, LinkInfo *linkInfo, int flags)
{
	IShellLink *psl;
	IPersistFile *ppf;
	HRESULT hRes;
	bool ret = false;

	hRes = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl);
    if(SUCCEEDED(hRes))
    {
        hRes = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
        if(SUCCEEDED(hRes))
        {
			hRes = ppf->Load(fileName.GetBuffer(), NULL);
			if(SUCCEEDED(hRes))
			{
				wchar_t buf[1000];

				if((flags & LI_DESCRIPTION) == LI_DESCRIPTION)
				{
					memset(buf, 0, sizeof(buf));
					hRes = psl->GetDescription(buf, 1000);
					if(SUCCEEDED(hRes) && (wcslen(buf) > 0))
					{
						linkInfo->description = buf;
						ret = true;
					}
					else
					{
						linkInfo->description.Empty();
					}
				}

				if((flags & LI_PATH) == LI_PATH)
				{
					LPITEMIDLIST pidl;
					hRes = psl->GetIDList(&pidl);
					if(SUCCEEDED(hRes) && pidl)
					{
						linkInfo->path = PIDLToString(pidl, NULL, SHGDN_FORPARSING);
						ILFree(pidl);
						ret = !linkInfo->path.IsEmpty();
					}
					else
					{
						linkInfo->path.Empty();
					}
				}

				if((flags & LI_ARGUMENTS) == LI_ARGUMENTS)
				{
					memset(buf, 0, sizeof(buf));
					hRes = psl->GetArguments(buf, 1000);
					if(SUCCEEDED(hRes) && (wcslen(buf) > 0))
					{
						linkInfo->arguments = buf;
						ret = true;
					}
					else
					{
						linkInfo->arguments.Empty();
					}
				}

				if((flags & LI_WORKDIRECTORY) == LI_WORKDIRECTORY)
				{
					memset(buf, 0, sizeof(buf));
					hRes = psl->GetWorkingDirectory(buf, 1000);
					if(SUCCEEDED(hRes) && (wcslen(buf) > 0))
					{
						linkInfo->workDirectory = buf;
						ret = true;
					}
					else
					{
						linkInfo->workDirectory.Empty();
					}
				}

				if((flags & LI_ICONLOCATION) == LI_ICONLOCATION)
				{
					int iIcon;
					memset(buf, 0, sizeof(buf));
					hRes = psl->GetIconLocation(buf, 1000, &iIcon);
					if(SUCCEEDED(hRes) && (wcslen(buf) > 0))
					{
						linkInfo->iconLocation = buf;
						linkInfo->iconIndex = iIcon;
						ret = true;
					}
					else
					{
						linkInfo->iconLocation.Empty();
						linkInfo->iconIndex = 0;
					}
				}
			}
            ppf->Release();
        }
        psl->Release();
	}
	return ret;
}