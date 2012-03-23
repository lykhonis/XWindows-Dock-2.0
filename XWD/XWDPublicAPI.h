#ifndef XWDPUBLICAPI_H
#define XWDPUBLICAPI_H

#include <afxwin.h>
#include <afxtempl.h>
#include "XWDAPI.h"
#include "XWDPAPI.h"
#include "utils.h"

void XWDPublicAPIDispatchMessage(CWnd* pWnd, COPYDATASTRUCT *pCopyDataStruct);

void XWDPublicAPICleanUp();

// Store Plugins Info

class XWDPluginInfo
{
public:
	XWDId id;
	XWDId uid;
	CString settingsPath;
	CString fileName;
	HWND hWnd;
	bool bounceable;
	bool keepInDock;
	bool activatable;
	bool exposable;
};

XWDPluginInfo* XWDPFindPluginByHWnd(HWND hWnd);
XWDBool XWDPAddPluginInfo(CString fileName, HWND hWnd, XWDId uid, XWDPluginInfo **plugin);
XWDId XWDPGetUniqId(XWDId startUid = 1);
XWDPluginInfo* XWDPGetPluginById(XWDId pluginId);
XWDVoid XWDPRemovePlugin(XWDPluginInfo *plugin);
XWDVoid XWDPAskForPlugins();

// XWD Public API

XWDId XWDPluginRegister(CString fileName, XWDId hWnd, CString iconName, CString title, XWDId uid, XWDId bounceable);
XWDVoid XWDPluginRemove(XWDPluginInfo* plugin);
XWDVoid XWDPluginDelete(XWDPluginInfo* plugin);
CString XWDPluginGetIcon(XWDPluginInfo* plugin);
XWDVoid XWDPluginSetIcon(XWDPluginInfo* plugin, CString iconName);
CString XWDPluginGetTitle(XWDPluginInfo* plugin);
XWDVoid XWDPluginSetTitle(XWDPluginInfo* plugin, CString title);
XWDVoid XWDPluginBounce(XWDPluginInfo* plugin, XWDId mode, XWDId count);
XWDVoid XWDPluginBounceStop(XWDPluginInfo* plugin);
XWDId XWDPluginGetIndicator(XWDPluginInfo* plugin);
XWDVoid XWDPluginSetIndicator(XWDPluginInfo* plugin, XWDId enabled);
XWDVoid XWDPluginSetNotification(XWDPluginInfo* plugin, XWDId visible, CString text, XWDId position);
XWDId XWDPluginGetDockEdge(XWDPluginInfo* plugin);
XWDPRect XWDPluginGetIconRect(XWDPluginInfo* plugin);
XWDVoid XWDPluginAddFolderWatcher(XWDPluginInfo* plugin, XWDId folderId, XWDId actions, CString folder);
XWDVoid XWDPluginRemoveFolderWatcher(XWDPluginInfo* plugin, XWDId folderId);

XWDVoid XWDPPluginEvent(XWDPluginInfo* plugin, XWDPEvent xevent, ...);
XWDId XWDPPluginMenu(XWDPluginInfo* plugin, XWDPMenu **menu);

#endif /* XWDPUBLICAPI_H */