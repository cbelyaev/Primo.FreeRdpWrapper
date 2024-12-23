#include <freerdp/freerdp.h>
#include <freerdp/gdi/gdi.h>

#include "Logging.h"
#include "FreeRdpWrapper.h"

#define TAG "Primo.FreeRdpWrapper"

typedef struct
{
	rdpContext context;
	pOnImageUpdated OnImageUpdatedPtr;
	pOnStopped OnStoppedPtr;
	HANDLE transportStopEvent;
} wrapper_context;

static void on_stopped(wrapper_context* wcontext)
{
	CHAR szMsgBuff[MAX_TRACE_MSG];
	rdpContext* context = (rdpContext*)wcontext;
	const UINT32 rdpError = freerdp_get_last_error(context);
	DWORD level;
	const char* rdpErrorString;
	if (rdpError == FREERDP_ERROR_SUCCESS)
	{
		rdpErrorString = "Disconnected normally.";
		sprintf_s(szMsgBuff, MAX_TRACE_MSG, "%s", rdpErrorString);
		level = WLOG_INFO;
	}
	else if (rdpError == 0x0001000c) // LOGOFF_BY_USER
	{
		rdpErrorString = "The disconnection was initiated by the user logging off their session on the server.";
		sprintf_s(szMsgBuff, MAX_TRACE_MSG, "%s", rdpErrorString);
		level = WLOG_INFO;
	}
	else
	{
		rdpErrorString = freerdp_get_last_error_string(rdpError);
		sprintf_s(szMsgBuff, MAX_TRACE_MSG, "RDP error 0x%08x: %s", rdpError, rdpErrorString);
		level = WLOG_ERROR;
	}

	WCHAR* wszMsgBuff = ConvertUtf8ToWCharAlloc(szMsgBuff, NULL);
	if (wszMsgBuff != NULL)
	{
		Log(level, (wchar_t*)wszMsgBuff);
		free(wszMsgBuff);
	}

	const pOnStopped callback = wcontext->OnStoppedPtr;
	if (callback == NULL)
	{
		return;
	}

	callback(rdpError, rdpErrorString);
}

static wrapper_context* create_free_rdp_instance(void)
{
	freerdp* instance = freerdp_new();
	if (instance == NULL)
	{
		WLog_ERR(TAG, "Failed creating RDP instance");
		return NULL;
	}

	instance->ContextSize = sizeof(wrapper_context);

	if (freerdp_context_new(instance) == FALSE)
	{
		WLog_ERR(TAG, "Failed creating RDP context");
		freerdp_free(instance);
		return NULL;
	}

	rdpContext* context = instance->context;
	context->instance = instance;
	wrapper_context* wcontext = (wrapper_context*)context;

	return wcontext;
}

static BOOL on_begin_paint(rdpContext* context)
{
	WINPR_ASSERT(context);
	WINPR_ASSERT(context->gdi);
	WINPR_ASSERT(context->gdi->primary);
	WINPR_ASSERT(context->gdi->primary->hdc);
	WINPR_ASSERT(context->gdi->primary->hdc->hwnd);
	WINPR_ASSERT(context->gdi->primary->hdc->hwnd->invalid);

	const rdpGdi* gdi = context->gdi;
	gdi->primary->hdc->hwnd->invalid->null = TRUE;
	gdi->primary->hdc->hwnd->ninvalid = 0;
	return TRUE;
}

static BOOL on_end_paint(rdpContext* context)
{
	WINPR_ASSERT(context);
	WINPR_ASSERT(context->instance);

	const pOnImageUpdated callback = ((wrapper_context*)context)->OnImageUpdatedPtr;

	if (callback == NULL)
	{
		return FALSE;
	}

	rdpGdi* gdi = context->gdi;

	WINPR_ASSERT(gdi);
	WINPR_ASSERT(gdi->primary_buffer);

	if (!gdi->width || !gdi->height || !gdi->stride)
	{
		return FALSE;
	}

	DesktopImageData image_data;
	image_data.Width = gdi->width;
	image_data.Height = gdi->height;
	image_data.Stride = gdi->stride;
	image_data.Format = gdi->dstFormat;
	image_data.PixelArray = gdi->primary_buffer;
	callback(&image_data);

	return TRUE;
}

static BOOL on_pre_connect(freerdp* instance)
{
	WINPR_ASSERT(instance);
	WINPR_ASSERT(instance->context);

	rdpSettings* settings = instance->context->settings;
	WINPR_ASSERT(settings);

	if (!freerdp_settings_set_uint32(settings, FreeRDP_OsMajorType, OSMAJORTYPE_WINDOWS))
		return FALSE;
	if (!freerdp_settings_set_uint32(settings, FreeRDP_OsMinorType, OSMINORTYPE_WINDOWS_NT))
		return FALSE;

	return TRUE;
}

