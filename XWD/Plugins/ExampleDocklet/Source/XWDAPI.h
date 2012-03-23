/*
	XWD Plugins SDK
*/

#ifndef XWDAPI_H
#define XWDAPI_H

#define XWDAPIVERSION 0x00000001 // 0.0.0.1

#define XWDAPICALL __stdcall

#define XWDChar wchar_t
#define XWDStringLength 256
#define XWDUInt8 unsigned char
#define XWDUInt16 unsigned short
#define XWDUInt32 unsigned int
#define XWDSInt8 signed char
#define XWDSInt16 signed short
#define XWDSInt32 signed int
#define XWDNull 0
#define XWDBool XWDUInt8
#define XWDFalse 0
#define XWDTrue 1
#define XWDVoid void
#define XWDId XWDUInt32

typedef XWDChar XWDString[XWDStringLength];

struct XWDPoint
{
	XWDUInt16 left;
	XWDUInt16 top;
};
typedef struct XWDPoint XWDPoint;

struct XWDRect
{
	XWDUInt16 left;
	XWDUInt16 top;
	XWDUInt16 width;
	XWDUInt16 height;
};
typedef struct XWDRect XWDRect;

union XWDPixel
{
	XWDUInt32 c; // whole color 32 bits / 4 bytes
	struct
	{
		XWDUInt8 b;
		XWDUInt8 g;
		XWDUInt8 r;
		XWDUInt8 a;
	};
};
typedef union XWDPixel XWDPixel;

enum XWDPluginType
{
	XWDPluginTypeNone = 0,
	XWDPluginDocklet
};
typedef enum XWDPluginType XWDPluginType;

enum XWDMenuFlags
{
	XWDMenuFlagsNone			= 0,
	XWDMenuFlagsLine			= 1 << 0, // create menu's item as line
	XWDMenuFlagsCheckbox		= 1 << 1, // create menu's item as checkbox
	XWDMenuFlagsChecked			= 1 << 2, // create menu's item as checked checkbox
};
typedef enum XWDMenuFlags XWDMenuFlags;

enum XWDFunction
{
	XWDFunctionNone = 0,

	// XWD API
	XWDGetRootPath,
	/*
		Return path of the dock

		XWDString - result
	*/

	// XWD Image API
	XWDImageCreate,
	/*
		Create new image's instance

		XWDId* - image's identificator
	*/
	
	XWDImageDelete,
	/*
		Delete image's instance from memory

		XWDId - image's identificator
	*/

	XWDImageFree,
	/*
		Delete all data of the image, but not delete its instance.

		XWDId - image's identificator
	*/

	XWDImageResize,
	/*
		Resize image

		XWDId - image's identificator
		XWDUInt16 - new width in pixels of the image
		XWDUInt16 - new height in pixels of the image
	*/

	XWDImageAssignImage,
	/*
		Copy from another image

		XWDId - image's identificator (destination)
		XWDId - image's identificator (source)
	*/

	XWDImageAssignGdiplusBitmap,
	/*
		Copy from Gdiplus::Bitmap

		XWDId - image's identificator
		Gdiplus::Bitmap* - Gdiplus Bitmap
	*/

	XWDImageAssignBits,
	/*
		Copy from memory (scan0/bits...)

		XWDId - image's identificator
		XWDUInt16 - width in pixels of the source data
		XWDUInt16 - height in pixels of the source data
		XWDVoid* - scan0 (bits) of the source data
	*/

	XWDImageAssignFile,
	/*
		Load from any files, icons, pictures and etc.

		XWDId - image's identificator
		XWDString - full file's path
	*/

	XWDImageFillFull,
	/*
		Fill all data of the image

		XWDId - image's identificator
	*/

	XWDImageFill,
	/*
		Fill part of image's data

		XWDId - image's identificator
		XWDUInt16 - left in pixels
		XWDUInt16 - top in pixels
		XWDUInt16 - width in pixels
		XWDUInt16 - height in pixels
	*/

	XWDImageReady,
	/*
		Check image's data, if something is wrong return XWDFalse

		XWDId - image's identificator
		XWDBool* - result
	*/

	XWDImageGetRect,
	/*
		Return image's rect

		XWDId - image's identificator
		XWDRect* - result
	*/

	XWDImageGetPixels,
	/*
		Return first pixel of image's data using left and top offsets
		IMPORTANT: the first row will be height - 1

		XWDId - image's identificator
		XWDUInt16 - left offset in pixels
		XWDUint16 - top offset in pixels
		XWDPixel** - result
	*/

