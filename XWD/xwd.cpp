#include "xwd.h"

BEGIN_MESSAGE_MAP(CXWD, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_QUERYENDSESSION()
	//ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_SHOWWINDOW()
	ON_WM_SETCURSOR()
	ON_WM_COPYDATA()
	ON_WM_KEYDOWN()

	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
	ON_MESSAGE(WM_SETTINGCHANGE, OnSettingChange)

	ON_REGISTERED_MESSAGE(WM_SCREENCMPNOTIFY, OnScreenCmpNotify)
	ON_REGISTERED_MESSAGE(WM_DOCKITEMNOTIFY, OnDockItemNotify)
	ON_REGISTERED_MESSAGE(WM_MOUSENOTIFY, OnMouseNotify)
	ON_REGISTERED_MESSAGE(WM_MENUNOTIFY, OnMenuNotify)
	ON_REGISTERED_MESSAGE(WM_XWDCORENOTIFY, OnCoreNotify)
	ON_REGISTERED_MESSAGE(WM_SETUPDOCK, OnSetupDock)
	ON_REGISTERED_MESSAGE(WM_XWDUPDATENOTIFY, OnXWDUpdateNotify)
	ON_REGISTERED_MESSAGE(WM_CONTROLNOTIFY, OnControlNotify)
	ON_REGISTERED_MESSAGE(WM_EXPOSENOTIFY, OnExposeNotify)
	ON_REGISTERED_MESSAGE(WM_FILECHANGED, OnFileChangesNotify)
END_MESSAGE_MAP()

using namespace Gdiplus;
using namespace ShellIO;

#define DOCK_ANIMATON_RESIZE_DELAY 200
#define DOCK_ANIMATON_ITEM_ADD 350
#define DOCK_APPLICATION_EXPOSE_DELAY 900

enum
{
	timerDockResize = 1,
	timerCheckRecyclerBin,
	timerUpdateApplicationRuning,
	timerDockPopup,
	timerDockActivate,
	timerDockCheckUpdate,
	timerDockCheckUpdateIfFail,
	timerDockApplicationExpose,
	timerDockCheckFullScreen,

	animationFlagDockResizeUp				= 1 << 0,
	animationFlagDockResizeTimer			= 1 << 1,
	animationFlagDockPopup					= 1 << 2,
	animationFlagDockActivate				= 1 << 3,

	idMenuNone = 0,

	idMenuDockAdd,
	//idMenuDockAddFiles,
	idMenuDockAddSeparator,
	idMenuDockAddMyComputer,
	idMenuDockAddRecycleBin,
	idMenuDockAddPlugins,

	idMenuDockSwitchMode,
	idMenuDockMode2D,
	idMenuDockMode3D,
	idMenuDockPositionOnScreen,
	idMenuDockPositionLeft,
	idMenuDockPositionRight,
	idMenuDockPositionTop,
	idMenuDockPositionBottom,
	idMenuDockPluginManager,
	idMenuDockPreferences,
	idMenuDockLock,
	idMenuDockClose,
	idMenuDockAbout,
	//idMenuDockCheckUpdates,

	idMenuItemOpen,
	idMenuItemOpenAs,
	idMenuItemProperties,
	idMenuItemRestoreRecycleBin,
	idMenuItemEmptyRecycleBin,
	idMenuItemShowInExplorer,
	idMenuItemQuitFromApplication,

	idMenuItemOptions,
	idMenuItemOptionsRemoveIcon,
	idMenuItemOptionsKeepInDock,
	idMenuItemOptionsRunWithWindows,

	idMenuItemDropSetupIcon,
	idMenuItemDropRunIn,
	idMenuItemDropCopyTo,
	idMenuItemDropMoveTo,
	idMenuItemDropMoveToRecycleBin,
	idMenuItemDropCancel,

	idPreferences,
	idAbout,
	idIconProperties
};

#define LANGUAGE_MENU_DOCK_ADD L"Add"
#define LANGUAGE_MENU_DOCK_ADD_FILES L"Files / Folders"
#define LANGUAGE_MENU_DOCK_ADD_SEPARATOR L"Separator"
#define LANGUAGE_MENU_DOCK_ADD_MYCOMPUTER L"My Computer"
#define LANGUAGE_MENU_DOCK_ADD_RECYCLEBIN L"Recylce Bin"
#define LANGUAGE_MENU_DOCK_ADD_PLUGINS L"Plugins"

#define LANGUAGE_MENU_DOCK_SWITCHMODE L"Switch Mode"
#define LANGUAGE_MENU_DOCK_MODE_2D L"2D"
#define LANGUAGE_MENU_DOCK_MODE_3D L"3D"
#define LANGUAGE_MENU_DOCK_POSITIONONSCREEN L"Position on Screen"
#define LANGUAGE_MENU_DOCK_POSITION_LEFT L"Left"
#define LANGUAGE_MENU_DOCK_POSITION_RIGHT L"Right"
#define LANGUAGE_MENU_DOCK_POSITION_TOP L"Top"
#define LANGUAGE_MENU_DOCK_POSITION_BOTTOM L"Bottom"
#define LANGUAGE_MENU_DOCK_ITEMS_LOCK L"Lock Items"
#define LANGUAGE_MENU_DOCK_PREFERENCES L"Preferences"
#define LANGUAGE_MENU_DOCK_CLOSE L"Close Dock"
#define LANGUAGE_MENU_DOCK_PLUGINMANAGER L"Plugin Manager"
#define LANGUAGE_MENU_DOCK_ABOUT L"About Dock"
#define LANGUAGE_MENU_DOCK_CHECKUPDATES L"Check for Updates"

#define LANGUAGE_MENU_ITEM_OPEN L"Open"
#define LANGUAGE_MENU_ITEM_OPENAS L"Open as..."
#define LANGUAGE_MENU_ITEM_PROPERTIES L"Properties"
#define LANGUAGE_MENU_ITEM_RECYCLEBINRESTORE L"Restore All Files"
#define LANGUAGE_MENU_ITEM_RECYCLEBINEMPTY L"Empty Recycle Bin"
#define LANGUAGE_MENU_ITEM_SHOWINEXPLORER L"Show in Explorer"
#define LANGUAGE_MENU_ITEM_QUITFROMAPPLICATION L"Quit from Application"

#define LANGUAGE_MENU_ITEM_OPTIONS L"Options"
#define LANGUAGE_MENU_ITEM_OPTIONS_REMOVEICON L"Remove Icon"
#define LANGUAGE_MENU_ITEM_OPTIONS_KEEPINDOCK L"Keep in Dock"
#define LANGUAGE_MENU_ITEM_OPTIONS_RUNWITHWINDOWS L"Run with Windows"

#define LANGUAGE_DROPMENU_COPYTO L"Copy"
#define LANGUAGE_DROPMENU_MOVETO L"Move"
#define LANGUAGE_DROPMENU_RUNIN L"Run"
#define LANGUAGE_DROPMENU_SETUPICON L"Setup Icon"
#define LANGUAGE_DROPMENU_CANCEL L"Cancel"
#define LANGUAGE_DROPMENU_DROPTO L"Drop"

// API
CXWD *xwd = NULL;

// Object Dock API
extern ODDockletIsVisible odDockletIsVisible;
extern ODDockletGetRect odDockletGetRect;
extern ODDockletGetLabel odDockletGetLabel;
extern ODDockletSetLabel odDockletSetLabel;
extern ODDockletLoadGDIPlusImage odDockletLoadGDIPlusImage;
extern ODDockletSetImage odDockletSetImage;
extern ODDockletSetImageFile odDockletSetImageFile;
extern ODDockletSetImageOverlay odDockletSetImageOverlay;
extern ODDockletBrowseForImage odDockletBrowseForImage;
extern ODDockletLockMouseEffect odDockletLockMouseEffect;
extern ODDockletDoAttentionAnimation odDockletDoAttentionAnimation;
extern ODDockletGetRelativeFolder odDockletGetRelativeFolder;
extern ODDockletGetRootFolder odDockletGetRootFolder;
extern ODDockletDefaultConfigDialog odDockletDefaultConfigDialog;
extern ODDockletQueryDockEdge odDockletQueryDockEdge;
extern ODDockletSetDockEdge odDockletSetDockEdge;
extern ODDockletQueryDockAlign odDockletQueryDockAlign;
extern ODDockletSetDockAlign odDockletSetDockAlign;

// XWindows Dock API
typedef XWDBool(*XWDExecProc)(XWDFunction function, XWDError &xwdLastError, va_list args);

extern XWDExecProc xwdExecProc;

CXWD::CXWD()
{
	wchar_t buff[MAX_PATH];

#ifdef _DEBUG
	appPath = L"E:\\Coding_C\\XWD\\XWD\\";
#else
	GetModuleFileName(AfxGetInstanceHandle(), buff, MAX_PATH);
	appPath = buff;
	appPath = appPath.Mid(0, appPath.ReverseFind(L'\\') + 1);
#endif

	CString cmd = CString(AfxGetApp()->m_lpCmdLine).MakeLower().Trim();
	if(cmd == L"-p")
	{
		dataPath = appPath + L"Settings\\";
		portable = true;
	}
	else
	{
		SHGetSpecialFolderPath(NULL, buff, CSIDL_APPDATA, TRUE);
		dataPath = buff;
		if(dataPath.Mid(dataPath.GetLength() - 1) != L'\\')
		{
			dataPath += L'\\';
		}
		dataPath += L"XWindows Dock\\";
		portable = false;
	}
	CreateDirectory(dataPath.GetBuffer(), NULL);

	// setup API
	xwd = this;

	// setup OD Docklets API
	odDockletIsVisible = ODDockletIsVisible;
	odDockletGetRect = ODDockletGetRect;
	odDockletGetLabel = ODDockletGetLabel;
	odDockletSetLabel = ODDockletSetLabel;
	odDockletLoadGDIPlusImage = ODDockletLoadGDIPlusImage;
	odDockletSetImage = ODDockletSetImage;
	odDockletSetImageFile = ODDockletSetImageFile;
	odDockletSetImageOverlay = ODDockletSetImageOverlay;
	odDockletBrowseForImage = ODDockletBrowseForImage;
	odDockletLockMouseEffect = ODDockletLockMouseEffect;
	odDockletDoAttentionAnimation = ODDockletDoAttentionAnimation;
	odDockletGetRelativeFolder = ODDockletGetRelativeFolder;
	odDockletGetRootFolder = ODDockletGetRootFolder;
	odDockletDefaultConfigDialog = ODDockletDefaultConfigDialog;
	odDockletQueryDockEdge = ODDockletQueryDockEdge;
	odDockletSetDockEdge = ODDockletSetDockEdge;
	odDockletQueryDockAlign = ODDockletQueryDockAlign;
	odDockletSetDockAlign = ODDockletSetDockAlign;

	// setup XWD API
	xwdExecProc = XWDExecProc;

	WNDCLASS wndClass = {0};
	wndClass.lpfnWndProc = AfxWndProc;
	wndClass.hInstance = AfxGetInstanceHandle();
	wndClass.hIcon = LoadIcon(AfxGetInstanceHandle(), IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.lpszClassName = L"XWindowsDockClass";
    wndClass.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wndClass.lpszMenuName = NULL;

	AfxRegisterClass(&wndClass);

	CreateEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW, wndClass.lpszClassName, L"XWindows Dock", WS_POPUP, CRect(0, 0, 0, 0), NULL, NULL);
}

CXWD::~CXWD()
{
}

CDockItem* ItemByPublicPluginInfo(XWDPluginInfo *plugin)
{
	POSITION p = xwd->items->GetHeadPosition();
	while (p)
	{
		CDockItem *item = xwd->items->GetAt(p);
		if (item->publicPluginInfo == plugin)
		{
			return item;
		}
		xwd->items->GetNext(p);
	}
	return NULL;
}

XWDId XWDPluginRegister(CString fileName, XWDId hWnd, CString iconName, CString title, XWDId uid, XWDId bounceable)
{
	XWDPluginInfo *plugin;
	if (XWDPAddPluginInfo(fileName, (HWND)hWnd, uid, &plugin))
	{
		plugin->bounceable = bounceable != 0;

		plugin->settingsPath = xwd->dataPath + L"Public Plugins\\";
		CreateDirectory(plugin->settingsPath.GetBuffer(), NULL);

		plugin->settingsPath.AppendFormat(L"%d\\", plugin->uid);
		CreateDirectory(plugin->settingsPath.GetBuffer(), NULL);

		POSITION p = xwd->items->GetHeadPosition();
		while(p)
		{
			if (xwd->items->GetAt(p)->type == DockItemTypeSeparator)
			{
				break;
			}
			xwd->items->GetNext(p);
		}

		CDockItem *item = new CDockItem();
		if(p)
		{
			xwd->items->InsertBefore(p, item);
		}
		else
		{
			xwd->items->AddTail(item);
		}
		item->publicPluginInfo = plugin;
		item->type = DockItemTypePublicPlugin;
		item->dockMode = xwd->dockMode;
		item->poof = xwd->poof;
		item->dockPosition = xwd->dockPosition;
		if(xwd->skin && (item->dockMode & DockMode3D))
		{
			item->reflectionOffset = xwd->iconSize * xwd->skin->iconReflectionOffset3d / 100;
			item->reflectionSkipTop = xwd->iconSize * xwd->skin->iconReflectionSkipTop3d / 100;
			item->reflectionSkipBottom = xwd->iconSize * xwd->skin->iconReflectionSkipBottom3d / 100;
			item->reflectionSize = max(0, item->reflectionOffset + xwd->iconSize * xwd->skin->iconPosition3d / 100);
			item->reflectionOpacity = (unsigned char)(255 * xwd->skin->iconReflectionOpacity3d / 100);
			item->reflectionOpacityFactor = (unsigned char)(xwd->skin->iconReflectionOpacityFactor3d);
			item->iconShadowEnabled = xwd->iconShadowEnabled;
		}
		else
		{
			item->reflectionSize = 0;
		}
		item->path = fileName;
		item->text = title;

		item->icon = iconName;
		if(!item->LoadImage())
		{
			item->image->image->Load(xwd->appPath + L"Images\\icon-blank.png");
			if(item->type != DockItemTypeSeparator)
			{
				item->reflection->image->Assign(item->image->image);
			}
		}
		
		item->indicator->image->Assign((xwd->dockMode == DockMode2D) ? xwd->skin->indicator2d : xwd->skin->indicator3d);
		CRect rect = xwd->ItemIndicatorRect(item);
		item->indicator->SetWindowPos(&xwd->wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);

		item->Rect(xwd->ItemRect(item, xwd->dockPosition));
		item->SetBottomWindow(xwd);
		item->TopMost(xwd->dockIsTopMost);

		xwd->AddItem(item);

		Core::DetachAddNotifer(item->path);
	}
	return plugin->id;
}

XWDVoid XWDPluginRemove(XWDPluginInfo* plugin)
{
	if (GetApplicationCopiesCount(plugin->fileName) <= 1)
	{
		CDockItem *item = ItemByPublicPluginInfo(plugin);
		if (item)
		{
			POSITION p = item->notifications.GetHeadPosition();
			while (p)
			{
				item->notifications.GetAt(p)->ShowWindow(SW_HIDE);
				item->notifications.GetNext(p);
			}
			if (plugin->keepInDock)
			{
				item->IndicatorShow(false, true);
			}
			else
			{
				item->IndicatorShow(false);
				Core::DetachRemoveNotifer(plugin->fileName);
				xwd->RemoveItem(item);
			}
		}
	}
}

XWDVoid XWDPluginSetIcon(XWDPluginInfo* plugin, CString iconName)
{
	CDockItem *item = ItemByPublicPluginInfo(plugin);
	if(xwd && item)
	{
		item->icon = iconName;
		if(!item->LoadImage())
		{
			item->image->image->Load(xwd->appPath + L"Images\\icon-blank.png");
			if(item->type != DockItemTypeSeparator)
			{
				item->reflection->image->Assign(item->image->image);
			}
		}
		item->image->LayerDraw();
		if(xwd->draging && (xwd->dragItem == item))
		{
			item->image->dib->AlphaBlend(item->image->dib->Rect(), 180, DrawFlagsPaint);
		}
		item->image->LayerUpdate();
		item->reflection->image->Assign(item->image->image);
		if(!(item->animationFlags & animationFlagReflectionShow))
		{
			item->reflection->LayerDraw();
			item->reflection->LayerUpdate();
		}
	}
}

CString XWDPluginGetIcon(XWDPluginInfo* plugin)
{
	CDockItem *item = ItemByPublicPluginInfo(plugin);
	if(xwd && item)
	{
		return item->icon;
	}
	return L"";
}

XWDVoid XWDPluginSetTitle(XWDPluginInfo* plugin, CString title)
{
	CDockItem *item = ItemByPublicPluginInfo(plugin);
	if(xwd && item)
	{
		item->text = title;
		if((xwd->hoverItem == item) && xwd->balloonText->IsWindowVisible() && !xwd->balloonText->hiding && (item->text != L" "))
		{
			xwd->balloonText->SetWindowText(item->text.GetBuffer());
		}
	}
}

CString XWDPluginGetTitle(XWDPluginInfo* plugin)
{
	CDockItem *item = ItemByPublicPluginInfo(plugin);
	if(xwd && item)
	{
		return item->text;
	}
	return L"";
}

XWDVoid XWDPluginBounce(XWDPluginInfo* plugin, XWDId mode, XWDId count)
{
	CDockItem *item = ItemByPublicPluginInfo(plugin);
	if(xwd && item)
	{
		item->Bounce((DockItemBounce)mode, count);
	}
}

XWDVoid XWDPluginBounceStop(XWDPluginInfo* plugin)
{
	CDockItem *item = ItemByPublicPluginInfo(plugin);
	if(xwd && item)
	{
		item->BounceStop();
	}
}

XWDVoid XWDPluginSetIndicator(XWDPluginInfo* plugin, XWDId enabled)
{
	CDockItem *item = ItemByPublicPluginInfo(plugin);
	if(xwd && item)
	{
		if(enabled)
		{
			item->identificator |= identificatorIndicator;
			CRect rect = xwd->ItemIndicatorRect(item);
			item->indicator->SetWindowPos(&xwd->wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
			item->IndicatorShow(xwd->IsWindowVisible() && !(xwd->draging && (xwd->dragItem == item)));
		}
		else
		{
			item->identificator &= ~identificatorIndicator;
			item->IndicatorShow(false, true);
		}
	}
}

XWDId XWDPluginGetIndicator(XWDPluginInfo* plugin)
{
	CDockItem *item = ItemByPublicPluginInfo(plugin);
	if(xwd && item)
	{
		return item->identificator & identificatorIndicator;
	}
	return 0;
}

XWDVoid XWDPluginSetNotification(XWDPluginInfo* plugin, XWDId visible, CString text, XWDId position)
{
	CDockItem *item = ItemByPublicPluginInfo(plugin);
	if(xwd && item)
	{
		CNotification *notification = NULL;
		POSITION p = item->notifications.GetHeadPosition();
		while (p)
		{
			if (item->notifications.GetAt(p)->position == (NotificationPosition)position)
			{
				notification = item->notifications.GetAt(p);
				break;
			}
			item->notifications.GetNext(p);
		}

		if(!notification)
		{
			notification = new CNotification();
			notification->CreateEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW, NULL, L"XWindows Dock", WS_POPUP, CRect(0, 0, 0, 0), item->image, NULL);
			item->notifications.AddTail(notification);

			notification->position = (NotificationPosition)position;
		}

		notification->SetWindowText(text);

		if (visible)
		{
			notification->SetWindowPos(&xwd->wndTop, 0, 0, 
				notification->CalculateWidth((int)(xwd->iconSize * 0.45f)), (int)(xwd->iconSize * 0.45f), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
			
			notification->LayerDraw();
			notification->LayerUpdate();

			item->NotificationUpdatePositions();

			notification->ShowWindow(SW_SHOWNOACTIVATE);
		}
		else
		{
			notification->ShowWindow(SW_HIDE);
		}
	}
}

XWDPRect XWDPluginGetIconRect(XWDPluginInfo* plugin)
{
	CDockItem *item = ItemByPublicPluginInfo(plugin);
	XWDPRect rcIcon = {0};
	if(xwd && item)
	{
		CRect rect = xwd->ItemRect(item, xwd->dockPosition);
		rcIcon.left = rect.left;
		rcIcon.top = rect.top;
		rcIcon.width = rect.Width();
		rcIcon.height = rect.Height();
	}
	return rcIcon;
}

XWDId XWDPluginGetDockEdge(XWDPluginInfo* plugin)
{
	CDockItem *item = ItemByPublicPluginInfo(plugin);
	if(xwd && item)
	{
		switch (xwd->dockPosition)
		{
		case DockPositionLeft:
			return 0;

		case DockPositionTop:
			return 1;

		case DockPositionRight:
			return 2;

		//case DockPositionBottom:
		default:
			return 3;
		}
	}
	return (XWDId)-1;
}

XWDVoid XWDPluginAddFolderWatcher(XWDPluginInfo* plugin, XWDId folderId, XWDId actions, CString folder)
{
	CDockItem *item = ItemByPublicPluginInfo(plugin);
	if(xwd && item)
	{
		FolderWatcher::FolderInfo *info = FolderWatcher::AddFolder((int)item, xwd->CFrameWnd::m_hWnd, (WORD)folderId, (WORD)actions, folder, item);
		if (!item->folderWatcherList.Find(info))
		{
			item->folderWatcherList.AddTail(info);
		}
	}
}

XWDVoid XWDPluginRemoveFolderWatcher(XWDPluginInfo* plugin, XWDId folderId)
{
	CDockItem *item = ItemByPublicPluginInfo(plugin);
	if(xwd && item)
	{
		FolderWatcher::FolderInfo *info = FolderWatcher::FindFolderInfo((int)item, (WORD)folderId);
		if (info)
		{
			POSITION p = item->folderWatcherList.Find(info);
			if (p)
			{
				item->folderWatcherList.RemoveAt(p);
			}
			FolderWatcher::RemoveFolder(info);
		}
	}
}

XWDBool CXWD::XWDExecProc(XWDFunction function, XWDError &xwdLastError, va_list args)
{
#define InvalidParam(p) \
	if(p) \
	{ \
		xwdLastError = XWDErrorInvalidParamaters; \
		return XWDFalse; \
	}

#define InvalidParamNull(p) InvalidParam(!p)

	xwdLastError = XWDOk;

	switch(function)
	{
	case XWDGetRootPath:
		{
			XWDChar *tmp = va_arg(args, XWDChar*);
			InvalidParamNull(tmp);
			wcscpy_s(tmp, XWDStringLength, xwd->appPath.GetBuffer());
		}
		break;

	case XWDImageCreate:
		{
			XWDId *image = va_arg(args, XWDId*);
			InvalidParamNull(image);
			(*image) = (XWDId)new CDIB();
		}
		break;

	case XWDImageDelete:
		{
			XWDId image = va_arg(args, XWDId);
			InvalidParamNull(image);
			delete (CDIB*)image;
		}
		break;

	case XWDImageFree:
		{
			XWDId image = va_arg(args, XWDId);
			InvalidParamNull(image);
			((CDIB*)image)->FreeImage();
		}
		break;

	case XWDImageResize:
		{
			XWDId image = va_arg(args, XWDId);
			InvalidParamNull(image);
			XWDUInt16 width = va_arg(args, XWDUInt16);
			InvalidParamNull(width);
			XWDUInt16 height = va_arg(args, XWDUInt16);
			InvalidParamNull(height);
			((CDIB*)image)->Resize(width, height);
		}
		break;

	case XWDImageAssignImage:
		{
			XWDId dst = va_arg(args, XWDId);
			InvalidParamNull(dst);
			XWDId src = va_arg(args, XWDId);
			InvalidParamNull(src);
			((CDIB*)dst)->Assign((CDIB*)src);
		}
		break;

	case XWDImageAssignGdiplusBitmap:
		{
			XWDId image = va_arg(args, XWDId);
			InvalidParamNull(image);
			Bitmap *bitmap = va_arg(args, Bitmap*);
			InvalidParamNull(bitmap);
			((CDIB*)image)->Assign(bitmap);
		}
		break;

	case XWDImageAssignBits:
		{
			XWDId image = va_arg(args, XWDId);
			InvalidParamNull(image);
			XWDUInt16 width = va_arg(args, XWDUInt16);
			InvalidParamNull(width);
			XWDUInt16 height = va_arg(args, XWDUInt16);
			InvalidParamNull(height);
			XWDVoid* scan0 = va_arg(args, XWDVoid*);
			InvalidParamNull(scan0);
			((CDIB*)image)->Assign(width, height, scan0);
		}
		break;

	case XWDImageAssignFile:
		{
			XWDId image = va_arg(args, XWDId);
			InvalidParamNull(image);
			XWDChar *path = va_arg(args, XWDChar*);
			InvalidParamNull(path);
			((CDIB*)image)->FreeImage();
			CString icon = path;
			CString ext = icon.Mid(icon.ReverseFind(L'.')).MakeLower();
			if((ext != L".ico") && (ext != L".icon"))
			{
				((CDIB*)image)->Load(icon);
			}
			if(!((CDIB*)image)->Ready())
			{
				Icons::GetIcon(icon, ((CDIB*)image));
			}
		}
		break;

	case XWDImageFillFull:
		{
			XWDId image = va_arg(args, XWDId);
			InvalidParamNull(image);
			((CDIB*)image)->Fill();
		}
		break;

	case XWDImageFill:
		{
			XWDId image = va_arg(args, XWDId);
			InvalidParamNull(image);
			XWDUInt16 left = va_arg(args, XWDUInt16);
			XWDUInt16 top = va_arg(args, XWDUInt16);
			XWDUInt16 width = va_arg(args, XWDUInt16);
			XWDUInt16 height = va_arg(args, XWDUInt16);
			((CDIB*)image)->Fill(left, top, width, height);
		}
		break;

	case XWDImageReady:
		{
			XWDId image = va_arg(args, XWDId);
			InvalidParamNull(image);
			XWDBool *result = va_arg(args, XWDBool*);
			InvalidParamNull(result);
			(*result) = ((CDIB*)image)->Ready();
		}
		break;

	case XWDImageGetRect:
		{
			XWDId image = va_arg(args, XWDId);
			InvalidParamNull(image);
			XWDRect *result = va_arg(args, XWDRect*);
			InvalidParamNull(result);
			CRect rect = ((CDIB*)image)->Rect();
			result->left = (XWDUInt16)rect.left;
			result->top = (XWDUInt16)rect.top;
			result->width = (XWDUInt16)rect.Width();
			result->height = (XWDUInt16)rect.Height();
		}
		break;

	case XWDImageGetPixels:
		{
			XWDId image = va_arg(args, XWDId);
			InvalidParamNull(image);
			XWDUInt16 left = va_arg(args, XWDUInt16);
			XWDUInt16 top = va_arg(args, XWDUInt16);
			XWDPixel **result = va_arg(args, XWDPixel**);
			InvalidParamNull(result);
			(*result) = (XWDPixel*)((CDIB*)image)->Pixels(left, top);
		}
		break;

	case XWDImageGetScan0:
		{
			XWDId image = va_arg(args, XWDId);
			InvalidParamNull(image);
			XWDPixel **result = va_arg(args, XWDPixel**);
			InvalidParamNull(result);
			(*result) = (XWDPixel*)((CDIB*)image)->scan0;
		}
		break;

	case XWDImageGetHDC:
		{
			XWDId image = va_arg(args, XWDId);
			InvalidParamNull(image);
			HDC *result = va_arg(args, HDC*);
			InvalidParamNull(result);
			(*result) = ((CDIB*)image)->dc;
		}
		break;

	case XWDImageGetHBITMAP:
		{
			XWDId image = va_arg(args, XWDId);
			InvalidParamNull(image);
			HBITMAP *result = va_arg(args, HBITMAP*);
			InvalidParamNull(result);
			(*result) = ((CDIB*)image)->bitmap;
		}
		break;

	case XWDImageGetGdiplusBitmap:
		{
			XWDId image = va_arg(args, XWDId);
			InvalidParamNull(image);
			Bitmap **result = va_arg(args, Bitmap**);
			InvalidParamNull(result);
			(*result) = ((CDIB*)image)->bmp;
		}
		break;

	case XWDImageRotate180:
		{
			XWDId image = va_arg(args, XWDId);
			InvalidParamNull(image);
			((CDIB*)image)->ReflectVertical();
		}
		break;

	case XWDDockletGetRelativePath:
		{
			XWDId id = va_arg(args, XWDId);
			InvalidParamNull(id);
			POSITION p = xwd->items->Find((CDockItem*)id);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDChar *tmp = va_arg(args, XWDChar*);
			InvalidParamNull(tmp);
			CString path = item->path;
			path.Delete(0, xwd->appPath.GetLength());
			path = path.Mid(0, path.ReverseFind(L'\\') + 1);
			wcscpy_s(tmp, XWDStringLength, path.GetBuffer());
		}
		break;

	case XWDDockletGetPath:
		{
			XWDId id = va_arg(args, XWDId);
			InvalidParamNull(id);
			POSITION p = xwd->items->Find((CDockItem*)id);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDChar *tmp = va_arg(args, XWDChar*);
			InvalidParamNull(tmp);
			CString path = item->path;
			path = path.Mid(0, path.ReverseFind(L'\\') + 1);
			wcscpy_s(tmp, XWDStringLength, path.GetBuffer());
		}
		break;

	case XWDDockletGetSettingsPath:
		{
			XWDId id = va_arg(args, XWDId);
			InvalidParamNull(id);
			POSITION p = xwd->items->Find((CDockItem*)id);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDChar *tmp = va_arg(args, XWDChar*);
			InvalidParamNull(tmp);
			CString path;
			path.Format(L"%sPlugins\\%d\\", xwd->dataPath, item->pluginId);
			CreateDirectory(path.GetBuffer(), NULL);
			wcscpy_s(tmp, XWDStringLength, path.GetBuffer());
		}
		break;

	case XWDDockletLockMouseEffect:
		{
			XWDId id = va_arg(args, XWDId);
			InvalidParamNull(id);
			POSITION p = xwd->items->Find((CDockItem*)id);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDBool lockMouse = va_arg(args, XWDBool);
			if(xwd && item && !(item->animationFlags & animationFlagPoof))
			{
				if(lockMouse)
				{
					xwd->lockMouseEffect++;
					if(xwd->balloonText->IsWindowVisible() && !xwd->balloonText->hiding)
					{
						xwd->balloonText->ShowWindow(SW_HIDE);
					}
				}
				else
				if(xwd->lockMouseEffect)
				{
					xwd->lockMouseEffect--;
					if(!xwd->lockMouseEffect && (xwd->hoverItem == item) && !xwd->balloonText->hiding && (item->text != L" "))
					{
						xwd->balloonText->SetWindowText(item->text.GetBuffer());
						xwd->balloonText->Popup();
					}
				}
			}
		}
		break;

	case XWDDockletGetPluginId:
		{
			XWDId id = va_arg(args, XWDId);
			InvalidParamNull(id);
			POSITION p = xwd->items->Find((CDockItem*)id);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDUInt32 *pluginId = va_arg(args, XWDUInt32*);
			InvalidParamNull(pluginId);
			(*pluginId) = item->pluginId;
		}
		break;

	case XWDDockletGetPanelHWND:
		{
			HWND *hWnd = va_arg(args, HWND*);
			InvalidParamNull(hWnd);
			(*hWnd) = xwd->CFrameWnd::m_hWnd;
		}
		break;

	case XWDDockletGetIconHWND:
		{
			XWDId id = va_arg(args, XWDId);
			InvalidParamNull(id);
			POSITION p = xwd->items->Find((CDockItem*)id);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			HWND *hWnd = va_arg(args, HWND*);
			InvalidParamNull(hWnd);
			(*hWnd) = item->image->m_hWnd;
		}
		break;

	case XWDDockletGetRect:
		{
			XWDId id = va_arg(args, XWDId);
			InvalidParamNull(id);
			POSITION p = xwd->items->Find((CDockItem*)id);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDRect *result = va_arg(args, XWDRect*);
			InvalidParamNull(result);
			if(!(item->animationFlags & animationFlagPoof))
			{
				CRect rect = xwd->ItemRect(item, xwd->dockPosition);
				result->left = (XWDUInt16)rect.left;
				result->top = (XWDUInt16)rect.top;
				result->width = (XWDUInt16)rect.Width();
				result->height = (XWDUInt16)rect.Height();
			}
			else
			{
				xwdLastError = XWDErrorUnknown;
				return XWDFalse;
			}
		}
		break;

	case XWDDockletGetImage:
		{
			XWDId id = va_arg(args, XWDId);
			InvalidParamNull(id);
			POSITION p = xwd->items->Find((CDockItem*)id);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDId *image = va_arg(args, XWDId*);
			InvalidParamNull(image);
			if(!(item->animationFlags & animationFlagPoof))
			{
				XWDExec(XWDImageCreate, image);
				XWDExec(XWDImageAssignImage, *image, (XWDId)item->image->image);
			}
			else
			{
				xwdLastError = XWDErrorUnknown;
				return XWDFalse;
			}
		}
		break;

	case XWDDockletSetImage:
		{
			XWDId id = va_arg(args, XWDId);
			InvalidParamNull(id);
			POSITION p = xwd->items->Find((CDockItem*)id);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDId image = va_arg(args, XWDId);
			InvalidParamNull(image);
			if(!(item->animationFlags & animationFlagPoof))
			{
				item->image->image->Assign((CDIB*)image);
				item->image->LayerDraw();
				if(xwd->draging && (xwd->dragItem == item))
				{
					item->image->dib->AlphaBlend(item->image->dib->Rect(), 180, DrawFlagsPaint);
				}
				item->image->LayerUpdate();
				item->reflection->image->Assign(item->image->image);
				if(!(item->animationFlags & animationFlagReflectionShow))
				{
					item->reflection->LayerDraw();
					item->reflection->LayerUpdate();
				}
			}
			else
			{
				xwdLastError = XWDErrorUnknown;
				return XWDFalse;
			}
		}
		break;

	case XWDDockletGetLabel:
		{
			XWDId id = va_arg(args, XWDId);
			InvalidParamNull(id);
			POSITION p = xwd->items->Find((CDockItem*)id);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDChar *label = va_arg(args, XWDChar*);
			InvalidParamNull(label);
			if(!(item->animationFlags & animationFlagPoof))
			{
				wcscpy_s(label, XWDStringLength, item->text.GetBuffer());
			}
			else
			{
				xwdLastError = XWDErrorUnknown;
				return XWDFalse;
			}
		}
		break;

	case XWDDockletSetLabel:
		{
			XWDId id = va_arg(args, XWDId);
			InvalidParamNull(id);
			POSITION p = xwd->items->Find((CDockItem*)id);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDChar *label = va_arg(args, XWDChar*);
			if(!(item->animationFlags & animationFlagPoof))
			{
				if(!label)
				{
					item->text.Empty();
				}
				else
				{
					item->text = label;
				}
				if((xwd->hoverItem == item) && xwd->balloonText->IsWindowVisible() && !xwd->balloonText->hiding && (item->text != L" "))
				{
					xwd->balloonText->SetWindowText(item->text.GetBuffer());
				}
			}
			else
			{
				xwdLastError = XWDErrorUnknown;
				return XWDFalse;
			}
		}
		break;

	case XWDDockletGetLabelVisible:
		{
			XWDId id = va_arg(args, XWDId);
			InvalidParamNull(id);
			POSITION p = xwd->items->Find((CDockItem*)id);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDBool *result = va_arg(args, XWDBool*);
			InvalidParamNull(result);
			if(!(item->animationFlags & animationFlagPoof))
			{
				(*result) = (xwd->hoverItem == item) && xwd->balloonText->IsWindowVisible();
			}
			else
			{
				xwdLastError = XWDErrorUnknown;
				return XWDFalse;
			}
		}
		break;

	case XWDDockletGetDockMonitor:
		{
			XWDUInt8 *result = va_arg(args, XWDUInt8*);
			InvalidParamNull(result);
			if(xwd->monitor)
			{
				XWDUInt8 i = 0;
				POSITION p = xwd->monitors->items.GetHeadPosition();
				while(p)
				{
					if(xwd->monitors->items.GetAt(p) == xwd->monitor)
					{
						break;
					}
					i++;
					xwd->monitors->items.GetNext(p);
				}
				(*result) = i;
			}
			else
			{
				xwdLastError = XWDErrorUnknown;
				return XWDFalse;
			}
		}
		break;

	case XWDDockletGetDockEdge:
		{
			XWDUInt8 *result = va_arg(args, XWDUInt8*);
			InvalidParamNull(result);
			switch(xwd->dockPosition)
			{
			case DockPositionLeft:
				{
					(*result) = 0;
				}
				break;

			case DockPositionTop:
				{
					(*result) = 1;
				}
				break;

			case DockPositionRight:
				{
					(*result) = 2;
				}
				break;

			//case DockPositionBottom:
			default:
				{
					(*result) = 3;
				}
				break;
			}
		}
		break;

	case XWDDockletGetIndicator:
		{
			XWDId id = va_arg(args, XWDId);
			InvalidParamNull(id);
			POSITION p = xwd->items->Find((CDockItem*)id);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDBool *result = va_arg(args, XWDBool*);
			InvalidParamNull(result);
			if(!(item->animationFlags & animationFlagPoof))
			{
				(*result) = item->identificator & identificatorIndicator;
			}
			else
			{
				xwdLastError = XWDErrorUnknown;
				return XWDFalse;
			}
		}
		break;

	case XWDDockletSetIndicator:
		{
			XWDId id = va_arg(args, XWDId);
			InvalidParamNull(id);
			POSITION p = xwd->items->Find((CDockItem*)id);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDBool enabled = va_arg(args, XWDBool);
			if(!(item->animationFlags & animationFlagPoof))
			{
				if(enabled)
				{
					item->identificator |= identificatorIndicator;
					CRect rect = xwd->ItemIndicatorRect(item);
					item->indicator->SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
					item->IndicatorShow(xwd->IsWindowVisible() && !(xwd->draging && (xwd->dragItem == item)));
				}
				else
				{
					item->identificator &= ~identificatorIndicator;
					item->IndicatorShow(false, true);
				}
			}
			else
			{
				xwdLastError = XWDErrorUnknown;
				return XWDFalse;
			}
		}
		break;

	case XWDDockletMenuAdd:
		{
			XWDId itemId = va_arg(args, XWDId);
			InvalidParamNull(itemId);
			POSITION p = xwd->items->Find((CDockItem*)itemId);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDId parent = va_arg(args, XWDId);
			XWDId* id = va_arg(args, XWDId*);
			XWDChar* text = va_arg(args, XWDChar*);
			XWDMenuFlags flags = va_arg(args, XWDMenuFlags);
			if(!(item->animationFlags & animationFlagPoof))
			{
				CLMenuItem *itemParent = xwd->menu->GetItem(parent);
				CLMenu *menu = (itemParent == NULL ? xwd->menu : itemParent->menu);
				CLMenuItem *menuItem;
				if(flags & XWDMenuFlagsLine)
				{
					if(id)
					{
						(*id) = 0;
					}
					menuItem = menu->InsertLine(NULL, false);
				}
				else
				{
					if(id)
					{
						(*id) = 1;
						while(xwd->menu->GetItem(*id))
						{
							(*id)++;
						}
					}
					menuItem = menu->Insert(NULL, text, id ? (*id) : 0, (flags & XWDMenuFlagsCheckbox) == XWDMenuFlagsCheckbox, false);
					if(flags & XWDMenuFlagsChecked)
					{
						menuItem->checked = true;
					}
				}
				menuItem->visible = false;
				item->menu.AddTail(menuItem);
			}
			else
			{
				xwdLastError = XWDErrorUnknown;
				return XWDFalse;
			}
		}
		break;

	case XWDDockletMenuRemove:
		{
			XWDId itemId = va_arg(args, XWDId);
			InvalidParamNull(itemId);
			POSITION p = xwd->items->Find((CDockItem*)itemId);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDId menuId = va_arg(args, XWDId);
			InvalidParamNull(menuId);
			CLMenuItem *menuItem = xwd->menu->GetItem(menuId);
			InvalidParamNull(menuItem);
			p = item->menu.Find(menuItem);
			InvalidParamNull(p);
			xwd->menu->Remove(menuItem);
		}
		break;

	case XWDDockletMenuGetText:
		{
			XWDId itemId = va_arg(args, XWDId);
			InvalidParamNull(itemId);
			POSITION p = xwd->items->Find((CDockItem*)itemId);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDId menuId = va_arg(args, XWDId);
			InvalidParamNull(menuId);
			CLMenuItem *menuItem = xwd->menu->GetItem(menuId);
			InvalidParamNull(menuItem);
			p = item->menu.Find(menuItem);
			InvalidParamNull(p);
			XWDChar *text = va_arg(args, XWDChar*);
			InvalidParamNull(text);
			wcscpy_s(text, XWDStringLength, menuItem->text.GetBuffer());
		}
		break;

	case XWDDockletMenuSetText:
		{
			XWDId itemId = va_arg(args, XWDId);
			InvalidParamNull(itemId);
			POSITION p = xwd->items->Find((CDockItem*)itemId);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDId menuId = va_arg(args, XWDId);
			InvalidParamNull(menuId);
			CLMenuItem *menuItem = xwd->menu->GetItem(menuId);
			InvalidParamNull(menuItem);
			p = item->menu.Find(menuItem);
			InvalidParamNull(p);
			XWDChar *text = va_arg(args, XWDChar*);
			if(text)
			{
				menuItem->text = text;
			}
			else
			{
				menuItem->text.Empty();
			}
		}
		break;

	case XWDDockletMenuGetChecked:
		{
			XWDId itemId = va_arg(args, XWDId);
			InvalidParamNull(itemId);
			POSITION p = xwd->items->Find((CDockItem*)itemId);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDId menuId = va_arg(args, XWDId);
			InvalidParamNull(menuId);
			CLMenuItem *menuItem = xwd->menu->GetItem(menuId);
			InvalidParamNull(menuItem);
			p = item->menu.Find(menuItem);
			InvalidParamNull(p);
			XWDBool *checked = va_arg(args, XWDBool*);
			InvalidParamNull(checked);
			(*checked) = menuItem->checked;
		}
		break;

	case XWDDockletMenuSetChecked:
		{
			XWDId itemId = va_arg(args, XWDId);
			InvalidParamNull(itemId);
			POSITION p = xwd->items->Find((CDockItem*)itemId);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDId menuId = va_arg(args, XWDId);
			InvalidParamNull(menuId);
			CLMenuItem *menuItem = xwd->menu->GetItem(menuId);
			InvalidParamNull(menuItem);
			p = item->menu.Find(menuItem);
			InvalidParamNull(p);
			menuItem->checked = va_arg(args, XWDBool) == XWDTrue;
		}
		break;

	case XWDDockletMenuGetEnabled:
		{
			XWDId itemId = va_arg(args, XWDId);
			InvalidParamNull(itemId);
			POSITION p = xwd->items->Find((CDockItem*)itemId);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDId menuId = va_arg(args, XWDId);
			InvalidParamNull(menuId);
			CLMenuItem *menuItem = xwd->menu->GetItem(menuId);
			InvalidParamNull(menuItem);
			p = item->menu.Find(menuItem);
			InvalidParamNull(p);
			XWDBool *enabled = va_arg(args, XWDBool*);
			InvalidParamNull(enabled);
			(*enabled) = menuItem->enabled;
		}
		break;

	case XWDDockletMenuSetEnabled:
		{
			XWDId itemId = va_arg(args, XWDId);
			InvalidParamNull(itemId);
			POSITION p = xwd->items->Find((CDockItem*)itemId);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDId menuId = va_arg(args, XWDId);
			InvalidParamNull(menuId);
			CLMenuItem *menuItem = xwd->menu->GetItem(menuId);
			InvalidParamNull(menuItem);
			p = item->menu.Find(menuItem);
			InvalidParamNull(p);
			menuItem->enabled = va_arg(args, XWDBool) == XWDTrue;
		}
		break;

	case XWDDockletNotificationCreate:
		{
			XWDId itemId = va_arg(args, XWDId);
			InvalidParamNull(itemId);
			POSITION p = xwd->items->Find((CDockItem*)itemId);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDId *result = va_arg(args, XWDId*);
			InvalidParamNull(result);
			CNotification *notification = new CNotification();
			notification->CreateEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW, NULL, L"XWindows Dock", WS_POPUP, CRect(0, 0, 0, 0), item->image, NULL);
			item->notifications.AddTail(notification);
			(*result) = (XWDId)notification;
		}
		break;

	case XWDDockletNotificationDelete:
		{
			XWDId itemId = va_arg(args, XWDId);
			InvalidParamNull(itemId);
			POSITION p = xwd->items->Find((CDockItem*)itemId);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDId notification = va_arg(args, XWDId);
			InvalidParamNull(notification);
			p = item->notifications.Find((CNotification*)notification);
			InvalidParamNull(p);
			item->notifications.RemoveAt(p);
			((CNotification*)notification)->DestroyWindow();
		}
		break;

	case XWDDockletNotificationGetPosition:
		{
			XWDId itemId = va_arg(args, XWDId);
			InvalidParamNull(itemId);
			POSITION p = xwd->items->Find((CDockItem*)itemId);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDId notification = va_arg(args, XWDId);
			InvalidParamNull(notification);
			p = item->notifications.Find((CNotification*)notification);
			InvalidParamNull(p);
			XWDUInt8 *result = va_arg(args, XWDUInt8*);
			InvalidParamNull(result);
			(*result) = (XWDUInt8)((CNotification*)notification)->position;
		}
		break;

	case XWDDockletNotificationSetPosition:
		{
			XWDId itemId = va_arg(args, XWDId);
			InvalidParamNull(itemId);
			POSITION p = xwd->items->Find((CDockItem*)itemId);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDId notification = va_arg(args, XWDId);
			InvalidParamNull(notification);
			p = item->notifications.Find((CNotification*)notification);
			InvalidParamNull(p);
			XWDUInt8 position = va_arg(args, XWDUInt8);
			((CNotification*)notification)->position = (NotificationPosition)position;
			item->NotificationUpdatePositions();
		}
		break;

	case XWDDockletNotificationGetVisible:
		{
			XWDId itemId = va_arg(args, XWDId);
			InvalidParamNull(itemId);
			POSITION p = xwd->items->Find((CDockItem*)itemId);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDId notification = va_arg(args, XWDId);
			InvalidParamNull(notification);
			p = item->notifications.Find((CNotification*)notification);
			InvalidParamNull(p);
			XWDBool *result = va_arg(args, XWDBool*);
			InvalidParamNull(result);
			(*result) = (XWDBool)((CNotification*)notification)->IsWindowVisible();
		}
		break;

	case XWDDockletNotificationSetVisible:
		{
			XWDId itemId = va_arg(args, XWDId);
			InvalidParamNull(itemId);
			POSITION p = xwd->items->Find((CDockItem*)itemId);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDId notification = va_arg(args, XWDId);
			InvalidParamNull(notification);
			p = item->notifications.Find((CNotification*)notification);
			InvalidParamNull(p);
			XWDBool visible = va_arg(args, XWDBool);
			((CNotification*)notification)->ShowWindow(visible ? SW_SHOWNOACTIVATE : SW_HIDE);
		}
		break;

	case XWDDockletNotificationGetText:
		{
			XWDId itemId = va_arg(args, XWDId);
			InvalidParamNull(itemId);
			POSITION p = xwd->items->Find((CDockItem*)itemId);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDId notification = va_arg(args, XWDId);
			InvalidParamNull(notification);
			p = item->notifications.Find((CNotification*)notification);
			InvalidParamNull(p);
			XWDChar* text = va_arg(args, XWDChar*);
			InvalidParamNull(text);
			((CNotification*)notification)->GetWindowText(text, XWDStringLength);
		}
		break;

	case XWDDockletNotificationSetText:
		{
			XWDId itemId = va_arg(args, XWDId);
			InvalidParamNull(itemId);
			POSITION p = xwd->items->Find((CDockItem*)itemId);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDId notification = va_arg(args, XWDId);
			InvalidParamNull(notification);
			p = item->notifications.Find((CNotification*)notification);
			InvalidParamNull(p);
			XWDChar* text = va_arg(args, XWDChar*);
			((CNotification*)notification)->SetWindowText(text);
			((CNotification*)notification)->SetWindowPos(&wndTop, 0, 0, 
				((CNotification*)notification)->CalculateWidth((int)(xwd->iconSize * 0.45f)), (int)(xwd->iconSize * 0.45f), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
			((CNotification*)notification)->LayerDraw();
			((CNotification*)notification)->LayerUpdate();
			item->NotificationUpdatePositions();
		}
		break;

	case XWDDockletTimerSet:
		{
			XWDId itemId = va_arg(args, XWDId);
			InvalidParamNull(itemId);
			POSITION p = xwd->items->Find((CDockItem*)itemId);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDId id = va_arg(args, XWDId); 
			InvalidParam(id < 100);
			XWDUInt32 elapse = va_arg(args, XWDUInt32); 
			InvalidParamNull(elapse);
			item->KillTimer(id);
			item->SetTimer(id, elapse, NULL);
		}
		break;

	case XWDDockletTimerDelete:
		{
			XWDId itemId = va_arg(args, XWDId);
			InvalidParamNull(itemId);
			POSITION p = xwd->items->Find((CDockItem*)itemId);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDId id = va_arg(args, XWDId); 
			InvalidParam(id < 100);
			item->KillTimer(id);
		}
		break;

	case XWDDockletBounce:
		{
			XWDId itemId = va_arg(args, XWDId);
			InvalidParamNull(itemId);
			POSITION p = xwd->items->Find((CDockItem*)itemId);
			InvalidParamNull(p);

			if(xwd->nowIsFullScreen)
			{
				xwdLastError = XWDErrorInternal;
				return XWDFalse;
			}

			CDockItem *item = xwd->items->GetAt(p);
			XWDUInt8 mode = va_arg(args, XWDUInt8);
			XWDUInt32 count = va_arg(args, XWDUInt32);
			item->Bounce((DockItemBounce)mode, count);
		}
		break;

	case XWDDockletBounceStop:
		{
			XWDId itemId = va_arg(args, XWDId);
			InvalidParamNull(itemId);
			POSITION p = xwd->items->Find((CDockItem*)itemId);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			item->BounceStop();
		}
		break;

	case XWDDockletIsBouncing:
		{
			XWDId itemId = va_arg(args, XWDId);
			InvalidParamNull(itemId);
			POSITION p = xwd->items->Find((CDockItem*)itemId);
			InvalidParamNull(p);
			CDockItem *item = xwd->items->GetAt(p);
			XWDBool *result = va_arg(args, XWDBool*);
			InvalidParamNull(result);
			(*result) = item->animationFlags & animationFlagBounce ? XWDTrue : XWDFalse;
		}
		break;

	default:
		{
			xwdLastError = XWDErrorUnsupportedFunction;
			return XWDFalse;
		}
		break;
	}
	return XWDTrue;
}

BOOL CXWD::ODDockletIsVisible(HWND hwndDocklet)
{
	TRACE("ODDockletIsVisible\n");
	CDockItem *item = (CDockItem*)GetProp(hwndDocklet, L"DockItem");
	if(xwd && item && !(item->animationFlags & animationFlagPoof))
	{
		return item->image->IsWindowVisible();
	}
	return FALSE;
}

BOOL CXWD::ODDockletGetRect(HWND hwndDocklet, RECT *rcDocklet)
{
	TRACE("ODDockletGetRect\n");
	CDockItem *item = (CDockItem*)GetProp(hwndDocklet, L"DockItem");
	if(xwd && item && rcDocklet && !(item->animationFlags & animationFlagPoof))
	{
		CRect rect = xwd->ItemRect(item, xwd->dockPosition);
		rcDocklet->left = rect.left;
		rcDocklet->top = rect.top;
		rcDocklet->bottom = rect.bottom;
		rcDocklet->right = rect.right;
		return TRUE;
	}
	return FALSE;
}

int CXWD::ODDockletGetLabel(HWND hwndDocklet, char *szLabel)
{
	TRACE("ODDockletGetLabel\n");
	CDockItem *item = (CDockItem*)GetProp(hwndDocklet, L"DockItem");
	if(xwd && item && !(item->animationFlags & animationFlagPoof))
	{
		if(szLabel)
		{
			strcpy_s(szLabel, MAX_PATH, CStringA(item->text).GetBuffer());
		}
		return (item->text.GetLength() + 1);
	}
	return 0;
}

int CXWD::ODDockletSetLabel(HWND hwndDocklet, char *szLabel)
{
	TRACE("ODDockletSetLabel\n");
	CDockItem *item = (CDockItem*)GetProp(hwndDocklet, L"DockItem");
	if(xwd && item && !(item->animationFlags & animationFlagPoof))
	{
		item->text = szLabel;
		if((xwd->hoverItem == item) && xwd->balloonText->IsWindowVisible() && !xwd->balloonText->hiding && (item->text != L" "))
		{
			xwd->balloonText->SetWindowText(item->text.GetBuffer());
		}
		return (item->text.GetLength() + 1);
	}
	return 0;
}

Gdiplus::Bitmap* CXWD::ODDockletLoadGDIPlusImage(char *szImage)
{
	TRACE("ODDockletLoadGDIPlusImage\n");
	if(!szImage)
	{
		return NULL;
	}
	CDIB tmp;
	CString icon = szImage;
	if(icon.Find(L':') < 0)
	{
		icon = xwd->appPath + szImage;
	}
	CString ext = icon.Mid(icon.ReverseFind(L'.')).MakeLower();
	if((ext != L".ico") && (ext != L".icon"))
	{
		tmp.Load(icon);
	}
	if(!tmp.Ready())
	{
		Icons::GetIcon(icon, &tmp);
	}
	Bitmap *bmp = new Bitmap(tmp.Width(), tmp.Height(), PixelFormat32bppARGB);
	BitmapData bmpData;
	if(bmp->LockBits(NULL, ImageLockModeRead, PixelFormat32bppARGB, &bmpData) == Ok)
	{
		memcpy(bmpData.Scan0, tmp.scan0, tmp.Size());
		bmp->UnlockBits(&bmpData);
	}
	return bmp;
}

void CXWD::ODDockletSetImage(HWND hwndDocklet, Gdiplus::Image *lpImageNew, BOOL bAutomaticallyDeleteImage)
{
	TRACE("ODDockletSetImage\n");
	CDockItem *item = (CDockItem*)GetProp(hwndDocklet, L"DockItem");
	if(xwd && item && lpImageNew && !(item->animationFlags & animationFlagPoof))
	{
		item->image->image->Assign((Bitmap*)lpImageNew);
		item->image->LayerDraw();
		if(xwd->draging && (xwd->dragItem == item))
		{
			item->image->dib->AlphaBlend(item->image->dib->Rect(), 180, DrawFlagsPaint);
		}
		item->image->LayerUpdate();
		item->reflection->image->Assign(item->image->image);
		if(!(item->animationFlags & animationFlagReflectionShow))
		{
			item->reflection->LayerDraw();
			item->reflection->LayerUpdate();
		}
		if(bAutomaticallyDeleteImage)
		{
			delete lpImageNew;
		}
	}
}

void CXWD::ODDockletSetImageFile(HWND hwndDocklet, char *szImage)
{
	TRACE("ODDockletSetImageFile\n");
	CDockItem *item = (CDockItem*)GetProp(hwndDocklet, L"DockItem");
	if(xwd && item && szImage && !(item->animationFlags & animationFlagPoof))
	{
		CString icon = szImage;
		if(icon.Find(L':') < 0)
		{
			icon = xwd->appPath + szImage;
		}
		CString ext = icon.Mid(icon.ReverseFind(L'.')).MakeLower();
		if((ext != L".ico") && (ext != L".icon"))
		{
			item->image->image->Load(icon);
		}
		if(!item->image->image->Ready())
		{
			Icons::GetIcon(icon, item->image->image);
		}
		item->image->LayerDraw();
		if(xwd->draging && (xwd->dragItem == item))
		{
			item->image->dib->AlphaBlend(item->image->dib->Rect(), 180, DrawFlagsPaint);
		}
		item->image->LayerUpdate();
		item->reflection->image->Assign(item->image->image);
		if(!(item->animationFlags & animationFlagReflectionShow))
		{
			item->reflection->LayerDraw();
			item->reflection->LayerUpdate();
		}
	}
}

void CXWD::ODDockletSetImageOverlay(HWND hwndDocklet, Gdiplus::Image *imageOverlay, BOOL bAutomaticallyDeleteImage)
{
	TRACE("ODDockletSetImageOverlay\n");
	CDockItem *item = (CDockItem*)GetProp(hwndDocklet, L"DockItem");
	if(xwd && item && !(item->animationFlags & animationFlagPoof))
	{
		item->image->imageOverlay->Assign((Bitmap*)imageOverlay);
		item->image->LayerDraw();
		if(xwd->draging && (xwd->dragItem == item))
		{
				item->image->dib->AlphaBlend(item->image->dib->Rect(), 180, DrawFlagsPaint);
		}
		item->image->LayerUpdate();
		item->reflection->imageOverlay->Assign((Bitmap*)imageOverlay);
		if(!(item->animationFlags & animationFlagReflectionShow))
		{
			item->reflection->LayerDraw();
			item->reflection->LayerUpdate();
		}
		if(bAutomaticallyDeleteImage)
		{
			delete imageOverlay;
		}
	}
}

BOOL CXWD::ODDockletBrowseForImage(HWND, char *, char *)
{
	TRACE("ODDockletBrowseForImage\n");
	return FALSE;
}

void CXWD::ODDockletLockMouseEffect(HWND hwndDocklet, BOOL bLock)
{
	TRACE("ODDockletLockMouseEffect\n");
	CDockItem *item = (CDockItem*)GetProp(hwndDocklet, L"DockItem");
	if(xwd && item && !(item->animationFlags & animationFlagPoof))
	{
		if(bLock)
		{
			xwd->lockMouseEffect++;
			if(xwd->balloonText->IsWindowVisible() && !xwd->balloonText->hiding)
			{
				xwd->balloonText->ShowWindow(SW_HIDE);
			}
		}
		else
		if(xwd->lockMouseEffect)
		{
			xwd->lockMouseEffect--;
			if(!xwd->lockMouseEffect && (xwd->hoverItem == item) && !xwd->balloonText->hiding && (item->text != L" "))
			{
				xwd->balloonText->SetWindowText(item->text.GetBuffer());
				xwd->balloonText->Popup();
			}
		}
	}
}

void CXWD::ODDockletDoAttentionAnimation(HWND hwndDocklet)
{
	TRACE("ODDockletDoAttentionAnimation\n");
	CDockItem *item = (CDockItem*)GetProp(hwndDocklet, L"DockItem");
	if(xwd && item && !(item->animationFlags & animationFlagPoof))
	{
	}
}

void CXWD::ODDockletGetRelativeFolder(HWND hwndDocklet, char *szFolder)
{
	TRACE("ODDockletGetRelativeFolder\n");
	CDockItem *item = (CDockItem*)GetProp(hwndDocklet, L"DockItem");
	if(xwd && item && !(item->animationFlags & animationFlagPoof))
	{
		CStringA path = item->docklet->path.Mid(xwd->appPath.GetLength());
		path = path.Mid(0, path.ReverseFind('\\') + 1);
		strcpy_s(szFolder, MAX_PATH, path.GetBuffer());
	}
}

void CXWD::ODDockletGetRootFolder(HWND hwndDocklet, char *szFolder)
{
	TRACE("ODDockletGetRootFolder\n");
	CDockItem *item = (CDockItem*)GetProp(hwndDocklet, L"DockItem");
	if(xwd && item && !(item->animationFlags & animationFlagPoof))
	{
		strcpy_s(szFolder, MAX_PATH, CStringA(xwd->appPath).GetBuffer());
	}
}

void CXWD::ODDockletDefaultConfigDialog(HWND hwndDocklet)
{
	TRACE("ODDockletDefaultConfigDialog\n");
	CDockItem *item = (CDockItem*)GetProp(hwndDocklet, L"DockItem");
	if(xwd && item && !(item->animationFlags & animationFlagPoof))
	{
	}
}

int CXWD::ODDockletQueryDockEdge(HWND hwndDocklet)
{
	TRACE("ODDockletQueryDockEdge\n");
	CDockItem *item = (CDockItem*)GetProp(hwndDocklet, L"DockItem");
	if(xwd && item && !(item->animationFlags & animationFlagPoof))
	{
		switch(xwd->dockPosition)
		{
		case DockPositionLeft:
			{
				return 2;
			}
			break;
			
		case DockPositionTop:
			{
				return 1;
			}
			break;

		case DockPositionRight:
			{
				return 3;
			}
			break;

		default:
		//case DockPositionBottom:
			{
				return 0;
			}
			break;
		}
	}
	return 0;
}

int CXWD::ODDockletSetDockEdge(HWND hwndDocklet, int)
{
	TRACE("ODDockletSetDockEdge\n");
	return ODDockletQueryDockEdge(hwndDocklet);
}

int CXWD::ODDockletQueryDockAlign(HWND)
{
	TRACE("ODDockletQueryDockAlign\n");
	return 1; // middle
}

int CXWD::ODDockletSetDockAlign(HWND, int)
{
	TRACE("ODDockletSetDockAlign\n");
	return 1; // middle
}

bool CXWD::IsFirstTimeStart()
{
	DWORD mode = SetErrorMode(SEM_FAILCRITICALERRORS);
	DWORD attr = GetFileAttributes(CString(dataPath + L"Settings.ini"));
	SetErrorMode(mode);
	if(attr == INVALID_FILE_ATTRIBUTES)
	{
		DWORD error = GetLastError();
		if((error == ERROR_FILE_NOT_FOUND) || (error == ERROR_PATH_NOT_FOUND))
		{
			return true;
		}
	}
	return false;
}

int CXWD::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CFrameWnd::OnCreate(lpCreateStruct);
	SetClassLong(CFrameWnd::m_hWnd, GCL_HICON, (LONG)LoadIcon(AfxGetInstanceHandle(), IDI_APPLICATION));

	//SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

	userDropTargetHelper = SUCCEEDED(CoCreateInstance(CLSID_DragDropHelper, NULL, 
		CLSCTX_INPROC_SERVER, IID_IDropTargetHelper, (void**)&dropTargetHelper));

	Register(this);

	dockBlurRegion = NULL;
	hoverItem = NULL;
	dropItem = NULL;
	dropItemOver = NULL;
	dragItem = NULL;
	draging = false;
	draged = false;
	exposed = false;
	dropRoot = NULL;
	preferences = NULL;
	about = NULL;
	editIcon = NULL;
	nowIsFullScreen = false;
	showAllRunningAppsInDock = false;

	lockMouseEffect = 0;
	animationPopup = 0;
	animationFlags = 0;

	plugins = new CPlugins();
	plugins->ScanDirectory(appPath + L"Plugins\\");

	odDocklets = new CODDocklets();
	odDocklets->ScanDirectory(appPath + L"Docklets\\");

	menu = new CLMenu();
	menu->Create(this);

	CLMenuItem *menuItem;

	menu->Add(LANGUAGE_MENU_DOCK_ITEMS_LOCK, idMenuDockLock, true);
	menu->Add(LANGUAGE_MENU_ITEM_OPEN, idMenuItemOpen);
	menu->Add(LANGUAGE_MENU_ITEM_OPENAS, idMenuItemOpenAs);

	menuItem = menu->Add(LANGUAGE_MENU_DOCK_ADD, idMenuDockAdd);
	menuItem->menu->Add(LANGUAGE_MENU_DOCK_ADD_MYCOMPUTER, idMenuDockAddMyComputer);
	menuItem->menu->Add(LANGUAGE_MENU_DOCK_ADD_RECYCLEBIN, idMenuDockAddRecycleBin);
	menuItem->menu->Add(LANGUAGE_MENU_DOCK_ADD_SEPARATOR, idMenuDockAddSeparator);
	menuItem->menu->Add(LANGUAGE_MENU_DOCK_ADD_PLUGINS, idMenuDockAddPlugins);
	//menuItem->menu->Add(LANGUAGE_MENU_DOCK_ADD_FILES, idMenuDockAddFiles);
	//menuItem->menu->VisibleItem(idMenuDockAddFiles, false);

	menuItem = menu->Add(LANGUAGE_MENU_DOCK_SWITCHMODE, idMenuDockSwitchMode);
	menuItem->menu->Add(LANGUAGE_MENU_DOCK_MODE_2D, idMenuDockMode2D, true);
	menuItem->menu->Add(LANGUAGE_MENU_DOCK_MODE_3D, idMenuDockMode3D, true);

	menuItem = menu->Add(LANGUAGE_MENU_DOCK_POSITIONONSCREEN, idMenuDockPositionOnScreen);
	menuItem->menu->Add(LANGUAGE_MENU_DOCK_POSITION_LEFT, idMenuDockPositionLeft, true);
	menuItem->menu->Add(LANGUAGE_MENU_DOCK_POSITION_RIGHT, idMenuDockPositionRight, true);
	menuItem->menu->Add(LANGUAGE_MENU_DOCK_POSITION_TOP, idMenuDockPositionTop, true);
	menuItem->menu->Add(LANGUAGE_MENU_DOCK_POSITION_BOTTOM, idMenuDockPositionBottom, true);

	menuItem = menu->Add(LANGUAGE_MENU_ITEM_OPTIONS, idMenuItemOptions);
	menuItem->menu->Add(LANGUAGE_MENU_ITEM_OPTIONS_KEEPINDOCK, idMenuItemOptionsKeepInDock, true);
	menuItem->menu->Add(LANGUAGE_MENU_ITEM_OPTIONS_RUNWITHWINDOWS, idMenuItemOptionsRunWithWindows, true);
	menuItem->menu->Add(LANGUAGE_MENU_ITEM_OPTIONS_REMOVEICON, idMenuItemOptionsRemoveIcon);

	menu->Add(LANGUAGE_MENU_ITEM_PROPERTIES, idMenuItemProperties);
	menu->Add(LANGUAGE_MENU_ITEM_SHOWINEXPLORER, idMenuItemShowInExplorer);

	menu->Add(LANGUAGE_DROPMENU_MOVETO, idMenuItemDropMoveToRecycleBin);
	menu->Add(LANGUAGE_DROPMENU_RUNIN, idMenuItemDropRunIn);
	menu->Add(LANGUAGE_DROPMENU_COPYTO, idMenuItemDropCopyTo);
	menu->Add(LANGUAGE_DROPMENU_MOVETO, idMenuItemDropMoveTo);
	menu->Add(LANGUAGE_DROPMENU_SETUPICON, idMenuItemDropSetupIcon);

	menu->Add(LANGUAGE_MENU_ITEM_RECYCLEBINRESTORE, idMenuItemRestoreRecycleBin);

	menu->Add(LANGUAGE_MENU_DOCK_PLUGINMANAGER, idMenuDockPluginManager);
	menu->Add(LANGUAGE_MENU_DOCK_PREFERENCES, idMenuDockPreferences);
	menu->Add(LANGUAGE_MENU_DOCK_ABOUT, idMenuDockAbout);
	//menu->Add(LANGUAGE_MENU_DOCK_CHECKUPDATES, idMenuDockCheckUpdates);

	menu->AddLine();

	menu->Add(LANGUAGE_MENU_ITEM_QUITFROMAPPLICATION, idMenuItemQuitFromApplication);
	menu->Add(LANGUAGE_MENU_ITEM_RECYCLEBINEMPTY, idMenuItemEmptyRecycleBin);
	menu->Add(LANGUAGE_MENU_DOCK_CLOSE, idMenuDockClose);
	menu->Add(LANGUAGE_DROPMENU_CANCEL, idMenuItemDropCancel);

	CIniReader ini(dataPath + L"settings.ini");

	expose = new CExpose();
	monitors = new CMonitors();
	minimizer = new CMinimizer();

	CString s = ini.getKeyValue(L"monitor", L"Dock");
	POSITION p = NULL;
	if(!s.IsEmpty())
	{
		p = monitors->items.FindIndex(_wtoi(s.GetBuffer()));
	}
	if(p)
	{
		monitor = monitors->items.GetAt(p);
	}
	else
	{
		monitor = monitors->Primary();
	}

	/*CString sb = L"Monitors information:\n";
	p = monitors->items.GetHeadPosition();
	int ni = 0;
	while(p)
	{
		CMonitor *m = monitors->items.GetAt(p);
		sb.AppendFormat(L"#%d [%s]     %dx%d     x=%d,y=%d,r=%d,b=%d		 xr=%d,yr=%d,rr=%d,br=%d\n", 
			++ni, m == monitor ? L"USE" : L"SECONDARY",
			m->rect.Width(), m->rect.Height(), m->rect.left, m->rect.top, m->rect.right, m->rect.bottom,
			m->workRect.left, m->workRect.top, m->workRect.right, m->workRect.bottom);
		monitors->items.GetNext(p);
	}
	AfxMessageBox(sb.GetBuffer());*/

	balloonText = new CBalloonText();
	SetWindowLong(balloonText->m_hWnd, GWL_HWNDPARENT, (LONG)CFrameWnd::m_hWnd); // create as a child
	balloonText->SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

	dib = new CDIB();
	bckg = new CDIB();
	poof = new CDIB();
	reflection = new CDIB();

	poof->Load(appPath + L"Images\\animation-poof.png");

	skinLoader = new CSkinLoader();
	skinLoader->Load(appPath + L"Skins\\");

	skin = NULL;
	if(!skinLoader->items.IsEmpty())
	{
		skin = skinLoader->Find(ini.getKeyValue(L"name", L"Skin"));
		if(!skin)
		{
			skin = skinLoader->items.GetHead();
		}
		skin->Load();
	}

	dockPosition = (DockPosition)_wtoi(ini.getKeyValue(L"position", L"Dock").GetBuffer());
	if(dockPosition == DockPositionNone)
	{
		dockPosition = DockPositionBottom;
	}

	s = ini.getKeyValue(L"mode", L"Dock");
	if(s.IsEmpty())
	{
		dockMode = DockMode3D;
	}
	else
	{
		dockMode = (DockMode)_wtoi(s.GetBuffer());
	}
	if(skin)
	{
		if((dockMode == DockMode2D) && !(skin->mode & DockMode2D))
		{
			dockMode = DockModeNone;
		}
		if((dockMode == DockMode3D) && ((dockPosition != DockPositionBottom) || !(skin->mode & DockMode3D)))
		{
			dockMode = DockModeNone;
		}
		if((dockMode == DockModeNone) && (skin->mode & DockMode2D))
		{
			dockMode = DockMode2D;
		}
		if((dockMode == DockModeNone) && (skin->mode & DockMode3D))
		{
			dockMode = DockMode3D;
			dockPosition = DockPositionBottom;
		}
	}

	s = ini.getKeyValue(L"iconSize", L"Dock");
	if(s.IsEmpty())
	{
		iconSize = 54;
	}
	else
	{
		iconSize = _wtoi(s.GetBuffer());
		if(iconSize < 20)
		{
			iconSize = 20;
		}
		if(iconSize > 256)
		{
			iconSize = 256;
		}
	}

	s = ini.getKeyValue(L"lock", L"Dock");
	itemsLock = (s.IsEmpty() ? true : (_wtoi(s.GetBuffer()) != 0));

	s = ini.getKeyValue(L"iconShadowEnabled", L"Dock");
	iconShadowEnabled = (s.IsEmpty() ? false : (_wtoi(s.GetBuffer()) != 0));

	s = ini.getKeyValue(L"windowsReflectionEnabled", L"Dock");
	windowsReflectionEnabled = (s.IsEmpty() ? true : (_wtoi(s.GetBuffer()) != 0));

	s = ini.getKeyValue(L"topMost", L"Dock");
	dockIsTopMost = (s.IsEmpty() ? false : (_wtoi(s.GetBuffer()) != 0));
	dockIsTopMostNow = dockIsTopMost;

	s = ini.getKeyValue(L"showAllRunningAppsInDock", L"Dock");
	showAllRunningAppsInDock = (s.IsEmpty() ? false : (_wtoi(s.GetBuffer()) != 0));

	s = ini.getKeyValue(L"checkForUpdates", L"Dock");
	if(s.IsEmpty())
	{
		checkForUpdates = true;
	}
	else
	{
		checkForUpdates = (_wtoi(s.GetBuffer()) != 0);
	}

	s = ini.getKeyValue(L"reserveScreen", L"Dock");
	if(s.IsEmpty())
	{
		reserveScreen = false;
	}
	else
	{
		reserveScreen = (_wtoi(s.GetBuffer()) != 0);
	}
	reserveScreenOk = false;

	balloonTextSize = _wtoi(ini.getKeyValue(L"size", L"BalloonText").GetBuffer());
	if(balloonTextSize < 6)
	{
		balloonTextSize = (int)balloonText->fontSize;
	}
	balloonText->fontSize = (float)balloonTextSize;

	// let's check recycler now
	recycleBinFull = false;
	SHQUERYRBINFO info;
	info.cbSize = sizeof(SHQUERYRBINFO);
	if(SUCCEEDED(SHQueryRecycleBin(NULL, &info)))
	{
		recycleBinFull = (info.i64NumItems > 0);
	}

	// load and draw items
	items = new CList<CDockItem*>;
	ItemsLoad();

	DockAdjustSize();

	CString pluginsPath = dataPath + L"Plugins\\";
	CreateDirectory(pluginsPath.GetBuffer(), NULL);
	CString dockletsPath = dataPath + L"Docklets\\";
	CreateDirectory(dockletsPath.GetBuffer(), NULL);

	// starting
	p = items->GetHeadPosition();
	while(p)
	{
		CDockItem *item = items->GetAt(p);
		if((item->type == DockItemTypeSeparator) && skin)
		{
			item->image->image->Assign(dockMode == DockMode2D ? skin->separator2d : skin->separator3d);
		}
		if(skin)
		{
			item->indicator->image->Assign((dockMode == DockMode2D) ? skin->indicator2d : skin->indicator3d);
		}
		item->Rect(ItemRect(item, dockPosition));
		switch(item->type)
		{
		case DockItemTypeODDocklet:
			{
				CStringA ini;
				ini.Format("%s%d.ini", CStringA(dockletsPath).GetBuffer(), item->odDockletId);

				DWORD mode = SetErrorMode(SEM_FAILCRITICALERRORS);
				DWORD attr = GetFileAttributesA(ini.GetBuffer());
				SetErrorMode(mode);

				if((attr == INVALID_FILE_ATTRIBUTES) && ((GetLastError() == ERROR_FILE_NOT_FOUND) ||
					(GetLastError() == ERROR_PATH_NOT_FOUND)))
				{
					item->docklet->OnCreate(item->image->m_hWnd, item->docklet->hModule, NULL, NULL);
				}
				else
				{
					item->docklet->OnCreate(item->image->m_hWnd, item->docklet->hModule, ini.GetBuffer(), "Settings");
				}
			}
			break;

		case DockItemTypeDocklet:
			{
				item->pluginData = item->plugin->PluginInitialize((XWDId)item);
				item->plugin->PluginEvent((XWDId)item, item->pluginData, XWDEventCreate);
			}
			break;
		}
		item->TopMost(dockIsTopMost);
		items->GetNext(p);
	}

	if(dockIsTopMost)
	{
		SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	}

	Core::MouseAddNotifer(this);
	Core::Run();

	// check dock's reflection
	switch(dockMode)
	{
	case DockMode3D:
		{
			if(windowsReflectionEnabled && skin && skin->reflectionEnable3d && monitor)
			{
				CRect rect = monitor->WorkRect();
				rect.top = rect.bottom - dockHeight + (int)dockHeightOffset3d + (int)dockHeightOffset3d % 2;
				rect.bottom -= (skin->bckgEdge3d->Height() - (int)dockHeightOffset3d % 2);
				rect.OffsetRect(0, -rect.Height());
				SceenCompare::Run(rect, reflection, this);
			}
		}
		break;
	}

	if(reserveScreen)
	{
		barData.cbSize = sizeof(APPBARDATA);
		barData.hWnd = CFrameWnd::m_hWnd;
		barData.uCallbackMessage = 0;
		CRect r = DockRect(dockPosition);
		switch(dockPosition)
		{
		case DockPositionLeft:
			{
				r.OffsetRect(r.Width(), 0);
				barData.uEdge = ABE_LEFT;
			}
			break;

		case DockPositionRight:
			{
				r.OffsetRect(-r.Width(), 0);
				barData.uEdge = ABE_RIGHT;
			}
			break;

		case DockPositionTop:
			{
				r.OffsetRect(0, r.Height());
				barData.uEdge = ABE_TOP;
			}
			break;

		case DockPositionBottom:
			{
				r.OffsetRect(0, -r.Height());
				barData.uEdge = ABE_BOTTOM;
				if(dockMode == DockMode3D)
				{
					r.top += 10;
				}
			}
			break;
		}
		barData.rc = r;
		barData.lParam = 0;
		reserveScreenOk = (SHAppBarMessage(ABM_NEW, &barData) == TRUE);
		if(reserveScreenOk)
		{
			SHAppBarMessage(ABM_SETPOS, &barData);
		}
	}

	MakeBlurBehind();

	// Check and remember update's date
	if(checkForUpdates)
	{
		XWDCheckNewVersion(this);
		SetTimer(timerDockCheckUpdate, 24 * 60 * 60 * 1000, NULL);
	}

	// popup
	animationPopupStartAt = GetTickCount();
	animationFlags |= animationFlagDockPopup;
	SetTimer(timerDockPopup, 10, NULL);
	SetTimer(timerCheckRecyclerBin, 500, NULL);
	SetTimer(timerUpdateApplicationRuning, 1500, NULL);
	SetTimer(timerDockCheckFullScreen, 1000, NULL);

	return 0;
}

void CXWD::OnDestroy()
{
	if(preferences)
	{
		preferences->DestroyWindow();
		preferences = NULL;
	}
	if(editIcon)
	{
		editIcon->DestroyWindow();
		editIcon = NULL;
	}
	if(about)
	{
		about->DestroyWindow();
		about = NULL;
	}

	SettingsSave();
	ItemsSave();

	Core::Stop();
	SceenCompare::Stop();
	FolderWatcher::CleanUp();

	// shutdown all docklets/plugins
	POSITION p = items->GetHeadPosition();
	while(p)
	{
		CDockItem *item = items->GetAt(p);
		item->DestroyWindow();
		items->GetNext(p);
	}

	XWDPublicAPICleanUp();
	
	minimizer->DestroyWindow();
	expose->DestroyWindow();
	delete items;
	delete poof;
	delete bckg;
	delete dib;
	delete reflection;
	delete skinLoader;
	delete monitors;
	delete plugins;
	delete odDocklets;

	CFrameWnd::OnDestroy();
}

void CXWD::OnClose()
{
	animationPopupStartAt = GetTickCount();
	for(;;)
	{
		animationPopup = 1 - (float)(GetTickCount() - animationPopupStartAt) / 250;
		if(animationPopup <= 0)
		{
			POSITION p = items->GetHeadPosition();
			while(p)
			{
				CDockItem *item = items->GetAt(p);
				if(item->type == DockItemTypeDocklet)
				{
					item->plugin->PluginEvent((XWDId)item, item->pluginData, XWDEventDockDestroy);
				}
				item->Show(false);
				item->IndicatorShow(false);
				items->GetNext(p);
			}

			if(reserveScreenOk)
			{
				SHAppBarMessage(ABM_REMOVE, &barData);
				reserveScreenOk = false;
			}

			DestroyWindow();
			return;
		}
		animationPopup = sin(animationPopup * PI / 2);
		if(animationPopup > 1)
		{
			animationPopup = 1;
		}
		DockAdjustPosition();
		Sleep(10);
	}
}

void CXWD::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CFrameWnd::OnShowWindow(bShow, nStatus);
	POSITION p = items->GetHeadPosition();
	while(p)
	{
		CDockItem *item = items->GetAt(p);
		item->Show(bShow == TRUE);
		item->IndicatorShow(bShow == TRUE);
		items->GetNext(p);
	}
}

