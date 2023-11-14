#pragma once

#include <winpr/wlog.h>

#define MAX_TRACE_MSG 2048

typedef void (*pLogCallback)(char* category, DWORD errorLevel, WCHAR* message);
typedef void (*pRegisterThreadScopeCallback)(char* category);


void Log(DWORD level, const wchar_t* fmt, ...);
void RegisterCurrentThreadScope(char* scope);

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32
	__declspec(dllexport)
#endif
	HRESULT InitializeLogging(pLogCallback logCallback, pRegisterThreadScopeCallback registerThreadScopeCallback);

#ifdef __cplusplus
}
#endif
