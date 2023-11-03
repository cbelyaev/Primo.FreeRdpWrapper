#include "Logging.h"
#include "winpr/wlog.h"
#include "winpr/string.h"

static pRegisterThreadScopeCallback _registerThreadScopeCallback = NULL;
static pLogCallback _clientLogCallback = NULL;
static wLogCallbacks _wlogCallbacks = { 0 };
static char _defaultCategory[] = "Primo.FreeRdpWrapper";

BOOL wLog_Message(const wLogMessage* msg)
{
	if (!_clientLogCallback)
	{
		return FALSE;
	}

	WCHAR *wbuffer = NULL;
	ConvertToUnicode(CP_UTF8, 0, msg->TextString, -1, &wbuffer, 0);

	_clientLogCallback(msg->PrefixString, msg->Level, wbuffer);
	if (wbuffer != NULL)
	{
		free(wbuffer);
	}

	return TRUE;
}

HRESULT InitializeLogging(pLogCallback logCallback, pRegisterThreadScopeCallback registerThreadScopeCallback)
{
	_clientLogCallback = logCallback;
	_registerThreadScopeCallback = registerThreadScopeCallback;

	if (!_clientLogCallback)
	{
		return S_OK;
	}

	wLog* log = WLog_GetRoot();
	WLog_SetLogAppenderType(log, WLOG_APPENDER_CALLBACK);
	wLogAppender*  appender = WLog_GetLogAppender(log);
	_wlogCallbacks.message = wLog_Message;
	WLog_ConfigureAppender(appender, "callbacks", &_wlogCallbacks);
	wLogLayout* layout = WLog_GetLogLayout(log);
	WLog_Layout_SetPrefixFormat(log, layout, "%mn");
#ifdef _DEBUG
	WLog_SetLogLevel(log, WLOG_INFO);
#else
	WLog_SetLogLevel(log, WLOG_ERROR);
#endif

	wLog* negoLog = WLog_Get("com.freerdp.core.nego");
#ifdef _DEBUG
	WLog_SetLogLevel(negoLog, WLOG_WARN);
#else
	WLog_SetLogLevel(negoLog, WLOG_ERROR);
#endif

	return S_OK;
}

void Log(DWORD level, const wchar_t* fmt, ...)
{
	if (!_clientLogCallback)
	{
		return;
	}

	va_list args;
	va_start(args, fmt);
	wchar_t wBuffer[MAX_TRACE_MSG];
	vswprintf(wBuffer, _countof(wBuffer), fmt, args);
	va_end(args);
	_clientLogCallback(_defaultCategory, level, (WCHAR*)wBuffer);
}

void RegisterCurrentThreadScope(char* scope)
{
	if (!_registerThreadScopeCallback)
	{
		return;
	}

	_registerThreadScopeCallback(scope);
}