BOOL CXWD::OnQueryEndSession()
{
	SettingsSave();
	ItemsSave();
	OnClose();
	return CFrameWnd::OnQueryEndSession();
}

/*
void CXWD::OnLButtonDown(UINT nFlags, CPoint point)
{
	CFrameWnd::OnLButtonDown(nFlags, point);

	CPoint pt;
	GetCursorPos(&pt);

	CDockItem *item = ItemAt(pt);
	if(item)
	{
		item->image->SendMessage(WM_LBUTTONDOWN, (WPARAM)nFlags, MAKELPARAM(pt.x, pt.y));
	}
}*/

void CXWD::OnRButtonDown(UINT nFlags, CPoint point)
{
	CFrameWnd::OnRButtonDown(nFlags, point);

	CPoint pt;
	GetCursorPos(&pt);

	// may we clicked on dock's item ?
	/*CDockItem *item = ItemAt(pt);
	if(item)
	{
		OnItemsRMouseDown(item);
		return;
	}*/

	OnMenuPopup(NULL, true, false);
	menu->Popup(NULL, dockPosition, pt.x, pt.y);
}

void CXWD::OnTimer(UINT_PTR nIDEvent)
{
	CFrameWnd::OnTimer(nIDEvent);

	switch(nIDEvent)
	{
	case timerDockApplicationExpose:
		{
			KillTimer(timerDockApplicationExpose);
			if(dropItemOver)
			{
				DoItemExpose(dropItemOver);
			}
			else
			if(!draging)
			{
				draged = false;
				dragItem->RemoveState(StateFlagPressed);
				DoItemExpose(dragItem);
				dragItem = NULL;
			}
		}
		break;

	case timerDockCheckUpdateIfFail:
	case timerDockCheckUpdate:
		{
			XWDCheckNewVersion(this);
		}
		break;

	case timerDockActivate:
		{
			SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
			SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			BringWindowToTop();
			SetForegroundWindow();

			if(animationFlags & animationFlagDockActivate)
			{
				animationFlags &= ~animationFlagDockActivate;
			}
			KillTimer(nIDEvent);
		}
		break;

	case timerDockCheckFullScreen:
		{
			CheckFullScreen();
		}
		break;

	case timerUpdateApplicationRuning:
		{
			HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
			if(hSnapshot)
			{
				PROCESSENTRY32 processEntry = {0};
				processEntry.dwSize = sizeof(PROCESSENTRY32);
				if(Process32First(hSnapshot, &processEntry))
				do
				{
					CString path = GetPathByPID(processEntry.th32ProcessID);
					if (!path.IsEmpty())
					{
						POSITION p = items->GetHeadPosition();
						while(p)
						{
							CDockItem *item = items->GetAt(p);
							if((item->identificator & identificatorApplication) && !(item->identificator & identificatorIndicator))
							{
								if(item->path.CompareNoCase(path) == 0)
								{
									item->identificator |= identificatorIndicator;
									CRect rect = ItemIndicatorRect(item);
									item->indicator->SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
									item->IndicatorShow(IsWindowVisible() && !(draging && (dragItem == item)));
									/*if(IsWindowVisible() && !(draging && (dragItem == item)) && !(item->animationFlags & animationFlagBounce) && !(animationFlags & animationFlagDockPopup))
									{
										item->Bounce(DockItemBounceNormal);
									}*/
									Core::DetachAddNotifer(item->path);
								}
								if(item->type == DockItemTypeRunningApp)
								{
									RemoveItem(item);
									break;
								}
							}
							items->GetNext(p);
						}
					}
				}
				while(Process32Next(hSnapshot, &processEntry));
				CloseHandle(hSnapshot);
			}
		}
		break;

	case timerCheckRecyclerBin:
		{
			bool full = false;
			SHQUERYRBINFO info;
			info.cbSize = sizeof(SHQUERYRBINFO);
			if(SUCCEEDED(SHQueryRecycleBin(NULL, &info)))
			{
				full = (info.i64NumItems > 0);
			}
			if(full != recycleBinFull)
			{
				recycleBinFull = full;
				POSITION p = items->GetHeadPosition();
				while(p)
				{
					CDockItem *item = items->GetAt(p);
					if(item->identificator & identificatorRecycleBin)
					{
						if(recycleBinFull)
						{
							item->icon = item->additionalIcon;
						}
						else
						{
							item->icon = item->additionalIcon2;
						}
						if(item->LoadImage())
						{
							item->image->LayerDraw();
							if(draging && (dragItem == item))
							{
								item->image->dib->AlphaBlend(item->image->dib->Rect(), 180, DrawFlagsPaint);
							}
							item->image->LayerUpdate();

							if(item->reflection->IsWindowVisible())
							{
								item->reflection->LayerDraw();
								item->reflection->LayerUpdate();
							}
						}
					}
					items->GetNext(p);
				}
			}
		}
		break;

	case timerDockPopup:
		{
			animationPopup = (float)(GetTickCount() - animationPopupStartAt) / 250;
			if(animationPopup >= 1)
			{
				animationPopup = 1;
				animationFlags &= ~animationFlagDockPopup;
				KillTimer(nIDEvent);

				// let's ask to re-register all public plugins
				XWDPAskForPlugins();

				minimizer->Minimize(NULL, NULL);
			}
			animationPopup = sin(animationPopup * PI / 2);
			if(animationPopup > 1)
			{
				animationPopup = 1;
			}
			DockAdjustPosition();
		}
		break;

	case timerDockResize:
		{
			if(draging)
			{
				dragItem->resizeFlag = (float)(GetTickCount() - dragItem->resizeStartAt) / DOCK_ANIMATON_RESIZE_DELAY;
				if(dragItem->resizeFlag >= 1)
				{
					dragItem->resizeFlag = 1;
					animationFlags &= ~animationFlagDockResizeTimer;
					KillTimer(nIDEvent);
					MakeBlurBehind();
				}
				if((animationFlags & animationFlagDockResizeUp) == 0)
				{
					dragItem->resizeFlag = 1 - dragItem->resizeFlag;
				}
			}
			if(dropItem)
			{
				dropItem->resizeFlag = (float)(GetTickCount() - dropItem->resizeStartAt) / DOCK_ANIMATON_RESIZE_DELAY;
				if(dropItem->resizeFlag >= 1)
				{
					dropItem->resizeFlag = 1;
					animationFlags &= ~animationFlagDockResizeTimer;
					KillTimer(nIDEvent);
					MakeBlurBehind();
				}
				if((animationFlags & animationFlagDockResizeUp) == 0)
				{
					dropItem->resizeFlag = 1 - dropItem->resizeFlag;
					if(dropItem->resizeFlag == 0)
					{
						if(exposed)
						{
							CRect r = ItemRect(exposedItem, dockPosition);
							if ((dockMode == DockMode3D) && iconShadowEnabled)
							{
								r.top -= exposedItem->imageShadowSize3d;
							}
							expose->SetIconRect(r, exposedItem->image->dib);
						}
						POSITION p = items->Find(dropItem);
						if(p)
						{
							items->RemoveAt(p);
							dropItem->DestroyWindow();
							dropItem = NULL;
							dropItemOver = NULL;
						}
					}
					if(dropIsPlugin)
					{
						POSITION p = items->GetHeadPosition();
						while(p)
						{
							CDockItem *item = items->GetAt(p);
							if(/*(item->type == DockItemTypeIcon) && */(item->identificator & identificatorIndicator))
							{
								CRect rect = ItemIndicatorRect(item);
								item->indicator->SetWindowPos(&wndTop, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
							}
							items->GetNext(p);
						}
					}
				}
			}
			DockAdjustSize(true);
		}
		break;
	}
}

LRESULT CXWD::OnSettingChange(WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = DefWindowProc(WM_SETTINGCHANGE, wParam, lParam);

	monitor = monitors->Primary();
	//UpdateReserveScreen();
	DockAdjustSize(false, true);

	return ret;
}

LRESULT CXWD::OnDisplayChange(WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = DefWindowProc(WM_DISPLAYCHANGE, wParam, lParam);

	monitor = monitors->Primary();
	//UpdateReserveScreen();
	DockAdjustSize(false, true);

	return ret;
}

LRESULT CXWD::OnXWDUpdateNotify(WPARAM wParam, LPARAM)
{
	KillTimer(timerDockCheckUpdateIfFail);
	if(wParam == 2) // failed on connecting
	{
		SetTimer(timerDockCheckUpdateIfFail, 60 * 60 * 1000, NULL);
		return 0;
	}
	if(wParam == 1) // latest version
	{
		return 0;
	}
	// have a new version
	POSITION pins = NULL;
	POSITION p = items->GetHeadPosition();
	while(p)
	{
		CDockItem *pitem = items->GetAt(p);
		switch(pitem->type)
		{
		case DockItemTypeSeparator:
			{
				if(!pins)
				{
					pins = p;
				}
			}
			break;

		case DockItemTypeUpdate:
			{
				return 0;
			}
			break;
		}
		items->GetNext(p);
	}
	CDockItem *item = new CDockItem();
	if(pins)
	{
		items->InsertAfter(pins, item);
	}
	else
	{
		items->AddTail(item);
	}
	item->SetBottomWindow(this);
	item->type = DockItemTypeUpdate;
	item->poof = poof;
	item->dockMode = dockMode;
	item->dockPosition = dockPosition;
	if(skin && (item->dockMode & DockMode3D))
	{
		item->reflectionOffset = iconSize * skin->iconReflectionOffset3d / 100;
		item->reflectionSkipTop = iconSize * skin->iconReflectionSkipTop3d / 100;
		item->reflectionSkipBottom = iconSize * skin->iconReflectionSkipBottom3d / 100;
		item->reflectionSize = max(0, item->reflectionOffset + iconSize * skin->iconPosition3d / 100);
		item->reflectionOpacity = (unsigned char)(255 * skin->iconReflectionOpacity3d / 100);
		item->reflectionOpacityFactor = (unsigned char)(skin->iconReflectionOpacityFactor3d);
		item->iconShadowEnabled = iconShadowEnabled;
	}
	else
	{
		item->reflectionSize = 0;
	}
	item->path = L"http://xwdock.aqua-soft.org";
	item->text = L"New version available";
	item->image->image->Load(appPath + L"Images\\icon-update.png");

	item->Rect(ItemRect(item, dockPosition));
	item->TopMost(dockIsTopMost);

	CRect rect = ItemIndicatorRect(item);
	item->identificator |= identificatorIndicator;
	item->indicator->image->Assign((dockMode == DockMode2D) ? skin->indicator2d : skin->indicator3d);
	item->indicator->SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
	item->IndicatorShow();

	AddItem(item);

	if(!nowIsFullScreen)
	{
		item->Bounce(DockItemBounceAttention, 1);
	}

	return 0;
}

LRESULT CXWD::OnControlNotify(WPARAM wParam, LPARAM)
{
	switch(LOWORD(wParam))
	{
	case idIconProperties:
		{
			switch(HIWORD(wParam))
			{
			case 2:
				{
					/*editIcon->item->icon.Empty();
					editIcon->item->LoadImage();
					editIcon->item->image->LayerDraw();
					editIcon->item->image->LayerUpdate();
					if(editIcon->item->reflection->IsWindowVisible())
					{
						editIcon->item->reflection->LayerDraw();
						editIcon->item->reflection->LayerUpdate();
					}*/
					editIcon->iconPath.Empty();
					editIcon->icon->Load(appPath + L"Images\\icon-blank.png");
					editIcon->Draw();
					editIcon->RedrawWindow();
				}
				break;

			case 1:
				{
					editIcon->item->icon = editIcon->iconPath;
					if(editIcon->item->identificator & identificatorRecycleBin)
					{
						if(recycleBinFull)
						{
							editIcon->item->additionalIcon = editIcon->item->icon;
						}
						else
						{
							editIcon->item->additionalIcon2 = editIcon->item->icon;
						}
					}
					editIcon->item->iconIndex = 0;
					if(editIcon->item->LoadImage()) // anyway, try to check
					{
						editIcon->item->image->LayerDraw();
						editIcon->item->image->LayerUpdate();
						if(editIcon->item->reflection->IsWindowVisible())
						{
							editIcon->item->reflection->LayerDraw();
							editIcon->item->reflection->LayerUpdate();
						}
					}

					editIcon->editDesc->GetWindowText(editIcon->item->text);
					editIcon->editArguments->GetWindowText(editIcon->item->arguments);
					editIcon->editWorkDir->GetWindowText(editIcon->item->workDirectory);

					editIcon->DestroyWindow();
					editIcon = NULL;
				}
				break;
			}
		}
		break;

	case idAbout:
		{
			about->DestroyWindow();
			about = NULL;
		}
		break;

	case idPreferences:
		{
		#ifndef _DEBUG
			wchar_t buff[MAX_PATH];
			SHGetSpecialFolderPath(NULL, buff, CSIDL_STARTUP, TRUE);
			CString path;
			path.Format(L"%s\\XWindows Dock.lnk", buff);

			if(preferences->runWithWindows->checked)
			{
				GetModuleFileName(AfxGetInstanceHandle(), buff, MAX_PATH);

				int flags = LI_DESCRIPTION | LI_PATH | LI_WORKDIRECTORY;
				LinkInfo info;
				info.description = L"XWindows Dock";
				info.path = buff;
				info.workDirectory = appPath;
				if(portable)
				{
					flags |= LI_ARGUMENTS;
					info.arguments = L"-p";
				}
				CreateLink(path, &info, flags);
			}
			else
			{
				DeleteFile(path.GetBuffer());
			}
		#endif

			itemsLock = preferences->lockItems->checked;
			
			dockIsTopMost = preferences->dockTopMost->checked;
			dockIsTopMostNow = dockIsTopMost;
			SetWindowPos(dockIsTopMost ? &wndTopMost : &wndNoTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
			
			POSITION p = items->GetHeadPosition();
			while(p)
			{
				items->GetAt(p)->TopMost(dockIsTopMost);
				items->GetNext(p);
			}

			iconShadowEnabled = preferences->enableIconsShadow->checked;
			windowsReflectionEnabled = preferences->enableWindowsReflection->checked;
			showAllRunningAppsInDock = preferences->showAllRunningAppsInDock->checked;

			SceenCompare::Pause();
			DockAdjustSize(false, true);
			if((dockMode == DockMode3D) && windowsReflectionEnabled && skin && skin->reflectionEnable3d && monitor)
			{
				CRect rect = monitor->WorkRect();
				rect.top = rect.bottom - dockHeight + (int)dockHeightOffset3d + (int)dockHeightOffset3d % 2;
				rect.bottom -= (skin->bckgEdge3d->Height() - (int)dockHeightOffset3d % 2);
				rect.OffsetRect(0, -rect.Height());
				if(SceenCompare::Runing())
				{
					SceenCompare::Rect(rect);
				}
				else
				{
					SceenCompare::Run(rect, reflection, this);
				}
			}
			else
			{
				SceenCompare::Stop();
			}
			SceenCompare::Pause(false);

			reserveScreen = preferences->reserveScreen->checked;
			UpdateReserveScreen();

			if(checkForUpdates != preferences->checkForUpdates->checked)
			{
				checkForUpdates = preferences->checkForUpdates->checked;
				if(checkForUpdates)
				{
					XWDCheckNewVersion(this);
					SetTimer(timerDockCheckUpdate, 24 * 60 * 60 * 1000, NULL);
				}
				else
				{
					KillTimer(timerDockCheckUpdate);
				}
			}

			SettingsSave();

			preferences->DestroyWindow();
			preferences = NULL;

			if (!showAllRunningAppsInDock)
			{
				p = items->GetHeadPosition();
				while(p)
				{
					CDockItem *item = items->GetAt(p);
					if (item->type == DockItemTypeRunningApp)
					{
						RemoveItem(item);
						p = items->GetHeadPosition();
					}
					else
					{
						items->GetNext(p);
					}
				}
			}
		}
		break;
	}
	return 0;
}

LRESULT CXWD::OnSetupDock(WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam))
	{
	case SetupDockSkin:
		{
			CSkin *tmpSkin = skinLoader->Find((wchar_t*)lParam);
			if(tmpSkin)
			{
				skin->Unload();
				skin = tmpSkin;
				skin->Load();

				if((dockMode == DockMode3D) && !(skin->mode & dockMode))
				{
					DockSwitchMode(DockMode2D);
				}
				if((dockMode == DockMode2D) && !(skin->mode & dockMode))
				{
					DockSwitchMode(DockMode3D);
				}

				if(preferences)
				{
					preferences->mode2d->EnableWindow(skin->mode & DockMode2D);
					preferences->mode3d->EnableWindow(skin->mode & DockMode3D);
					preferences->positionLeft->EnableWindow(skin->mode & DockMode2D);
					preferences->positionRight->EnableWindow(skin->mode & DockMode2D);
					preferences->positionTop->EnableWindow(skin->mode & DockMode2D);
					preferences->positionLeft->Checked(dockPosition == DockPositionLeft);
					preferences->positionRight->Checked(dockPosition == DockPositionRight);
					preferences->positionTop->Checked(dockPosition == DockPositionTop);
					preferences->positionBottom->Checked(dockPosition == DockPositionBottom);
					preferences->mode3d->Checked(dockMode == DockMode3D);
					preferences->mode2d->Checked(dockMode == DockMode2D);
					preferences->skinsList->preferMode = dockMode;
					preferences->skinsList->Draw();
					preferences->skinsList->RedrawWindow();
				}

				SceenCompare::Pause();

				POSITION p = items->GetHeadPosition();
				while(p)
				{
					CDockItem *item = items->GetAt(p);
					if((item->type == DockItemTypeSeparator) && skin)
					{
						item->image->image->Assign(dockMode == DockMode2D ? skin->separator2d : skin->separator3d);
					}
					if(skin)
					{
						item->indicator->image->Assign((dockMode == DockMode2D) ? skin->indicator2d : skin->indicator3d);
					}
					items->GetNext(p);
				}

				DockAdjustSize(false, true);

				p = items->GetHeadPosition();
				while(p)
				{
					CDockItem *item = items->GetAt(p);
					item->image->LayerDraw();
					item->image->LayerUpdate();
					item->reflection->LayerDraw();
					item->reflection->LayerUpdate();
					item->indicator->LayerDraw();
					item->indicator->LayerUpdate();
					items->GetNext(p);
				}

				if((dockMode == DockMode3D) && windowsReflectionEnabled && skin && skin->reflectionEnable3d && monitor)
				{
					CRect rect = monitor->WorkRect();
					rect.top = rect.bottom - dockHeight + (int)dockHeightOffset3d + (int)dockHeightOffset3d % 2;
					rect.bottom -= (skin->bckgEdge3d->Height() - (int)dockHeightOffset3d % 2);
					rect.OffsetRect(0, -rect.Height());
					if(SceenCompare::Runing())
					{
						SceenCompare::Rect(rect);
					}
					else
					{
						SceenCompare::Run(rect, reflection, this);
					}
				}
				else
				{
					SceenCompare::Stop();
				}
				SceenCompare::Pause(false);

				UpdateReserveScreen();
				MakeBlurBehind();
			}
		}
		break;

	case SetupDockMonitor:
		{
			POSITION p = monitors->items.FindIndex(HIWORD(wParam));
			if(p)
			{
				monitor = monitors->items.GetAt(p);
	
				SceenCompare::Pause();
				DockAdjustSize();

				switch(dockMode)
				{
				case DockMode3D:
					{
						if(windowsReflectionEnabled && skin && skin->reflectionEnable3d && monitor)
						{
							CRect rect = monitor->WorkRect();
							rect.top = rect.bottom - dockHeight + (int)dockHeightOffset3d + (int)dockHeightOffset3d % 2;
							rect.bottom -= (skin->bckgEdge3d->Height() - (int)dockHeightOffset3d % 2);
							rect.OffsetRect(0, -rect.Height());
							SceenCompare::Rect(rect);
						}
					}
					break;
				}
				SceenCompare::Pause(false);

				UpdateReserveScreen();
				MakeBlurBehind();
			}
		}
		break;

	case SetupDockMode:
		{
			DockMode mode = (DockMode)HIWORD(wParam);
			if(((mode == DockMode2D) || (mode == DockMode3D)) && DockSwitchMode(mode))
			{
				return 1;
			}
		}
		break;

	case SetupDockPosition:
		{
			DockPosition position = (DockPosition)HIWORD(wParam);
			if(((position == DockPositionLeft) || (position == DockPositionRight) ||
				(position == DockPositionTop) || (position == DockPositionBottom)) && DockSwitchPosition(position))
			{
				return 1;
			}
		}
		break;

	case SetupDockIconSize:
		{
			int isize = HIWORD(wParam);
			if(isize < 20)
			{
				isize = 20;
			}
			if(isize > 256)
			{
				isize = 256;
			}
			if(dockMode == DockMode2D)
			{
				int misize = 256;
				int dw;
				for(;;)
				{
					dw = skin->skipLeft2d + skin->skipRight2d;
					POSITION p = items->GetHeadPosition();
					while(p)
					{
						CDockItem *item = items->GetAt(p);
						switch(item->type)
						{
						case DockItemTypeRunningApp:
						case DockItemTypeIcon:
						case DockItemTypeDocklet:
						case DockItemTypeODDocklet:
						case DockItemTypeUpdate:
						case DockItemTypePublicPlugin:
							{
								dw += (int)((skin->iconSizeBetween3d + misize) * item->resizeFlag);
							}
							break;
						
						case DockItemTypeSeparator:
							{
								int w = skin->separator2d->Width() * misize / skin->separator2d->Height();
								dw += (int)((skin->iconSizeBetween3d + w) * item->resizeFlag);
							}
							break;
						}
						items->GetNext(p);
					}
					if(((dw <= monitor->WorkRect().Height()) && 
						((dockPosition == DockPositionLeft) || (dockPosition == DockPositionRight))) ||
						((dw <= monitor->WorkRect().Width()) && 
						((dockPosition == DockPositionTop) || (dockPosition == DockPositionBottom))))
					{
						break;
					}
					misize--;
				}
				if(isize > misize)
				{
					isize = misize;
				}
			}
			if(dockMode == DockMode3D)
			{
				int misize = 256;
				int dw;
				for(;;)
				{
					int dh = misize + (int)(2.0f * misize * skin->iconPosition3d / 100);
					int dho = (int)(dh * sin(PI * (30 + 10.0f * skin->iconPosition3d / 100) / 180));
					dh += skin->bckgEdge3d->Height();
				
					dw = (int)((dh - skin->bckgEdge3d->Height()) * sin(PI * skin->bckgEdgeAngle3d / 180)) + skin->bckgEdgeOffset3d * 2;

					POSITION p = items->GetHeadPosition();
					while(p)
					{
						CDockItem *item = items->GetAt(p);
						switch(item->type)
						{
						case DockItemTypeRunningApp:
						case DockItemTypeIcon:
						case DockItemTypeDocklet:
						case DockItemTypeODDocklet:
						case DockItemTypeUpdate:
						case DockItemTypePublicPlugin:
							{
								dw += (int)((skin->iconSizeBetween3d + misize) * item->resizeFlag);
							}
							break;
						
						case DockItemTypeSeparator:
							{
								int w = skin->separator3d->Width() * (dh - (int)dho - 
									(int)dho % 2 - skin->bckgEdge3d->Height()) / skin->separator3d->Height();
									
								dw += (int)((skin->iconSizeBetween3d + w) * item->resizeFlag);
							}
							break;
						}
						items->GetNext(p);
					}
					if(dw <= monitor->WorkRect().Width())
					{
						break;
					}
					misize--;
				}
				if(isize > misize)
				{
					isize = misize;
				}
			}
			if(iconSize == isize)
			{
				break;
			}
			iconSize = isize;

			SceenCompare::Pause();
			DockAdjustSize(false, true);
			switch(dockMode)
			{
			case DockMode3D:
				{
					if(windowsReflectionEnabled && skin && skin->reflectionEnable3d && monitor)
					{
						CRect rect = monitor->WorkRect();
						rect.top = rect.bottom - dockHeight + (int)dockHeightOffset3d + (int)dockHeightOffset3d % 2;
						rect.bottom -= (skin->bckgEdge3d->Height() - (int)dockHeightOffset3d % 2);
						rect.OffsetRect(0, -rect.Height());
						SceenCompare::Rect(rect);
					}
				}
				break;
			}
			SceenCompare::Pause(false);

			UpdateReserveScreen();
			MakeBlurBehind();

			// Update plugins
			POSITION p = items->GetHeadPosition();
			while(p)
			{
				CDockItem *item = items->GetAt(p);
				if(item->type == DockItemTypeDocklet)
				{
					item->plugin->PluginEvent((XWDId)item, item->pluginData, XWDEventAdjustIconSize);
				}
				items->GetNext(p);
			}

			return iconSize;
		}
		break;
	}
	return 0;
}