	XWDImageGetScan0,
	/*
		Return image's data

		XWDId - image's identificator
		XWDPixel** - result
	*/

	XWDImageGetHDC,
	/*
		Return HDC of image's data

		XWDId - image's identificator
		HDC* - result
	*/

	XWDImageGetHBITMAP,
	/*
		Return HBITMAP of image's data

		XWDId - image's identificator
		HBITMAP* - result
	*/

	XWDImageGetGdiplusBitmap,
	/*
		Return Gdiplus::Bitmap of image's data

		XWDId - image's identificator
		Gdiplus::Bitmap** - result
	*/

	XWDImageRotate180,
	/*
		Rotate image's data on 180 degrees

		XWDId - image's identificator
	*/


	// XWD Docklet API
	XWDDockletGetRelativePath,
	/*
		Return relative path of the plugin

		XWDId - docklet's identificator
		XWDString - result
	*/

	XWDDockletGetPath,
	/*
		Return path of the plugin

		XWDId - docklet's identificator
		XWDString - result
	*/

	XWDDockletLockMouseEffect,
	/*
		Lock Mouse Effects

		XWDId - docklet's identificator 
		XWDBool - XWDTrue to lock mouse effects, XWDFalse to unlock it
	*/

	XWDDockletGetPluginId,
	/*
		Return plugin's id

		XWDId - docklet's identificator
		XWDUInt32* - result
	*/

	XWDDockletGetPanelHWND,
	/*
		Return HWND of the dock's panel

		HWND* - result
	*/

	XWDDockletGetIconHWND,
	/*
		Return HWND of the docklet

		XWDId - docklet's identificator
		HWND* - result
	*/

	XWDDockletGetRect,
	/*
		Return rect of the docklet's image

		XWDId - docklet's identificator
		XWDRect* - result
	*/

	XWDDockletGetImage,
	/*
		Return the copy of real docklet's image

		XWDId - docklet's identificator
		XWDId* - image's identificator
	*/

	XWDDockletSetImage,
	/*
		Setup new image for the docklet

		XWDId - docklet's identificator
		XWDId - image's identificator
	*/

	XWDDockletGetLabel,
	/*
		Return label of the docklet

		XWDId - docklet's identificator
		XWDString - result
	*/

	XWDDockletSetLabel,
	/*
		Setup new label for the docklet

		XWDId - docklet's identificator
		XWDString - new label, if NULL, label is empty
	*/

	XWDDockletGetLabelVisible,
	/*
		Check if label is shown for the docklet now

		XWDId - docklet's identificator
		XWDBool* - result
	*/

	XWDDockletGetDockMonitor,
	/*
		Return the monitor, where the dock

		XWDUInt8* - result. Number begining 0,1,2,3... use EnumDisplayMonitors to find the monitor
	*/

	XWDDockletGetDockEdge,
	/*
		Return current dock's edge

		XWDUInt8* - result.
			0 - left
			1 - top
			2 - right
			3 - bottom
	*/

	XWDDockletGetIndicator,
	/*
		Check if indicator is enabled for the docklet

		XWDId - docklet's identificator
		XWDBool* - result
	*/

	XWDDockletSetIndicator,
	/*
		Setup indicator for the docklet

		XWDId - docklet's identificator
		XWDBool - XWDTrue to enable, else to disable
	*/

	XWDDockletMenuAdd,
	/*
		Create new item in menu

		XWDId - docklet's identificator
		XWDId - parent menu, it also can be sub-menu. Use XWDNull to place item in popup menu in dock
		XWDId* - return uniqe identificator for menu.
		XWDString - text of the item
		XWDMenuFlags - flags (to setup item as line or checkbox and etc.)
	*/

	XWDDockletMenuRemove,
	/*
		Delete item from menu

		XWDId - docklet's identificator
		XWDId - item's identificator
	*/

	XWDDockletMenuGetText,
	/*
		Return menu item's text

		XWDId - docklet's identificator
		XWDId - item's identificator
		XWDString - result
	*/

	XWDDockletMenuSetText,
	/*
		Setup menu item's text

		XWDId - docklet's identificator
		XWDId - item's identificator
		XWDString - item's text. To empty the text, setup this parameters to XWDNull
	*/

	XWDDockletMenuGetChecked,
	/*
		Check if the item is checked

		XWDId - docklet's identificator
		XWDId - item's identificator
		XWDBool* - result
	*/

	XWDDockletMenuSetChecked,
	/*
		Check the item

		XWDId - docklet's identificator
		XWDId - item's identificator
		XWDBool - XWDTrue if it's checked, else to uncheck it
	*/

