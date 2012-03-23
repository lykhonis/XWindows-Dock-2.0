#include "plugins.h"

CPlugin::CPlugin()
{
	hModule = NULL;
	count = 0;
}

CPlugin::~CPlugin()
{
	Unload();
}

bool CPlugin::Check(CString fileName)
{
	HMODULE hModule = LoadLibrary(fileName.GetBuffer());
	if(!hModule)
	{
		return false;
	}
	XWDGetPluginType xwdGetPluginType = (XWDGetPluginType)GetProcAddress(hModule, "XWDGetPluginType");
	FreeLibrary(hModule);
	return (xwdGetPluginType != NULL);
}

bool CPlugin::Load(CString fileName)
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
	xwdGetPluginType = (XWDGetPluginType)GetProcAddress(hModule, "XWDGetPluginType");
	if(!xwdGetPluginType)
	{
		FreeLibrary(hModule);
		hModule = NULL;
		return false;
	}

	xwdGetPluginIcon = (XWDGetPluginIcon)GetProcAddress(hModule, "XWDGetPluginIcon");
	xwdGetPluginInformation = (XWDGetPluginInformation)GetProcAddress(hModule, "XWDGetPluginInformation");
	xwdPluginInitialize = (XWDPluginInitialize)GetProcAddress(hModule, "XWDPluginInitialize");
	xwdPluginDeinitialize = (XWDPluginDeinitialize)GetProcAddress(hModule, "XWDPluginDeinitialize");
	xwdPluginEvent = (XWDPluginEvent)GetProcAddress(hModule, "XWDPluginEvent");

	return true;
}

void CPlugin::Unload()
{
	if(hModule)
	{
		FreeLibrary(hModule);
		hModule = NULL;
	}
}

XWDPluginType CPlugin::GetPluginType()
{
	if(hModule && xwdGetPluginType)
	{
		return xwdGetPluginType();
	}
	return (XWDPluginType)NULL;
}

XWDBool CPlugin::GetPluginIcon(XWDString buff)
{
	if(hModule && xwdGetPluginIcon)
	{
		return xwdGetPluginIcon(buff);
	}
	return XWDFalse;
}

XWDVoid CPlugin::GetPluginInformation(XWDString name, XWDString author, XWDString description, XWDUInt32 *version)
{
	if(hModule && xwdGetPluginInformation)
	{
		xwdGetPluginInformation(name, author, description, version);
	}
}

XWDVoid* CPlugin::PluginInitialize(XWDId id)
{
	if(count == 0)
	{
		Load();
	}
	if(hModule && xwdPluginInitialize)
	{
		count++;
		return xwdPluginInitialize(id);
	}
	return XWDNull;
}

XWDVoid CPlugin::PluginDeinitialize(XWDId id, XWDVoid* data)
{
	if(hModule && xwdPluginDeinitialize)
	{
		xwdPluginDeinitialize(id, data);
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

XWDBool CPlugin::PluginEvent(XWDId id, XWDVoid* data, XWDEvent uEvent, ...)
{
	if(hModule && xwdPluginEvent)
	{
		va_list args;
		va_start(args, uEvent);
		XWDBool ret = xwdPluginEvent(id, data, uEvent, args);
		va_end(args);
		return ret;
	}
	return XWDFalse;
}

CPlugins::CPlugins()
{
}

CPlugins::~CPlugins()
{
	RemoveAll();
}

void CPlugins::RemoveAll()
{
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		delete items.GetAt(p);
		items.GetNext(p);
	}
	items.RemoveAll();
}

CPlugin* CPlugins::Find(CString path)
{
	path.MakeLower();
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		CPlugin *plugin = items.GetAt(p);
		if(CString(plugin->path).MakeLower() == path)
		{
			return plugin;
		}
		items.GetNext(p);
	}
	return NULL;
}

void CPlugins::EnumFile(CString path, int flags, void *param)
{
	CPlugins *plugins = (CPlugins*)param;
	switch(flags)
	{
	case EnumFilesFlagFile:
		{
			if(!CPlugin::Check(path))
			{
				break;
			}
			if(plugins->Find(path))
			{
				break;
			}
			CPlugin *plugin = new CPlugin();
			plugin->path = path;
			plugins->items.AddTail(plugin);
		}
		break;
	}
}

void CPlugins::ScanDirectory(CString path)
{
	EnumFiles(path, EnumFile, this);
}