LRESULT CXWD::OnCoreNotify(WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam))
	{
	case xwdCoreProcessAttach:
		{
			bool found = false;
			CString path = GetPathByPID(lParam);
			if (!path.IsEmpty())
			{
				POSITION p = items->GetHeadPosition();
				while(p)
				{
					CDockItem *item = items->GetAt(p);
					if(path.CompareNoCase(item->path) == 0)
					{
						found = true;
						if(item->type == DockItemTypePublicPlugin)
						{
							Core::DetachAddNotifer(item->path);
						}
						else
						if((item->identificator & identificatorApplication) && !(item->identificator & identificatorIndicator))
						{
							item->identificator |= identificatorIndicator;
							CRect rect = ItemIndicatorRect(item);
							item->indicator->SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
							item->IndicatorShow(IsWindowVisible() && !(draging && (dragItem == item)));
							/*if(IsWindowVisible() && !(draging && (dragItem == item)) && !(item->animationFlags & animationFlagBounce) && !(animationFlags & animationFlagDockPopup))
							{
							item->Bounce(DockItemBounceNormal);
							}*/
							Core::DetachAddNotifer(item->path);
						}
					}
					items->GetNext(p);
				}
				if (!found) // let's add new running app
				{
					Core::WaitForAppWindowNotifer(path);
				}
			}
		}
		break;

	case xwdCoreProcessDetach:
		{
			CString *path = (CString*)lParam;
			POSITION p = items->GetHeadPosition();
			while(p)
			{
				CDockItem *item = (CDockItem*)items->GetAt(p);
				if(item->path.CompareNoCase(*path) == 0)
				{
					if(item->type == DockItemTypePublicPlugin)
					{
						XWDPluginRemove(item->publicPluginInfo);
						break;
					}
					if(item->type == DockItemTypeRunningApp)
					{
						RemoveItem(item);
						break;
					}
					if(item->identificator & identificatorIndicator)
					{
						item->BounceCancelAttention();
						item->identificator &= ~identificatorIndicator;
						item->IndicatorShow(false, true);
					}
				}
				items->GetNext(p);
			}
			delete path;
		}
		break;

	case xwdCoreApplicationHasWindow:
		{
			CString *path = (CString*)lParam;
			if (showAllRunningAppsInDock)
			{
				Core::PushApplication(*path);
			}
			delete path;
		}
		break;

	case xwdCoreWindowGotFocus:
		{
			CString path = GetPathByHWND((HWND)lParam);
			POSITION p = items->GetHeadPosition();
			while(p)
			{
				CDockItem *item = items->GetAt(p);
				if((item->identificator & identificatorApplication) && (path.CompareNoCase(item->path) == 0))
				{
					if(!(item->identificator & identificatorIndicator))
					{
						item->identificator |= identificatorIndicator;
						CRect rect = ItemIndicatorRect(item);
						item->indicator->SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
						item->IndicatorShow(IsWindowVisible() && !(draging && (dragItem == item)));
						Core::DetachAddNotifer(item->path);
					}
					if(item->bounceCancelOnFocus)
					{
						item->BounceCancelAttention();
					}
				}
				items->GetNext(p);
			}
		}
		break;

	case xwdCoreWindowFlash:
		{
			if(::IsWindowVisible((HWND)lParam) || ::IsIconic((HWND)lParam))
			{
				WORD Flags = HIWORD(wParam);
				CString path = GetPathByHWND((HWND)lParam);
				POSITION p = items->GetHeadPosition();
				while(p)
				{
					CDockItem *item = items->GetAt(p);
					if((item->identificator & identificatorApplication) && (path.CompareNoCase(item->path) == 0))
					{
						if(!(item->identificator & identificatorIndicator))
						{
							item->identificator |= identificatorIndicator;
							CRect rect = ItemIndicatorRect(item);
							item->indicator->SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
							item->IndicatorShow(IsWindowVisible() && !(draging && (dragItem == item)));
							Core::DetachAddNotifer(item->path);
						}
						if(Flags == FLASHW_STOP)
						{
							item->BounceCancelAttention();
						}
						else
						if(!(animationFlags & animationFlagDockResizeTimer) && !nowIsFullScreen)
						{
							item->bounceWindow = (HWND)lParam;
							item->bounceCancelOnFocus = (Flags & FLASHW_TIMERNOFG) == FLASHW_TIMERNOFG;
							item->Bounce(DockItemBounceAttention);
						}
					}
					items->GetNext(p);
				}
			}
		}
		break;

	case xwdCoreWindowChangedSize:
		{
			CheckFullScreen();
		}
		break;
		
	case xwdCoreWindowMinimize:
		{
			minimizer->Minimize(CFrameWnd::m_hWnd, (HWND)lParam);
		}
		break;

	case xwdCoreWindowShow:
	case xwdCoreWindowRestore:
		{
			if (showAllRunningAppsInDock)
			{
				CString path = GetPathByHWND((HWND)lParam);
				if (showAllRunningAppsInDock && !path.IsEmpty() && Core::IsApplicationWindow((HWND)lParam))
				{
					Core::PushApplication(path);
				}
			}
		}
		break;

	case xwdCoreWindowClose:
	case xwdCoreWindowHide:
		{
			CString path = GetPathByHWND((HWND)lParam);
			if (!path.IsEmpty())
			{
				Core::PopApplication(path);
			}
		}
		break;

	case xwdCoreApplicationStack:
		{
			CString *path = (CString*)lParam;
			switch (HIWORD(wParam))
			{
			case ApplicationStackAdded:
				{
					if (showAllRunningAppsInDock)
					{
						AddRunningItem(*path);
					}
				}
				break;

			case ApplicationStackRemoved:
				{
					if (GetApplicationVisibleWindows(*path) == 0)
					{
						RemoveRunningItem(*path);
					}
					else
					if (showAllRunningAppsInDock)
					{
						AddRunningItem(*path);
					}
				}
				break;
			}
			delete path;
		}
		break;
	}
	return 0;
}

