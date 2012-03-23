#include <afxwin.h>
#include "XWDPublicAPI.h"

static CList<XWDPluginInfo*> pluginsList;

static XWDId *menuCount;
static XWDPMenu **menu;

void XWDPublicAPISendAnswer(XWDPluginInfo *plugin, XWDPFunction function, ...)
{
	COPYDATASTRUCT cds = {0};
	cds.dwData = WM_XWDPUBLICAPICALL;

	va_list args;
	va_start(args, function);

	switch (function)
	{
	case XWDPPluginGetSettingsPath:
	case XWDPPluginGetTitle:
	case XWDPPluginGetIcon:
		{
			cds.cbData = sizeof(XWDId) + sizeof(XWDString);
			cds.lpData = malloc(cds.cbData);

			*((XWDId*)cds.lpData) = (XWDId)function; 

			CString str = va_arg(args, CString);

			memcpy((void*)((int)cds.lpData + 4), str.GetBuffer(), min(256, str.GetLength()) * 2);

			if (str.GetLength() < 256)
			{
				char *p = (char*)((int)cds.lpData + 4 + str.GetLength() * 2);
				for (int i = 0; i < 256 - str.GetLength(); i++, p++)
				{
					*p = 0x0;
				}
			}
		}
		break;

	case XWDPPluginGetConfig:
		{
			cds.cbData = sizeof(XWDId) * 5;
			cds.lpData = malloc(cds.cbData);

			*((XWDId*)cds.lpData) = (XWDId)function;
			*((XWDId*)((int)cds.lpData + sizeof(XWDId) * 1)) = (XWDId)plugin->keepInDock;
			*((XWDId*)((int)cds.lpData + sizeof(XWDId) * 2)) = (XWDId)plugin->activatable;
			*((XWDId*)((int)cds.lpData + sizeof(XWDId) * 3)) = (XWDId)plugin->exposable;
			*((XWDId*)((int)cds.lpData + sizeof(XWDId) * 4)) = (XWDId)plugin->bounceable;
		}
		break;

	case XWDPPluginGetIconRect:
		{
			cds.cbData = sizeof(XWDId) + sizeof(XWDPRect);
			cds.lpData = malloc(cds.cbData);

			XWDPRect *rect = va_arg(args, XWDPRect*);

			*((XWDId*)cds.lpData) = (XWDId)function; 
			*((XWDPRect*)((int)cds.lpData + 4)) = *rect;
		}
		break;

	case XWDPPluginGetEvent:
		{
			cds.cbData = sizeof(XWDId) * 2;
			XWDId xevent = va_arg(args, XWDId);
			switch(xevent)
			{
			case XWDPEventMenuSelect:
				{
					cds.cbData += sizeof(XWDId);
				}
				break;
			}

			cds.lpData = malloc(cds.cbData);

			*((XWDId*)cds.lpData) = (XWDId)function;
			*((XWDId*)((int)cds.lpData + 4)) = xevent;

			switch(xevent)
			{
			case XWDPEventMenuSelect:
				{
					XWDId nId = va_arg(args, XWDId);

					*((XWDId*)((int)cds.lpData + 8)) = nId;
				}
				break;
			}
		}
		break;

	case XWDPPluginGetMenu:
		{
			menuCount = va_arg(args, XWDId*);
			menu = va_arg(args, XWDPMenu**);

			cds.cbData = sizeof(XWDId);
			cds.lpData = malloc(cds.cbData);

			*((XWDId*)cds.lpData) = (XWDId)function;
		}
		break;

	default:
		va_end(args);
		return;
	}

	va_end(args);

	SendMessage(plugin->hWnd, WM_COPYDATA, NULL, (LPARAM)&cds);

	free(cds.lpData);
}

#define XWDFunctionPopParam(var) \
	memcpy(&var, params, sizeof(var)); \
	params = (void*)((unsigned int)params + sizeof(var));

