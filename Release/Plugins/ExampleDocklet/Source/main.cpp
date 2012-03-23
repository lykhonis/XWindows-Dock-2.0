#include <windows.h>
#include "XWDAPI.h"

#pragma comment(lib, "user32.lib")

BOOL WINAPI DllMain(__in HINSTANCE hinstDLL, __in DWORD fdwReason, __in LPVOID lpvReserved)
{
	// do nothing here
	return TRUE;
}

// just return one of the constants to let the dock to identificate your plugin
XWDPluginType XWDAPICALL XWDGetPluginType()
{
	return XWDPluginDocklet;
}

XWDBool XWDAPICALL XWDGetPluginIcon(XWDString buff)
{
	// if you haven't got icon for the plugin, then
	// return XWDFalse;
	// else...
	wcscpy_s(buff, XWDStringLength, L"ExampleDocklet.ico"); // must exists in the same folder with the plugin
	// also, you can free use .png, .bmp and so on
	return XWDTrue;
}

// call when dock needs to know information about the plugin
XWDVoid XWDAPICALL XWDGetPluginInformation(XWDString name, XWDString author, XWDString description, XWDUInt32 *version)
{
	wcscpy_s(name, XWDStringLength, L"Example Docklet");
	wcscpy_s(author, XWDStringLength, L"Lichonos Vladimir");
	wcscpy_s(description, XWDStringLength, L"Example how to create own plugins for XWindows Dock 2.x.x");
	(*version) = 0x00000001; // 0.0.0.1
}

// declare plugin's data here, like own/new structure/class
struct XWDData
{
	XWDId menuItem1;
	XWDId menuItem2;
	XWDId menuItem3;
	XWDId menuItem4;
	XWDId notification1;
	XWDBool timer1Enabled;
};
typedef struct XWDData XWDData, *XWDPData;

// call when dock loads/intializes the plugin
XWDPData XWDAPICALL XWDPluginInitialize(XWDId id)
{
	XWDPData data = (XWDPData)malloc(sizeof(XWDData));
	data->timer1Enabled = false;
	return data;
}

// call when dock unloads the plugin (not delete if it's docklet, just unload to free memory)
XWDVoid XWDAPICALL XWDPluginDeinitialize(XWDId id, XWDPData data)
{
	free(data);
}

// call when dock informs the plugin about some events (for example if user click on the icon or something else)
XWDBool XWDAPICALL XWDPluginEvent(XWDId id, XWDPData data, XWDEvent uEvent, va_list args)
{
	switch(uEvent)
	{
	// Docklet API
	case XWDEventCreate:
		{
			// Setup icon
			XWDId icon;
			XWDString tmp;
			XWDExec(XWDDockletGetPath, id, tmp);
			wcscat_s(tmp, XWDStringLength, L"ExampleDocklet.ico");
			
			XWDExec(XWDImageCreate, &icon);
			XWDExec(XWDImageAssignFile, icon, tmp);
			XWDExec(XWDDockletSetImage, id, icon);
			XWDExec(XWDImageDelete, icon);

			// Setup label
			XWDExec(XWDDockletSetLabel, id, L"Exmaple Docklet 0.0.0.1");

			// Setup menu, fill in it from bottom to top
			XWDExec(XWDDockletMenuAdd, id, XWDNull, XWDNull, XWDNull, XWDMenuFlagsLine);
			XWDExec(XWDDockletMenuAdd, id, XWDNull, &data->menuItem1, L"Sub items", XWDMenuFlagsNone);
			XWDExec(XWDDockletMenuAdd, id, data->menuItem1, &data->menuItem2, L"To do something...", XWDMenuFlagsNone);
			XWDExec(XWDDockletMenuAdd, id, XWDNull, &data->menuItem3, L"Disabled item", XWDMenuFlagsNone);
			XWDExec(XWDDockletMenuSetEnabled, id, data->menuItem3, XWDFalse);
			XWDExec(XWDDockletMenuAdd, id, XWDNull, &data->menuItem4, L"Enable Indicator", XWDMenuFlagsCheckbox);

			// Setup notification
			XWDExec(XWDDockletNotificationCreate, id, &data->notification1);
			XWDExec(XWDDockletNotificationSetPosition, id, data->notification1, 2);
			XWDExec(XWDDockletNotificationSetText, id, data->notification1, L"1");
			XWDExec(XWDDockletNotificationSetVisible, id, data->notification1, XWDTrue);
		}
		break;

	case XWDEventLButtonClick:
		{
			// Test clicked coords
			XWDPoint pt = va_arg(args, XWDPoint);
			XWDString tmp;
			wsprintf(tmp, L"Clicked here - left: %d, top: %d", pt.left, pt.top);
			XWDExec(XWDDockletSetLabel, id, tmp);

			// Enable/Disable timer to change notification's value
			data->timer1Enabled = !data->timer1Enabled;
			XWDExec(XWDDockletTimerDelete, id, 100);
			if(data->timer1Enabled)
			{
				XWDExec(XWDDockletTimerSet, id, 100, 1000);
			}
		}
		break;

	case XWDEventTimer:
		{
			XWDId timerId = va_arg(args, XWDId);

			switch(timerId)
			{
			case 100:
				{
					// Move and increase notification
					XWDString tmp;
					XWDExec(XWDDockletNotificationGetText, id, data->notification1, tmp);
					int num = _wtoi(tmp);
					wsprintf(tmp, L"%d", ++num);
					XWDExec(XWDDockletNotificationSetText, id, data->notification1, tmp);

					XWDUInt8 position;
					XWDExec(XWDDockletNotificationGetPosition, id, data->notification1, &position);
					position++;
					if(position > 7)
					{
						position = 0;
					}
					XWDExec(XWDDockletNotificationSetPosition, id, data->notification1, position);
				}
				break;
			}
		}
		break;

	case XWDEventMenuItemClick:
		{
			// get selected menu's id
			XWDId menuId = va_arg(args, XWDId);
			
			if(data->menuItem4 == menuId)
			{
				XWDBool checked;
				XWDExec(XWDDockletMenuGetChecked, id, menuId, &checked);
				XWDExec(XWDDockletMenuSetChecked, id, menuId, !checked);
				XWDExec(XWDDockletSetIndicator, id, !checked);
				XWDExec(XWDDockletMenuSetText, id, data->menuItem4, checked ? L"Enable Indicator" : L"Disable Indicator");
			}
		}
		break;
	}
	return XWDFalse;
}