BOOL CXWD::FullScreenEnumWindows(HWND hWnd, LPARAM lParam)
{
	if(!::IsWindowVisible(hWnd))
	{
		return TRUE;
	}

	DWORD pid = NULL;
	GetWindowThreadProcessId(hWnd, &pid);
	if(pid == GetCurrentProcessId())
	{
		return TRUE;
	}

	wchar_t buff[256];
	GetClassName(hWnd, buff, 256);
	if((wcscmp(buff, L"Progman") == 0) || (wcscmp(buff, L"WorkerW") == 0))
	{
		return TRUE;
	}
	CRect rect;
	::GetWindowRect(hWnd, &rect);

	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);

	if((rect.left == 0) && (rect.top == 0) && (rect.right == width) && (rect.bottom == height))
	{
		DWORD style = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
		if((style & WS_EX_LAYERED) || (style &  WS_EX_TRANSPARENT))
		{
			return TRUE;
		}
		(*((bool*)lParam)) = true;
		return FALSE;
	}
	return TRUE;
}

void CXWD::CheckFullScreen()
{
	bool ret = false;
	EnumWindows(FullScreenEnumWindows, (LPARAM)&ret);
	/*if(dockIsTopMost && (ret == dockIsTopMostNow))
	{
		dockIsTopMostNow = !ret;
		SetWindowPos(dockIsTopMostNow ? &wndTopMost : &wndBottom, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
		if(dockIsTopMostNow)
		{
			BringWindowToTop();
			POSITION p = items->GetHeadPosition();
			while(p)
			{
				items->GetAt(p)->reflection->BringWindowToTop();
				items->GetAt(p)->indicator->BringWindowToTop();
				items->GetAt(p)->image->BringWindowToTop();
				items->GetNext(p);
			}
			SetForegroundWindow();
		}
	}*/
	if(ret != nowIsFullScreen)
	{
		nowIsFullScreen = ret;
		animationFlags &= ~animationFlagDockPopup;
		KillTimer(timerDockPopup);
		if(nowIsFullScreen)
		{
			animationPopup = 0;
			DockAdjustSize();
		}
		else
		{
			animationPopupStartAt = GetTickCount();
			animationFlags |= animationFlagDockPopup;
			SetTimer(timerDockPopup, 10, NULL);
		}
	}
}

LRESULT CXWD::OnExposeNotify(WPARAM, LPARAM)
{
	if(exposed)
	{
		exposedItem = NULL;
		exposed = false;
		lockMouseEffect--;
	}
	return 0;
}

LRESULT CXWD::OnFileChangesNotify(WPARAM, LPARAM lParam)
{
	CDockItem *item = (CDockItem*)lParam;
	if (items->Find(item))
	{
		item->Bounce(DockItemBounceAttention, 1);
	}
	return 0;
}

LRESULT CXWD::OnMenuNotify(WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam))
	{
	case MenuSelect:
		{
			OnMenuSelect(lParam);
		}
		break;

	case MenuHide:
		{
			OnMenuHide(HIWORD(wParam) == TRUE);
		}
		break;
	}
	return 0;
}

LRESULT CXWD::OnMouseNotify(WPARAM, LPARAM)
{
	if(!IsWindowVisible() || !IsWindowEnabled() || lockMouseEffect)
	{
		return 0;
	}

	CPoint pt;
	GetCursorPos(&pt);

	/*unsigned int message = (unsigned int)GetProp(CFrameWnd::m_hWnd, L"MouseEvent");

	switch(message)
	{
	case WM_LBUTTONDOWN:
		{
			CDockItem *item = ItemAt(pt);
			if(item)
			{
				OnItemsLMouseDown(item);
			}
		}
		break;

	case WM_LBUTTONUP:
		{
			CDockItem *item = ItemAt(pt);
			if(dragItem)
			{
				bool click = (dragItem == item);
				OnItemsLMouseUp(dragItem);
				if(click)
				{
					OnItemsLClick(item);
				}
			}
		}
		break;

	case WM_RBUTTONDOWN:
		{
			CDockItem *item = ItemAt(pt);
			if(item)
			{
				OnItemsRMouseDown(item);
			}
			else
			if(WindowFromPoint(pt) == this)
			{
				OnMenuPopup(NULL, true, false);
				menu->Popup(NULL, dockPosition, pt.x, pt.y);
			}
		}
		break;

	case WM_RBUTTONUP:
		{
		}
		break;

	case WM_MOUSEMOVE:
		{*/
			if(!draging && !itemsLock && dragItem && (dragItem->type != DockItemTypeRunningApp) && (dragItem->type != DockItemTypeSeparator) && ((abs(dragItemPt.x - pt.x) > 8) || (abs(dragItemPt.y - pt.y) > 8)))
			{
				hoverItem = NULL;
				balloonText->Hide();
				dragItemPt = pt;
				dragItem->ReflectionShow(false);
				dragItem->IndicatorShow(false);
				dragItem->image->ScreenToClient(&dragItemPt);
				dragItemPt.y -= ((dragItem->dockMode == DockMode3D) && (dragItem->type != DockItemTypeSeparator) ? dragItem->imageShadowSize3d : 0);
				if(dragItem->type != DockItemTypeSeparator)
				{
					dragItem->image->dib->AlphaBlend(dragItem->image->dib->Rect(), 180, DrawFlagsPaint);
					dragItem->image->LayerUpdate();
				}
				draging = true;
			}
			if(draging)
			{
				dragItem->Move(pt.x - dragItemPt.x, pt.y - dragItemPt.y);

				CRect rect;
				GetWindowRect(&rect);

				if(dockMode == DockMode3D)
				{
					rect.top += ((int)dockHeightOffset3d - iconSize + iconSize * skin->iconPosition3d / 100);
				}

				if(!rect.PtInRect(pt))
				{
					if((dragItem->resizeFlag > 0) && !((animationFlags & animationFlagDockResizeTimer) && ((animationFlags & animationFlagDockResizeUp) == 0)))
					{
						if(animationFlags & animationFlagDockResizeTimer)
						{
							dragItem->resizeStartAt = (DWORD)(GetTickCount() - (1 - dragItem->resizeFlag) * DOCK_ANIMATON_RESIZE_DELAY);
						}
						else
						{
							dragItem->resizeStartAt = GetTickCount();
						}
						StopBouncing();
						animationFlags |= animationFlagDockResizeTimer;
						animationFlags &= ~animationFlagDockResizeUp;
						SetTimer(timerDockResize, 10, NULL);
					}
				}
				else
				{
					if((dragItem->resizeFlag < 1) && !((animationFlags & animationFlagDockResizeTimer) && (animationFlags & animationFlagDockResizeUp)))
					{
						if(animationFlags & animationFlagDockResizeTimer)
						{
							dragItem->resizeStartAt = (DWORD)(GetTickCount() - dragItem->resizeFlag * DOCK_ANIMATON_RESIZE_DELAY);
						}
						else
						{
							dragItem->resizeStartAt = GetTickCount();
						}
						StopBouncing();
						animationFlags |= animationFlagDockResizeTimer;
						animationFlags |= animationFlagDockResizeUp;
						SetTimer(timerDockResize, 10, NULL);
					}
					CDockItem *item = ItemAt(pt, true);
					if(item && (item != dragItem))
					{
						POSITION p1 = items->Find(dragItem);
						POSITION p2 = items->Find(item);

						rect = ItemRect(item, dockPosition);

						items->RemoveAt(p1);

						switch(dockPosition)
						{
						case DockPositionLeft:
						case DockPositionRight:
							{
								if(pt.y >= rect.top + rect.Height() / 2)
								{
									items->InsertAfter(p2, dragItem);
								}
								else
								{
									items->InsertBefore(p2, dragItem);
								}
							}
							break;

						// case DockPositionTop:
						// case DockPositionBottom:
						default:
							{
								if(pt.x >= rect.left + rect.Width() / 2)
								{
									items->InsertAfter(p2, dragItem);
								}
								else
								{
									items->InsertBefore(p2, dragItem);
								}
							}
							break;
						}
						
						POSITION p = items->GetHeadPosition();
						while(p)
						{
							CDockItem *item = items->GetAt(p);
							if(item != dragItem)
							{
								CRect rect = ItemRect(item, dockPosition);
								item->Move(rect.left, rect.top, true);
							}
							items->GetNext(p);
						}
					}
				}
			}
			else
			if(dragItem && (dragItem->type == DockItemTypeSeparator))
			{
				if((GetKeyState(VK_SHIFT) & 0x8000) == 0x8000)
				{
					if(monitor)
					{
						DockPosition position = dockPosition;
						CRect rect = monitor->Rect();
						if(pt.x < rect.left + 100)
						{
							position = DockPositionLeft;
						}
						if(pt.x > rect.right - 100)
						{
							position = DockPositionRight;
						}
						if(pt.y < rect.top + 100)
						{
							position = DockPositionTop;
						}
						if(pt.y > rect.bottom - 100)
						{
							position = DockPositionBottom;
						}
						if((position != dockPosition) && SendMessage(WM_SETUPDOCK, MAKEWPARAM(SetupDockPosition, position)))
						{
							if(preferences)
							{
								preferences->mode2d->Checked(dockMode == DockMode2D);
								preferences->mode3d->Checked(dockMode == DockMode3D);
								preferences->positionLeft->Checked(dockPosition == DockPositionLeft);
								preferences->positionRight->Checked(dockPosition == DockPositionRight);
								preferences->positionTop->Checked(dockPosition == DockPositionTop);
								preferences->positionBottom->Checked(dockPosition == DockPositionBottom);
							}
						}
					}
				}
				else
				{
					int size = iconSize;
					switch(dockPosition)
					{
					case DockPositionBottom:
						{
							size += (dragItemPt.y - pt.y);
						}
						break;

					case DockPositionTop:
						{
							size += (pt.y - dragItemPt.y);
						}
						break;

					case DockPositionLeft:
						{
							size += (pt.x - dragItemPt.x);
						}
						break;

					case DockPositionRight:
						{
							size += (dragItemPt.x - pt.x);
						}
						break;
					}
					switch(dockPosition)
					{
					case DockPositionTop:
					case DockPositionBottom:
						{
							dragItemPt.y = pt.y;
						}
						break;

					case DockPositionLeft:
					case DockPositionRight:
						{
							dragItemPt.x = pt.x;
						}
						break;
					}
					if((iconSize != size) && (size <= 256) && (size >= 20))
					{
						CRect rect;
						GetWindowRect(&rect);
						if(rect.PtInRect(pt) && SendMessage(WM_SETUPDOCK, MAKEWPARAM(SetupDockIconSize, size)))
						{
							if(preferences)
							{
								preferences->iconSize->SetPosition(preferences->iconSize->max * (iconSize - 20) / (256 - 20));
							}
						}
					}
				}
			}
			else
			{
				if(!(dockIsTopMost && dockIsTopMostNow) && monitor && !nowIsFullScreen)
				{
					CRect rect = monitor->Rect();
					switch(dockPosition)
					{
					case DockPositionBottom:
						{
							if((pt.y >= rect.bottom - 1) && (pt.y < rect.bottom) && IsDockCoveredByAnyOfWindows())
							{
								if(!(animationFlags & animationFlagDockActivate))
								{
									animationFlags |= animationFlagDockActivate;
									SetTimer(timerDockActivate, 300, NULL);
								}
							}
							else
							if(animationFlags & animationFlagDockActivate)
							{
								animationFlags &= ~animationFlagDockActivate;
								KillTimer(timerDockActivate);
							}
						}
						break;

					case DockPositionTop:
						{
							if((pt.y >= rect.top) && (pt.y < rect.top + 1) && IsDockCoveredByAnyOfWindows())
							{
								if(!(animationFlags & animationFlagDockActivate))
								{
									animationFlags |= animationFlagDockActivate;
									SetTimer(timerDockActivate, 300, NULL);
								}
							}
							else
							if(animationFlags & animationFlagDockActivate)
							{
								animationFlags &= ~animationFlagDockActivate;
								KillTimer(timerDockActivate);
							}
						}
						break;

					case DockPositionLeft:
						{
							if((pt.x >= rect.left) && (pt.x < rect.left + 1) && IsDockCoveredByAnyOfWindows())
							{
								if(!(animationFlags & animationFlagDockActivate))
								{
									animationFlags |= animationFlagDockActivate;
									SetTimer(timerDockActivate, 300, NULL);
								}
							}
							else
							if(animationFlags & animationFlagDockActivate)
							{
								animationFlags &= ~animationFlagDockActivate;
								KillTimer(timerDockActivate);
							}
						}
						break;

					//case DockPositionRight:
					default:
						{
							if((pt.x >= rect.right - 1) && (pt.x < rect.right) && IsDockCoveredByAnyOfWindows())
							{
								if(!(animationFlags & animationFlagDockActivate))
								{
									animationFlags |= animationFlagDockActivate;
									SetTimer(timerDockActivate, 300, NULL);
								}
							}
							else
							if(animationFlags & animationFlagDockActivate)
							{
								animationFlags &= ~animationFlagDockActivate;
								KillTimer(timerDockActivate);
							}
						}
						break;
					}
				}

				CDockItem *item = ItemAt(pt, true);
				if(item && !(draging && (item == dragItem)) && (item->type == DockItemTypeSeparator) && skin && !menu->IsWindowVisible())
				{
					balloonText->Hide();
					if(item != hoverItem)
					{
						hoverItem = item;
						SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
					}
				}
				else
				if(item && !(draging && (item == dragItem)) && !item->text.IsEmpty() && (item->text != L" ") && skin && !menu->IsWindowVisible())
				{
					if(item != hoverItem)
					{
						hoverItem = item;

						CPoint pt;
						CRect rect = hoverItem->rect;
						switch(dockPosition)
						{
						case DockPositionBottom:
							{
								pt.x = rect.left + rect.Width() / 2;
								pt.y = rect.top - (dockMode == DockMode3D ? 8 : (skin->skipTop2d + 2));
								if((dockMode == DockMode3D) && item->iconShadowEnabled)
								{
									pt.y += item->imageShadowSize3d;
								}
							}
							break;

						case DockPositionTop:
							{
								pt.x = rect.left + rect.Width() / 2;
								pt.y = rect.bottom + (skin->skipTop2d + 2);
							}
							break;

						case DockPositionLeft:
							{
								pt.x = rect.right + (skin->skipTop2d + 2);
								pt.y = rect.top + rect.Height() / 2;
							}
							break;

						//case DockPositionRight:
						default:
							{
								pt.x = rect.left - (skin->skipTop2d + 2);
								pt.y = rect.top + rect.Height() / 2;
							}
							break;
						}

						CString s;
						if(dropItemOver == hoverItem)
						{
							if(dropIsIcon && (hoverItem->type == DockItemTypeIcon))
							{
								if(!s.IsEmpty())
								{
									s += L" / ";
								}
								s += LANGUAGE_DROPMENU_SETUPICON;
							}
							if(hoverItem->identificator & identificatorRecycleBin)
							{
								if(!s.IsEmpty())
								{
									s += L" / ";
								}
								s += LANGUAGE_DROPMENU_MOVETO;
							}
							if(hoverItem->identificator & identificatorApplication)
							{
								if(!s.IsEmpty())
								{
									s += L" / ";
								}
								s += LANGUAGE_DROPMENU_RUNIN;
							}
							if(hoverItem->identificator & identificatorDirectory)
							{
								if(!s.IsEmpty())
								{
									s += L" / ";
								}
								s += LANGUAGE_DROPMENU_COPYTO;
							}
							if(!s.IsEmpty())
							{
								s += L" - ";
							}
						}
						s.AppendFormat(L"%s", hoverItem->text.GetBuffer());
						balloonText->prepearing = true;
						balloonText->SetWindowText(s.GetBuffer());
						balloonText->Prepare(pt, dockPosition);
						balloonText->prepearing = false;
						balloonText->Popup();
					}
				}
				else
				if(hoverItem)
				{
					if(hoverItem->type == DockItemTypeSeparator)
					{
						SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
					}
					hoverItem = NULL;
					balloonText->Hide();
				}
			}
		/*}
		break;
	}*/
	return 0;
}

void CXWD::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ((nChar == 80/* 'p' */) && ((GetKeyState(VK_CONTROL) & 0x8000) == 0x8000) && ((GetKeyState(VK_SHIFT) & 0x8000) == 0x8000))
	{
		MakeDockScreenshot();
	}
	CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CXWD::MakeDockScreenshot()
{
	CFileDialog dlg(FALSE, NULL, NULL, OFN_HIDEREADONLY, L"Image (*.png)|*.png", this);
	if (dlg.DoModal() == IDOK)
	{
		CString fileName = dlg.GetPathName();

		CRect r;
		CDIB tmp, tmp2;
		tmp.Assign(dib);
		tmp.ReflectVertical();
		tmp.DecodeAlpha();

		Graphics g(tmp.bmp);
		g.SetCompositingMode(CompositingModeSourceOver);
		g.SetSmoothingMode(SmoothingModeAntiAlias);

		POSITION p = items->GetHeadPosition();
		while (p)
		{
			CDockItem *item = items->GetAt(p);

			if((dockMode == DockMode3D) && (item->type != DockItemTypeSeparator))
			{
				item->reflection->GetWindowRect(&r);
				ScreenToClient(&r);

				tmp2.Assign(item->reflection->dib);
				tmp2.ReflectVertical();
				tmp2.DecodeAlpha();
				g.DrawImage(tmp2.bmp, r.left, r.top, r.Width(), r.Height());
			}

			if (item->identificator & identificatorIndicator)
			{
				item->indicator->GetWindowRect(&r);
				ScreenToClient(&r);

				tmp2.Assign(item->indicator->dib);
				tmp2.ReflectVertical();
				tmp2.DecodeAlpha();
				g.DrawImage(tmp2.bmp, r.left, r.top, r.Width(), r.Height());
			}

			item->image->GetWindowRect(&r);
			ScreenToClient(&r);

			tmp2.Assign(item->image->dib);
			tmp2.ReflectVertical();
			tmp2.DecodeAlpha();
			g.DrawImage(tmp2.bmp, r.left, r.top, r.Width(), r.Height());

			items->GetNext(p);
		}

		CLSID pngClsid;
		GetEncoderClsid(L"image/png", &pngClsid);
		tmp.bmp->Save(fileName, &pngClsid);
	}
}

int ZOrderIndex(HWND hWnd)
{
	int index = 0;
	HWND h = FindWindow(L"Progman", NULL);
	while (h && (h != hWnd))
	{
		index++;
		h = GetWindow(h, GW_HWNDPREV);
	}
	return index;
}

struct WindowIsCoveredData
{
	RECT rect;
	HWND hWnd;
	int zorder;
	bool intersect;
};
typedef struct WindowIsCoveredData WindowIsCoveredData;

BOOL CALLBACK CXWD::WindowIsCoveredEnumWindows(HWND hWnd, LPARAM lParam)
{
	WindowIsCoveredData *data = (WindowIsCoveredData*)lParam;
	if (::IsWindowVisible(hWnd) && !::IsIconic(hWnd) && (data->hWnd != hWnd) &&
		(GetWindowLongPtr(hWnd, GWL_HWNDPARENT) == NULL) && !(GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST))
	{
		CRect rect;
		::GetWindowRect(hWnd, &rect);
		if (rect.IntersectRect(&data->rect, &rect))
		{
			wchar_t buff[MAX_PATH];
			GetClassName(hWnd, buff, MAX_PATH);
			if((wcscmp(buff, L"Progman") != 0) && (wcscmp(buff, L"WorkerW") != 0) && (wcscmp(buff, L"Shell_TrayWnd") != 0) &&
				(ZOrderIndex(hWnd) > data->zorder))
			{
				data->intersect = true;
				return FALSE;
			}
		}
	}
	return TRUE;
}

bool CXWD::IsDockCoveredByAnyOfWindows()
{
	if (::GetForegroundWindow() == CFrameWnd::m_hWnd)
	{
		return false;
	}

	WindowIsCoveredData data;
	data.rect = DockRect(dockPosition);
	data.hWnd = CFrameWnd::m_hWnd;
	data.zorder = ZOrderIndex(CFrameWnd::m_hWnd);
	data.intersect = false;

	EnumWindows(WindowIsCoveredEnumWindows, (LPARAM)&data);

	return data.intersect;
}

BOOL CXWD::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if(hoverItem && (hoverItem->type == DockItemTypeSeparator))
	{
		switch(dockPosition)
		{
		case DockPositionTop:
		case DockPositionBottom:
			{
				::SetCursor(AfxGetApp()->LoadCursor(MAKEINTRESOURCE(32764)));
			}
			break;

		case DockPositionLeft:
		case DockPositionRight:
			{
				::SetCursor(AfxGetApp()->LoadCursor(MAKEINTRESOURCE(32765)));
			}
			break;
		}
		return TRUE;
	}
	return CFrameWnd::OnSetCursor(pWnd, nHitTest, message);
}

LRESULT CXWD::OnDockItemNotify(WPARAM wParam, LPARAM lParam)
{
	if(!IsWindowEnabled())
	{
		return 0;
	}
	switch(LOWORD(wParam))
	{
	case DockItemLClick:
		{
			OnItemsLClick((CDockItem*)lParam);
		}
		break;

	case DockItemLMouseDown:
		{
			OnItemsLMouseDown((CDockItem*)lParam);
		}
		break;

	case DockItemLMouseUp:
		{
			OnItemsLMouseUp((CDockItem*)lParam);
		}
		break;

	case DockItemRMouseDown:
		{
			OnItemsRMouseDown((CDockItem*)lParam);
		}
		break;

	case DockItemMoving:
		{
			OnItemsItemMoving((CDockItem*)lParam);
		}
		break;
	}
	return 0;
}

BOOL CXWD::OnCopyData(CWnd* pWnd, COPYDATASTRUCT *pCopyDataStruct)
{
	if (pCopyDataStruct->dwData == WM_XWDPUBLICAPICALL)
	{
		XWDPublicAPIDispatchMessage(pWnd, pCopyDataStruct);
		return TRUE;
	}
	return CFrameWnd::OnCopyData(pWnd, pCopyDataStruct);
}