static BOOL on_post_connect(freerdp* instance)
{
	WINPR_ASSERT(instance);
	WINPR_ASSERT(instance->context);
	WINPR_ASSERT(instance->context->settings);
	WINPR_ASSERT(instance->context->update);

	if (!gdi_init(instance, PIXEL_FORMAT_XRGB32))
	{
		return FALSE;
	}

	instance->context->update->BeginPaint = on_begin_paint;
	instance->context->update->EndPaint = on_end_paint;

	return TRUE;
}

static void on_post_disconnect(freerdp* instance)
{
	gdi_free(instance);
}

#ifdef _DEBUG
static int on_logon_error_info(freerdp* instance, UINT32 data, UINT32 type)
{
	const char* str_data = freerdp_get_logon_error_info_data(data);
	const char* str_type = freerdp_get_logon_error_info_type(type);

	if (instance == NULL || instance->context == NULL)
		return -1;

	WLog_WARN(TAG, "Logon Warning Info %s [%s]", str_data, str_type);

	return 1;
}
#endif

static BOOL settings_set_wstring(rdpSettings* settings, const FreeRDP_Settings_Keys_String id, const WCHAR* wstr)
{
	char* str = ConvertWCharToUtf8Alloc(wstr, NULL);
	if (str != NULL)
	{
		const BOOL rc = freerdp_settings_set_string(settings, id, str);
		free(str);
		return rc;
	}

	return FALSE;
}

static void prepare_rdp_context(rdpContext* context, const ConnectOptions* rdpOptions)
{
    settings_set_wstring(context->settings, FreeRDP_ServerHostname, rdpOptions->HostName);

	if (rdpOptions->Port > 0)
	{
        freerdp_settings_set_uint32(context->settings, FreeRDP_ServerPort, rdpOptions->Port);
	}

    settings_set_wstring(context->settings, FreeRDP_Domain, rdpOptions->Domain);
    settings_set_wstring(context->settings, FreeRDP_Username, rdpOptions->User);
    settings_set_wstring(context->settings, FreeRDP_Password, rdpOptions->Pass);

	if (rdpOptions->ClientName)
	{
        settings_set_wstring(context->settings, FreeRDP_ClientHostname, rdpOptions->ClientName);
	}

    freerdp_settings_set_uint32(context->settings, FreeRDP_ProxyType, PROXY_TYPE_IGNORE);
    freerdp_settings_set_bool(context->settings, FreeRDP_LocalConnection, TRUE);
    freerdp_settings_set_bool(context->settings, FreeRDP_IgnoreCertificate, TRUE);
	
	freerdp_settings_set_bool(context->settings, FreeRDP_AutoReconnectionEnabled, rdpOptions->AutoReconnectionEnabled);
	
	freerdp_settings_set_uint32(context->settings, FreeRDP_AutoReconnectMaxRetries, rdpOptions->AutoReconnectMaxRetries);
	freerdp_settings_set_uint32(context->settings, FreeRDP_TcpAckTimeout, rdpOptions->TcpAckTimeout);
	freerdp_settings_set_uint32(context->settings, FreeRDP_TcpConnectTimeout, rdpOptions->TcpConnectTimeout);
	




	if (rdpOptions->Width > 0)
	{
        freerdp_settings_set_uint32(context->settings, FreeRDP_DesktopWidth, rdpOptions->Width);
	}

	if (rdpOptions->Height > 0)
	{
        freerdp_settings_set_uint32(context->settings, FreeRDP_DesktopHeight, rdpOptions->Height);
	}

	if (rdpOptions->Depth > 0)
	{
        freerdp_settings_set_uint32(context->settings, FreeRDP_ColorDepth, rdpOptions->Depth);
	}

	context->instance->PreConnect = on_pre_connect;
	context->instance->PostConnect = on_post_connect;
	context->instance->PostDisconnect = on_post_disconnect;
#ifdef _DEBUG
	context->instance->LogonErrorInfo = on_logon_error_info;
#endif

	((wrapper_context*)context)->OnImageUpdatedPtr = rdpOptions->OnImageUpdatedPtr;
    freerdp_settings_set_bool(context->settings, FreeRDP_SoftwareGdi, TRUE);
    freerdp_settings_set_bool(context->settings, FreeRDP_BitmapCacheEnabled, TRUE);
	freerdp_settings_set_bool(context->settings, FreeRDP_AllowFontSmoothing, rdpOptions->FontSmoothing);

	if (rdpOptions->OnStoppedPtr)
	{
		((wrapper_context*)context)->OnStoppedPtr = rdpOptions->OnStoppedPtr;
	}
}