	XWDDockletMenuGetEnabled,
	/*
		Check if the item is enabled

		XWDId - docklet's identificator
		XWDId - item's identificator
		XWDBool* - result
	*/

	XWDDockletMenuSetEnabled,
	/*
		Enable the item

		XWDId - docklet's identificator
		XWDId - item's identificator
		XWDBool - XWDTrue if it's enabled, else to disable it
	*/

	XWDDockletNotificationCreate,
	/*
		Create notification of the docklet

		XWDId - docklet's identificator
		XWDId* - result
	*/

	XWDDockletNotificationDelete,
	/*
		Delete notification of the docklet

		XWDId - docklet's identificator
		XWDId - notification
	*/

	XWDDockletNotificationGetPosition,
	/*
		Return position of the docklet's notification

		XWDId - docklet's identificator
		XWDId - notification
		XWDUInt8* - result
			0 - left/top
			1 - top/middle
			2 - right/top
			3 - right/middle
			4 - right/bottom
			5 - bottom/middle
			6 - left/bottom
			7 - left/middle
	*/

	XWDDockletNotificationSetPosition,
	/*
		Setup position of the docklet's notification

		XWDId - docklet's identificator
		XWDId - notification
		XWDUInt8 - position
			0 - left/top
			1 - top/middle
			2 - right/top
			3 - right/middle
			4 - right/bottom
			5 - bottom/middle
			6 - left/bottom
			7 - left/middle
	*/

	XWDDockletNotificationGetVisible,
	/*
		Check if the docklet's notification is visible now

		XWDId - docklet's identificator
		XWDId - notification
		XWDBool* - result
	*/

	XWDDockletNotificationSetVisible,
	/*
		Show/Hide the docklet's notification

		XWDId - docklet's identificator
		XWDId - notification
		XWDBool - XWDTrue to show, else to hide
	*/

	XWDDockletNotificationGetText,
	/*
		Return a number of docklet's notification

		XWDId - docklet's identificator
		XWDId - notification's identificator
		XWDString - result
	*/

	XWDDockletNotificationSetText,
	/*
		Setup a number for docklet's notification

		XWDId - docklet's identificator
		XWDId - notification's identificator
		XWDString - number
	*/

	XWDDockletTimerSet,
	/*
		Setup timer's properties and create it if it doesn't exist

		XWDId - docklet's indentificator
		XWDId - timer's identificator, to create new timer use id value above or equal 100
		XWDUInt32 - elapse
	*/

	XWDDockletTimerDelete,
	/*
		Delete timer of the docklet

		XWDId - docklet's indentificator
		XWDId - timer's identificator
	*/
};
typedef enum XWDFunction XWDFunction;

XWDBool XWDAPICALL XWDExec(XWDFunction function, ...);

enum XWDError
{
	XWDOk = 0,
	XWDErrorUnsupportedFunction,
	XWDErrorInvalidParamaters,
	XWDErrorInternal,
	XWDErrorUnknown = 0xffffffff
};
typedef enum XWDError XWDError;

XWDError XWDAPICALL XWDGetLastError();

enum XWDEvent
{
	XWDEventNone = 0,

	XWDEventCreate,
	/*
		None
	*/

	XWDEventDestroy,
	/*
		None
	*/

	XWDEventDockDestroy,
	/*
		None
	*/

	XWDEventDelete,
	/*
		None
	*/

	XWDEventAdjustIconSize,
	/*
		None
	*/

	XWDEventLButtonDown,
	/*
		XWDPoint - mouse's point
	*/

	XWDEventLButtonUp,
	/*
		XWDPoint - mouse's point
	*/

	XWDEventLButtonClick,
	/*
		XWDPoint - mouse's point
	*/

	XWDEventMenuPopup,
	/*
		None
	*/

	XWDEventMenuHide,
	/*
		None
	*/

	XWDEventMenuItemClick,
	/*
		XWDId - item's id of menu
	*/

	XWDEventTimer,
	/*
		XWDId - timer's id
	*/

	XWDEventDropEnter,
	/*
		Return XWDTrue whether want to continue Drop process

		LPITEMIDLIST - root for parsing
		LPITEMIDLIST* - array of drop items
		XWDUInt16 - count items in array
	*/

	XWDEventDropLeave,
	/*
		None
	*/

	XWDEventDrop,
	/*
		LPITEMIDLIST - root for parsing
		LPITEMIDLIST* - array of drop items
		XWDUInt16 - count items in array
	*/
};
typedef enum XWDEvent XWDEvent;

#endif /* XWDAPI_H */