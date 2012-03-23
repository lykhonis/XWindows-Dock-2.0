#ifndef PLUGINS_H
#define PLUGINS_H

#include <afxwin.h>
#include <afxtempl.h>
#include "XWDAPI.h"
#include "utils.h"

typedef XWDPluginType(XWDAPICALL *XWDGetPluginType)();
typedef XWDBool(XWDAPICALL *XWDGetPluginIcon)(XWDString buff);
typedef XWDVoid(XWDAPICALL *XWDGetPluginInformation)(XWDString name, XWDString author, XWDString description, XWDUInt32 *version);
typedef XWDVoid*(XWDAPICALL *XWDPluginInitialize)(XWDId id);
typedef XWDVoid(XWDAPICALL *XWDPluginDeinitialize)(XWDId id, XWDVoid* data);
typedef XWDBool(XWDAPICALL *XWDPluginEvent)(XWDId id, XWDVoid* data, XWDEvent uEvent, va_list args);

class CPlugin
{
public:
	CPlugin();
	~CPlugin();

public:
	XWDPluginType type;

public:
	XWDGetPluginType xwdGetPluginType;
	XWDGetPluginIcon xwdGetPluginIcon;
	XWDGetPluginInformation xwdGetPluginInformation;
	XWDPluginInitialize xwdPluginInitialize;
	XWDPluginDeinitialize xwdPluginDeinitialize;
	XWDPluginEvent xwdPluginEvent;

public:
	XWDPluginType GetPluginType();
	XWDBool GetPluginIcon(XWDString buff);
	XWDVoid GetPluginInformation(XWDString name, XWDString author, XWDString description, XWDUInt32 *version);
	XWDVoid* PluginInitialize(XWDId id);
	XWDVoid PluginDeinitialize(XWDId id, XWDVoid* data);
	XWDBool PluginEvent(XWDId id, XWDVoid* data, XWDEvent uEvent, ...);

public:
	HMODULE hModule;
	CString path;
	int count;

public:
	bool Load(CString fileName = L"");
	void Unload();

	static bool Check(CString fileName);
};

class CPlugins
{
public:
	CPlugins();
	~CPlugins();

public:
	CList<CPlugin*> items;

public:
	void RemoveAll();

	CPlugin* Find(CString path);

	static void EnumFile(CString path, int flags, void *param);
	void ScanDirectory(CString path);
};

#endif /* PLUGINS_H */