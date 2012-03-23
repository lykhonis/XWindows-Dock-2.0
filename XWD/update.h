#ifndef UPDATE_H
#define UPDATE_H

#include <afxwin.h>
#include <json/json.h>
#include <curl/curl.h>
#include "version.h"

static const UINT WM_XWDUPDATENOTIFY = RegisterWindowMessage(L"WM_XWDUPDATENOTIFY");

void XWDCheckNewVersion(CWnd *notifer);

#endif /* UPDATE_H */