static DWORD release_all(wrapper_context* wcontext)
{
	if (wcontext == NULL)
	{
		WLog_ERR(TAG, "release_all: Invalid wrapper context");
		return ERROR_INVALID_PARAMETER;
	}

	freerdp* instance = ((rdpContext*)wcontext)->instance;
	if (wcontext->transportStopEvent != NULL)
	{
		freerdp_disconnect(instance);
		CloseHandle(wcontext->transportStopEvent);
		wcontext->transportStopEvent = NULL;
	}

	freerdp_context_free(instance);
	freerdp_free(instance);

	return ERROR_SUCCESS;
}

static void register_thread_scope(const rdpSettings* settings)
{
	const char* server_name = freerdp_settings_get_server_name(settings);
	const char* user_name = freerdp_settings_get_string(settings, FreeRDP_Username);

	if (server_name == NULL || user_name == NULL)
	{
		return;
	}

	const size_t scopeLen = strlen(server_name) + strlen(user_name) + 14;
	char *scope = calloc(1, scopeLen);
	if (scope)
	{
		sprintf_s(scope, scopeLen, "host=%s, user=%s", server_name, user_name);
		RegisterCurrentThreadScope(scope);
		free(scope);
	}
}

static DWORD WINAPI transport_thread(LPVOID pData)
{
	HANDLE handles[64] = {0};

	wrapper_context* wcontext = pData;
	if (wcontext == NULL || wcontext->transportStopEvent == NULL)
	{
		WLog_ERR(TAG, "Invalid wrapper context");
		return 1;
	}

	rdpContext* context = (rdpContext*)wcontext;
	register_thread_scope(context->settings);
	handles[0] = wcontext->transportStopEvent;

	while (1)
	{
		DWORD nCount = 1; // transportStopEvent
		DWORD nCountTmp = freerdp_get_event_handles(context, &handles[nCount], 64 - nCount);
		if (nCountTmp == 0)
		{
			WLog_ERR(TAG, "freerdp_get_event_handles failed");
			break;
		}

		nCount += nCountTmp;
		DWORD status = WaitForMultipleObjects(nCount, handles, FALSE, INFINITE);

		if (status == WAIT_OBJECT_0)
		{
			break;
		}

		if (status > WAIT_OBJECT_0 && status < (WAIT_OBJECT_0 + nCount))
		{
			freerdp_check_event_handles(context);
			if (freerdp_shall_disconnect_context(context))
			{
				break;
			}
		}
		else
		{
			WLog_ERR(TAG, "WaitForMultipleObjects returned 0x%08x", status);
			break;
		}
	}

	on_stopped(wcontext);
	return release_all(wcontext);
}

static BOOL transport_start(wrapper_context* wcontext)
{
	wcontext->transportStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (wcontext->transportStopEvent == NULL)
	{
		WLog_ERR(TAG, "Failed to create freerdp transport stop event, error: %u", GetLastError());
		return FALSE;
	}

	HANDLE transportThreadHandle = CreateThread(NULL, 0, transport_thread, wcontext, 0, NULL);
	if (transportThreadHandle == NULL)
	{
		WLog_ERR(TAG, "Failed to create freerdp transport client thread, error: %u", GetLastError());
		CloseHandle(wcontext->transportStopEvent);
		return FALSE;
	}

	CloseHandle(transportThreadHandle);

	return TRUE;
}

DWORD RdpStart(const ConnectOptions* rdpOptions, HANDLE* wcontextPtr)
{
	*wcontextPtr = NULL;
	wrapper_context* wcontext = create_free_rdp_instance();
	if (wcontext == NULL)
	{
		return E_OUTOFMEMORY;
	}

	rdpContext* context = (rdpContext*)wcontext;
	freerdp* instance = context->instance;
	prepare_rdp_context(context, rdpOptions);
	const BOOL connectResult = freerdp_connect(instance);
	if (connectResult)
	{
		const BOOL started = transport_start(wcontext);
		if (started)
		{
			*wcontextPtr = wcontext;
			return S_OK;
		}

		WLog_ERR(TAG, "Failed starting the transport thread");
	}

	on_stopped(wcontext);
	release_all(wcontext);
	return E_FAIL;
}

DWORD RdpStop(HANDLE wcontextPtr)
{
	wrapper_context* wcontext = wcontextPtr;

	wcontext->OnStoppedPtr = NULL; // to prevent from calling after dispose
	wcontext->OnImageUpdatedPtr = NULL; // to prevent from calling after dispose

	if (wcontext->transportStopEvent == NULL)
	{
		return NO_ERROR;
	}

	DWORD result = 0;
	if (!SetEvent(wcontext->transportStopEvent))
	{
		result = GetLastError();
	}

	return result;
}
