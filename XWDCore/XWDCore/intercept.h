#ifndef INTERCEPT_H
#define INTERCEPT_H

#include <windows.h>
#include <TlHelp32.h>

bool ThreadsState(bool suspend);

bool InterceptAPI(wchar_t *module, char *functionName, void *functionInterceptAddr, void **functionOldAddr);
bool InterceptAPIRestore(void *functionOldAddr);

#define INTERCEPT_DECLARE_INFO(a) void *oldAddr##a##;
#define INTERCEPT_API(a, b) InterceptAPI(a, #b, &ic##b, &oldAddr##b);
#define INTERCEPT_RESTORE(a) InterceptAPIRestore(oldAddr##a);
#define INTERCEPT_CALLREAL(a) oldAddr##a

#endif /* INTERCEPT_H */