void XWDPublicAPIDispatchMessage(CWnd* pWnd, COPYDATASTRUCT *pCopyDataStruct)
{
	if (pCopyDataStruct->dwData == WM_XWDPUBLICAPICALL)
	{
		if (pWnd)
		{
			SetProp(pWnd->m_hWnd, L"XWDResult", (HANDLE)-1);
		}
		if (pCopyDataStruct->cbData >= sizeof(XWDPData))
		{
			XWDPData *data = (XWDPData*)pCopyDataStruct->lpData;
			void *params = (void*)((unsigned int)data + sizeof(XWDPData));

			switch ((XWDPFunction)data->function)
			{
			case XWDPPluginRegister:
				if (pCopyDataStruct->cbData == sizeof(XWDPData) + sizeof(XWDString) * 3 + sizeof(XWDId) * 3)
				{
					XWDString fileName = {0};
					XWDId hWnd = 0;
					XWDString iconName = {0};
					XWDString title = {0};
					XWDId uid = 0;
					XWDId bounceOnClick = 0;

					XWDFunctionPopParam(fileName);
					XWDFunctionPopParam(hWnd);
					XWDFunctionPopParam(iconName);
					XWDFunctionPopParam(title);
					XWDFunctionPopParam(uid);
					XWDFunctionPopParam(bounceOnClick);

					XWDId id = XWDPluginRegister(fileName, hWnd, iconName, title, uid, bounceOnClick);
					if (pWnd)
					{
						SetProp(pWnd->m_hWnd, L"XWDResult", (HANDLE)id);
					}
				}
				break;

			case XWDPPluginRemove:
				if (pCopyDataStruct->cbData == sizeof(XWDPData) + sizeof(XWDId))
				{
					XWDId pluginId = 0;

					XWDFunctionPopParam(pluginId);

					XWDPluginInfo *plugin = XWDPGetPluginById(pluginId);
					if (plugin)
					{
						XWDPluginRemove(plugin);
						if (pWnd)
						{
							SetProp(pWnd->m_hWnd, L"XWDResult", (HANDLE)0);
						}
					}
				}
				break;

			case XWDPPluginSetConfig:
				if (pCopyDataStruct->cbData == sizeof(XWDPData) + sizeof(XWDId) * 5)
				{
					XWDId pluginId = 0;
					XWDId keepInDock = 0;
					XWDId activatable = 0;
					XWDId exposable = 0;
					XWDId bounceable = 0;

					XWDFunctionPopParam(pluginId);
					XWDFunctionPopParam(keepInDock);
					XWDFunctionPopParam(activatable);
					XWDFunctionPopParam(exposable);
					XWDFunctionPopParam(bounceable);

					XWDPluginInfo *plugin = XWDPGetPluginById(pluginId);
					if (plugin)
					{
						plugin->keepInDock = keepInDock != 0;
						plugin->activatable = activatable != 0;
						plugin->exposable = exposable != 0;
						plugin->bounceable = bounceable != 0;
						if (pWnd)
						{
							SetProp(pWnd->m_hWnd, L"XWDResult", (HANDLE)0);
						}
					}
				}
				break;

			case XWDPPluginGetConfig:
				if (pCopyDataStruct->cbData == sizeof(XWDPData) + sizeof(XWDId) * 5)
				{
					XWDId pluginId = 0;

					XWDFunctionPopParam(pluginId);

					XWDPluginInfo *plugin = XWDPGetPluginById(pluginId);
					if (plugin)
					{
						XWDPublicAPISendAnswer(plugin, XWDPPluginGetConfig);
						if (pWnd)
						{
							SetProp(pWnd->m_hWnd, L"XWDResult", (HANDLE)0);
						}
					}
				}
				break;

			case XWDPPluginSetIcon:
				if (pCopyDataStruct->cbData == sizeof(XWDPData) + sizeof(XWDId) + sizeof(XWDString))
				{
					XWDId pluginId = 0;
					XWDString iconName = {0};

					XWDFunctionPopParam(pluginId);
					XWDFunctionPopParam(iconName);

					XWDPluginInfo *plugin = XWDPGetPluginById(pluginId);
					if (plugin)
					{
						XWDPluginSetIcon(plugin, iconName);
						if (pWnd)
						{
							SetProp(pWnd->m_hWnd, L"XWDResult", (HANDLE)0);
						}
					}
				}
				break;

			case XWDPPluginGetIcon:
				if (pCopyDataStruct->cbData == sizeof(XWDPData) + sizeof(XWDId))
				{
					XWDId pluginId = 0;

					XWDFunctionPopParam(pluginId);

					XWDPluginInfo *plugin = XWDPGetPluginById(pluginId);
					if (plugin)
					{
						CString iconName = XWDPluginGetIcon(plugin);
						XWDPublicAPISendAnswer(plugin, XWDPPluginGetIcon, iconName);
						if (pWnd)
						{
							SetProp(pWnd->m_hWnd, L"XWDResult", (HANDLE)0);
						}
					}
				}
				break;

			case XWDPPluginSetTitle:
				if (pCopyDataStruct->cbData == sizeof(XWDPData) + sizeof(XWDId) + sizeof(XWDString))
				{
					XWDId pluginId = 0;
					XWDString title = {0};

					XWDFunctionPopParam(pluginId);
					XWDFunctionPopParam(title);

					XWDPluginInfo *plugin = XWDPGetPluginById(pluginId);
					if (plugin)
					{
						XWDPluginSetTitle(plugin, title);
						if (pWnd)
						{
							SetProp(pWnd->m_hWnd, L"XWDResult", (HANDLE)0);
						}
					}
				}
				break;

			case XWDPPluginGetTitle:
				if (pCopyDataStruct->cbData == sizeof(XWDPData) + sizeof(XWDId))
				{
					XWDId pluginId = 0;

					XWDFunctionPopParam(pluginId);

					XWDPluginInfo *plugin = XWDPGetPluginById(pluginId);
					if (plugin)
					{
						CString title = XWDPluginGetTitle(plugin);
						XWDPublicAPISendAnswer(plugin, XWDPPluginGetIcon, title);
						if (pWnd)
						{
							SetProp(pWnd->m_hWnd, L"XWDResult", (HANDLE)0);
						}
					}
				}
				break;

			case XWDPPluginBounce:
				if (pCopyDataStruct->cbData == sizeof(XWDPData) + sizeof(XWDId) * 3)
				{
					XWDId pluginId = 0;
					XWDId mode = 0;
					XWDId count = 0;

					XWDFunctionPopParam(pluginId);
					XWDFunctionPopParam(mode);
					XWDFunctionPopParam(count);

					XWDPluginInfo *plugin = XWDPGetPluginById(pluginId);
					if (plugin)
					{
						XWDPluginBounce(plugin, (XWDUInt8)mode, count);
						if (pWnd)
						{
							SetProp(pWnd->m_hWnd, L"XWDResult", (HANDLE)0);
						}
					}
				}
				break;

			case XWDPPluginBounceStop:
				if (pCopyDataStruct->cbData == sizeof(XWDPData) + sizeof(XWDId))
				{
					XWDId pluginId = 0;

					XWDFunctionPopParam(pluginId);

					XWDPluginInfo *plugin = XWDPGetPluginById(pluginId);
					if (plugin)
					{
						XWDPluginBounceStop(plugin);
						if (pWnd)
						{
							SetProp(pWnd->m_hWnd, L"XWDResult", (HANDLE)0);
						}
					}
				}
				break;

			case XWDPPluginSetIndicator:
				if (pCopyDataStruct->cbData == sizeof(XWDPData) + sizeof(XWDId) * 2)
				{
					XWDId pluginId = 0;
					XWDId enabled = 0;

					XWDFunctionPopParam(pluginId);
					XWDFunctionPopParam(enabled);

					XWDPluginInfo *plugin = XWDPGetPluginById(pluginId);
					if (plugin)
					{
						XWDPluginSetIndicator(plugin, enabled);
						if (pWnd)
						{
							SetProp(pWnd->m_hWnd, L"XWDResult", (HANDLE)0);
						}
					}
				}
				break;

			case XWDPPluginGetIndicator:
				if (pCopyDataStruct->cbData == sizeof(XWDPData) + sizeof(XWDId))
				{
					XWDId pluginId = 0;

					XWDFunctionPopParam(pluginId);

					XWDPluginInfo *plugin = XWDPGetPluginById(pluginId);
					if (plugin)
					{
						XWDId enabled = XWDPluginGetIndicator(plugin);
						if (pWnd)
						{
							SetProp(pWnd->m_hWnd, L"XWDResult", (HANDLE)enabled);
						}
					}
				}
				break;

			case XWDPPluginGetSettingsPath:
				if (pCopyDataStruct->cbData == sizeof(XWDPData) + sizeof(XWDId))
				{
					XWDId pluginId = 0;

					XWDFunctionPopParam(pluginId);

					XWDPluginInfo *plugin = XWDPGetPluginById(pluginId);
					if (plugin)
					{
						XWDPublicAPISendAnswer(plugin, XWDPPluginGetSettingsPath, plugin->settingsPath);
						if (pWnd)
						{
							SetProp(pWnd->m_hWnd, L"XWDResult", (HANDLE)0);
						}
					}
				}
				break;

			case XWDPPluginGetUId:
				if (pCopyDataStruct->cbData == sizeof(XWDPData) + sizeof(XWDId))
				{
					XWDId pluginId = 0;

					XWDFunctionPopParam(pluginId);

					XWDPluginInfo *plugin = XWDPGetPluginById(pluginId);
					if (plugin)
					{
						if (pWnd)
						{
							SetProp(pWnd->m_hWnd, L"XWDResult", (HANDLE)plugin->uid);
						}
					}
				}
				break;

			case XWDPPluginSetNotification:
				if (pCopyDataStruct->cbData == sizeof(XWDPData) + sizeof(XWDId) * 3 + sizeof(XWDString))
				{
					XWDId pluginId = 0;
					XWDId visible = 0;
					XWDString text = {0};
					XWDId position = 0;

					XWDFunctionPopParam(pluginId);
					XWDFunctionPopParam(visible);
					XWDFunctionPopParam(text);
					XWDFunctionPopParam(position);

					XWDPluginInfo *plugin = XWDPGetPluginById(pluginId);
					if (plugin)
					{
						XWDPluginSetNotification(plugin, visible, text, position);
						if (pWnd)
						{
							SetProp(pWnd->m_hWnd, L"XWDResult", (HANDLE)1);
						}
					}
				}
				break;

			case XWDPPluginGetDockEdge:
				if (pCopyDataStruct->cbData == sizeof(XWDPData) + sizeof(XWDId))
				{
					XWDId pluginId = 0;

					XWDFunctionPopParam(pluginId);

					XWDPluginInfo *plugin = XWDPGetPluginById(pluginId);
					if (plugin)
					{
						XWDId edge = XWDPluginGetDockEdge(plugin);
						if (pWnd)
						{
							SetProp(pWnd->m_hWnd, L"XWDResult", (HANDLE)edge);
						}
					}
				}
				break;

			case XWDPPluginGetIconRect:
				if (pCopyDataStruct->cbData == sizeof(XWDPData) + sizeof(XWDId))
				{
					XWDId pluginId = 0;

					XWDFunctionPopParam(pluginId);

					XWDPluginInfo *plugin = XWDPGetPluginById(pluginId);
					if (plugin)
					{
						XWDPRect rect = XWDPluginGetIconRect(plugin);
						XWDPublicAPISendAnswer(plugin, XWDPPluginGetIconRect, &rect);
						if (pWnd)
						{
							SetProp(pWnd->m_hWnd, L"XWDResult", (HANDLE)1);
						}
					}
				}
				break;

			case XWDPPluginAddFolderWatcher:
				if (pCopyDataStruct->cbData == sizeof(XWDPData) + sizeof(XWDId) * 3 + sizeof(XWDString))
				{
					XWDId pluginId = 0;
					XWDId folderId = 0;
					XWDId actions = 0;
					XWDString folder = {0};

					XWDFunctionPopParam(pluginId);
					XWDFunctionPopParam(folderId);
					XWDFunctionPopParam(actions);
					XWDFunctionPopParam(folder);

					XWDPluginInfo *plugin = XWDPGetPluginById(pluginId);
					if (plugin)
					{
						XWDPluginAddFolderWatcher(plugin, folderId, actions, folder);
						if (pWnd)
						{
							SetProp(pWnd->m_hWnd, L"XWDResult", (HANDLE)1);
						}
					}
				}
				break;

			case XWDPPluginRemoveFolderWatcher:
				if (pCopyDataStruct->cbData == sizeof(XWDPData) + sizeof(XWDId) * 2)
				{
					XWDId pluginId = 0;
					XWDId folderId = 0;

					XWDFunctionPopParam(pluginId);
					XWDFunctionPopParam(folderId);

					XWDPluginInfo *plugin = XWDPGetPluginById(pluginId);
					if (plugin)
					{
						XWDPluginRemoveFolderWatcher(plugin, folderId);
						if (pWnd)
						{
							SetProp(pWnd->m_hWnd, L"XWDResult", (HANDLE)1);
						}
					}
				}
				break;

			case XWDPPluginGetMenu:
				if (pCopyDataStruct->cbData >= sizeof(XWDPData) + sizeof(XWDId) * 2)
				{
					XWDId pluginId = 0;
					XWDId count = 0;

					XWDFunctionPopParam(pluginId);
					XWDFunctionPopParam(count);

					XWDPluginInfo *plugin = XWDPGetPluginById(pluginId);
					if (plugin)
					{
						if (pCopyDataStruct->cbData == sizeof(XWDPData) + sizeof(XWDId) * 2 + sizeof(XWDPMenu) * count)
						{
							if (menuCount && menu)
							{
								*menuCount = count;
								*menu = (XWDPMenu*)malloc(sizeof(XWDPMenu) * count);
								memcpy(*menu, params, sizeof(XWDPMenu) * count);
							}
						}
					}
				}
				break;
			}
		}
	}
}