void CXWD::OnItemsItemMoving(CDockItem *item)
{
	if(item->identificator & identificatorIndicator)
	{
		CRect rect = ItemIndicatorRect(item);
		item->indicator->SetWindowPos(&wndTop, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}
	if(menu->IsWindowVisible() && (menuItem == item))
	{
		CPoint pt;
		CRect rect = menuItem->rect;
		switch(dockPosition)
		{
		case DockPositionBottom:
			{
				pt.x = rect.left + rect.Width() / 2;
				pt.y = rect.top - (dockMode == DockMode3D ? 8 : (skin->skipTop2d + 2));
				if((dockMode == DockMode3D) && item->iconShadowEnabled)
				{
					pt.y += item->imageShadowSize3d;
				}
			}
			break;

		case DockPositionTop:
			{
				pt.x = rect.left + rect.Width() / 2;
				pt.y = rect.bottom + (skin->skipTop2d + 2);
			}
			break;

		case DockPositionLeft:
			{
				pt.x = rect.right + (skin->skipTop2d + 2);
				pt.y = rect.top + rect.Height() / 2;
			}
			break;

		//case DockPositionRight:
		default:
			{
				pt.x = rect.left - (skin->skipTop2d + 2);
				pt.y = rect.top + rect.Height() / 2;
			}
			break;
		}
		menu->UpdatePosition(pt);
	}
	// may update balloon text ?
	if(balloonText->IsWindowVisible() && hoverItem && (hoverItem == item)) // to be sure :)
	{
		CPoint pt;
		CRect rect = hoverItem->rect;
		switch(dockPosition)
		{
		case DockPositionBottom:
			{
				pt.x = rect.left + rect.Width() / 2;
				pt.y = rect.top - (dockMode == DockMode3D ? 8 : (skin->skipTop2d + 2));
				if((dockMode == DockMode3D) && item->iconShadowEnabled)
				{
					pt.y += item->imageShadowSize3d;
				}
			}
			break;

		case DockPositionTop:
			{
				pt.x = rect.left + rect.Width() / 2;
				pt.y = rect.bottom + (skin->skipTop2d + 2);
			}
			break;

		case DockPositionLeft:
			{
				pt.x = rect.right + (skin->skipTop2d + 2);
				pt.y = rect.top + rect.Height() / 2;
			}
			break;

		//case DockPositionRight:
		default:
			{
				pt.x = rect.left - (skin->skipTop2d + 2);
				pt.y = rect.top + rect.Height() / 2;
			}
			break;
		}
		balloonText->UpdatePosition(pt, dockPosition);
	}
}

void CXWD::OnItemsLClick(CDockItem *item)
{
	if(draged || exposed)
	{
		return;
	}
	switch(item->type)
	{
	case DockItemTypeDocklet:
		{
			CPoint pt;
			GetCursorPos(&pt);
			item->image->ScreenToClient(&pt);
			if(dockMode == DockMode3D)
			{
				pt.y -= item->imageShadowSize3d;
			}
			XWDPoint point = {(XWDUInt16)pt.x, (XWDUInt16)pt.y};
			item->plugin->PluginEvent((XWDId)item, item->pluginData, XWDEventLButtonClick, point);
		}
		break;

	case DockItemTypeODDocklet:
		{
			lockMouseEffect = 0;
			CPoint pt;
			GetCursorPos(&pt);
			item->image->ScreenToClient(&pt);
			if(dockMode == DockMode3D)
			{
				pt.y -= item->imageShadowSize3d;
			}
			CSize size = ItemSize(item, dockPosition);
			item->docklet->OnLeftButtonClick(item->image->m_hWnd, &pt, &size);
		}
		break;

	case DockItemTypeUpdate:
		{
			item->BounceStop();
			item->Exec(DockItemExecNormal, false, true);
			item->IndicatorShow(false);
			RemoveItem(item);
		}
		break;

	case DockItemTypeRunningApp:
	case DockItemTypeIcon:
	case DockItemTypePublicPlugin:
		{
			bool ctrl = ((GetKeyState(VK_CONTROL) & 0x8000) == 0x8000);
			bool alt = ((GetKeyState(VK_MENU) & 0x8000) == 0x8000);
			if(alt && (item->identificator & identificatorIndicator) && DwmApi::Ready() &&
				!((item->type == DockItemTypePublicPlugin) && !item->publicPluginInfo->exposable))
			{
				DoItemExpose(item);
			}
			else
			if(ctrl)
			{
				item->BounceStop();
				item->Exec(item->exec == DockItemExecNormal ? DockItemExecIfNotRun : DockItemExecNormal, false, 
					!((item->type == DockItemTypePublicPlugin) && !item->publicPluginInfo->bounceable));
			}
			else
			{
				if(item->animationFlags & animationFlagBounceAttention)
				{
					item->BounceStop();
					if(::IsIconic(item->bounceWindow))
					{
						::ShowWindow(item->bounceWindow, SW_RESTORE);
					}
					::BringWindowToTop(item->bounceWindow);
					::SetForegroundWindow(item->bounceWindow);
				}
				else
				{
					DockItemExec exec = item->exec;
					if ((item->type == DockItemTypePublicPlugin) && (!item->publicPluginInfo->activatable))
					{
						exec = DockItemExecNormal;
					}
					item->BounceStop();
					item->Exec(exec, false, !((item->type == DockItemTypePublicPlugin) && !item->publicPluginInfo->bounceable));

					if (item->type == DockItemTypePublicPlugin)
					{
						XWDPPluginEvent(item->publicPluginInfo, XWDPEventLButtonClick);
					}
				}
			}
		}
		break;
	}
}

void CXWD::OnItemsLMouseDown(CDockItem *item)
{
	dragItem = item;
	GetCursorPos(&dragItemPt);
	if(item->type != DockItemTypeSeparator)
	{
		dragItem->SetState(StateFlagPressed);
		
		if(((item->type == DockItemTypeIcon) || (item->type == DockItemTypePublicPlugin) || (item->type == DockItemTypeRunningApp)) && (item->identificator & identificatorIndicator) && DwmApi::Ready())
		{
			SetTimer(timerDockApplicationExpose, DOCK_APPLICATION_EXPOSE_DELAY, NULL);
		}
	}
	if(item->type == DockItemTypeDocklet)
	{
		CPoint pt;
		GetCursorPos(&pt);
		dragItem->image->ScreenToClient(&pt);
		if(dockMode == DockMode3D)
		{
			pt.y -= dragItem->imageShadowSize3d;
		}
		XWDPoint point = {(XWDUInt16)pt.x, (XWDUInt16)pt.y};
		item->plugin->PluginEvent((XWDId)item, item->pluginData, XWDEventLButtonDown, point);
	}
}

void CXWD::OnItemsLMouseUp(CDockItem*)
{
	KillTimer(timerDockApplicationExpose);
	if(!dragItem)
	{
		return;
	}
	if(dragItem->type == DockItemTypeDocklet)
	{
		CPoint pt;
		GetCursorPos(&pt);
		dragItem->image->ScreenToClient(&pt);
		if(dockMode == DockMode3D)
		{
			pt.y -= dragItem->imageShadowSize3d;
		}
		XWDPoint point = {(XWDUInt16)pt.x, (XWDUInt16)pt.y};
		dragItem->plugin->PluginEvent((XWDId)dragItem, dragItem->pluginData, XWDEventLButtonUp, point);
	}
	if(draging)
	{
		if(animationFlags & animationFlagDockResizeTimer)
		{
			animationFlags &= ~animationFlagDockResizeTimer;
			KillTimer(timerDockResize);
		}
		draging = false;
		dragItem->RemoveState(StateFlagPressed);

		if(dragItem->resizeFlag < 1)
		{
			if(dragItem->type == DockItemTypeODDocklet)
			{
				CString s;
				s.Format(L"%sDocklets\\%d.ini", dataPath.GetBuffer(), dragItem->odDockletId);
				DeleteFile(s.GetBuffer());
			}
			if(dragItem->type == DockItemTypeDocklet)
			{
				dragItem->plugin->PluginEvent((XWDId)dragItem, dragItem->pluginData, XWDEventDelete);
				POSITION p = dragItem->notifications.GetHeadPosition();
				while(p)
				{
					CNotification *notification = dragItem->notifications.GetAt(p);
					notification->ShowWindow(SW_HIDE);
					dragItem->notifications.GetNext(p);
				}
			}
			POSITION p = items->Find(dragItem);
			if(p)
			{
				items->RemoveAt(p);
			}
			dragItem->Poof();
		}
		else
		{
			dragItem->ReflectionShow(true, true);
			dragItem->IndicatorShow();
		}
		dragItem = NULL;
		draged = true;
		DockAdjustSize(true);
	}
	else
	{
		draged = false;
		dragItem->RemoveState(StateFlagPressed);
		dragItem = NULL;
	}
}

void CXWD::OnItemsRMouseDown(CDockItem *item)
{
	if(lockMouseEffect)
	{
		return;
	}
	item->BounceStop();
	if(item->type == DockItemTypeODDocklet)
	{
		CPoint pt;
		GetCursorPos(&pt);
		item->image->ScreenToClient(&pt);
		if(dockMode == DockMode3D)
		{
			pt.y -= item->imageShadowSize3d;
		}
		CSize size = ItemSize(item, dockPosition);
		if(item->docklet->OnRightButtonClick(item->image->m_hWnd, &pt, &size))
		{
			return;
		}
	}
	if(!skin)
	{
		return;
	}
	CRect rect = ItemRect(item, dockPosition);
	CPoint pt;
	
	switch(dockPosition)
	{
	case DockPositionBottom:
		{
			pt.x = rect.left + rect.Width() / 2;
			pt.y = rect.top - (dockMode == DockMode3D ? 8 : (skin->skipTop2d + 2));
		}
		break;

	case DockPositionTop:
		{
			pt.x = rect.left + rect.Width() / 2;
			pt.y = rect.bottom + (skin->skipTop2d + 2);
		}
		break;

	case DockPositionLeft:
		{
			pt.x = rect.right + (skin->skipTop2d + 2);
			pt.y = rect.top + rect.Height() / 2;
		}
		break;

	//case DockPositionRight:
	default:
		{
			pt.x = rect.left - (skin->skipTop2d + 2);
			pt.y = rect.top + rect.Height() / 2;
		}
		break;
	}

	OnMenuPopup(item, false, false);
	menu->Popup(NULL, dockPosition, pt.x, pt.y);
}

void CXWD::OnItemsDragEnter(CDockItem *item)
{
	switch(item->type)
	{
	case DockItemTypeODDocklet:
		{
			if(item->docklet->odOnDropFiles)
			{
				item->SetState(StateFlagDropOver);
				CString s = LANGUAGE_DROPMENU_DROPTO;
				s.AppendFormat(L" - %s", item->text.GetBuffer());
				balloonText->SetWindowText(s.GetBuffer());
			}
		}
		break;

	case DockItemTypeDocklet:
		{
			LPITEMIDLIST root = ILClone(dropRoot);
			XWDUInt16 count = (XWDUInt16)dropFiles.GetCount();
			LPITEMIDLIST *items = (LPITEMIDLIST*)malloc(count * sizeof(LPITEMIDLIST));

			LPITEMIDLIST *pId = items;
			POSITION p = dropFiles.GetHeadPosition();
			while(p)
			{
				(*pId) = ILClone(dropFiles.GetAt(p));
				pId++;
				dropFiles.GetNext(p);
			}

			item->pluginCanDrop = (item->plugin->PluginEvent((XWDId)item, item->pluginData, XWDEventDropEnter,
				root, items, count) == XWDTrue);

			if(item->pluginCanDrop)
			{
				item->SetState(StateFlagDropOver);
			}

			ILFree(root);
			pId = items;
			for(int i = 0; i < count; i++, pId++)
			{
				ILFree((*pId));
			}
			free(items);
		}
		break;

	case DockItemTypeRunningApp:
	case DockItemTypeIcon:
	case DockItemTypePublicPlugin:
		{
			if(!dropIsIcon && ((item->identificator & ~identificatorIndicator) == 0) && !IsMyComputer(item->path) && (item->type != DockItemTypePublicPlugin))
			{
				break;
			}
			item->SetState(StateFlagDropOver);
			CString s;
			if(dropIsIcon && (item->type != DockItemTypeRunningApp))
			{
				if(!s.IsEmpty())
				{
					s += L" / ";
				}
				s += LANGUAGE_DROPMENU_SETUPICON;
			}
			if(item->identificator & identificatorRecycleBin)
			{
				if(!s.IsEmpty())
				{
					s += L" / ";
				}
				s += LANGUAGE_DROPMENU_MOVETO;
			}
			if((item->identificator & identificatorApplication) || (item->type == DockItemTypePublicPlugin))
			{
				if(!s.IsEmpty())
				{
					s += L" / ";
				}
				s += LANGUAGE_DROPMENU_RUNIN;
			}
			if(item->identificator & identificatorDirectory)
			{
				if(!s.IsEmpty())
				{
					s += L" / ";
				}
				s.AppendFormat(L"%s / %s", LANGUAGE_DROPMENU_COPYTO, LANGUAGE_DROPMENU_MOVETO);
			}
			if(!s.IsEmpty())
			{
				s += L" - ";
			}
			s.AppendFormat(L"%s", item->text.GetBuffer());
			balloonText->SetWindowText(s.GetBuffer());

			if(item->identificator & identificatorIndicator)
			{
				SetTimer(timerDockApplicationExpose, DOCK_APPLICATION_EXPOSE_DELAY, NULL);
			}
		}
		break;
	}
}

void CXWD::OnItemsDragLeave(CDockItem *item)
{
	KillTimer(timerDockApplicationExpose);
	if(item->type == DockItemTypeDocklet)
	{
		item->plugin->PluginEvent((XWDId)item, item->pluginData, XWDEventDropLeave);
	}
	item->RemoveState(StateFlagDropOver);
	balloonText->SetWindowText(item->text.GetBuffer());
}

void CXWD::OnItemsDragBegin()
{
	dropIsIcon = false;
	if(!dropFiles.IsEmpty())
	{
		CString path = PIDLToString(dropFiles.GetHead(), dropRoot, SHGDN_FORPARSING);
		CString ext = path.Mid(path.ReverseFind(L'.')).MakeLower();

		CDIB icon;
		if((ext == L".ico") || (ext == L".icon"))
		{
			Icons::GetIcon(path, &icon);
		}
		else
		{
			icon.Load(path);
		}
		dropIsIcon = icon.Ready();
	}
}

bool CXWD::OnItemsDrop(CDockItem *item)
{
	KillTimer(timerDockApplicationExpose);
	switch(item->type)
	{
	case DockItemTypeUpdate:
		{
			return true;
		}
		break;
	}
	item->RemoveState(StateFlagDropOver);
	if(!skin)
	{
		return false;
	}
	switch(item->type)
	{
	case DockItemTypeODDocklet:
		{
			if(item->docklet->odOnDropFiles)
			{
				int buffSize = sizeof(DROPFILES) + 2 * sizeof(wchar_t);
				POSITION p = dropFiles.GetHeadPosition();
				while(p)
				{
					CString path = PIDLToString(dropFiles.GetAt(p), dropRoot, SHGDN_FORPARSING);
					buffSize += (path.GetLength() + 1) * sizeof(wchar_t);
					dropFiles.GetNext(p);
				}
				buffSize = (buffSize / 32 + 1) * 32;

				HGLOBAL hMem = GlobalAlloc(GHND | GMEM_DDESHARE, buffSize);
				if(hMem == 0)
				{
					break;
				}

				wchar_t *buff = (wchar_t*)GlobalLock(hMem);
				if(!buff)
				{
					GlobalFree(hMem);
					break;
				}

				DROPFILES *drop = (DROPFILES*)buff;
				drop->pFiles = sizeof(DROPFILES);
				drop->fWide = TRUE;

				wchar_t* ph = (wchar_t*)((int)buff + sizeof(DROPFILES));

				p = dropFiles.GetHeadPosition();
				while(p)
				{
					CString path = PIDLToString(dropFiles.GetAt(p), dropRoot, SHGDN_FORPARSING);
					memcpy(ph, path.GetBuffer(), path.GetLength() * sizeof(wchar_t));
					ph += path.GetLength() + 1;
					dropFiles.GetNext(p);
				}

				GlobalUnlock(hMem);

				item->docklet->OnDropFiles(item->image->m_hWnd, (HDROP)hMem);

				GlobalFree(hMem);
			}
		}
		break;

	case DockItemTypeDocklet:
		{
			if(item->pluginCanDrop)
			{
				LPITEMIDLIST root = ILClone(dropRoot);
				XWDUInt16 count = (XWDUInt16)dropFiles.GetCount();
				LPITEMIDLIST *items = (LPITEMIDLIST*)malloc(count * sizeof(LPITEMIDLIST));

				LPITEMIDLIST *pId = items;
				POSITION p = dropFiles.GetHeadPosition();
				while(p)
				{
					(*pId) = ILClone(dropFiles.GetAt(p));
					pId++;
					dropFiles.GetNext(p);
				}

				item->plugin->PluginEvent((XWDId)item, item->pluginData, XWDEventDrop, root, items, count);
				item->pluginCanDrop = false;

				ILFree(root);
				pId = items;
				for(int i = 0; i < count; i++, pId++)
				{
					ILFree((*pId));
				}
				free(items);
			}
		}
		break;

	case DockItemTypeRunningApp:
	case DockItemTypeIcon:
	case DockItemTypePublicPlugin:
		{
			if(((item->identificator & ~identificatorIndicator) == 0) && !dropIsIcon && (item->type != DockItemTypePublicPlugin))
			{
				break;
			}
			int n = 0;
			if(dropIsIcon) n++;
			if((item->identificator & identificatorApplication) || (item->type == DockItemTypePublicPlugin)) n++;
			if(item->identificator & identificatorDirectory) n += 2;
			if(item->identificator & identificatorRecycleBin) n++;

			if(n == 1)
			{
				if(dropIsIcon)
				{
					DoItemDropSetupIcon(item);
				}
				if((item->identificator & identificatorApplication) || (item->type == DockItemTypePublicPlugin))
				{
					DoItemDropRunIn(item);
				}
				if(item->identificator & identificatorRecycleBin)
				{
					DoItemDropMoveToRecyclerBin();
				}
			}
			else
			{
				item->SetState(StateFlagPressed);

				CRect rect = ItemRect(item, dockPosition);
				CPoint pt;
				
				switch(dockPosition)
				{
				case DockPositionBottom:
					{
						pt.x = rect.left + rect.Width() / 2;
						pt.y = rect.top - (dockMode == DockMode3D ? 8 : (skin->skipTop2d + 2));
					}
					break;

				case DockPositionTop:
					{
						pt.x = rect.left + rect.Width() / 2;
						pt.y = rect.bottom + (skin->skipTop2d + 2);
					}
					break;

				case DockPositionLeft:
					{
						pt.x = rect.right + (skin->skipTop2d + 2);
						pt.y = rect.top + rect.Height() / 2;
					}
					break;

				//case DockPositionRight:
				default:
					{
						pt.x = rect.left - (skin->skipTop2d + 2);
						pt.y = rect.top + rect.Height() / 2;
					}
					break;
				}

				OnMenuPopup(item, false, true);
				menu->Popup(NULL, dockPosition, pt.x, pt.y);
			}
		}
		break;
	}
	return false;
}

void CXWD::OnMenuSelect(int nID)
{
	switch(nID)
	{
	case idMenuDockClose:
		{
			OnClose();
		}
		break;

	case idMenuDockPluginManager:
		{
			XWDPAskForPlugins();

			CString path = appPath + L"PluginManager.exe";
			ShellExecute(NULL, NULL, path.GetBuffer(), NULL, NULL, SW_SHOW);
		}
		break;

	case idMenuDockAddPlugins:
	case idMenuDockPreferences:
		{
			if(!monitor)
			{
				break;
			}
			if(preferences)
			{
				preferences->SetForegroundWindow();
				break;
			}
			preferences = new CPreferences();
			preferences->nID = idPreferences;
			preferences->notifer = this;

			preferences->btnClose->image->Load(appPath + L"Images\\button-window-close.png");
			preferences->btnSkinNext->image->Load(appPath + L"Images\\button-large.png");
			preferences->btnSkinPrev->image->Load(appPath + L"Images\\button-large.png");
			preferences->btnSkinApply->image->Load(appPath + L"Images\\button-large.png");
//			preferences->logo->Load(appPath + L"Images\\logo.png");
//			preferences->logo->AlphaBlend(preferences->logo->Rect(), (unsigned char)(255 * 7 / 100));

			preferences->tabs->FindItem(idTabsGeneral)->icon->Load(appPath + L"Images\\icon-tab-general.png");
			preferences->tabs->FindItem(idTabsSkins)->icon->Load(appPath + L"Images\\icon-tab-skins.png");
			preferences->tabs->FindItem(idTabsPlugins)->icon->Load(appPath + L"Images\\icon-tab-plugins.png");

			preferences->pluginsList->scrollBottom->Load(appPath + L"Images\\scroll-bottom.png");
			preferences->pluginsList->scrollButton->Load(appPath + L"Images\\scroll-button.png");
			preferences->pluginsList->scrollMiddle->Load(appPath + L"Images\\scroll-middle.png");
			preferences->pluginsList->scrollTop->Load(appPath + L"Images\\scroll-top.png");

			preferences->iconSize->bckgTrack->Load(appPath + L"Images\\button-trackbar-bckg.png");
			preferences->iconSize->button->Load(appPath + L"Images\\button-trackbar-button.png");

			preferences->mode2d->Checked(dockMode == DockMode2D);
			preferences->mode3d->Checked(dockMode == DockMode3D);

			preferences->positionLeft->Checked(dockPosition == DockPositionLeft);
			preferences->positionRight->Checked(dockPosition == DockPositionRight);
			preferences->positionTop->Checked(dockPosition == DockPositionTop);
			preferences->positionBottom->Checked(dockPosition == DockPositionBottom);

			preferences->lockItems->Checked(itemsLock);
			preferences->dockTopMost->Checked(dockIsTopMost && dockIsTopMostNow);
			preferences->reserveScreen->Checked(reserveScreen);
			preferences->iconSize->SetPosition(preferences->iconSize->max * (iconSize - 20) / (256 - 20));
			preferences->enableIconsShadow->Checked(iconShadowEnabled);
			preferences->enableWindowsReflection->Checked(windowsReflectionEnabled);
			preferences->showAllRunningAppsInDock->Checked(showAllRunningAppsInDock);
			preferences->checkForUpdates->Checked(checkForUpdates);

			preferences->monitors->RemoveAll();
			int n = 0;
			POSITION pm = monitors->items.GetHeadPosition();
			while(pm)
			{
				CMonitor *item = monitors->items.GetAt(pm);
				CString s;
				s.Format(L"%d: %dx%d", n + 1, item->rect.Width(), item->rect.Height()); 
				CGComboBoxItem *gitem = preferences->monitors->Add(s, n++);
				if(item == monitor)
				{
					preferences->monitors->selected = gitem;
				}
				monitors->items.GetNext(pm);
			}

			preferences->SetWindowPos(&wndTop, monitor->Rect().left + (monitor->Rect().Width() - 650) / 2, 
				monitor->Rect().top + (monitor->Rect().Height() - 450) / 2, 650, 450, SWP_NOZORDER);

			preferences->pluginsList->RemoveAll();

			plugins->ScanDirectory(appPath + L"Plugins\\");
			POSITION p = plugins->items.GetHeadPosition();
			while(p)
			{
				CPlugin *plugin = plugins->items.GetAt(p);
				if(!plugin->count)
				{
					plugin->Load();
				}
				if(plugin->GetPluginType() == XWDPluginDocklet)
				{
					XWDBool useIcon = XWDFalse;
					XWDString icon;
					XWDString name;
					XWDString author;
					XWDString description;
					XWDUInt32 version = 0;

					plugin->GetPluginInformation(name, author, description, &version);
					useIcon = plugin->GetPluginIcon(icon);

					CPluginListItem *item = preferences->pluginsList->Add();
					item->text.Format(L"%s %d.%d.%d.%d", name, (XWDUInt8)(version >> 24), (XWDUInt8)(version >> 16),
						(XWDUInt8)(version >> 8), (XWDUInt8)(version));
					item->description.Format(L"%s", author);
					item->icon->Load(useIcon == XWDTrue ? plugin->path.Mid(0, plugin->path.ReverseFind(L'\\') + 1) + icon 
						: appPath + L"Images\\icon-docklet.png");
					item->data = plugin;
				}
				if(!plugin->count)
				{
					plugin->Unload();
				}
				plugins->items.GetNext(p);
			}

			odDocklets->ScanDirectory(appPath + L"Docklets\\");
			p = odDocklets->items.GetHeadPosition();
			while(p)
			{
				CODDocklet *docklet = odDocklets->items.GetAt(p);

				char name[256];
				char author[256];
				int version = 0;
				char notes[256];

				if(!docklet->count)
				{
					docklet->Load();
				}

				docklet->OnGetInfomation(name, author, &version, notes);

				if(!docklet->count)
				{
					docklet->Unload();
				}

				CPluginListItem *item = preferences->pluginsList->Add();
				item->text.Format(L"%s %d.%d", CString(name).GetBuffer(), HIWORD(version), LOWORD(version));
				item->description.Format(L"%s", CString(author));
				item->icon->Load(appPath + L"Images\\icon-docklet.png");
				item->data = docklet;

				odDocklets->items.GetNext(p);
			}
			preferences->pluginsList->AdjustSize();

			skinLoader->Load(appPath + L"Skins\\");
			preferences->skinsList->loader->Load(appPath + L"Skins\\");
			preferences->skinsList->skin = preferences->skinsList->loader->Find(skin->name);
			preferences->skinsList->preferMode = dockMode;
			preferences->skinsList->Draw();

			if(preferences->tabs->selected)
			{
				preferences->tabs->selected->selected = false;
			}
			if(nID == idMenuDockPreferences)
			{
				preferences->tabs->selected = preferences->tabs->FindItem(idTabsGeneral);
			}
			else
			if(nID == idMenuDockAddPlugins)
			{
				preferences->tabs->selected = preferences->tabs->FindItem(idTabsPlugins);
			}
			preferences->tabs->selected->selected = true;
			preferences->tabs->Draw();

			wchar_t buff[MAX_PATH];
			SHGetSpecialFolderPath(NULL, buff, CSIDL_STARTUP, TRUE);
			CString path;
			path.Format(L"%s\\XWindows Dock.lnk", buff);

			DWORD mode = SetErrorMode(SEM_FAILCRITICALERRORS);
			DWORD attr = GetFileAttributes(path.GetBuffer());
			SetErrorMode(mode);

			preferences->runWithWindows->Checked(!((attr == INVALID_FILE_ATTRIBUTES) && 
				((GetLastError() == ERROR_FILE_NOT_FOUND) || (GetLastError() == ERROR_PATH_NOT_FOUND))));

			preferences->mode2d->EnableWindow(skin->mode & DockMode2D);
			preferences->mode3d->EnableWindow(skin->mode & DockMode3D);
			preferences->positionLeft->EnableWindow(skin->mode & DockMode2D);
			preferences->positionRight->EnableWindow(skin->mode & DockMode2D);
			preferences->positionTop->EnableWindow(skin->mode & DockMode2D);

			preferences->OpenTab(preferences->tabs->selected);

			preferences->ShowWindow(SW_SHOW);
		}
		break;

/*	case idMenuDockCheckUpdates:
		{
			XWDCheckNewVersion(this);
		}
		break;*/

	case idMenuDockAbout:
		{
			if(!monitor)
			{
				break;
			}
			if(about)
			{
				about->SetForegroundWindow();
				break;
			}
			about = new CAbout();
			about->nID = idAbout;
			about->notifer = this;
			about->imageClose->Load(appPath + L"Images\\button-window-close.png");
			about->SetWindowPos(&wndTop, monitor->Rect().left + (monitor->Rect().Width() - 500) / 2, 
				monitor->Rect().top + (monitor->Rect().Height() - 360) / 2, 500, 360, SWP_NOZORDER);
			about->LayerDraw();
			about->LayerUpdate();
			about->ShowWindow(SW_SHOW);
		}
		break;

	case idMenuDockMode2D:
		{
			DockSwitchMode(DockMode2D);
			if(preferences)
			{
				preferences->mode3d->Checked(false);
				preferences->mode2d->Checked();
				preferences->skinsList->preferMode = DockMode2D;
				preferences->skinsList->Draw();
				preferences->skinsList->RedrawWindow();
			}
		}
		break;

	case idMenuDockMode3D:
		{
			DockSwitchMode(DockMode3D);
			if(preferences)
			{
				preferences->mode3d->Checked();
				preferences->mode2d->Checked(false);
				preferences->positionLeft->Checked(false);
				preferences->positionRight->Checked(false);
				preferences->positionTop->Checked(false);
				preferences->positionBottom->Checked();
				preferences->skinsList->preferMode = DockMode3D;
				preferences->skinsList->Draw();
				preferences->skinsList->RedrawWindow();
			}
		}
		break;

	case idMenuDockPositionLeft:
		{
			DockSwitchPosition(DockPositionLeft);
			if(preferences)
			{
				preferences->positionLeft->Checked();
				preferences->positionRight->Checked(false);
				preferences->positionTop->Checked(false);
				preferences->positionBottom->Checked(false);
				preferences->mode3d->Checked(false);
				preferences->mode2d->Checked();
				preferences->skinsList->preferMode = DockMode2D;
				preferences->skinsList->Draw();
				preferences->skinsList->RedrawWindow();
			}
		}
		break;

	case idMenuDockPositionRight:
		{
			DockSwitchPosition(DockPositionRight);
			if(preferences)
			{
				preferences->positionLeft->Checked(false);
				preferences->positionRight->Checked();
				preferences->positionTop->Checked(false);
				preferences->positionBottom->Checked(false);
				preferences->mode3d->Checked(false);
				preferences->mode2d->Checked();
				preferences->skinsList->preferMode = DockMode2D;
				preferences->skinsList->Draw();
				preferences->skinsList->RedrawWindow();
			}
		}
		break;

	case idMenuDockPositionTop:
		{
			DockSwitchPosition(DockPositionTop);
			if(preferences)
			{
				preferences->positionLeft->Checked(false);
				preferences->positionRight->Checked(false);
				preferences->positionTop->Checked();
				preferences->positionBottom->Checked(false);
				preferences->mode3d->Checked(false);
				preferences->mode2d->Checked();
				preferences->skinsList->preferMode = DockMode2D;
				preferences->skinsList->Draw();
				preferences->skinsList->RedrawWindow();
			}
		}
		break;

	case idMenuDockPositionBottom:
		{
			DockSwitchPosition(DockPositionBottom);
			if(preferences)
			{
				preferences->positionLeft->Checked(false);
				preferences->positionRight->Checked(false);
				preferences->positionTop->Checked(false);
				preferences->positionBottom->Checked();
			}
		}
		break;

	case idMenuDockLock:
		{
			itemsLock = !itemsLock;
			if(preferences)
			{
				preferences->lockItems->Checked(itemsLock);
			}
		}
		break;

	case idMenuItemOpen:
		{
			menuItem->Exec(DockItemExecNormal, false, !((menuItem->type == DockItemTypePublicPlugin) && !menuItem->publicPluginInfo->bounceable));
			if(menuItem->type == DockItemTypeUpdate)
			{
				menuItem->IndicatorShow(false);
				RemoveItem(menuItem);
			}
		}
		break;

	case idMenuItemOpenAs:
		{
			menuItem->Exec(DockItemExecNormal, true, true);
		}
		break;

	case idMenuItemProperties:
		{
			switch(menuItem->type)
			{
			case DockItemTypeODDocklet:
				{
					menuItem->docklet->OnConfigure(menuItem->image->m_hWnd);

					CString dockletsPath = dataPath + L"Docklets\\";
					CreateDirectory(dockletsPath.GetBuffer(), NULL);

					CStringA ini;
					ini.Format("%s%d.ini", CStringA(dockletsPath).GetBuffer(), menuItem->odDockletId);
					menuItem->docklet->OnSave(menuItem->image->m_hWnd, ini.GetBuffer(), "Settings", FALSE);
				}
				break;

			case DockItemTypeIcon:
				{
					if(editIcon)
					{
						if(menuItem == editIcon->item)
						{
							editIcon->SetForegroundWindow();
							break;
						}
						else
						{
							editIcon->item->icon = editIcon->iconPath;
							if(editIcon->item->identificator & identificatorRecycleBin)
							{
								if(recycleBinFull)
								{
									editIcon->item->additionalIcon = editIcon->item->icon;
								}
								else
								{
									editIcon->item->additionalIcon2 = editIcon->item->icon;
								}
							}
							editIcon->item->iconIndex = 0;
							if(editIcon->item->LoadImage()) // anyway, try to check
							{
								editIcon->item->image->LayerDraw();
								editIcon->item->image->LayerUpdate();
								if(editIcon->item->reflection->IsWindowVisible())
								{
									editIcon->item->reflection->LayerDraw();
									editIcon->item->reflection->LayerUpdate();
								}
							}

							editIcon->editDesc->GetWindowText(editIcon->item->text);
							editIcon->editArguments->GetWindowText(editIcon->item->arguments);
							editIcon->editWorkDir->GetWindowText(editIcon->item->workDirectory);
						}
					}
					else
					{
						editIcon = new CEditIcon();		
						editIcon->notifer = this;
						editIcon->nID = idIconProperties;
						editIcon->btnClose->image->Load(appPath + L"Images\\button-window-close.png");
//						editIcon->logo->Load(appPath + L"Images\\logo.png");
//						editIcon->logo->AlphaBlend(editIcon->logo->Rect(), (unsigned char)(255 * 7 / 100));

						editIcon->SetWindowPos(&wndTop, monitor->Rect().left + (monitor->Rect().Width() - 600) / 2, 
							monitor->Rect().top + (monitor->Rect().Height() - 340) / 2, 600, 340, SWP_NOZORDER);
					}

					editIcon->item = menuItem;

					editIcon->iconPath = menuItem->icon;

					CString ext = editIcon->iconPath.Mid(editIcon->iconPath.ReverseFind(L'.')).MakeLower();
					if((ext == L".ico") || (ext == L".icon"))
					{
						Icons::GetIcon(editIcon->iconPath, editIcon->icon);
					}
					else
					{
						editIcon->icon->Load(editIcon->iconPath);
					}
					if(!editIcon->icon->Ready())
					{
						editIcon->iconPath.Empty();
						editIcon->icon->Load(appPath + L"Images\\icon-blank.png");
					}

					editIcon->editDesc->SetWindowText(menuItem->text.GetBuffer());
					editIcon->editArguments->SetWindowText(menuItem->arguments.GetBuffer());
					editIcon->editWorkDir->SetWindowText(menuItem->workDirectory.GetBuffer());

					ext = menuItem->path.Mid(menuItem->path.ReverseFind(L'.')).MakeLower();

					editIcon->hideArgWorkDir = (((menuItem->identificator & identificatorApplication) == 0) &&
						(menuItem->path.Find(L"::{") < 0) && (ext != L".bat") && (ext != L".dll") && (ext != L".ocx") &&
						(ext != L".scr") && (ext != L".bin") && (ext != L".cpl"));
					if(editIcon->hideArgWorkDir)
					{
						editIcon->editArguments->ShowWindow(SW_HIDE);
						editIcon->editWorkDir->ShowWindow(SW_HIDE);
					}
					else
					{
						editIcon->editArguments->ShowWindow(SW_SHOWNOACTIVATE);
						editIcon->editWorkDir->ShowWindow(SW_SHOWNOACTIVATE);
					}

					editIcon->Draw();
					
					if(editIcon->IsWindowVisible())
					{
						editIcon->SetForegroundWindow();
						editIcon->RedrawWindow();
					}
					else
					{
						editIcon->ShowWindow(SW_SHOW);
					}
				}
				break;
			}
		}
		break;

	case idMenuItemOptionsRemoveIcon:
		{
			if(menuItem->identificator & identificatorRecycleBin)
			{
				if (recycleBinFull)
				{
					menuItem->additionalIcon.Empty();
				}
				else
				{
					menuItem->additionalIcon2.Empty();
				}
			}
			else
			{
				menuItem->additionalIcon.Empty();
				menuItem->additionalIcon2.Empty();
			}
			menuItem->icon.Empty();
			menuItem->LoadImage();
			menuItem->image->LayerDraw();
			menuItem->image->LayerUpdate();
			if(menuItem->reflection->IsWindowVisible())
			{
				menuItem->reflection->LayerDraw();
				menuItem->reflection->LayerUpdate();
			}
			if(editIcon && (editIcon->item == menuItem))
			{
				editIcon->iconPath.Empty();
				editIcon->icon->Load(appPath + L"Images\\icon-blank.png");
				editIcon->Draw();
				editIcon->RedrawWindow();
			}
		}
		break;

	case idMenuItemShowInExplorer:
		{
			CString path = menuItem->path.Mid(0, menuItem->path.ReverseFind(L'\\') + 1);
			ShellExecute(NULL, NULL, path.GetBuffer(), NULL, NULL, SW_SHOW);
		}
		break;

	case idMenuItemEmptyRecycleBin:
		{
			if(recycleBinFull)
			{
				SHEmptyRecycleBin(NULL, NULL, NULL);
			}
		}
		break;

	case idMenuItemOptionsRunWithWindows:
		{
			wchar_t buff[MAX_PATH];
			SHGetSpecialFolderPath(NULL, buff, CSIDL_STARTUP, TRUE);
			CString path;
			
			path = menuItem->path.Mid(menuItem->path.ReverseFind(L'\\') + 1, menuItem->path.GetLength());
			path = CString(buff) + L"\\" + path.Mid(0, path.ReverseFind(L'.')) + L".lnk";

			if (menu->IsCheckedItem(idMenuItemOptionsRunWithWindows))
			{
				DeleteFile(path);
			}
			else
			{
				int flags = LI_DESCRIPTION | LI_PATH | LI_WORKDIRECTORY | LI_ARGUMENTS;
				LinkInfo info;
				info.description = menuItem->text;
				info.path = menuItem->path;
				info.workDirectory = menuItem->workDirectory;
				info.arguments = menuItem->arguments;
				CreateLink(path, &info, flags);
			}
		}
		break;

	case idMenuItemQuitFromApplication:
		{
			menuItem->Close();
		}
		break;

	case idMenuItemRestoreRecycleBin:
		{
			if(recycleBinFull)
			{
			}
		}
		break;

	case idMenuItemOptionsKeepInDock:
		{
			if (menu->IsCheckedItem(idMenuItemOptionsKeepInDock))
			{
				if(menuItem->type == DockItemTypeODDocklet)
				{
					CString s;
					s.Format(L"%sDocklets\\%d.ini", dataPath.GetBuffer(), menuItem->odDockletId);
					DeleteFile(s.GetBuffer());
				}
				if(menuItem->type == DockItemTypeDocklet)
				{
					menuItem->plugin->PluginEvent((XWDId)menuItem, menuItem->pluginData, XWDEventDelete);
					POSITION p = menuItem->notifications.GetHeadPosition();
					while(p)
					{
						CNotification *notification = menuItem->notifications.GetAt(p);
						notification->ShowWindow(SW_HIDE);
						menuItem->notifications.GetNext(p);
					}
				}
				RemoveItem(menuItem);
			}
			else 
			if (menuItem->type == DockItemTypeRunningApp)
			{
				menuItem->type = DockItemTypeIcon;
			}
		}
		break;

	case idMenuItemDropSetupIcon:
		{
			DoItemDropSetupIcon(menuItem);
		}
		break;

	case idMenuItemDropRunIn:
		{
			DoItemDropRunIn(menuItem);
		}
		break;

	case idMenuItemDropCopyTo:
	case idMenuItemDropMoveTo:
		{
			DoItemDropCopyTo(menuItem, nID == idMenuItemDropMoveTo);
		}
		break;

	case idMenuItemDropMoveToRecycleBin:
		{
			DoItemDropMoveToRecyclerBin();
		}
		break;

/*	case idMenuDockAddFiles:
		{
		}
		break;*/

	case idMenuDockAddSeparator:
		{
			CDockItem *item = new CDockItem();
			POSITION p = items->Find(menuItem);
			if(p)
			{
				items->InsertAfter(p, item);
			}
			else
			{
				items->AddTail(item);
			}

			item->SetBottomWindow(this);
			item->poof = poof;
			item->type = DockItemTypeSeparator;
			item->dockMode = dockMode;
			item->dockPosition = dockPosition;
			if(skin)
			{
				item->image->image->Assign(dockMode == DockMode2D ? skin->separator2d : skin->separator3d);
			}

			item->Rect(ItemRect(item, dockPosition));
			item->TopMost(dockIsTopMost);

			AddItem(item);
		}
		break;

	case idMenuDockAddMyComputer:
		{
			LPITEMIDLIST pidl;
			SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidl);
			if(!pidl)
			{
				break;
			}
			CDockItem *item = new CDockItem();
			POSITION p = items->Find(menuItem);
			if(p)
			{
				items->InsertAfter(p, item);
			}
			else
			{
				items->AddTail(item);
			}

			item->SetBottomWindow(this);
			item->poof = poof;
			item->dockMode = dockMode;
			item->dockPosition = dockPosition;
			if(skin && (item->dockMode & DockMode3D))
			{
				item->reflectionOffset = iconSize * skin->iconReflectionOffset3d / 100;
				item->reflectionSkipTop = iconSize * skin->iconReflectionSkipTop3d / 100;
				item->reflectionSkipBottom = iconSize * skin->iconReflectionSkipBottom3d / 100;
				item->reflectionSize = max(0, item->reflectionOffset + iconSize * skin->iconPosition3d / 100);
				item->reflectionOpacity = (unsigned char)(255 * skin->iconReflectionOpacity3d / 100);
				item->reflectionOpacityFactor = (unsigned char)(skin->iconReflectionOpacityFactor3d);
				item->iconShadowEnabled = iconShadowEnabled;
			}
			else
			{
				item->reflectionSize = 0;
			}
			item->path = GetSpeacialFolderLocation(CSIDL_DRIVES);

			SHFILEINFO sfi;
			SHGetFileInfo((LPCWSTR)pidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_PIDL);
			item->text = sfi.szDisplayName;
			if(item->text.IsEmpty())
			{
				item->text = item->path.Mid(item->path.ReverseFind(L'\\') + 1);
				if(item->text.IsEmpty())
				{
					item->text = item->path;
				}
			}
			ILFree(pidl);

			if(!item->LoadImage())
			{
				item->image->image->Load(appPath + L"Images\\icon-blank.png");
				if(item->type != DockItemTypeSeparator)
				{
					item->reflection->image->Assign(item->image->image);
				}
			}

			item->Rect(ItemRect(item, dockPosition));
			item->TopMost(dockIsTopMost);

			CRect rect = ItemIndicatorRect(item);
			item->identificator |= identificatorIndicator;
			item->indicator->image->Assign((dockMode == DockMode2D) ? skin->indicator2d : skin->indicator3d);
			item->indicator->SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
			item->IndicatorShow();

			AddItem(item);
		}
		break;

	case idMenuDockAddRecycleBin:
		{
			LPITEMIDLIST pidl;
			SHGetSpecialFolderLocation(NULL, CSIDL_BITBUCKET, &pidl);
			if(!pidl)
			{
				break;
			}
			CDockItem *item = new CDockItem();
			POSITION p = items->Find(menuItem);
			if(p)
			{
				items->InsertAfter(p, item);
			}
			else
			{
				items->AddTail(item);
			}

			item->SetBottomWindow(this);
			item->poof = poof;
			item->dockMode = dockMode;
			item->dockPosition = dockPosition;
			if(skin && (item->dockMode & DockMode3D))
			{
				item->reflectionOffset = iconSize * skin->iconReflectionOffset3d / 100;
				item->reflectionSkipTop = iconSize * skin->iconReflectionSkipTop3d / 100;
				item->reflectionSkipBottom = iconSize * skin->iconReflectionSkipBottom3d / 100;
				item->reflectionSize = max(0, item->reflectionOffset + iconSize * skin->iconPosition3d / 100);
				item->reflectionOpacity = (unsigned char)(255 * skin->iconReflectionOpacity3d / 100);
				item->reflectionOpacityFactor = (unsigned char)(skin->iconReflectionOpacityFactor3d);
				item->iconShadowEnabled = iconShadowEnabled;
			}
			else
			{
				item->reflectionSize = 0;
			}
			item->identificator |= identificatorRecycleBin;
			item->path = GetSpeacialFolderLocation(CSIDL_BITBUCKET);

			SHFILEINFO sfi;
			SHGetFileInfo((LPCWSTR)pidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_PIDL);
			item->text = sfi.szDisplayName;
			if(item->text.IsEmpty())
			{
				item->text = item->path.Mid(item->path.ReverseFind(L'\\') + 1);
				if(item->text.IsEmpty())
				{
					item->text = item->path;
				}
			}
			ILFree(pidl);

			if(!item->LoadImage())
			{
				item->image->image->Load(appPath + L"Images\\icon-blank.png");
				if(item->type != DockItemTypeSeparator)
				{
					item->reflection->image->Assign(item->image->image);
				}
			}

			item->Rect(ItemRect(item, dockPosition));
			item->TopMost(dockIsTopMost);

			AddItem(item);
		}
		break;

	default:
		{
			switch(menuItem->type)
			{
			case DockItemTypeDocklet:
				{
					CLMenuItem *item = menu->GetItem(nID);
					if(menuItem->menu.Find(item))
					{
						menuItem->plugin->PluginEvent((XWDId)menuItem, menuItem->pluginData, XWDEventMenuItemClick, item->nID);
					}
				}
				break;

			case DockItemTypePublicPlugin:
				{
					CLMenuItem *item = menu->GetItem(nID);
					if(menuItem->menu.Find(item))
					{
						if (item->AliasId)
						{
							XWDPPluginEvent(menuItem->publicPluginInfo, XWDPEventMenuSelect, item->AliasId);
						}
						else
						{
							if(::IsIconic(item->hWnd))
							{
								::ShowWindow(item->hWnd, SW_RESTORE);
							}
							::SetForegroundWindow(item->hWnd);
							::BringWindowToTop(item->hWnd);
						}
					}
				}
				break;

			case DockItemTypeRunningApp:
			case DockItemTypeIcon:
				{
					CLMenuItem *item = menu->GetItem(nID);
					if(menuItem->menu.Find(item))
					{
						if(::IsIconic(item->hWnd))
						{
							::ShowWindow(item->hWnd, SW_RESTORE);
						}
						::SetForegroundWindow(item->hWnd);
						::BringWindowToTop(item->hWnd);
					}
				}
				break;
			}
		}
		break;
	}
}

