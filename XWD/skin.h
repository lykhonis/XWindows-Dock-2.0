#ifndef SKIN_H
#define SKIN_H

#include <afxwin.h>
#include <afxtempl.h>
#include "consts.h"
#include "ini.h"
#include "dib.h"
#include "utils.h"

class CSkin
{
public:
	CSkin();
	~CSkin();

	bool Load(CString fileName, bool onlyInformation);
	bool Load();
	void Unload();

public:
	CString path;
	CString name;
	CString description;

	bool ready;

	unsigned int mode;

	unsigned int bckgOpacity2d;

	CDIB *bckgLeftTop2d;
	CDIB *bckgTop2d;
	CDIB *bckgRightTop2d;
	CDIB *bckgRight2d;
	CDIB *bckgRightBottom2d;
	CDIB *bckgBottom2d;
	CDIB *bckgLeftBottom2d;
	CDIB *bckgLeft2d;
	CDIB *bckgMiddle2d;
	CDIB *separator2d;
	CDIB *indicator2d;

	unsigned int skipLeft2d;
	unsigned int skipTop2d;
	unsigned int skipRight2d;
	unsigned int skipBottom2d;
	unsigned int skipBetweenIcons2d;
	unsigned int indicatorSkipBottom2d;

	CDIB *bckg3d;
	CDIB *bckgEdge3d;
	CDIB *separator3d;
	CDIB *indicator3d;

	bool reflectionEnable3d;
	unsigned int reflectionOpacity3d;
	unsigned int indicatorSkipBottom3d;

	bool blurbehindDockEnabled;

	unsigned int iconReflectionSkipTop3d;
	unsigned int iconReflectionSkipBottom3d;
	int iconReflectionOffset3d;
	unsigned int iconReflectionOpacity3d;
	unsigned int iconReflectionOpacityFactor3d;

	unsigned int iconPosition3d;
	unsigned int iconSizeBetween3d;

	unsigned int bckgOpacity3d;
	unsigned int bckgEdgeOffset3d;
	unsigned int bckgEdgeAngle3d;
};

class CSkinLoader
{
public:
	CSkinLoader();
	~CSkinLoader();

	bool Load(CString path);

	CSkin* Find(CString name);

public:
	CList<CSkin*> items;

private:
	static void EnumSkins(CString path, int flags, void *param);
};

#endif /* SKIN_H */