void XWDPublicAPICleanUp()
{
	POSITION p = pluginsList.GetHeadPosition();
	while (p)
	{
		delete pluginsList.GetAt(p);
		pluginsList.GetNext(p);
	}
	pluginsList.RemoveAll();
}

// Helpers

XWDPluginInfo* XWDPFindPluginByUId(XWDId uid)
{
	POSITION p = pluginsList.GetHeadPosition();
	while (p)
	{
		XWDPluginInfo *plugin = pluginsList.GetAt(p);
		if (plugin->uid == uid)
		{
			return plugin;
		}
		pluginsList.GetNext(p);
	}
	return NULL;
}

XWDPluginInfo* XWDPFindPluginByFileName(CString fileName)
{
	POSITION p = pluginsList.GetHeadPosition();
	while (p)
	{
		XWDPluginInfo *plugin = pluginsList.GetAt(p);
		if (plugin->fileName.CompareNoCase(fileName) == 0)
		{
			return plugin;
		}
		pluginsList.GetNext(p);
	}
	return NULL;
}

XWDId XWDPGetUniqId(XWDId startUid)
{
	XWDId uid = startUid;
	POSITION p = pluginsList.GetHeadPosition();
	while (p)
	{
		if (pluginsList.GetAt(p)->uid == uid)
		{
			return XWDPGetUniqId(++uid);
		}
		pluginsList.GetNext(p);
	}
	return uid;
}

