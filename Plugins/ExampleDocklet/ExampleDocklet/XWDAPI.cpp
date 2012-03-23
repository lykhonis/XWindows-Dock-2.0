#include <windows.h>
#include "XWDAPI.h"

XWDBool XWDAPICALL XWDExec(XWDFunction function, ...)
{
	typedef XWDBool(XWDAPICALL *XWDFunctionType)(XWDFunction function, va_list args); 
	XWDFunctionType XWDExec = (XWDFunctionType)GetProcAddress(GetModuleHandle(NULL), "XWDExec");
	va_list args;
	va_start(args, function);
	XWDBool ret = XWDExec(function, args);
	va_end(args);
	return ret;
}

XWDError XWDAPICALL XWDGetLastError()
{
	typedef XWDError(XWDAPICALL *XWDFunctionType)(); 
	XWDFunctionType XWDGetLastError = (XWDFunctionType)GetProcAddress(GetModuleHandle(NULL), "XWDGetLastError");
	return XWDGetLastError();
}
