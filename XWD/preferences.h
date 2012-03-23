#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <afxwin.h>
#include <gdiplus.h>
#include <afxole.h>
#include "consts.h"
#include "shellio.h"
#include "dib.h"
#include "icons.h"
#include "imagebutton.h"
#include "tabscontrol.h"
#include "checkbox.h"
#include "trackbar.h"
#include "gcombobox.h"
#include "pluginlist.h"
#include "skinlist.h"

enum
{
	idTabsGeneral,
	idTabsSkins,
	idTabsPlugins
};

class CPreferences: public CFrameWnd
{
public:
	CPreferences();
	~CPreferences();

	BOOL PreCreateWindow(CREATESTRUCT &cs);
	BOOL PreTranslateMessage(MSG *pMSG);

	void Draw(CDIB *dib, CTabControl *tab);
	void DrawAnim(CDIB *dib, CTabControl *tab);

	void OpenTab(CTabControl *tab);

public:
	//CDIB *logo;
	CDIB *dib;
	CTabControl *lastTab;
	CDIB *anim;
	bool animating;

	CImageButton *btnClose;

	CTabsControl *tabs;

	CTrackBar *iconSize;

	CCheckBox *mode2d;
	CCheckBox *mode3d;

	CCheckBox *positionLeft;
	CCheckBox *positionRight;
	CCheckBox *positionTop;
	CCheckBox *positionBottom;

	CCheckBox *runWithWindows;
	CCheckBox *lockItems;
	CCheckBox *dockTopMost;
	CCheckBox *reserveScreen;
	CCheckBox *enableIconsShadow;
	CCheckBox *enableWindowsReflection;
	CCheckBox *checkForUpdates;
	CCheckBox *showAllRunningAppsInDock;

	CGComboBox *monitors;

	CPluginList *pluginsList;
	CSkinList *skinsList;

	CImageButton *btnSkinNext;
	CImageButton *btnSkinPrev;
	CImageButton *btnSkinApply;

	CWnd *notifer;
	int nID;

public:
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnPaint();
	afx_msg void OnNcPaint();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS *lpncsp);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnWindowPosChanged(WINDOWPOS *lpwndpos);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnControlNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();

	DECLARE_MESSAGE_MAP();
};

#endif /* PREFERENCES_H */