XWDBool XWDPAddPluginInfo(CString fileName, HWND hWnd, XWDId uid, XWDPluginInfo **plugin)
{
	XWDPluginInfo *pluginNew = XWDPFindPluginByFileName(fileName);
	if (pluginNew)
	{
		if (pluginNew->uid == 0)
		{
			pluginNew->uid = XWDPGetUniqId();
		}
		uid = pluginNew->uid;
	}
	if (uid == 0)
	{
		uid = XWDPGetUniqId();
	}
	pluginNew = XWDPFindPluginByUId(uid);
	bool added = !pluginNew;
	if (added)
	{
		pluginNew = new XWDPluginInfo();
		pluginNew->keepInDock = false;
		pluginNew->activatable = true;
		pluginNew->exposable = true;
		pluginNew->bounceable = true;
		pluginNew->id = (XWDId)pluginNew;
		pluginNew->uid = uid;
		pluginNew->fileName = fileName;
		pluginsList.AddTail(pluginNew);
	}
	pluginNew->hWnd = hWnd;
	if (plugin)
	{
		*plugin = pluginNew;
	}
	return added;
}

XWDPluginInfo* XWDPGetPluginById(XWDId pluginId)
{
	XWDPluginInfo *plugin = (XWDPluginInfo*)pluginId;
	if (pluginsList.Find(plugin))
	{
		return plugin;
	}
	return NULL;
}

