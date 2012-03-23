#ifndef XWDPAPI_H
#define XWDPAPI_H

#include "XWDAPI.h"

struct XWDPRect
{
	XWDId left;
	XWDId top;
	XWDId width;
	XWDId height;
};
typedef struct XWDPRect XWDPRect;

struct XWDPMenu
{
	XWDId id;
	XWDId parentId;
	XWDString text;
	XWDId isCheckbox;
	XWDId isChecked;
};
typedef struct XWDPMenu XWDPMenu;

static const unsigned int WM_XWDPUBLICAPICALL = RegisterWindowMessage(L"WM_XWDPUBLICAPICALL");

enum XWDPFunction
{
	XWDPPluginFunctionNone = 0,

	XWDPPluginRegister,
	/*
		Register new plugin in the dock

		XWDString - fileName of executable plugin file
		XWDId - hWnd of callback window,
		XWDString - default icon
		XWDString - title
		XWDId - non zero no bounce on click, otherwise bounce on click
	*/

	XWDPPluginRemove,
	/*
		Unregister the plugin from the dock

		XWDId - plugin's id
	*/

	XWDPPluginSetConfig,
	/*
		Setup configuration for the plugin's icon

		XWDId - plugin's id
		XWDId - keep the plugin in dock or not after dock restarting
		XWDId - activate window of the plugin on click
		XWDId - make window of the plugin exposable or not ONLY on click
		XWDId - bounce of the plugin's icon on click
	*/

	XWDPPluginGetConfig,
	/*
		Returns configuration of the plugin's icon

		XWDId - plugin's id
	*/

	XWDPPluginSetIcon,
	/*
		Set up new icon for the plugin

		XWDId - plugin's id
		XWDString - icon's name
	*/

	XWDPPluginGetIcon,
	/*
		Get icon's path

		XWDId - plugin's id
	*/

	XWDPPluginSetTitle,
	/*
		Set up new title for the plugin

		XWDId - plugin's id
		XWDString - title
	*/

	XWDPPluginGetTitle,
	/*
		Get title of the plugin

		XWDId - plugin's id
	*/

	XWDPPluginBounce,
	/*
		Bounce icon of the plugin

		XWDId - plugin's id
		XWDId - mode of bouncing
			0 - Normal
			1 - Attention
		XWDId - amount of bouncing, set 0 to repeat forever (only for Attention mode)
	*/

	XWDPPluginBounceStop,
	/*
		Stop bouncing icon of the plugin

		XWDId - plugin's id
	*/

	XWDPPluginSetIndicator,
	/*
		Setup indicator for the plugin

		XWDId - plugin's id
		XWDId - non zero value to enable, otherwise to disable
	*/

	XWDPPluginGetIndicator,
	/*
		Get indicator of the plugin

		XWDId - plugin's id
	*/

	XWDPPluginGetSettingsPath,
	/*
		Returns settings directory

		XWDId - plugin's id
	*/

	XWDPPluginGetUId,
	/*
		Returns uniq id of the plugin

		XWDId - plugin's id
	*/

	XWDPPluginSetNotification,
	/*
		XWDId - plugin's id
		XWDId - non zero to show, otherwise to hide
		XWDString - text
		XWDId - position
			0 - left/top
			1 - top/middle
			2 - right/top
			3 - right/middle
			4 - right/bottom
			5 - bottom/middle
			6 - left/bottom
			7 - left/middle
	*/

	XWDPPluginGetDockEdge,
	/*
		Returns dock's edge
			0 - left
			1 - top
			2 - right
			3 - bottom

		XWDId - plugin's id
	*/

	XWDPPluginGetIconRect,
	/*
		Returns icon's rect relative to the screen

		XWDId - plugin's id
	*/

	XWDPPluginAddFolderWatcher,
	/*
		Add a wathcer to the folder

		XWDId - plugin's id
		XWDId - folder's id
		XWDId - actions
		XWDString - folder
	*/

	XWDPPluginRemoveFolderWatcher,
	/*
		Remove a wathcer of the folder

		XWDId - plugin's id
		XWDId - folder's id
	*/

	XWDPPluginGetEvent,
	XWDPPluginGetMenu
};
typedef enum XWDPFunction XWDPFunction;

struct XWDPData
{
	unsigned int function;
	/* data of parameters */
};
typedef struct XWDPData XWDPData;

enum XWDPEvent
{
	XWDPEventLButtonClick,
	XWDPEventMenuSelect
};
typedef enum XWDPEvent XWDPEvent;

#endif /* XWDPAPI_H */