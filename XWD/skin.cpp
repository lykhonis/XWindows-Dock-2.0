#include "skin.h"

CSkin::CSkin()
{
	ready = false;
	bckgLeftTop2d = NULL;
	bckgTop2d = NULL;
	bckgRightTop2d = NULL;
	bckgRight2d = NULL;
	bckgRightBottom2d = NULL;
	bckgBottom2d = NULL;
	bckgLeftBottom2d = NULL;
	bckgLeft2d = NULL;
	bckgMiddle2d = NULL;
	separator2d = NULL;
	indicator2d = NULL;
	bckg3d = NULL;
	bckgEdge3d = NULL;
	separator3d = NULL;
	indicator3d = NULL;
}

CSkin::~CSkin()
{
	Unload();
}

bool CSkin::Load()
{
	return Load(path, false);
}

bool CSkin::Load(CString fileName, bool onlyInformation)
{
	Unload();

	path = fileName;

	CIniReader ini(path);
	if(!ini.sectionExists(L"skin"))
	{
		return false;
	}

	name = ini.getKeyValue(L"skin-name", L"skin");
	description = ini.getKeyValue(L"skin-description", L"skin");
	
	CString value;
	CStringList values;
	CString dir = path.Mid(0, path.ReverseFind(L'\\') + 1);

	mode = DockModeNone;
	ExtractParams(ini.getKeyValue(L"dock-mode", L"skin"), values);
	if(values.Find(L"2d"))
	{
		mode |= DockMode2D;
	}
	if(values.Find(L"3d"))
	{
		mode |= DockMode3D;
	}
	if(mode == DockModeNone)
	{
		return false;
	}

	if(mode & DockMode2D)
	{
		bckgOpacity2d = _wtoi(ini.getKeyValue(L"2d-background-opacity", L"skin").GetBuffer());
		skipLeft2d = _wtoi(ini.getKeyValue(L"2d-skip-left", L"skin").GetBuffer());
		skipTop2d = _wtoi(ini.getKeyValue(L"2d-skip-top", L"skin").GetBuffer());
		skipRight2d = _wtoi(ini.getKeyValue(L"2d-skip-right", L"skin").GetBuffer());
		skipBottom2d = _wtoi(ini.getKeyValue(L"2d-skip-bottom", L"skin").GetBuffer());
		skipBetweenIcons2d = _wtoi(ini.getKeyValue(L"2d-skip-between-icons", L"skin").GetBuffer());
		indicatorSkipBottom2d = _wtoi(ini.getKeyValue(L"2d-indicator-skip-bottom", L"skin").GetBuffer());

		if(bckgOpacity2d < 0) bckgOpacity2d = 0;
		if(bckgOpacity2d > 100) bckgOpacity2d = 100;
	}
	if(mode & DockMode3D)
	{
		bckgOpacity3d = _wtoi(ini.getKeyValue(L"3d-background-opacity", L"skin").GetBuffer());
		bckgEdgeOffset3d = _wtoi(ini.getKeyValue(L"3d-background-edge-offset", L"skin").GetBuffer());
		bckgEdgeAngle3d = _wtoi(ini.getKeyValue(L"3d-background-edge-angle", L"skin").GetBuffer());
		reflectionOpacity3d = _wtoi(ini.getKeyValue(L"3d-reflection-opacity", L"skin").GetBuffer());
		reflectionEnable3d = (_wtoi(ini.getKeyValue(L"3d-reflection-enable", L"skin").GetBuffer()) != 0);
		iconPosition3d = _wtoi(ini.getKeyValue(L"3d-icon-position", L"skin").GetBuffer());
		iconSizeBetween3d = _wtoi(ini.getKeyValue(L"3d-icon-size-between", L"skin").GetBuffer());
		iconReflectionSkipTop3d = _wtoi(ini.getKeyValue(L"3d-icon-reflection-skip-top", L"skin").GetBuffer());
		iconReflectionSkipBottom3d = _wtoi(ini.getKeyValue(L"3d-icon-reflection-skip-bottom", L"skin").GetBuffer());
		iconReflectionOffset3d = _wtoi(ini.getKeyValue(L"3d-icon-reflection-offset", L"skin").GetBuffer());
		iconReflectionOpacity3d = _wtoi(ini.getKeyValue(L"3d-icon-reflection-opacity", L"skin").GetBuffer());
		iconReflectionOpacityFactor3d = _wtoi(ini.getKeyValue(L"3d-icon-reflection-opacity-factor", L"skin").GetBuffer());
		indicatorSkipBottom3d = _wtoi(ini.getKeyValue(L"3d-indicator-skip-bottom", L"skin").GetBuffer());

		if(bckgOpacity3d > 100) bckgOpacity3d = 100;
		if(bckgEdgeAngle3d > 60) bckgEdgeAngle3d = 60;
		if(reflectionOpacity3d > 100) reflectionOpacity3d = 100;
		if(iconPosition3d > 100) iconPosition3d = 100;
		if(iconReflectionOpacity3d > 100) iconReflectionOpacity3d = 100;
		if(iconReflectionOpacityFactor3d > 100) iconReflectionOpacityFactor3d = 100;
		if(iconReflectionSkipTop3d > 100) iconReflectionSkipTop3d = 100;
		if(iconReflectionSkipBottom3d > 100) iconReflectionSkipBottom3d = 100;
		if(indicatorSkipBottom3d > 100) indicatorSkipBottom3d = 100;
		if(iconReflectionOffset3d < -100) iconReflectionOffset3d = -100;
		if(iconReflectionOffset3d > 100) iconReflectionOffset3d = 100;
	}

	blurbehindDockEnabled = (_wtoi(ini.getKeyValue(L"blurbehind-dock-enabled", L"skin").GetBuffer()) != 0);

	if(onlyInformation)
	{
		ready = true;
		return true;
	}

#define LoadPart(key, dib, src) \
	if(!ExtractParams(ini.getKeyValue(key, L"skin"), rect)) \
	{ \
		goto onError; \
	} \
	dib->Resize(max(rect.Width, 1), max(rect.Height, 1)); \
	if(!dib->Ready()) \
	{ \
		goto onError; \
	} \
	dib->Draw(CRect(0, 0, rect.Width, rect.Height), rect.X, rect.Y, &src);

	if(mode & DockMode2D)
	{
		CDIB bckg;
		bckg.Load(dir + ini.getKeyValue(L"2d-background-image", L"skin"));
		if(!bckg.Ready())
		{
			goto onError;
		}

		RectI rect;

		bckgLeftTop2d = new CDIB();
		bckgTop2d = new CDIB();
		bckgRightTop2d = new CDIB();
		bckgRight2d = new CDIB();
		bckgRightBottom2d = new CDIB();
		bckgBottom2d = new CDIB();
		bckgLeftBottom2d = new CDIB();
		bckgLeft2d = new CDIB();
		bckgMiddle2d = new CDIB();

		LoadPart(L"2d-background-left-top", bckgLeftTop2d, bckg);
		LoadPart(L"2d-background-top", bckgTop2d, bckg);
		LoadPart(L"2d-background-right-top", bckgRightTop2d, bckg);
		LoadPart(L"2d-background-right", bckgRight2d, bckg);
		LoadPart(L"2d-background-right-bottom", bckgRightBottom2d, bckg);
		LoadPart(L"2d-background-bottom", bckgBottom2d, bckg);
		LoadPart(L"2d-background-left-bottom", bckgLeftBottom2d, bckg);
		LoadPart(L"2d-background-left", bckgLeft2d, bckg);
		LoadPart(L"2d-background-middle", bckgMiddle2d, bckg);

		bckg.Load(dir + ini.getKeyValue(L"2d-separator-image", L"skin"));
		if(!bckg.Ready())
		{
			goto onError;
		}

		separator2d = new CDIB();
		LoadPart(L"2d-separator-rect", separator2d, bckg);

		bckg.Load(dir + ini.getKeyValue(L"2d-indicator-image", L"skin"));
		if(!bckg.Ready())
		{
			goto onError;
		}

		indicator2d = new CDIB();
		LoadPart(L"2d-indicator-rect", indicator2d, bckg);
	}
	if(mode & DockMode3D)
	{
		CDIB bckg;
		bckg.Load(dir + ini.getKeyValue(L"3d-background-image", L"skin"));
		if(!bckg.Ready())
		{
			goto onError;
		}

		RectI rect;

		bckg3d = new CDIB();
		LoadPart(L"3d-background-rect", bckg3d, bckg);

		bckg.Load(dir + ini.getKeyValue(L"3d-background-edge-image", L"skin"));
		if(!bckg.Ready())
		{
			//goto onError;
		}

		bckgEdge3d = new CDIB();
		if(bckg.Ready())
		{
			LoadPart(L"3d-background-edge-rect", bckgEdge3d, bckg);
		}

		bckg.Load(dir + ini.getKeyValue(L"3d-separator-image", L"skin"));
		if(!bckg.Ready())
		{
			goto onError;
		}

		separator3d = new CDIB();
		LoadPart(L"3d-separator-rect", separator3d, bckg);

		bckg.Load(dir + ini.getKeyValue(L"3d-indicator-image", L"skin"));
		if(!bckg.Ready())
		{
			goto onError;
		}

		indicator3d = new CDIB();
		LoadPart(L"3d-indicator-rect", indicator3d, bckg);
	}

	ready = true;
	return true;

onError:
	FreeNull(bckgLeftTop2d);
	FreeNull(bckgTop2d);
	FreeNull(bckgRightTop2d);
	FreeNull(bckgRight2d);
	FreeNull(bckgRightBottom2d);
	FreeNull(bckgBottom2d);
	FreeNull(bckgLeftBottom2d);
	FreeNull(bckgLeft2d);
	FreeNull(bckgMiddle2d);
	FreeNull(separator2d);
	FreeNull(indicator2d);
	FreeNull(bckg3d);
	FreeNull(bckgEdge3d);
	FreeNull(separator3d);
	FreeNull(indicator3d);

	return false;
}