XWDVoid XWDPRemovePlugin(XWDPluginInfo *plugin)
{
	POSITION p = pluginsList.Find(plugin);
	if (p)
	{
		DeleteDirectory(plugin->settingsPath);
		pluginsList.RemoveAt(p);
		delete plugin;
	}
}

XWDVoid XWDPAskForPlugins()
{
	PostMessage(HWND_BROADCAST, WM_XWDPUBLICAPICALL, 1, 0);
}

XWDVoid XWDPPluginEvent(XWDPluginInfo* plugin, XWDPEvent xevent, ...)
{
	va_list args;
	va_start(args, xevent);

	switch (xevent)
	{
	case XWDPEventLButtonClick:
		XWDPublicAPISendAnswer(plugin, XWDPPluginGetEvent, xevent);
		break;

	case XWDPEventMenuSelect:
		XWDId nId = va_arg(args, XWDId);
		XWDPublicAPISendAnswer(plugin, XWDPPluginGetEvent, xevent, nId);
		break;
	}

	va_end(args);
}

XWDId XWDPPluginMenu(XWDPluginInfo* plugin, XWDPMenu **menu)
{
	XWDId count = 0;
	XWDPublicAPISendAnswer(plugin, XWDPPluginGetMenu, &count, menu);
	return count;
}