static bool IsFirstFindMenuWindow;
BOOL CALLBACK CXWD::FindMenuEnumWindows(HWND hWnd, LPARAM lParam)
{
	DWORD pid = NULL;
	GetWindowThreadProcessId(hWnd, &pid);
	if(((DWORD)lParam == pid) && ::IsWindowVisible(hWnd))
	{
		DWORD style = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
		if((style & WS_EX_TOOLWINDOW) || ((::GetParent(hWnd) != NULL) && ((style & WS_EX_APPWINDOW) == 0)))
		{
			return TRUE;
		}
		wchar_t buff[MAX_PATH];
		GetClassName(hWnd, buff, MAX_PATH);
		if((wcscmp(buff, L"Progman") == 0) || (wcscmp(buff, L"WorkerW") == 0) || 
			(wcscmp(buff, L"Shell_TrayWnd") == 0) || (wcscmp(buff, L"XWindowsDockClass") == 0))
		{
			return TRUE;
		}
		::GetWindowText(hWnd, buff, MAX_PATH);

		CString s = buff;
		if(s.GetLength() > 25 + 3)
		{
			s = s.Mid(0, 25) + L"...";
		}

		if(IsFirstFindMenuWindow)
		{
			IsFirstFindMenuWindow = false;
			xwd->menuItem->menu.AddTail(xwd->menu->InsertLine(NULL, false));
		}

		int id = 1;
		while(xwd->menu->GetItem(id))
		{
			id++;
		}

		CLMenuItem *item = xwd->menu->Insert(NULL, s, id, false, false);
		item->hWnd = hWnd;

		HICON hIcon = (HICON)GetClassLongPtr(hWnd, GCLP_HICON);
		if(hIcon)
		{
			Icons::GetDIBFromIcon(hIcon, item->icon);
		}
		else
		{
			Icons::GetIcon(GetPathByHWND(hWnd), item->icon);
		}

		xwd->menuItem->menu.AddTail(item);
		if(xwd->menuItem->menu.GetCount() > 10)
		{
			item = xwd->menu->Insert(NULL, L"...", 0, false, false);
			xwd->menu->EnableItem(item, false);
			xwd->menuItem->menu.AddTail(item);
			return FALSE;
		}
	}
	return TRUE;
}

void CXWD::OnMenuPopup(CDockItem *item, bool dockMenu, bool drop)
{
	menuItem = item;
	bool isUpdate = !drop && !dockMenu && menuItem && (menuItem->type == DockItemTypeUpdate);
	bool isIcon = !drop && !dockMenu && menuItem && (menuItem->type == DockItemTypeIcon);
	bool isSeparator = !drop && !dockMenu && menuItem && (menuItem->type == DockItemTypeSeparator);
	bool isODDocklet = !drop && !dockMenu && menuItem && (menuItem->type == DockItemTypeODDocklet);
	bool isDocklet = !drop && !dockMenu && menuItem && (menuItem->type == DockItemTypeDocklet);
	bool isPublicPlugin = !drop && !dockMenu && menuItem && (menuItem->type == DockItemTypePublicPlugin);
	bool isRunningApp = !drop && !dockMenu && menuItem && (menuItem->type == DockItemTypeRunningApp);

	menu->VisibleItem(idMenuDockSwitchMode, (dockMenu || isSeparator) && (dockPosition == DockPositionBottom) 
		&& skin && (skin->mode & DockMode2D) && (skin->mode & DockMode3D));
	menu->CheckedItem(idMenuDockMode2D, (dockMode == DockMode2D));
	menu->CheckedItem(idMenuDockMode3D, (dockMode == DockMode3D));

	menu->VisibleItem(idMenuDockPositionOnScreen, (dockMenu || isSeparator) && skin && (skin->mode & DockMode2D));
	menu->CheckedItem(idMenuDockPositionLeft, dockPosition == DockPositionLeft);
	menu->CheckedItem(idMenuDockPositionRight, dockPosition == DockPositionRight);
	menu->CheckedItem(idMenuDockPositionTop, dockPosition == DockPositionTop);
	menu->CheckedItem(idMenuDockPositionBottom, dockPosition == DockPositionBottom);

	menu->VisibleItem(idMenuDockLock, dockMenu || isSeparator);
	menu->CheckedItem(idMenuDockLock, itemsLock);

	CString path = appPath + L"PluginManager.exe";
	DWORD mode = SetErrorMode(SEM_FAILCRITICALERRORS);
	DWORD attr = GetFileAttributes(path.GetBuffer());
	SetErrorMode(mode);

	menu->VisibleItem(idMenuDockPluginManager, (dockMenu || isSeparator) && !((attr == INVALID_FILE_ATTRIBUTES) && ((GetLastError() == ERROR_FILE_NOT_FOUND) || (GetLastError() == ERROR_PATH_NOT_FOUND))));
	
	menu->VisibleItem(idMenuDockPreferences, dockMenu || isSeparator);
	menu->VisibleItem(idMenuDockAbout, dockMenu || isSeparator);
//	menu->VisibleItem(idMenuDockCheckUpdates, dockMenu || isSeparator);
	menu->VisibleItem(idMenuDockClose, !drop/*dockMenu || isSeparator*/);

	menu->VisibleItem(idMenuItemOpen, isIcon || isUpdate || isRunningApp || isPublicPlugin);
	menu->VisibleItem(idMenuItemOpenAs, (isIcon || isRunningApp || isPublicPlugin) && (menuItem->identificator & identificatorApplication));
	menu->VisibleItem(idMenuItemQuitFromApplication, (isIcon || isRunningApp || isPublicPlugin) && (menuItem->identificator & identificatorApplication) && (menuItem->identificator & identificatorIndicator));
	menu->VisibleItem(idMenuItemProperties, isIcon || (isODDocklet && menuItem->docklet->odOnConfigure));

	if((isIcon || isODDocklet || isPublicPlugin || isRunningApp || isDocklet) && !(menuItem->identificator & identificatorDirectory))
	{
		CString path = menuItem->path.Mid(0, menuItem->path.ReverseFind(L'\\') + 1);

		DWORD mode = SetErrorMode(SEM_FAILCRITICALERRORS);
		DWORD attr = GetFileAttributes(path.GetBuffer());
		SetErrorMode(mode);

		menu->VisibleItem(idMenuItemShowInExplorer, (attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY));
	}
	else
	{
		menu->VisibleItem(idMenuItemShowInExplorer, false);
	}

	menu->CheckedItem(idMenuItemOptionsKeepInDock, !isRunningApp);
	menu->VisibleItem(idMenuItemOptionsRemoveIcon, isIcon && !menuItem->icon.IsEmpty());
	menu->CheckedItem(idMenuItemOptions, !drop);
	menu->VisibleItem(idMenuItemOptions, !dockMenu && !drop);

	if ((isIcon && (menuItem->identificator & identificatorApplication)) || isPublicPlugin)
	{
		menu->VisibleItem(idMenuItemOptionsRunWithWindows, true);

		wchar_t buff[MAX_PATH];
		SHGetSpecialFolderPath(NULL, buff, CSIDL_STARTUP, TRUE);
		CString path;

		path = menuItem->path.Mid(menuItem->path.ReverseFind(L'\\') + 1, menuItem->path.GetLength());
		path = CString(buff) + L"\\" + path.Mid(0, path.ReverseFind(L'.')) + L".lnk";

		DWORD mode = SetErrorMode(SEM_FAILCRITICALERRORS);
		DWORD attr = GetFileAttributes(path.GetBuffer());
		SetErrorMode(mode);

		menu->CheckedItem(idMenuItemOptionsRunWithWindows, !((attr == INVALID_FILE_ATTRIBUTES) && 
			((GetLastError() == ERROR_FILE_NOT_FOUND) || (GetLastError() == ERROR_PATH_NOT_FOUND))));
	}
	else
	{
		menu->VisibleItem(idMenuItemOptionsRunWithWindows, false);
	}
	
	menu->VisibleItem(idMenuItemRestoreRecycleBin, false && isIcon && (menuItem->identificator & identificatorRecycleBin));
	menu->VisibleItem(idMenuItemEmptyRecycleBin, isIcon && (menuItem->identificator & identificatorRecycleBin));
	menu->EnableItem(idMenuItemRestoreRecycleBin, recycleBinFull);
	menu->EnableItem(idMenuItemEmptyRecycleBin, recycleBinFull);

	menu->VisibleItem(idMenuItemDropSetupIcon, drop && dropIsIcon && !isRunningApp);
	menu->VisibleItem(idMenuItemDropRunIn, drop && ((menuItem->identificator & identificatorApplication) || (menuItem->type == DockItemTypePublicPlugin)));
	menu->VisibleItem(idMenuItemDropCopyTo, drop && (menuItem->identificator & identificatorDirectory));
	menu->VisibleItem(idMenuItemDropMoveTo, drop && (menuItem->identificator & identificatorDirectory));
	menu->VisibleItem(idMenuItemDropMoveToRecycleBin, drop && (menuItem->identificator & identificatorRecycleBin));
	menu->VisibleItem(idMenuItemDropCancel, drop);

	menu->VisibleItem(idMenuDockAdd, !drop/*dockMenu || isSeparator*/);

	if(menuItem)
	{
		POSITION p = items->GetHeadPosition();
		while(p)
		{
			CDockItem *item = items->GetAt(p);
			if((item->type == DockItemTypeIcon) || (item->type == DockItemTypeRunningApp) || (item->type == DockItemTypePublicPlugin))
			{
				POSITION p2 = item->menu.GetHeadPosition();
				while(p2)
				{
					menu->Remove(item->menu.GetAt(p2));
					item->menu.GetNext(p2);
				}
				item->menu.RemoveAll();
			}
			items->GetNext(p);
		}

		if (isPublicPlugin)
		{
			XWDPMenu *menuPlugin;
			XWDId count = XWDPPluginMenu(menuItem->publicPluginInfo, &menuPlugin);
			if (count)
			{
				bool isFirstOne = true;
				for (XWDId i = 0; i < count; i++)
				{
					if (menuPlugin[i].id)
					{
						if (isFirstOne)
						{
							isFirstOne = false;
							menuItem->menu.AddTail(menu->InsertLine(NULL, false));
						}

						int id = 1;
						while(menu->GetItem(id))
						{
							id++;
						}

						CLMenu *menuParent = menu;
						if (menuPlugin[i].parentId)
						{
							if (CLMenuItem *menuParentItem = menu->GetItemByAlias(menuPlugin[i].parentId))
							{
								menuParent = menuParentItem->menu;
							}
						}

						CLMenuItem *item = menuParent->Insert(NULL, menuPlugin[i].text, id, menuPlugin[i].isCheckbox != 0, false);
						item->checked = menuPlugin[i].isChecked != 0;
						item->AliasId = menuPlugin[i].id;
						item->AliasParentId = menuPlugin[i].parentId;

						menuItem->menu.AddTail(item);
					}
				}
				free(menuPlugin);
			}
		}

		p = menuItem->menu.GetHeadPosition();
		while(p)
		{
			menu->VisibleItem(menuItem->menu.GetAt(p));
			menuItem->menu.GetNext(p);
		}

		if(menuItem->identificator & identificatorIndicator)
		{
			IsFirstFindMenuWindow = true;

			CString path;
			if(IsMyComputer(menuItem->path))
			{
				wchar_t buff[MAX_PATH] = {0};
				GetWindowsDirectory(buff, MAX_PATH);
				path = CString(buff) + L"\\explorer.exe";
			}
			else
			{
				path = menuItem->path;
			}

			HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
			if(hSnapshot)
			{
				PROCESSENTRY32 processEntry = {0};
				processEntry.dwSize = sizeof(PROCESSENTRY32);
				if(Process32First(hSnapshot, &processEntry))
				do
				{
					if(GetPathByPID(processEntry.th32ProcessID).CompareNoCase(path) == 0)
					{
						EnumWindows(FindMenuEnumWindows, (LPARAM)processEntry.th32ProcessID);
						break;
					}
				}
				while(Process32Next(hSnapshot, &processEntry));
				CloseHandle(hSnapshot);
			}
		}
	}

	if(menu->ItemCount(true, true) > 0)
	{
		balloonText->Hide();
		if(menuItem && (menuItem->type != DockItemTypeSeparator))
		{
			menuItem->SetState(StateFlagPressed);
		}
	}

	if(isDocklet)
	{
		menuItem->plugin->PluginEvent((XWDId)menuItem, menuItem->pluginData, XWDEventMenuPopup);
	}
}

void CXWD::OnMenuHide(bool)
{
	if(menuItem)
	{
		POSITION p = menuItem->menu.GetHeadPosition();
		while(p)
		{
			menu->VisibleItem(menuItem->menu.GetAt(p), false);
			menuItem->menu.GetNext(p);
		}
		menuItem->RemoveState(StateFlagPressed);
		if(menuItem->type == DockItemTypeDocklet)
		{
			menuItem->plugin->PluginEvent((XWDId)menuItem, menuItem->pluginData, XWDEventMenuHide);
		}
	}
}

LRESULT CXWD::OnScreenCmpNotify(WPARAM, LPARAM)
{
	dib->Assign(bckg);
	if(dib->Ready())
	{
		DrawDock3DReflection(dib);
		LayerUpdate(dib);
	}
	return 0;
}

