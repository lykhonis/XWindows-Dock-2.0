#include "update.h"

int UpdateWriter(char *data, size_t size, size_t nmemb, CStringA *buff)  
{
	int count = size * nmemb;
	buff->Append(data, count);
	return count;
}

bool ParseVersion(CString version, int &v1, int &v2, int &v3, int &v4)
{
	CString s;

	int i = version.Find('.');
	s = version.Mid(0, i);
	if(s.IsEmpty())
	{
		return false;
	}
	version.Delete(0, i + 1);
	v1 = _wtoi(s.GetBuffer());

	i = version.Find('.');
	s = version.Mid(0, i);
	if(s.IsEmpty())
	{
		return false;
	}
	version.Delete(0, i + 1);
	v2 = _wtoi(s.GetBuffer());

	i = version.Find('.');
	s = version.Mid(0, i);
	if(s.IsEmpty())
	{
		return false;
	}
	version.Delete(0, i + 1);
	v3 = _wtoi(s.GetBuffer());

	if(version.IsEmpty())
	{
		return false;
	}
	v4 = _wtoi(version.GetBuffer());

	return true;
}

UINT AFX_CDECL UpdateThread(LPVOID wParam)
{
	CStringA buff;
	CURLcode code;
	CURL *curl = curl_easy_init();
	if(!curl)
	{
		return 0;
	}
	curl_easy_setopt(curl, CURLOPT_URL, "http://xwdock.aqua-soft.org/version.php");
	curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, TRUE);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, UpdateWriter);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);

	code = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	
	if(code == CURLE_OK)
	{
		Json::Value root;
		Json::Reader reader;
		if(!reader.parse(buff.GetBuffer(), root))
		{
			return 0;
		}
		if(root["version"].isString())
		{
			int vo1, vo2, vo3, vo4, vn1, vn2, vn3, vn4;
			if(!ParseVersion(root["version"].asCString(), vn1, vn2, vn3, vn4) || !ParseVersion(XWDVERSION, vo1, vo2, vo3, vo4))
			{
				return 0;
			}
			if((vo1 < vn1) || (vo2 < vn2) || (vo3 < vn3) || (vo4 < vn4))
			{
				((CWnd*)wParam)->PostMessage(WM_XWDUPDATENOTIFY, 0);
			}
			else
			{
				((CWnd*)wParam)->PostMessage(WM_XWDUPDATENOTIFY, 1);
			}
		}
	}
	else
	{
		((CWnd*)wParam)->PostMessage(WM_XWDUPDATENOTIFY, 2);
	}
	return 0;
}

void XWDCheckNewVersion(CWnd *notifer)
{
	if(notifer)
	{
		AfxBeginThread(UpdateThread, notifer);
	}
}