void CSkin::Unload()
{
	if(!ready)
	{
		return;
	}

	ready = false;

	FreeNull(bckgLeftTop2d);
	FreeNull(bckgTop2d);
	FreeNull(bckgRightTop2d);
	FreeNull(bckgRight2d);
	FreeNull(bckgRightBottom2d);
	FreeNull(bckgBottom2d);
	FreeNull(bckgLeftBottom2d);
	FreeNull(bckgLeft2d);
	FreeNull(bckgMiddle2d);
	FreeNull(separator2d);
	FreeNull(indicator2d);
	FreeNull(bckg3d);
	FreeNull(bckgEdge3d);
	FreeNull(separator3d);
	FreeNull(indicator3d);
}

CSkinLoader::CSkinLoader()
{
}

CSkinLoader::~CSkinLoader()
{
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		delete items.GetAt(p);
		items.GetNext(p);
	}
	items.RemoveAll();
}

void CSkinLoader::EnumSkins(CString path, int flags, void *param)
{
	if(flags != EnumFilesFlagFile)
	{
		return;
	}
	if(path.Mid(path.ReverseFind(L'\\') + 1).MakeLower() != L"skin.ini")
	{
		return;
	}
	CSkinLoader *skinLoader = (CSkinLoader*)param;

	CSkin *skin = new CSkin();
	if(!skin->Load(path, true) || skinLoader->Find(skin->name))
	{
		delete skin;
		return;
	}
	skin->Unload();
	if(!skin->Load())
	{
		delete skin;
		return;
	}
	skin->Unload();
	skinLoader->items.AddTail(skin);
}

bool CSkinLoader::Load(CString path)
{
	EnumFiles(path, EnumSkins, this);
	return !items.IsEmpty();
}

CSkin* CSkinLoader::Find(CString name)
{
	POSITION p = items.GetHeadPosition();
	while(p)
	{
		CSkin *item = items.GetAt(p);
		if(item->name == name)
		{
			return item;
		}
		items.GetNext(p);
	}
	return NULL;
}