void CXWD::DockAdjustPosition(bool animating)
{
	StopBouncing();
	if(!skin)
	{
		return;
	}

	// change positions and size...
	CRect rect = DockRect(dockPosition);
	SetWindowPos(&wndTop, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	// position items
	POSITION p = items->GetHeadPosition();
	while(p)
	{
		CDockItem *item = items->GetAt(p);
		if(!(draging && (item == dragItem)))
		{
			CRect rect = ItemRect(item, dockPosition);
			item->Move(rect.left, rect.top, animating);
			if(!animating && (item->identificator & identificatorIndicator))
			{
				rect = ItemIndicatorRect(item);
				item->indicator->SetWindowPos(&wndTop, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
			}
		}
		items->GetNext(p);
	}
}

void CXWD::DockAdjustSize(bool animating, bool resize)
{
	StopBouncing();
	if(!skin || !(skin->mode & dockMode))
	{
		return;
	}
	switch(dockMode)
	{
	case DockMode2D:
		{
			dockWidth = skin->skipLeft2d + skin->skipRight2d;
			dockHeight = skin->skipTop2d + skin->skipBottom2d + iconSize;

			POSITION p = items->GetHeadPosition();
			while(p)
			{
				CDockItem *item = items->GetAt(p);
				dockWidth += (int)((skin->skipBetweenIcons2d + ItemSize(item, DockPositionBottom).cx) * item->resizeFlag);
				items->GetNext(p);
			}
		}
		break;

	case DockMode3D:
		{
			dockHeight = iconSize + (int)(2.0f * iconSize * skin->iconPosition3d / 100);
			dockHeightOffset3d = dockHeight * sin(PI * (30 + 10.0f * skin->iconPosition3d / 100) / 180);
			dockHeight += skin->bckgEdge3d->Height();
	
			dockWidth = (int)((dockHeight - skin->bckgEdge3d->Height()) * sin(PI * skin->bckgEdgeAngle3d / 180)) + skin->bckgEdgeOffset3d * 2;

			POSITION p = items->GetHeadPosition();
			while(p)
			{
				CDockItem *item = items->GetAt(p);
				dockWidth += (int)((skin->iconSizeBetween3d + ItemSize(item, DockPositionBottom).cx) * item->resizeFlag);
				items->GetNext(p);
			}
		}
		break;
	}

	// draw panel
	LayerDraw(bckg);
	dib->Assign(bckg);
	if(dockMode == DockMode3D)
	{
		DrawDock3DReflection(dib);
	}

	// change positions and size...
	CRect rect = DockRect(dockPosition);
	SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

	// and now update dock without hasitating :)
	LayerUpdate(dib);

	// position & setup settings for items
	POSITION p = items->GetHeadPosition();
	while(p)
	{
		CDockItem *item = items->GetAt(p);
		item->dockMode = dockMode;
		item->dockPosition = dockPosition;
		if(skin && (item->dockMode & DockMode3D))
		{
			item->reflectionOffset = iconSize * skin->iconReflectionOffset3d / 100;
			item->reflectionSkipTop = iconSize * skin->iconReflectionSkipTop3d / 100;
			item->reflectionSkipBottom = iconSize * skin->iconReflectionSkipBottom3d / 100;
			item->reflectionSize = max(0, item->reflectionOffset + iconSize * skin->iconPosition3d / 100);
			item->reflectionOpacity = (unsigned char)(255 * skin->iconReflectionOpacity3d / 100);
			item->reflectionOpacityFactor = (unsigned char)(skin->iconReflectionOpacityFactor3d);
			item->iconShadowEnabled = iconShadowEnabled;
		}
		else
		{
			item->reflectionSize = 0;
		}
		if((item->type == DockItemTypeDocklet) || (item->type == DockItemTypePublicPlugin))
		{
			POSITION p = item->notifications.GetHeadPosition();
			while(p)
			{
				CNotification *notification = item->notifications.GetAt(p);
				if(notification->IsWindowVisible())
				{
					notification->SetWindowPos(&wndTop, 0, 0, notification->CalculateWidth((int)(iconSize * 0.45f)), (int)(iconSize * 0.45f), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
					notification->LayerDraw();
					notification->LayerUpdate();
				}
				item->notifications.GetNext(p);
			}
		}
		if(!(draging && (item == dragItem)))
		{
			item->ReflectionShow(item->image->IsWindowVisible() == TRUE);
			CRect rect = ItemRect(item, dockPosition);
			if(resize)
			{
				item->Rect(rect);
			}
			else
			{
				item->Move(rect.left, rect.top, animating);
			}
			if(item->identificator & identificatorIndicator)
			{
				rect = ItemIndicatorRect(item);
				if(resize)
				{
					item->indicator->SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
				}
				else
				{
					item->indicator->SetWindowPos(&wndTop, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
				}
			}
		}
		items->GetNext(p);
	}
}

CRect CXWD::DockRect(unsigned int position)
{
	CRect rect(0, 0, 0, 0);
	if(!skin || !monitor)
	{
		return rect;
	}
	switch(dockMode)
	{
	case DockMode3D:
		{
			rect.left = monitor->WorkRect().left + (monitor->WorkRect().Width() - dockWidth) / 2;
			rect.top = monitor->WorkRect().bottom - (int)(dockHeight * animationPopup);
			rect.right = rect.left + dockWidth;
			rect.bottom = rect.top + dockHeight;
		}
		break;

	case DockMode2D:
		{
			switch(position)
			{
			case DockPositionLeft:
				{
					rect.left = monitor->WorkRect().left - (int)(dockHeight * (1 - animationPopup));
					rect.top = monitor->WorkRect().top + (monitor->WorkRect().Height() - dockWidth) / 2;
					rect.right = rect.left + dockHeight;
					rect.bottom = rect.top + dockWidth;
				}
				break;

			case DockPositionTop:
				{
					rect.left = monitor->WorkRect().left + (monitor->WorkRect().Width() - dockWidth) / 2;
					rect.top = monitor->WorkRect().top - (int)(dockHeight * (1 - animationPopup));
					rect.right = rect.left + dockWidth;
					rect.bottom = rect.top + dockHeight;
				}
				break;

			case DockPositionRight:
				{
					rect.left = monitor->WorkRect().right - (int)(dockHeight * animationPopup);
					rect.top = monitor->WorkRect().top + (monitor->WorkRect().Height() - dockWidth) / 2;
					rect.right = rect.left + dockHeight;
					rect.bottom = rect.top + dockWidth;
				}
				break;

			//case DockPositionBottom:
			default:
				{
					rect.left = monitor->WorkRect().left + (monitor->WorkRect().Width() - dockWidth) / 2;
					rect.top = monitor->WorkRect().bottom - (int)(dockHeight * animationPopup);
					rect.right = rect.left + dockWidth;
					rect.bottom = rect.top + dockHeight;
				}
				break;
			}
		}
		break;
	}
	rect.OffsetRect(monitor->Rect().left, monitor->Rect().top);
	return rect;
}

bool CXWD::DockSwitchPosition(DockPosition position)
{
	if(position != DockPositionBottom)
	{
		if(!DockSwitchMode(DockMode2D))
		{
			return false;
		}
	}
	if(dockPosition != position)
	{
		dockPosition = position;
		DockAdjustSize();

		POSITION p = items->GetHeadPosition();
		while(p)
		{
			CDockItem *item = items->GetAt(p);
			item->BounceStop();
			if((item->type == DockItemTypeSeparator) && skin)
			{
				item->image->image->Assign(skin->separator2d);
			}
			if(skin)
			{
				item->indicator->image->Assign(skin->indicator2d);
				if(item->identificator & identificatorIndicator)
				{
					CRect rect = ItemIndicatorRect(item);
					item->indicator->SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
				}
			}
			item->Rect(ItemRect(item, dockPosition));
			if(item->type == DockItemTypeSeparator)
			{
				item->image->LayerDraw();
				item->image->LayerUpdate();
			}
			if(item->type == DockItemTypeDocklet)
			{
				item->plugin->PluginEvent((XWDId)item, item->pluginData, XWDEventAdjustDockEdge);
			}
			items->GetNext(p);
		}

		SendMessage(WM_SETUPDOCK, MAKEWPARAM(SetupDockIconSize, iconSize));
		UpdateReserveScreen();
		MakeBlurBehind();
	}
	return true;
}

bool CXWD::DockSwitchMode(DockMode mode)
{
	if(!skin || !(skin->mode & mode))
	{
		return false;
	}
	bool positionChanged = false;
	if(mode == DockMode3D)
	{
		dockPosition = DockPositionBottom;
		positionChanged = true;
	}
	if(dockMode != mode)
	{
		dockMode = mode;
		SceenCompare::Pause();
		DockAdjustSize();
		switch(dockMode)
		{
		case DockMode3D:
			{
				if(windowsReflectionEnabled && skin && skin->reflectionEnable3d && monitor)
				{
					CRect rect = monitor->WorkRect();
					rect.top = rect.bottom - dockHeight + (int)dockHeightOffset3d + (int)dockHeightOffset3d % 2;
					rect.bottom -= (skin->bckgEdge3d->Height() - (int)dockHeightOffset3d % 2);
					rect.OffsetRect(0, -rect.Height());
					if(SceenCompare::Runing())
					{
						SceenCompare::Rect(rect);
					}
					else
					{
						SceenCompare::Run(rect, reflection, this);
					}
				}
			}
			break;

		case DockMode2D:
			{
				SceenCompare::Stop();
			}
			break;
		}
		SceenCompare::Pause(false);

		POSITION p = items->GetHeadPosition();
		while(p)
		{
			CDockItem *item = items->GetAt(p);
			item->BounceStop();
			if((item->type == DockItemTypeSeparator) && skin)
			{
				item->image->image->Assign(dockMode == DockMode2D ? skin->separator2d : skin->separator3d);
			}
			if(skin)
			{
				item->indicator->image->Assign(dockMode == DockMode2D ? skin->indicator2d : skin->indicator3d);
				if(item->identificator & identificatorIndicator)
				{
					CRect rect = ItemIndicatorRect(item);
					item->indicator->SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
				}
			}
			item->Rect(ItemRect(item, dockPosition));
			if(item->type == DockItemTypeSeparator)
			{
				item->image->LayerDraw();
				item->image->LayerUpdate();
			}
			item->ReflectionShow(dockMode == DockMode3D);
			if(positionChanged && (item->type == DockItemTypeDocklet))
			{
				item->plugin->PluginEvent((XWDId)item, item->pluginData, XWDEventAdjustDockEdge);
			}
			items->GetNext(p);
		}
	}
	SendMessage(WM_SETUPDOCK, MAKEWPARAM(SetupDockIconSize, iconSize));
	UpdateReserveScreen();
	MakeBlurBehind();
	return true;
}

void CXWD::LayerDraw(CDIB *dib)
{
	if(!skin)
	{
		return;
	}
	//switch(dockMode)
	//{
	//case DockMode3D:
	//	{
			dib->Resize(dockWidth, dockHeight);
	//	}
	//	break;

	//case DockMode2D:
	//default:
	//	{
	//		dib->Resize(dockWidth, dockHeight);
	//	}
	//	break;
	//}
	if(!dib->Ready())
	{
		return;
	}

	RectF rf(0, 0, (REAL)dib->Width(), (REAL)dib->Height());

	Graphics g(dib->dc);
	g.SetCompositingMode(CompositingModeSourceCopy);

	switch(dockMode)
	{
	case DockMode2D:
		{
			DrawDock2D(dib, g, rf);

			switch(dockPosition)
			{
			case DockPositionLeft:
				{
					Bitmap tmp(dib->Width(), dib->Height(), dib->Width() * 4, PixelFormat32bppARGB, (BYTE*)dib->scan0);
					tmp.RotateFlip(Rotate270FlipNone);
					dib->Assign(&tmp);
				}
				break;

			case DockPositionRight:
				{
					Bitmap tmp(dib->Width(), dib->Height(), dib->Width() * 4, PixelFormat32bppARGB, (BYTE*)dib->scan0);
					tmp.RotateFlip(Rotate90FlipY);
					dib->Assign(&tmp);
				}
				break;

			case DockPositionTop:
				{
					Bitmap tmp(dib->Width(), dib->Height(), dib->Width() * 4, PixelFormat32bppARGB, (BYTE*)dib->scan0);
					tmp.RotateFlip(Rotate180FlipX);
					dib->Assign(&tmp);
				}
				break;
			}
		}
		break;

	case DockMode3D:
		{
			DrawDock3D(dib, g, rf);
		}
		break;
	}
}

void CXWD::LayerUpdate(CDIB *dib)
{
	if(!dib->Ready())
	{
		return;
	}
	CRect rect;
	GetWindowRect(&rect);

	CWnd *wndDst = GetDesktopWindow();
	CDC *hdcDst = wndDst->GetDC();
	CDC *dc = new CDC();
	dc->Attach(dib->dc);
	
	BLENDFUNCTION blend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
	CPoint zp(0, 0);
	CPoint dstPt(rect.left, rect.top);
	CSize dstSize(rect.Width(), rect.Height());

	UpdateLayeredWindow(hdcDst, &dstPt, &dstSize, dc, &zp, NULL, &blend, ULW_ALPHA);

	dc->Detach();
	delete dc;
	wndDst->ReleaseDC(hdcDst);
}

void CXWD::DrawDock2D(CDIB *dib, Gdiplus::Graphics&, RectF&)
{
	dib->Draw(CRect(
		0,
		0,
		skin->bckgLeftTop2d->Width(),
		skin->bckgLeftTop2d->Height()),
		0, 0, skin->bckgLeftTop2d, DrawFlagsPaint | DrawFlagsReflectDest);

	dib->DrawTail(CRect(
		skin->bckgLeftTop2d->Width(), 
		0,
		dib->Width() - skin->bckgRightTop2d->Width(),
		skin->bckgTop2d->Height()),
		skin->bckgTop2d->Rect(), skin->bckgTop2d, DrawFlagsPaint | DrawFlagsReflectDest);

	dib->Draw(CRect(
		dib->Width() - skin->bckgLeftTop2d->Width(),
		0,
		dib->Width(),
		skin->bckgRightTop2d->Height()),
		0, 0, skin->bckgRightTop2d, DrawFlagsPaint | DrawFlagsReflectDest);

	dib->DrawTail(CRect(
		dib->Width() - skin->bckgRight2d->Width(), 
		skin->bckgRightTop2d->Height(),
		dib->Width(),
		dib->Height() - skin->bckgRightBottom2d->Height()),
		skin->bckgRight2d->Rect(), skin->bckgRight2d, DrawFlagsPaint | DrawFlagsReflectDest);

	dib->Draw(CRect(
		dib->Width() - skin->bckgRightBottom2d->Width(),
		dib->Height() - skin->bckgRightTop2d->Height(),
		dib->Width(),
		dib->Height()),
		0, 0, skin->bckgRightBottom2d, DrawFlagsPaint | DrawFlagsReflectDest);

	dib->DrawTail(CRect(
		skin->bckgLeftBottom2d->Width(), 
		dib->Height() - skin->bckgBottom2d->Height(),
		dib->Width() - skin->bckgRightBottom2d->Width(),
		dib->Height()),
		skin->bckgBottom2d->Rect(), skin->bckgBottom2d, DrawFlagsPaint | DrawFlagsReflectDest);

	dib->Draw(CRect(
		0,
		dib->Height() - skin->bckgLeftBottom2d->Height(),
		skin->bckgLeftBottom2d->Width(),
		dib->Height()),
		0, 0, skin->bckgLeftBottom2d, DrawFlagsPaint | DrawFlagsReflectDest);

	dib->DrawTail(CRect(
		0, 
		skin->bckgLeftTop2d->Height(),
		skin->bckgLeft2d->Width(),
		dib->Height() - skin->bckgLeftBottom2d->Height()),
		skin->bckgLeft2d->Rect(), skin->bckgLeft2d, DrawFlagsPaint | DrawFlagsReflectDest);

	dib->DrawTail(CRect(
		skin->bckgLeft2d->Width(), 
		skin->bckgTop2d->Height(),
		dib->Width() - skin->bckgRight2d->Width(),
		dib->Height() - skin->bckgBottom2d->Height()),
		skin->bckgMiddle2d->Rect(), skin->bckgMiddle2d, DrawFlagsPaint | DrawFlagsReflectDest);

	dib->AlphaBlend(dib->Rect(), (unsigned char)(255 * skin->bckgOpacity2d / 100), DrawFlagsPaint);
}

void CXWD::DrawDock3DReflection(CDIB *dib)
{
	if(!windowsReflectionEnabled || !skin || !skin->reflectionEnable3d || !reflection->Ready() || !dib->Ready())
	{
		return;
	}
	int dw = min(reflection->Width(), dib->Width());
	int dh = min(reflection->Height(), dib->Height() - (int)dockHeightOffset3d);
	int dx = max(0, (monitor->WorkRect().Width() - dockWidth) / 2);
	int dsx = abs(min(0, (monitor->WorkRect().Width() - dockWidth) / 2));
	
	for(int y = 0; y < dh; y++)
	{
		DIB_AARGB *ps = (DIB_AARGB*)((int)reflection->scan0 + (y * reflection->Width() + dx) * 4);
		DIB_AARGB *pd = (DIB_AARGB*)((int)dib->scan0 + ((dib->Height() - 1 - y - (int)dockHeightOffset3d) * dib->Width() + dsx) * 4);
		for(int x = 0; x < dw; x++)
		{
			float k = pd[x]->a / 255.0f * skin->reflectionOpacity3d / 100.0f;
			pd[x]->r = (unsigned char)(pd[x]->r * (1 - k) + ps[x]->r * k);
			pd[x]->g = (unsigned char)(pd[x]->g * (1 - k) + ps[x]->g * k);
			pd[x]->b = (unsigned char)(pd[x]->b * (1 - k) + ps[x]->b * k);
		}
	}
}

void CXWD::DrawDock3D(CDIB *dib, Gdiplus::Graphics&, RectF&)
{
	CRect rect;
	rect.left = 0;
	rect.top = skin->bckgEdge3d->Height();
	rect.right = dib->Width();
	rect.bottom = dib->Height();
	rect.bottom -= (int)(rect.Height() * sin(PI * (30 + 10.0f * skin->iconPosition3d / 100) / 180));

	Dock3D::DrawImage(dib, rect, (int)(rect.Height() * sin(PI * skin->bckgEdgeAngle3d / 180)), skin->bckg3d);
	
	if(skin->bckgEdge3d->Ready())
	{
		//g.DrawImage(skin->bckgEdge3d->bmp, RectF(0, 0, (REAL)dib->Width(), (REAL)skin->bckgEdge3d->Height()));

		//dib->Draw(CRect(0, 0, dib->Width(), skin->bckgEdge3d->Height()), 
		//	skin->bckgEdge3d->Rect(), skin->bckgEdge3d, DrawFlagsPaint | DrawFlagsReflectSrc);

		dib->DrawTail(CRect(1, 0, dib->Width() - 1, skin->bckgEdge3d->Height()), 
			skin->bckgEdge3d->Rect(), skin->bckgEdge3d, DrawFlagsPaint | DrawFlagsReflectSrc);
	}

	dib->AlphaBlend(dib->Rect(), (unsigned char)(255 * skin->bckgOpacity3d / 100), DrawFlagsPaint);
}

bool CXWD::MakeBlurBehind(bool disabled)
{
	if (dockBlurRegion)
	{
		DeleteObject(dockBlurRegion);
		dockBlurRegion = NULL;
	}
	
	if (!DwmApi::Ready())
	{
		return false;
	}

	DWM_BLURBEHIND blurBehind = {0};
	blurBehind.dwFlags = DWM_BB_ENABLE | DWM_BB_TRANSITIONONMAXIMIZED;
	blurBehind.fTransitionOnMaximized = TRUE;

	if (disabled)
	{
		blurBehind.fEnable = FALSE;
		return SUCCEEDED(DwmApi::DwmEnableBlurBehindWindow(CFrameWnd::m_hWnd, &blurBehind));
	}

	blurBehind.fEnable = (BOOL)skin->blurbehindDockEnabled;
	
	if (blurBehind.fEnable)
	{
		DIB_ARGB *p = dib->scan0;
		int w = dib->Width();
		int h = dib->Height();

		const DWORD SizeOfRGNDATAHEADER = sizeof(RGNDATAHEADER);
		const DWORD SizeOfRECT = sizeof(RECT);
		const DWORD AddRectsCount = 40;

		DWORD dwRectsCount = AddRectsCount;

		RGNDATAHEADER *rgnData = (RGNDATAHEADER*)malloc(SizeOfRGNDATAHEADER + dwRectsCount * SizeOfRECT);
		memset(rgnData, 0, SizeOfRGNDATAHEADER + dwRectsCount * SizeOfRECT);
		rgnData->dwSize = SizeOfRGNDATAHEADER;
		rgnData->iType = RDH_RECTANGLES;

		LPRECT rects = (LPRECT)((DWORD)rgnData + SizeOfRGNDATAHEADER);

		for (int y = h - 1; y >= 0; y--)
		{
			for (int x = 0; x < w; x++, p++)
			{
				if (p->a)
				{
					rects[rgnData->nCount++] = CRect(x, y, x + 1, y + 1);
					if (rgnData->nCount >= dwRectsCount)
					{
						dwRectsCount += AddRectsCount;
						rgnData = (RGNDATAHEADER*)realloc(rgnData, SizeOfRGNDATAHEADER + dwRectsCount * SizeOfRECT);
						rects = (LPRECT)((DWORD)rgnData + SizeOfRGNDATAHEADER);
					}
				}
			}
		}

		dockBlurRegion = ExtCreateRegion(NULL, SizeOfRGNDATAHEADER + rgnData->nCount * SizeOfRECT, (LPRGNDATA)rgnData);
		free(rgnData);

		blurBehind.dwFlags |= DWM_BB_BLURREGION;
		blurBehind.hRgnBlur = dockBlurRegion;
	}

	return SUCCEEDED(DwmApi::DwmEnableBlurBehindWindow(CFrameWnd::m_hWnd, &blurBehind));
}

void CXWD::SettingsSave()
{
	CIniReader ini(dataPath + L"settings.ini");
	ini.setKey((int)dockPosition, L"position", L"Dock");
	ini.setKey((int)dockMode, L"mode", L"Dock");
	ini.setKey(iconSize, L"iconSize", L"Dock");
	ini.setKey((int)iconShadowEnabled, L"iconShadowEnabled", L"Dock");
	ini.setKey((int)windowsReflectionEnabled, L"windowsReflectionEnabled", L"Dock");
	ini.setKey((int)itemsLock, L"lock", L"Dock");
	ini.setKey((int)dockIsTopMost, L"topMost", L"Dock");
	ini.setKey((int)checkForUpdates, L"checkForUpdates", L"Dock");
	ini.setKey((int)reserveScreen, L"reserveScreen", L"Dock");
	ini.setKey((int)showAllRunningAppsInDock, L"showAllRunningAppsInDock", L"Dock");
	if(monitor)
	{
		int i = 0;
		POSITION p = monitors->items.GetHeadPosition();
		while(p)
		{
			if(monitors->items.GetAt(p) == monitor)
			{
				break;
			}
			i++;
			monitors->items.GetNext(p);
		}
		ini.setKey(i, L"monitor", L"Dock");
	}
	if(skin)
	{
		ini.setKey(skin->name, L"name", L"Skin");
	}
	ini.setKey(balloonTextSize, L"size", L"BalloonText");
}

void CXWD::ItemsSave()
{
	DeleteFile(dataPath + L"items.ini");
	CIniReader ini(dataPath + L"items.ini");

	CString name;
	int i = 0;
	int n = 0;
	int k = 0;

	POSITION p = items->GetHeadPosition();
	while(p)
	{
		name.Format(L"%d", ++i);

		CDockItem *item = items->GetAt(p);

		switch(item->type)
		{
		case DockItemTypeDocklet:
			{
				ini.setKey((int)item->type, L"type", name);
				ini.setKey(item->path, L"path", name);
				ini.setKey(item->pluginId, L"id", name);
			}
			break;

		case DockItemTypeODDocklet:
			{
				ini.setKey((int)item->type, L"type", name);
				ini.setKey(item->path, L"path", name);
				ini.setKey(item->odDockletId, L"id", name);
			}
			break;

		case DockItemTypePublicPlugin:
			{
				if(item->publicPluginInfo->keepInDock)
				{
					ini.setKey((int)item->type, L"type", name);
					ini.setKey(item->text, L"text", name);
					ini.setKey(item->path, L"path", name);
					ini.setKey(item->icon, L"icon", name);
					ini.setKey(item->iconIndex, L"iconIndex", name);
					ini.setKey(item->arguments, L"arguments", name);
					ini.setKey(item->workDirectory, L"workDirectory", name);
					ini.setKey((int)item->showAs, L"showAs", name);
					ini.setKey((int)item->exec, L"exec", name);

					if(item->identificator & identificatorRecycleBin)
					{
						ini.setKey(item->additionalIcon, L"icon2", name);
						ini.setKey(item->additionalIcon2, L"icon3", name);
					}

					ini.setKey(item->publicPluginInfo->uid, L"publicUId", name);
					ini.setKey((int)item->publicPluginInfo->bounceable, L"bounceable", name);
					ini.setKey((int)item->publicPluginInfo->activatable, L"activatable", name);
					ini.setKey((int)item->publicPluginInfo->exposable, L"exposable", name);

					if (item->folderWatcherList.GetCount())
					{
						CString key;
						key.Format(L"FolderWatcherList%d", ++n);
						ini.setKey(key, L"folderWatcherList", name);

						POSITION p = item->folderWatcherList.GetHeadPosition();
						while (p)
						{
							CString name1;
							name1.Format(L"FolderWatcher%d", ++k);

							ini.setKey(L"", name1, key);

							FolderWatcher::FolderInfo *info = item->folderWatcherList.GetAt(p);

							ini.setKey(info->actions, L"actions", name1);
							ini.setKey(info->id, L"id", name1);
							ini.setKey(info->folder, L"folder", name1);

							item->folderWatcherList.GetNext(p);
						}
					}
				}
			}
			break;

		case DockItemTypeIcon:
			{
				ini.setKey((int)item->type, L"type", name);
				ini.setKey(item->text, L"text", name);
				ini.setKey(item->path, L"path", name);
				ini.setKey(item->icon, L"icon", name);
				ini.setKey(item->iconIndex, L"iconIndex", name);
				ini.setKey(item->arguments, L"arguments", name);
				ini.setKey(item->workDirectory, L"workDirectory", name);
				ini.setKey((int)item->showAs, L"showAs", name);
				ini.setKey((int)item->exec, L"exec", name);

				if(item->identificator & identificatorRecycleBin)
				{
					ini.setKey(item->additionalIcon, L"icon2", name);
					ini.setKey(item->additionalIcon2, L"icon3", name);
				}
			}
			break;

		case DockItemTypeSeparator:
			{
				ini.setKey((int)item->type, L"type", name);
			}
			break;
		}

		items->GetNext(p);
	}
}

void CXWD::ItemsLoadDefault()
{
	CDockItem *item;
	
	// My Computer
	LPITEMIDLIST pidl;
	SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidl);
	if(!pidl)
	{
		return;
	}
	item = new CDockItem();
	items->AddTail(item);

	item->SetBottomWindow(this);
	item->poof = poof;
	item->type = DockItemTypeIcon;
	item->path = GetSpeacialFolderLocation(CSIDL_DRIVES);

	SHFILEINFO sfi;
	SHGetFileInfo((LPCWSTR)pidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_PIDL);
	item->text = sfi.szDisplayName;
	if(item->text.IsEmpty())
	{
		item->text = item->path.Mid(item->path.ReverseFind(L'\\') + 1);
		if(item->text.IsEmpty())
		{
			item->text = item->path;
		}
	}
	ILFree(pidl);

	if(!item->LoadImage())
	{
		item->image->image->Load(appPath + L"Images\\icon-blank.png");
		if(item->type != DockItemTypeSeparator)
		{
			item->reflection->image->Assign(item->image->image);
		}
	}

	CRect rect = ItemIndicatorRect(item);
	item->identificator |= identificatorIndicator;
	item->indicator->image->Assign((dockMode == DockMode2D) ? skin->indicator2d : skin->indicator3d);
	item->indicator->SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
	item->IndicatorShow();
	
	// Separator
	item = new CDockItem();
	item->SetBottomWindow(this);
	item->poof = poof;
	item->type = DockItemTypeSeparator;
	items->AddTail(item);

	// Recycler Bin
	SHGetSpecialFolderLocation(NULL, CSIDL_BITBUCKET, &pidl);
	if(!pidl)
	{
		return;
	}
	item = new CDockItem();
	items->AddTail(item);

	item->SetBottomWindow(this);
	item->poof = poof;
	item->type = DockItemTypeIcon;
	item->identificator |= identificatorRecycleBin;
	item->path = GetSpeacialFolderLocation(CSIDL_BITBUCKET);

	SHGetFileInfo((LPCWSTR)pidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_PIDL);
	item->text = sfi.szDisplayName;
	if(item->text.IsEmpty())
	{
		item->text = item->path.Mid(item->path.ReverseFind(L'\\') + 1);
		if(item->text.IsEmpty())
		{
			item->text = item->path;
		}
	}
	ILFree(pidl);

	if(!item->LoadImage())
	{
		item->image->image->Load(appPath + L"Images\\icon-blank.png");
		if(item->type != DockItemTypeSeparator)
		{
			item->reflection->image->Assign(item->image->image);
		}
	}
}

void CXWD::ItemsLoad()
{
	if(IsFirstTimeStart())
	{
		ItemsLoadDefault();
		return;
	}

	CIniReader ini(dataPath + L"items.ini");
	CStringList *names = ini.getSectionNames();

	CString name;
	DWORD mode = SetErrorMode(SEM_FAILCRITICALERRORS);

	POSITION p = names->GetHeadPosition();
	while(p)
	{
		name = names->GetAt(p);
	
		DockItemType type = (DockItemType)_wtoi(ini.getKeyValue(L"type", name).GetBuffer());

		switch(type)
		{
		case DockItemTypeDocklet:
			{
				CPlugin *plugin = plugins->Find(ini.getKeyValue(L"path", name));
				if(plugin)
				{
					plugin->Load();
					if(plugin->GetPluginType() == XWDPluginDocklet)
					{
						CDockItem *item = new CDockItem();
						
						item->Show(false);
						item->SetBottomWindow(this);
						item->poof = poof;
						item->type = type;
						item->plugin = plugin;
						item->path = plugin->path;
						item->pluginId = _wtoi(ini.getKeyValue(L"id", name).GetBuffer());
						item->image->image->Load(appPath + L"Images\\icon-blank.png");
						item->reflection->image->Assign(item->image->image);

						for(;;)
						{
							bool found = false;
							POSITION p = items->GetHeadPosition();
							while(p)
							{
								CDockItem *pitem = items->GetAt(p);
								if((pitem->type == DockItemTypeDocklet) && (pitem->pluginId == item->pluginId))
								{
									found = true;
									break;
								}
								items->GetNext(p);
							}
							if(!found)
							{
								break;
							}
							item->pluginId++;
						}

						items->AddTail(item);
					}
					plugin->Unload();
				}
			}
			break;

		case DockItemTypeODDocklet:
			{
				CODDocklet *docklet = odDocklets->Find(ini.getKeyValue(L"path", name));
				if(docklet)
				{
					CDockItem *item = new CDockItem();
					
					item->Show(false);
					item->SetBottomWindow(this);
					item->poof = poof;
					item->type = type;
					item->docklet = docklet;
					item->path = docklet->path;
					item->odDockletId = _wtoi(ini.getKeyValue(L"id", name).GetBuffer());
					item->image->image->Load(appPath + L"Images\\icon-blank.png");
					item->reflection->image->Assign(item->image->image);

					for(;;)
					{
						bool found = false;
						POSITION p = items->GetHeadPosition();
						while(p)
						{
							CDockItem *pitem = items->GetAt(p);
							if((pitem->type == DockItemTypeODDocklet) && (pitem->odDockletId == item->odDockletId))
							{
								found = true;
								break;
							}
							items->GetNext(p);
						}
						if(!found)
						{
							break;
						}
						item->odDockletId++;
					}

					items->AddTail(item);
				}
			}
			break;

		case DockItemTypePublicPlugin:
			{
				XWDId uid = _wtoi(ini.getKeyValue(L"publicUId", name).GetBuffer());
				if (uid)
				{
					CDockItem *item = new CDockItem();
				
					item->Show(false);
					item->SetBottomWindow(this);
					item->poof = poof;
					item->type = type;
					item->text = ini.getKeyValue(L"text", name);
					item->path = ini.getKeyValue(L"path", name);
					item->icon = ini.getKeyValue(L"icon", name);
					item->iconIndex = _wtoi(ini.getKeyValue(L"iconIndex", name).GetBuffer());
					item->arguments = ini.getKeyValue(L"arguments", name);
					item->workDirectory = ini.getKeyValue(L"workDirectory", name);
					item->showAs = (DockItemShowAs)_wtoi(ini.getKeyValue(L"showAs", name).GetBuffer());
					item->exec = (DockItemExec)_wtoi(ini.getKeyValue(L"exec", name).GetBuffer());

					if(!item->LoadImage())
					{
						item->image->image->Load(appPath + L"Images\\icon-blank.png");
						if(item->type != DockItemTypeSeparator)
						{
							item->reflection->image->Assign(item->image->image);
						}
					}

					items->AddTail(item);

					XWDPAddPluginInfo(item->path, NULL, uid, &item->publicPluginInfo);
					item->publicPluginInfo->keepInDock = true;
					item->publicPluginInfo->bounceable = _wtoi(ini.getKeyValue(L"bounceable", name).GetBuffer()) != 0;
					item->publicPluginInfo->activatable = _wtoi(ini.getKeyValue(L"activatable", name).GetBuffer()) != 0;
					item->publicPluginInfo->exposable = _wtoi(ini.getKeyValue(L"exposable", name).GetBuffer()) != 0;

					item->publicPluginInfo->settingsPath = dataPath + L"Public Plugins\\";
					CreateDirectory(item->publicPluginInfo->settingsPath.GetBuffer(), NULL);

					item->publicPluginInfo->settingsPath.AppendFormat(L"%d\\", item->publicPluginInfo->uid);
					CreateDirectory(item->publicPluginInfo->settingsPath.GetBuffer(), NULL);

					CString folderWatcherList = ini.getKeyValue(L"folderWatcherList", name);
					if (ini.sectionExists(folderWatcherList))
					{
						CStringList *list = ini.getSectionData(folderWatcherList);

						POSITION p = list->GetHeadPosition();
						while (p)
						{
							CString folderWatcher = list->GetAt(p);
							folderWatcher.Delete(folderWatcher.Find(L"="), folderWatcher.GetLength());

							int actions = _wtoi(ini.getKeyValue(L"actions", folderWatcher).GetBuffer());
							int folderId = _wtoi(ini.getKeyValue(L"id", folderWatcher).GetBuffer());
							CString folder = ini.getKeyValue(L"folder", folderWatcher);

							XWDPluginAddFolderWatcher(item->publicPluginInfo, folderId, actions, folder);
				
							list->GetNext(p);
						}
					}
				}
			}
			break;

		case DockItemTypeIcon:
			{
				CString path = ini.getKeyValue(L"path", name);
				if (!path.IsEmpty())
				{
					CDockItem *item = new CDockItem();
				
					item->Show(false);
					item->SetBottomWindow(this);
					item->poof = poof;
					item->type = type;
					item->text = ini.getKeyValue(L"text", name);
					item->path = path;
					item->icon = ini.getKeyValue(L"icon", name);
					item->iconIndex = _wtoi(ini.getKeyValue(L"iconIndex", name).GetBuffer());
					item->arguments = ini.getKeyValue(L"arguments", name);
					item->workDirectory = ini.getKeyValue(L"workDirectory", name);
					item->showAs = (DockItemShowAs)_wtoi(ini.getKeyValue(L"showAs", name).GetBuffer());
					item->exec = (DockItemExec)_wtoi(ini.getKeyValue(L"exec", name).GetBuffer());

					// may we can work with it ?
					DWORD attr = GetFileAttributes(item->path.GetBuffer());
					if((attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY))
					{
						item->identificator |= identificatorDirectory;
					}

					if(item->path.Mid(item->path.ReverseFind(L'.')).MakeLower() == L".exe")
					{
						item->identificator |= identificatorApplication;
					}

					if(IsRecycleBin(item->path))
					{
						item->identificator |= identificatorRecycleBin;

						item->additionalIcon = ini.getKeyValue(L"icon2", name);
						item->additionalIcon2 = ini.getKeyValue(L"icon3", name);

						if(recycleBinFull)
						{
							item->icon = item->additionalIcon;
						}
						else
						{
							item->icon = item->additionalIcon2;
						}
					}

					if(IsMyComputer(item->path))
					{
						CRect rect = ItemIndicatorRect(item);
						item->identificator |= identificatorIndicator;
						item->indicator->image->Assign((dockMode == DockMode2D) ? skin->indicator2d : skin->indicator3d);
						item->indicator->SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
						item->IndicatorShow();
					}

					if(!item->LoadImage())
					{
						item->image->image->Load(appPath + L"Images\\icon-blank.png");
						if(item->type != DockItemTypeSeparator)
						{
							item->reflection->image->Assign(item->image->image);
						}
					}

					items->AddTail(item);
				}
			}
			break;

		case DockItemTypeSeparator:
			{
				CDockItem *item = new CDockItem();
					
				item->Show(false);
				item->SetBottomWindow(this);
				item->poof = poof;
				item->type = type;
	
				items->AddTail(item);
			}
			break;
		}

		names->GetNext(p);
	}
	SetErrorMode(mode);
}

void CXWD::DoItemExpose(CDockItem *item)
{
	if(!item || !DwmApi::Ready())
	{
		return;
	}
	CString path;
	if(IsMyComputer(item->path))
	{
		wchar_t buff[MAX_PATH] = {0};
		GetWindowsDirectory(buff, MAX_PATH);
		path = CString(buff) + L"\\explorer.exe";
	}
	else
	{
		path = item->path;
	}
	exposedItem = item;
	CRect r = ItemRect(exposedItem, dockPosition);
	if ((dockMode == DockMode3D) && iconShadowEnabled)
	{
		r.top -= exposedItem->imageShadowSize3d;
	}
	exposed = expose->Expose(CFrameWnd::m_hWnd, r, exposedItem->image->dib, path, dockPosition, (GetKeyState(VK_SHIFT) & 0x8000) == 0x8000);
	if(exposed)
	{
		lockMouseEffect++;
		hoverItem = NULL;
		if(balloonText->IsWindowVisible() && !balloonText->hiding)
		{
			balloonText->ShowWindow(SW_HIDE);
		}
	}
}

void CXWD::DoItemDropSetupIcon(CDockItem *item)
{
	if(item->identificator & identificatorRecycleBin)
	{
		if(recycleBinFull)
		{
			item->additionalIcon = PIDLToString(dropFiles.GetHead(), dropRoot, SHGDN_FORPARSING);
			item->icon = item->additionalIcon;
		}
		else
		{
			item->additionalIcon2 = PIDLToString(dropFiles.GetHead(), dropRoot, SHGDN_FORPARSING);
			item->icon = item->additionalIcon2;
		}
	}
	else
	{
		item->icon = PIDLToString(dropFiles.GetHead(), dropRoot, SHGDN_FORPARSING);
	}
	item->iconIndex = 0;
	if(item->LoadImage()) // anyway, try to check
	{
		item->image->LayerDraw();
		item->image->LayerUpdate();
		if(item->reflection->IsWindowVisible())
		{
			item->reflection->LayerDraw();
			item->reflection->LayerUpdate();
		}
	}
	if(editIcon && (editIcon->item == item))
	{
		editIcon->iconPath = item->icon;

		CString ext = editIcon->iconPath.Mid(editIcon->iconPath.ReverseFind(L'.')).MakeLower();
		if((ext == L".ico") || (ext == L".icon"))
		{
			Icons::GetIcon(editIcon->iconPath, editIcon->icon);
		}
		else
		{
			editIcon->icon->Load(editIcon->iconPath);
		}
		if(!editIcon->icon->Ready())
		{
			editIcon->iconPath.Empty();
			editIcon->icon->Load(appPath + L"Images\\icon-blank.png");
		}

		editIcon->Draw();
		editIcon->RedrawWindow();
	}
}

void CXWD::DoItemDropRunIn(CDockItem *item)
{
	CString params;
	POSITION p = dropFiles.GetHeadPosition();
	while(p)
	{
		params.AppendFormat(L"\"%s\" ", PIDLToString(dropFiles.GetAt(p), dropRoot, SHGDN_FORPARSING));
		dropFiles.GetNext(p);
	}
	item->Exec(params, false);
}

void CXWD::DoItemDropCopyTo(CDockItem *item, bool move)
{
	// prepearing source files/folders
	int srcSize = 2 * sizeof(wchar_t); // indicates the end of paths

	// calculating size
	POSITION p = dropFiles.GetHeadPosition();
	while(p)
	{
		CString path = PIDLToString(dropFiles.GetAt(p), dropRoot, SHGDN_FORPARSING);
		srcSize += (path.GetLength() + 1) * sizeof(wchar_t);
		dropFiles.GetNext(p);
	}

	wchar_t *src = (wchar_t*)malloc(srcSize);
	memset(src, 0, srcSize);
	wchar_t *n = src;

	// filling in data
	p = dropFiles.GetHeadPosition();
	while(p)
	{
		CString path = PIDLToString(dropFiles.GetAt(p), dropRoot, SHGDN_FORPARSING);
		memcpy(n, path.GetBuffer(), path.GetLength() * sizeof(wchar_t));
		n += path.GetLength() + 1;
		dropFiles.GetNext(p);
	}

	int dstSize = (item->path.GetLength() + 2) * sizeof(wchar_t);
	wchar_t *dst = (wchar_t*)malloc(dstSize);
	memset(dst, 0, dstSize);
	memcpy(dst, item->path.GetBuffer(), item->path.GetLength() * sizeof(wchar_t));

	SHFILEOPSTRUCT fop = {0};
	fop.wFunc = (move ? FO_MOVE : FO_COPY); 
	fop.pFrom = src;
	fop.pTo = dst;

	SHFileOperation(&fop);

	free(src);
	free(dst);
}

void CXWD::DoItemDropMoveToRecyclerBin()
{
	// prepearing source files/folders
	int srcSize = 2 * sizeof(wchar_t); // indicates the end of paths

	// calculating size
	POSITION p = dropFiles.GetHeadPosition();
	while(p)
	{
		CString path = PIDLToString(dropFiles.GetAt(p), dropRoot, SHGDN_FORPARSING);
		srcSize += (path.GetLength() + 1) * sizeof(wchar_t);
		dropFiles.GetNext(p);
	}

	wchar_t *src = (wchar_t*)malloc(srcSize);
	memset(src, 0, srcSize);
	wchar_t *n = src;

	// filling in data
	p = dropFiles.GetHeadPosition();
	while(p)
	{
		CString path = PIDLToString(dropFiles.GetAt(p), dropRoot, SHGDN_FORPARSING);
		memcpy(n, path.GetBuffer(), path.GetLength() * sizeof(wchar_t));
		n += path.GetLength() + 1;
		dropFiles.GetNext(p);
	}

	SHFILEOPSTRUCT fop = {0};
	fop.wFunc = FO_DELETE;
	fop.pFrom = src;
	fop.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION;

	SHFileOperation(&fop);
	free(src);
}

CSize CXWD::ItemSize(CDockItem *item, unsigned int position)
{
	switch(item->type)
	{
	case DockItemTypeRunningApp:
	case DockItemTypeIcon:
	case DockItemTypeDocklet:
	case DockItemTypeODDocklet:
	case DockItemTypeUpdate:
	case DockItemTypePublicPlugin:
		{
			return CSize(iconSize, iconSize);
		}
		break;
	}
	if(item->type == DockItemTypeSeparator)
	{
		if(dockMode == DockMode2D)
		{
			if((position == DockPositionLeft) || (position == DockPositionRight))
			{
				CSize size(dockHeight, skin->separator2d->Width());
				size.cy = size.cy * iconSize / skin->separator2d->Height();
				return size;
			}
			if((position == DockPositionTop) || (position == DockPositionBottom))
			{
				CSize size(skin->separator2d->Width(), dockHeight);
				size.cx = size.cx * iconSize / skin->separator2d->Height();
				return size;
			}
		}
		if(dockMode == DockMode3D)
		{
			CSize size(skin->separator3d->Width(), dockHeight - (int)dockHeightOffset3d - (int)dockHeightOffset3d % 2 - skin->bckgEdge3d->Height());
			size.cx = size.cx * size.cy / skin->separator3d->Height();
			return size;
		}
	}
	return CSize(0, 0);
}

void CXWD::DockRectOffset(CRect &rect)
{
	CRect dockRect;
	GetWindowRect(&dockRect);
	rect.OffsetRect(dockRect.left, dockRect.top);
}

void CXWD::DockRectOffset(CPoint &point)
{
	CRect dockRect;
	GetWindowRect(&dockRect);
	point.Offset(dockRect.left, dockRect.top);
}

void CXWD::UpdateReserveScreen()
{
	if(reserveScreenOk && !reserveScreen)
	{
		SHAppBarMessage(ABM_REMOVE, &barData);
		reserveScreenOk = false;
	}
	if(reserveScreen)
	{
		CRect r = DockRect(dockPosition);
		if(reserveScreenOk && (r == barData.rc))
		{
			return;
		}
		barData.cbSize = sizeof(APPBARDATA);
		barData.hWnd = CFrameWnd::m_hWnd;
		barData.uCallbackMessage = 0;
		barData.rc = r;
		barData.lParam = 0;
		switch(dockPosition)
		{
		case DockPositionLeft:
			{
				barData.uEdge = ABE_LEFT;
			}
			break;

		case DockPositionRight:
			{
				barData.uEdge = ABE_RIGHT;
			}
			break;

		case DockPositionTop:
			{
				barData.uEdge = ABE_TOP;
			}
			break;

		case DockPositionBottom:
			{
				barData.uEdge = ABE_BOTTOM;
				if(dockMode == DockMode3D)
				{
					barData.rc.top += 10;
				}
			}
			break;
		}
		if(!reserveScreenOk)
		{
			reserveScreenOk = (SHAppBarMessage(ABM_NEW, &barData) == TRUE);
		}
		if(reserveScreenOk)
		{
			SHAppBarMessage(ABM_SETPOS, &barData);
		}
	}
}

CRect CXWD::ItemRect(CDockItem *item, unsigned int position, bool onScreen, bool offset)
{
	CRect rect(0, 0, 0, 0);
	if(skin)
	{
		CSize size = ItemSize(item, position);
		switch(dockMode)
		{
		case DockMode2D:
			{
				switch(position)
				{
				case DockPositionLeft:
					{	
						int x = 0;
						POSITION p = items->GetHeadPosition();
						while(p)
						{
							CDockItem *pItem = items->GetAt(p);
							if(item == pItem)
							{
								break;
							}
							x += (int)((skin->skipBetweenIcons2d + ItemSize(pItem, position).cy) * pItem->resizeFlag);
							items->GetNext(p);
						}
						rect.top = skin->skipLeft2d + x;
						rect.bottom = rect.top + size.cy;
						if(onScreen)
						{
							rect.left = 0;
							rect.right = dockHeight;
						}
						else
						{
							rect.left = item->type == DockItemTypeSeparator ? 0 : skin->skipBottom2d;
							rect.right = rect.left + size.cx;
						}
					}
					break;

				case DockPositionTop:
					{
						int x = 0;
						POSITION p = items->GetHeadPosition();
						while(p)
						{
							CDockItem *pItem = items->GetAt(p);
							if(item == pItem)
							{
								break;
							}
							x += (int)((skin->skipBetweenIcons2d + ItemSize(pItem, position).cx) * pItem->resizeFlag);
							items->GetNext(p);
						}
						rect.left = skin->skipLeft2d + x;
						rect.right = rect.left + size.cx;
						if(onScreen)
						{
							rect.top = 0;
							rect.bottom = dockHeight;
						}
						else
						{
							rect.top = item->type == DockItemTypeSeparator ? 0 : skin->skipBottom2d;
							rect.bottom = rect.top + size.cy;
						}
					}
					break;

				case DockPositionRight:
					{
						int x = 0;
						POSITION p = items->GetHeadPosition();
						while(p)
						{
							CDockItem *pItem = items->GetAt(p);
							if(item == pItem)
							{
								break;
							}
							x += (int)((skin->skipBetweenIcons2d + ItemSize(pItem, position).cy) * pItem->resizeFlag);
							items->GetNext(p);
						}
						rect.top = skin->skipRight2d + x;
						rect.bottom = rect.top + size.cy;
						if(onScreen)
						{
							rect.left = 0;
							rect.right = dockHeight;
						}
						else
						{
							rect.left = item->type == DockItemTypeSeparator ? 0 : skin->skipTop2d;
							rect.right = rect.left + size.cx;
						}
					}
					break;

				//case DockPositionBottom:
				default:
					{
						int x = 0;
						POSITION p = items->GetHeadPosition();
						while(p)
						{
							CDockItem *pItem = items->GetAt(p);
							if(item == pItem)
							{
								break;
							}
							x += (int)((skin->skipBetweenIcons2d + ItemSize(pItem, position).cx) * pItem->resizeFlag);
							items->GetNext(p);
						}
						rect.left = skin->skipLeft2d + x;
						rect.right = rect.left + size.cx;
						if(onScreen)
						{
							rect.top = 0;
							rect.bottom = dockHeight;
						}
						else
						{
							rect.top = item->type == DockItemTypeSeparator ? 0 : skin->skipTop2d;
							rect.bottom = rect.top + size.cy;
						}
					}
					break;
				}
			}
			break;

		case DockMode3D:
			{
				int x = 0;
				POSITION p = items->GetHeadPosition();
				while(p)
				{
					CDockItem *pItem = items->GetAt(p);
					if(item == pItem)
					{
						break;
					}
					x += (int)((skin->iconSizeBetween3d + ItemSize(pItem, position).cx) * pItem->resizeFlag);
					items->GetNext(p);
				}
				//rect.left = (int)dockHeightOffset3d + x;
				rect.left = (int)((dockHeight - skin->bckgEdge3d->Height()) * sin(PI * skin->bckgEdgeAngle3d / 180)) / 2 + 
					skin->bckgEdgeOffset3d + x;
				rect.right = rect.left + size.cx;
				switch(item->type)
				{
				case DockItemTypeDocklet:
				case DockItemTypeODDocklet:
				case DockItemTypeRunningApp:
				case DockItemTypeIcon:
				case DockItemTypeUpdate:
				case DockItemTypePublicPlugin:
					{
						rect.top = dockHeight - size.cy - iconSize * skin->iconPosition3d / 100;
					}
					break;

				//case DockItemTypeSeparator:
				default:
					{
						rect.top = dockHeight - size.cy;
					}
					break;
				}
				rect.top -= skin->bckgEdge3d->Height();
				if(onScreen)
				{
					rect.bottom = dockHeight;
				}
				else
				{
					rect.bottom = rect.top + size.cy;
				}
			}
			break;
		}
	}
	if (offset)
	{
		DockRectOffset(rect);
	}
	return rect;
}

CDockItem* CXWD::ItemAt(CPoint point, bool onScreen)
{
	if(!skin)
	{
		return NULL;
	}
	POSITION p = items->GetHeadPosition();
	while(p)
	{
		CDockItem *item = items->GetAt(p);
		if(ItemRect(item, dockPosition, onScreen).PtInRect(point))
		{
			return item;
		}
		items->GetNext(p);
	}
	return NULL;
}
/*
CRect CXWD::ItemIndicatorRect(CDockItem*, CRect itemRect)
{
	CRect rect(0, 0, 0, 0);
	if(skin)
	{
		switch(dockMode)
		{
		case DockMode2D:
			{
				switch(dockPosition)
				{
				case DockPositionLeft:
					{	
						float k = (float)itemRect.Height() / skin->indicator2d->Height();
						if(k > 1)
						{
							k = 1;
						}
						rect.left = skin->indicatorSkipBottom2d;
						rect.right = rect.left + (int)(skin->indicator2d->Height() * k);
						rect.top = itemRect.top + (itemRect.Height() - (int)(skin->indicator2d->Width() * k)) / 2;
						rect.bottom = rect.top + (int)(skin->indicator2d->Width() * k);

						CPoint pt;
						pt.x = rect.left;
						DockRectOffset(pt);
						rect.MoveToX(pt.x);
					}
					break;

				case DockPositionTop:
					{
						float k = (float)itemRect.Width() / skin->indicator2d->Width();
						if(k > 1)
						{
							k = 1;
						}
						rect.left = itemRect.left + (itemRect.Width() - (int)(skin->indicator2d->Width() * k)) / 2;
						rect.right = rect.left + (int)(skin->indicator2d->Width() * k);
						rect.top = skin->indicatorSkipBottom2d;
						rect.bottom = rect.top + (int)(skin->indicator2d->Height() * k);

						CPoint pt;
						pt.y = rect.top;
						DockRectOffset(pt);
						rect.MoveToY(pt.y);
					}
					break;

				case DockPositionRight:
					{
						float k = (float)itemRect.Height() / skin->indicator2d->Height();
						if(k > 1)
						{
							k = 1;
						}
						rect.left = dockHeight - (int)(skin->indicator2d->Height() * k) - skin->indicatorSkipBottom2d;
						rect.right = rect.left + (int)(skin->indicator2d->Height() * k);
						rect.top = itemRect.top + (itemRect.Height() - (int)(skin->indicator2d->Width() * k)) / 2;
						rect.bottom = rect.top + (int)(skin->indicator2d->Width() * k);

						CPoint pt;
						pt.x = rect.left;
						DockRectOffset(pt);
						rect.MoveToX(pt.x);
					}
					break;

				//case DockPositionBottom:
				default:
					{
						float k = (float)itemRect.Width() / skin->indicator2d->Width();
						if(k > 1)
						{
							k = 1;
						}
						rect.left = itemRect.left + (itemRect.Width() - (int)(skin->indicator2d->Width() * k)) / 2;
						rect.right = rect.left + (int)(skin->indicator2d->Width() * k);
						rect.top = dockHeight - (int)(skin->indicator2d->Height() * k) - skin->indicatorSkipBottom2d;
						rect.bottom = rect.top + (int)(skin->indicator2d->Height() * k);

						CPoint pt;
						pt.y = rect.top;
						DockRectOffset(pt);
						rect.MoveToY(pt.y);
					}
					break;
				}
			}
			break;

		case DockMode3D:
			{
				float k = (float)itemRect.Width() / skin->indicator3d->Width();
				if(k > 1)
				{
					k = 1;
				}
				rect.left = itemRect.left + (itemRect.Width() - (int)(skin->indicator3d->Width() * k)) / 2;
				rect.right = rect.left + (int)(skin->indicator3d->Width() * k);
				rect.top = dockHeight - (int)(skin->indicator3d->Height() * k) - skin->indicatorSkipBottom3d;
				rect.bottom = rect.top + (int)(skin->indicator3d->Height() * k);

				CPoint pt;
				pt.y = rect.top;
				DockRectOffset(pt);
				rect.MoveToY(pt.y);
			}
			break;
		}
	}
	return rect;
}*/

CRect CXWD::ItemIndicatorRect(CDockItem *item)
{
	CRect rect(0, 0, 0, 0);
	CSize size = ItemSize(item, dockPosition);
	if(skin)
	{
		switch(dockMode)
		{
		case DockMode2D:
			{
				switch(dockPosition)
				{
				case DockPositionLeft:
					{	
						int x = 0;
						POSITION p = items->GetHeadPosition();
						while(p)
						{
							CDockItem *pItem = items->GetAt(p);
							if(item == pItem)
							{
								break;
							}
							x += (int)((skin->skipBetweenIcons2d + ItemSize(pItem, dockPosition).cy) * pItem->resizeFlag);
							items->GetNext(p);
						}
						float k = (float)size.cy / skin->indicator2d->Height();
						if(k > 1)
						{
							k = 1;
						}
						rect.left = skin->indicatorSkipBottom2d;
						rect.right = rect.left + (int)(skin->indicator2d->Height() * k);
						rect.top = skin->skipLeft2d + x + (size.cy - (int)(skin->indicator2d->Width() * k)) / 2;
						rect.bottom = rect.top + (int)(skin->indicator2d->Width() * k);
					}
					break;

				case DockPositionTop:
					{
						int x = 0;
						POSITION p = items->GetHeadPosition();
						while(p)
						{
							CDockItem *pItem = items->GetAt(p);
							if(item == pItem)
							{
								break;
							}
							x += (int)((skin->skipBetweenIcons2d + ItemSize(pItem, dockPosition).cx) * pItem->resizeFlag);
							items->GetNext(p);
						}
						float k = (float)size.cx / skin->indicator2d->Width();
						if(k > 1)
						{
							k = 1;
						}
						rect.left = skin->skipLeft2d + x + (size.cx - (int)(skin->indicator2d->Width() * k)) / 2;
						rect.right = rect.left + (int)(skin->indicator2d->Width() * k);
						rect.top = skin->indicatorSkipBottom2d;
						rect.bottom = rect.top + (int)(skin->indicator2d->Height() * k);
					}
					break;

				case DockPositionRight:
					{
						int x = 0;
						POSITION p = items->GetHeadPosition();
						while(p)
						{
							CDockItem *pItem = items->GetAt(p);
							if(item == pItem)
							{
								break;
							}
							x += (int)((skin->skipBetweenIcons2d + ItemSize(pItem, dockPosition).cy) * pItem->resizeFlag);
							items->GetNext(p);
						}
						float k = (float)size.cy / skin->indicator2d->Height();
						if(k > 1)
						{
							k = 1;
						}
						rect.left = dockHeight - (int)(skin->indicator2d->Height() * k) - skin->indicatorSkipBottom2d;
						rect.right = rect.left + (int)(skin->indicator2d->Height() * k);
						rect.top = skin->skipRight2d + x + (size.cy - (int)(skin->indicator2d->Width() * k)) / 2;
						rect.bottom = rect.top + (int)(skin->indicator2d->Width() * k);
					}
					break;

				//case DockPositionBottom:
				default:
					{
						int x = 0;
						POSITION p = items->GetHeadPosition();
						while(p)
						{
							CDockItem *pItem = items->GetAt(p);
							if(item == pItem)
							{
								break;
							}
							x += (int)((skin->skipBetweenIcons2d + ItemSize(pItem, dockPosition).cx) * pItem->resizeFlag);
							items->GetNext(p);
						}
						float k = (float)size.cx / skin->indicator2d->Width();
						if(k > 1)
						{
							k = 1;
						}
						rect.left = skin->skipLeft2d + x + (size.cx - (int)(skin->indicator2d->Width() * k)) / 2;
						rect.right = rect.left + (int)(skin->indicator2d->Width() * k);
						rect.top = dockHeight - (int)(skin->indicator2d->Height() * k) - skin->indicatorSkipBottom2d;
						rect.bottom = rect.top + (int)(skin->indicator2d->Height() * k);
					}
					break;
				}
			}
			break;

		case DockMode3D:
			{
				int x = 0;
				POSITION p = items->GetHeadPosition();
				while(p)
				{
					CDockItem *pItem = items->GetAt(p);
					if(item == pItem)
					{
						break;
					}
					x += (int)((skin->iconSizeBetween3d + ItemSize(pItem, dockPosition).cx) * pItem->resizeFlag);
					items->GetNext(p);
				}
				float k = (float)size.cx / skin->indicator3d->Width();
				if(k > 1)
				{
					k = 1;
				}
				rect.left = (int)((dockHeight - skin->bckgEdge3d->Height()) * sin(PI * skin->bckgEdgeAngle3d / 180)) / 2 + 
					skin->bckgEdgeOffset3d + x + (size.cx - (int)(skin->indicator3d->Width() * k)) / 2;
				rect.right = rect.left + (int)(skin->indicator3d->Width() * k);
				rect.top = dockHeight - (int)(skin->indicator3d->Height() * k) - dockHeight * skin->indicatorSkipBottom3d / 100;
				rect.bottom = rect.top + (int)(skin->indicator3d->Height() * k);
			}
			break;
		}
	}
	DockRectOffset(rect);
	return rect;
}

DROPEFFECT CXWD::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD, CPoint point)
{
	DROPEFFECT dwEffect = DROPEFFECT_NONE;
	if(pDataObject->IsDataAvailable(CF_SHELLIDLIST) || pDataObject->IsDataAvailable(CF_HDROP) 
		|| pDataObject->IsDataAvailable(CF_PLUGINLIST))
	{
		dwEffect = DROPEFFECT_MOVE;
		FORMATETC fmtetc = {CF_SHELLIDLIST, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		HGLOBAL hMem = pDataObject->GetGlobalData(CF_SHELLIDLIST, &fmtetc);
		if(hMem)
		{
			LPIDA ida = (LPIDA)GlobalLock(hMem);
			if(!ida)
			{
				return TRUE;
			}

			if(ida->cidl == 0)
			{
				GlobalUnlock(hMem);
				return TRUE;
			}

			if(dropRoot)
			{
				ILFree(dropRoot);
			}
			POSITION p = dropFiles.GetHeadPosition();
			while(p)
			{
				ILFree(dropFiles.GetAt(p));
				dropFiles.GetNext(p);
			}
			dropFiles.RemoveAll();

			dropRoot = ILClone((LPITEMIDLIST)((UINT)ida + ida->aoffset[0]));
			for(UINT i = 0; i < ida->cidl; i++)
			{
				dropFiles.AddTail(ILClone((LPITEMIDLIST)((UINT)ida + ida->aoffset[i + 1])));
			}

			GlobalUnlock(hMem);

			dropIsPlugin = false;
			OnDragEnter(point, NULL);
		}

		fmtetc.cfFormat = CF_HDROP;
		fmtetc.ptd = NULL;
		fmtetc.dwAspect = DVASPECT_CONTENT;
		fmtetc.lindex = -1;
		fmtetc.tymed = TYMED_HGLOBAL;
		hMem = pDataObject->GetGlobalData(CF_HDROP, &fmtetc);
		if(hMem)
		{
			void *buff = GlobalLock(hMem);
			if(!buff)
			{
				return TRUE;
			}

			if(dropRoot)
			{
				ILFree(dropRoot);
			}
			POSITION p = dropFiles.GetHeadPosition();
			while(p)
			{
				ILFree(dropFiles.GetAt(p));
				dropFiles.GetNext(p);
			}
			dropFiles.RemoveAll();

			SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &dropRoot);

			DROPFILES *drop = (DROPFILES*)buff;
			void *pFile = (void*)((int)drop + drop->pFiles);
			for(;;)
			{
				if(drop->fWide)
				{
					if(wcslen((wchar_t*)pFile) == 0)
					{
						break;
					}
				}
				else
				{
					if(strlen((char*)pFile) == 0)
					{
						break;
					}
				}
				CString path;
				if(drop->fWide)
				{
					path = (wchar_t*)pFile;
					pFile = (void*)((UINT)pFile + (path.GetLength() + 1) * sizeof(wchar_t));
				}
				else
				{
					path = (char*)pFile;
					pFile = (void*)((UINT)pFile + path.GetLength() + 1);
				}
				dropFiles.AddTail(StringToPIDL(path));
			}

			GlobalUnlock(hMem);

			dropIsPlugin = false;
			OnDragEnter(point, NULL);
		}

		fmtetc.cfFormat = CF_PLUGINLIST;
		fmtetc.ptd = NULL;
		fmtetc.dwAspect = DVASPECT_CONTENT;
		fmtetc.lindex = -1;
		fmtetc.tymed = TYMED_HGLOBAL;
		hMem = pDataObject->GetGlobalData(CF_PLUGINLIST, &fmtetc);
		if(hMem)
		{
			PluginListDrop *data = (PluginListDrop*)GlobalLock(hMem);
			if(!data)
			{
				return TRUE;
			}
			void *plugin = data->plugin;
			GlobalUnlock(hMem);

			dropIsPlugin = true;
			OnDragEnter(point, plugin);
		}
	}
	if(userDropTargetHelper)
	{
		IDataObject *dataObject = pDataObject->GetIDataObject(FALSE);
		dropTargetHelper->DragEnter(pWnd->GetSafeHwnd(), dataObject, &point, dwEffect);
	}
	return dwEffect;
}
	
DROPEFFECT CXWD::OnDragOver(CWnd*, COleDataObject* pDataObject, DWORD, CPoint point)
{
	DROPEFFECT dwEffect = DROPEFFECT_NONE;
	if(pDataObject->IsDataAvailable(CF_SHELLIDLIST) || pDataObject->IsDataAvailable(CF_HDROP) 
		|| pDataObject->IsDataAvailable(CF_PLUGINLIST))
	{
		dwEffect = DROPEFFECT_MOVE;
		void *plugin = NULL;
		if(dropIsPlugin)
		{
			FORMATETC fmtetc = {CF_PLUGINLIST, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
			HGLOBAL hMem = pDataObject->GetGlobalData(CF_PLUGINLIST, &fmtetc);
			if(hMem)
			{
				PluginListDrop *data = (PluginListDrop*)GlobalLock(hMem);
				if(data)
				{
					plugin = data->plugin;
					GlobalUnlock(hMem);
				}
			}
		}
		OnDragOver(point, plugin);
	}
	if(userDropTargetHelper)
	{
		dropTargetHelper->DragOver(&point, dwEffect);
	}
	return dwEffect;
}

BOOL CXWD::OnDrop(CWnd*, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	dropEffect = DROPEFFECT_MOVE;
	if(userDropTargetHelper)
	{
		IDataObject *dataObject = pDataObject->GetIDataObject(FALSE);
		dropTargetHelper->Drop(dataObject, &point, dropEffect);
	}
	OnDrop();
	return TRUE;
}

void CXWD::OnDragLeave(CWnd*)
{
	if(userDropTargetHelper)
	{
		dropTargetHelper->DragLeave();
	}
	OnDragLeave();
}

void CXWD::OnDragEnter(CPoint, void *)
{
	BringWindowToTop();
	SetForegroundWindow();
	dropLastItemOver = NULL;
	if(!dropIsPlugin)
	{
		OnItemsDragBegin();
	}
}

void CXWD::OnDragOver(CPoint point, void *plugin)
{
	DockRectOffset(point);
	CDockItem *item = ItemAt(point, true);
	int count = items->GetCount();

	if(!dropItem && (dropIsPlugin || items->IsEmpty() || 
		((count == 1 ? dropItemOver != NULL : dropItemOver && dropLastItemOver) && 
		(dropItemOver != (count == 1 ? item : dropLastItemOver)))))
	{
		dropItem = new CDockItem();
		dropItem->SetBottomWindow(this);
		dropItem->poof = poof;
		dropItem->dockMode = dockMode;
		dropItem->dockPosition = dockPosition;
		if(skin)
		{
			dropItem->indicator->image->Assign((dockMode == DockMode2D) ? skin->indicator2d : skin->indicator3d);
		}
		if(skin && (dropItem->dockMode & DockMode3D))
		{
			dropItem->reflectionOffset = iconSize * skin->iconReflectionOffset3d / 100;
			dropItem->reflectionSkipTop = iconSize * skin->iconReflectionSkipTop3d / 100;
			dropItem->reflectionSkipBottom = iconSize * skin->iconReflectionSkipBottom3d / 100;
			dropItem->reflectionSize = max(0, dropItem->reflectionOffset + iconSize * skin->iconPosition3d / 100);
			dropItem->reflectionOpacity = (unsigned char)(255 * skin->iconReflectionOpacity3d / 100);
			dropItem->reflectionOpacityFactor = (unsigned char)(skin->iconReflectionOpacityFactor3d);
			dropItem->iconShadowEnabled = iconShadowEnabled;
		}
		else
		{
			dropItem->reflectionSize = 0;
		}
		if(dropIsPlugin)
		{
			POSITION p = items->GetHeadPosition();
			while(p)
			{
				CDockItem *item = items->GetAt(p);
				if((item->identificator & identificatorIndicator))
				{
					item->IndicatorShow(false);
				}
				items->GetNext(p);
			}

			// Docklet ?
			p = plugins->items.Find((CPlugin*)plugin);
			if(p)
			{
				CPlugin *plugin = plugins->items.GetAt(p);

				dropItem->type = DockItemTypeDocklet;
				dropItem->plugin = plugin;
				dropItem->path = dropItem->plugin->path;
				dropItem->pluginId = 0;
				dropItem->image->image->Load(appPath + L"Images\\icon-blank.png");
				dropItem->reflection->image->Assign(dropItem->image->image);

				for(;;)
				{
					bool found = false;
					POSITION p = items->GetHeadPosition();
					while(p)
					{
						CDockItem *pitem = items->GetAt(p);
						if((pitem->type == DockItemTypeDocklet) && (pitem->pluginId == dropItem->pluginId))
						{
							found = true;
							break;
						}
						items->GetNext(p);
					}
					if(!found)
					{
						break;
					}
					dropItem->pluginId++;
				}
			}

			// OD Docklet ?
			p = odDocklets->items.Find((CODDocklet*)plugin);
			if(p)
			{
				CODDocklet *docklet = odDocklets->items.GetAt(p);

				dropItem->type = DockItemTypeODDocklet;
				dropItem->docklet = docklet;
				dropItem->path = dropItem->docklet->path;
				dropItem->odDockletId = 0;
				dropItem->image->image->Load(appPath + L"Images\\icon-blank.png");
				dropItem->reflection->image->Assign(dropItem->image->image);

				for(;;)
				{
					bool found = false;
					POSITION p = items->GetHeadPosition();
					while(p)
					{
						CDockItem *pitem = items->GetAt(p);
						if((pitem->type == DockItemTypeODDocklet) && (pitem->odDockletId == dropItem->odDockletId))
						{
							found = true;
							break;
						}
						items->GetNext(p);
					}
					if(!found)
					{
						break;
					}
					dropItem->odDockletId++;
				}
			}
		}
		if(dropItemOver)
		{
			POSITION p = items->Find(dropItemOver);
			CRect rect = ItemRect(dropItemOver, dockPosition, true);

			switch(dockPosition)
			{
			case DockPositionLeft:
			case DockPositionRight:
				{
					if(point.y >= rect.top + rect.Height() / 2)
					{
						items->InsertAfter(p, dropItem);
					}
					else
					{
						items->InsertBefore(p, dropItem);
					}
				}
				break;

			// case DockPositionTop:
			// case DockPositionBottom:
			default:
				{
					if(point.x >= rect.left + rect.Width() / 2)
					{
						items->InsertAfter(p, dropItem);
					}
					else
					{
						items->InsertBefore(p, dropItem);
					}
				}
				break;
			}
		}
		else
		{
			if(point.x < dockWidth / 2)
			{
				items->AddHead(dropItem);
			}
			else
			{
				items->AddTail(dropItem);
			}
		}
		if(animationFlags & animationFlagDockResizeTimer)
		{
			dropItem->resizeStartAt = (DWORD)(GetTickCount() - dropItem->resizeFlag * DOCK_ANIMATON_RESIZE_DELAY);
		}
		else
		{
			dropItem->resizeStartAt = GetTickCount();
		}
		StopBouncing();
		animationFlags |= animationFlagDockResizeUp;
		animationFlags |= animationFlagDockResizeTimer;
		SetTimer(timerDockResize, 10, NULL);
	}

	if(item)
	{
		dropLastItemOver = dropItemOver;
		if(item == dropItem)
		{
			if(dropItemOver)
			{
				if(!dropIsPlugin)
				{
					OnItemsDragLeave(dropItemOver);
				}
				dropItemOver = NULL;
			}
			return;
		}
		if(item != dropItemOver)
		{
			if(dropItemOver && !dropIsPlugin)
			{
				OnItemsDragLeave(dropItemOver);
			}
			dropItemOver = item;
			if(!dropIsPlugin)
			{
				OnItemsDragEnter(dropItemOver);
			}
		}
		if(dropItem)
		{
			POSITION p1 = items->Find(dropItem);
			POSITION p2 = items->Find(dropItemOver);
			
			bool goingFromLeft = false;
			POSITION p = items->GetHeadPosition();
			while(p && (p != p2))
			{
				if(p == p1)
				{
					goingFromLeft = true;
					break;
				}
				items->GetNext(p);
			}

			bool move = false;
			if(goingFromLeft)
			{
				p = p1;
				items->GetNext(p);
				move = p && (p != p2);
			}
			else
			{
				p = p1;
				items->GetPrev(p);
				move = p && (p != p2);
			}

			if(move)
			{
				if(goingFromLeft)
				{
					p = p2;
					items->RemoveAt(p1);
					items->InsertBefore(p, dropItem);
				}
				else
				{
					p = p2;
					items->RemoveAt(p1);
					items->InsertAfter(p, dropItem);
				}
				p = items->GetHeadPosition();
				while(p)
				{
					CDockItem *item = items->GetAt(p);
					CRect rect = ItemRect(item, dockPosition);
					item->Move(rect.left, rect.top, true);
					items->GetNext(p);
				}
			}

			/*CRect rect = ItemRect(dropItemOver, dockPosition, true);

			items->RemoveAt(p1);

			switch(dockPosition)
			{
			case DockPositionLeft:
			case DockPositionRight:
				{
					if(point.y >= rect.top + rect.Height() / 2)
					{
						items->InsertAfter(p2, dropItem);
					}
					else
					{
						items->InsertBefore(p2, dropItem);
					}
				}
				break;

			// case DockPositionTop:
			// case DockPositionBottom:
			default:
				{
					if(point.x >= rect.left + rect.Width() / 2)
					{
						items->InsertAfter(p2, dropItem);
					}
					else
					{
						items->InsertBefore(p2, dropItem);
					}
				}
				break;
			}

			POSITION p = items->GetHeadPosition();
			while(p)
			{
				CDockItem *item = items->GetAt(p);
				CRect rect = ItemRect(item, dockPosition);
				item->Move(rect.left, rect.top, true);
				items->GetNext(p);
			}*/
		}
	}
}

bool CXWD::OnDrop()
{
	// to complete animation
	if(dropItem)
	{
		dropItem->resizeFlag = 1;
		animationFlags &= ~animationFlagDockResizeTimer;
		KillTimer(timerDockResize);

		if(dropIsPlugin)
		{
			CDockItem *item = dropItem;

			CRect rect = ItemRect(item, dockPosition);
			item->Size(rect.Width(), rect.Height());

			dropItem = NULL;
			dropItemOver = NULL;
			DockAdjustSize(true);

			switch(item->type)
			{
			case DockItemTypeODDocklet:
				{
					item->docklet->OnCreate(item->image->m_hWnd, item->docklet->hModule, NULL, NULL);
				}
				break;

			case DockItemTypeDocklet:
				{
					item->pluginData = item->plugin->PluginInitialize((XWDId)item);
					item->plugin->PluginEvent((XWDId)item, item->pluginData, XWDEventCreate);
				}
				break;
			}

			item->TopMost(dockIsTopMost);
			AddItem(item, false);

			POSITION p = items->GetHeadPosition();
			while(p)
			{
				CDockItem *item = items->GetAt(p);
				if((item->identificator & identificatorIndicator))
				{
					CRect rect = ItemIndicatorRect(item);
					item->indicator->SetWindowPos(&wndTop, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
					item->IndicatorShow();
				}
				items->GetNext(p);
			}

			return true;
		}
	}

	if(dropItemOver && (dropItemOver != dropItem))
	{
		if(!OnItemsDrop(dropItemOver))
		{
			OnDragLeave();
			return false;
		}
	}

	if(!dropItem)
	{
		return true;
	}
	
	CDockItem *item = dropItem;
	POSITION pItem = items->Find(item);

	dropItem = NULL;
	dropItemOver = NULL;

	POSITION p = dropFiles.GetHeadPosition();
	while(p)
	{
		if(p != dropFiles.GetHeadPosition())
		{
			item = new CDockItem();
			item->dockMode = dockMode;
			item->dockPosition = dockPosition;
			if(skin && (item->dockMode & DockMode3D))
			{
				item->reflectionOffset = iconSize * skin->iconReflectionOffset3d / 100;
				item->reflectionSkipTop = iconSize * skin->iconReflectionSkipTop3d / 100;
				item->reflectionSkipBottom = iconSize * skin->iconReflectionSkipBottom3d / 100;
				item->reflectionSize = max(0, item->reflectionOffset + iconSize * skin->iconPosition3d / 100);
				item->reflectionOpacity = (unsigned char)(255 * skin->iconReflectionOpacity3d / 100);
				item->reflectionOpacityFactor = (unsigned char)(skin->iconReflectionOpacityFactor3d);
				item->iconShadowEnabled = iconShadowEnabled;
			}
			else
			{
				item->reflectionSize = 0;
			}
			pItem = items->InsertAfter(pItem, item);
		}

		LPITEMIDLIST pidl = dropFiles.GetAt(p);
		LinkInfo linkInfo;

		CString path = PIDLToString(pidl, dropRoot, SHGDN_FORPARSING);

		/*if(GetLinkInfo(path, &linkInfo, LI_DESCRIPTION | LI_PATH | LI_ARGUMENTS | LI_WORKDIRECTORY | LI_ICONLOCATION))
		{
			item->path = linkInfo.path;
			item->arguments = linkInfo.arguments;
			item->workDirectory = linkInfo.workDirectory;
			item->icon = linkInfo.iconLocation;
			item->iconIndex = linkInfo.iconIndex;
		}
		else
		{*/
			item->path = path;
			item->workDirectory = path.Mid(0, path.ReverseFind(L'\\') + 1);
		//}

		SHFILEINFO sfi;
		SHGetFileInfo((LPCWSTR)pidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_PIDL);
		item->text = sfi.szDisplayName;
		if(item->text.IsEmpty())
		{
			item->text = item->path.Mid(item->path.ReverseFind(L'\\') + 1);
			if(item->text.IsEmpty())
			{
				item->text = path;
			}
		}

		if(!item->LoadImage())
		{
			item->image->image->Load(appPath + L"Images\\icon-blank.png");
			if(item->type != DockItemTypeSeparator)
			{
				item->reflection->image->Assign(item->image->image);
			}
		}

		// may we can work with it ?
		DWORD mode = SetErrorMode(SEM_FAILCRITICALERRORS);
		DWORD attr = GetFileAttributes(item->path.GetBuffer());
		SetErrorMode(mode);
		if((attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY))
		{
			item->identificator |= identificatorDirectory;
		}

		if(item->path.Mid(item->path.ReverseFind(L'.')).MakeLower() == L".exe")
		{
			item->identificator |= identificatorApplication;
			if(skin)
			{
				item->indicator->image->Assign((dockMode == DockMode2D) ? skin->indicator2d : skin->indicator3d);
			}
			if(IsApplicationRunning(item->path))
			{
				item->identificator |= identificatorIndicator;
				CRect rect = ItemIndicatorRect(item);
				item->indicator->SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
				item->IndicatorShow();
			}
		}

		if(IsRecycleBin(item->path))
		{
			item->identificator |= identificatorRecycleBin;
		}

		if(IsMyComputer(item->path))
		{
			CRect rect = ItemIndicatorRect(item);
			item->identificator |= identificatorIndicator;
			item->indicator->image->Assign((dockMode == DockMode2D) ? skin->indicator2d : skin->indicator3d);
			item->indicator->SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
			item->IndicatorShow();
		}

		CRect rect = ItemRect(item, dockPosition);
		item->Size(rect.Width(), rect.Height());
		item->SetBottomWindow(this);
		item->poof = poof;
		item->TopMost(dockIsTopMost);

		AddItem(item, p != dropFiles.GetHeadPosition());

		dropFiles.GetNext(p);
	}
	return true;
}

void CXWD::OnDragLeave()
{
	if(dropItemOver)
	{
		if(!dropIsPlugin)
		{
			OnItemsDragLeave(dropItemOver);
		}
		dropItemOver = NULL;
	}
	if(dropIsPlugin)
	{
		POSITION p = items->GetHeadPosition();
		while(p)
		{
			CDockItem *item = items->GetAt(p);
			if((item->identificator & identificatorIndicator))
			{
				item->IndicatorShow();
			}
			items->GetNext(p);
		}
	}
	if(dropItem)
	{
		if(animationFlags & animationFlagDockResizeTimer)
		{
			dropItem->resizeStartAt = (DWORD)(GetTickCount() - (1 - dropItem->resizeFlag) * DOCK_ANIMATON_RESIZE_DELAY);
		}
		else
		{
			dropItem->resizeStartAt = GetTickCount();
		}
		StopBouncing();
		animationFlags |= animationFlagDockResizeTimer;
		animationFlags &= ~animationFlagDockResizeUp;
		SetTimer(timerDockResize, 10, NULL);
	}
}

void CXWD::RemoveItem(CDockItem *item)
{
	DWORD startAt = GetTickCount();
	item->IndicatorShow(false);
	item->image->LayerDraw(item->addTmp);
	item->addTmp->ReflectVertical();
	for(;;)
	{
		item->resizeFlag = 1 - (float)(GetTickCount() - startAt) / DOCK_ANIMATON_ITEM_ADD;
		if(item->resizeFlag <= 0)
		{
			POSITION p = items->Find(item);
			if(p)
			{
				items->RemoveAt(p);
			}
			item->Delete();
			DockAdjustSize();
			MakeBlurBehind();
			break;
		}
		item->AddDraw(item->resizeFlag);
		DockAdjustSize();
	}
}

void CXWD::AddItem(CDockItem *item, bool resizeDock)
{
	StopBouncing();

	item->image->LayerDraw(item->addTmp);
	item->addTmp->ReflectVertical();
	item->Show();
	if(resizeDock)
	{
		DWORD startAt = GetTickCount();
		for(;;)
		{
			item->resizeFlag = (float)(GetTickCount() - startAt) / DOCK_ANIMATON_ITEM_ADD;
			if(item->resizeFlag >= 1)
			{
				item->resizeFlag = 1;
			}
			item->AddDraw(item->resizeFlag);
			DockAdjustSize();
			if(item->resizeFlag == 1)
			{
				MakeBlurBehind();
				break;
			}
		}
	}
	else
	{
		DWORD startAt = GetTickCount();
		for(;;)
		{
			float k = (float)(GetTickCount() - startAt) / DOCK_ANIMATON_ITEM_ADD;
			if(k >= 1)
			{
				k = 1;
			}
			item->AddDraw(k);
			if(k == 1)
			{
				break;
			}
		}
	}
}

void CXWD::StopBouncing()
{
	POSITION p = items->GetHeadPosition();
	while(p)
	{
		items->GetAt(p)->BounceStop();
		items->GetNext(p);
	}
}

void CXWD::RemoveRunningItem(CString path)
{
	POSITION p = items->GetHeadPosition();
	while(p)
	{
		CDockItem *item = (CDockItem*)items->GetAt(p);
		if(item->path.CompareNoCase(path) == 0)
		{
			if(item->type == DockItemTypeRunningApp)
			{
				RemoveItem(item);
				break;
			}
		}
		items->GetNext(p);
	}
}

void CXWD::AddRunningItem(CString path)
{
	CString tmp = CString(path).MakeLower();
	if ((tmp.Find(L"explorer.exe") >= 0) || (tmp.Find(L"xwd.exe") >= 0) || (tmp.Find(L"svchost.exe") >= 0))
	{
		return;
	}
	
	POSITION p = items->GetHeadPosition();
	while(p)
	{
		CDockItem *item = items->GetAt(p);
		if(path.CompareNoCase(item->path) == 0)
		{
			return;
		}
		items->GetNext(p);
	}

	p = items->GetHeadPosition();
	while(p)
	{
		if (items->GetAt(p)->type == DockItemTypeSeparator)
		{
			break;
		}
		items->GetNext(p);
	}

	CDockItem *item = new CDockItem();
	if(p)
	{
		items->InsertBefore(p, item);
	}
	else
	{
		items->AddTail(item);
	}
	item->type = DockItemTypeRunningApp;
	item->dockMode = dockMode;
	item->dockPosition = dockPosition;
	if(skin && (item->dockMode & DockMode3D))
	{
		item->reflectionOffset = iconSize * skin->iconReflectionOffset3d / 100;
		item->reflectionSkipTop = iconSize * skin->iconReflectionSkipTop3d / 100;
		item->reflectionSkipBottom = iconSize * skin->iconReflectionSkipBottom3d / 100;
		item->reflectionSize = max(0, item->reflectionOffset + iconSize * skin->iconPosition3d / 100);
		item->reflectionOpacity = (unsigned char)(255 * skin->iconReflectionOpacity3d / 100);
		item->reflectionOpacityFactor = (unsigned char)(skin->iconReflectionOpacityFactor3d);
		item->iconShadowEnabled = iconShadowEnabled;
	}
	else
	{
		item->reflectionSize = 0;
	}

	item->path = path;
	item->workDirectory = item->path.Mid(0, item->path.ReverseFind(L'\\') + 1);
	ShellIO::GetFileVersionInfo(item->path, &item->text);
	if (item->text.Trim().IsEmpty())
	{
		HWND hWnd = GetApplicationMainWindow(item->path);
		if (hWnd)
		{
			wchar_t buff[MAX_PATH];
			::GetWindowText(hWnd, buff, MAX_PATH);
			item->text = buff;
		}
	}

	if(!item->LoadImage())
	{
		item->image->image->Load(appPath + L"Images\\icon-blank.png");
		item->reflection->image->Assign(item->image->image);
	}

	item->identificator |= identificatorApplication;
	if(skin)
	{
		item->indicator->image->Assign((dockMode == DockMode2D) ? skin->indicator2d : skin->indicator3d);
	}

	item->identificator |= identificatorIndicator;
	CRect rect = ItemIndicatorRect(item);
	item->indicator->SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
	item->IndicatorShow();

	rect = ItemRect(item, dockPosition);
	item->Size(rect.Width(), rect.Height());
	item->SetBottomWindow(this);
	item->poof = poof;
	item->TopMost(dockIsTopMost);

	AddItem(item);
	Core::DetachAddNotifer(item->path);
}