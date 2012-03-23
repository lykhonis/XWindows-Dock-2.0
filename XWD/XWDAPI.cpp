#include <afxwin.h>
#include "XWDAPI.h"

typedef XWDBool(*XWDExecProc)(XWDFunction function, XWDError &xwdLastError, va_list args);

XWDError xwdLastError = XWDOk;
XWDExecProc xwdExecProc = NULL;

XWDError XWDAPICALL XWDGetLastError()
{
	return xwdLastError;
}

XWDBool XWDAPICALL XWDExecArgs(XWDFunction function, va_list args)
{
	if(xwdExecProc)
	{
		return xwdExecProc(function, xwdLastError, args);
	}
	xwdLastError = XWDErrorInternal;
	return XWDFalse;
}

XWDBool XWDAPICALL XWDExec(XWDFunction function, ...)
{
	va_list args;
	va_start(args, function);
	XWDBool ret = XWDExecArgs(function, args);
	va_end(args);
	return ret;
}