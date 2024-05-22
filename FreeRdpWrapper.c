#include <freerdp/freerdp.h>
#include <freerdp/gdi/gdi.h>

#include "Logging.h"
#include "FreeRdpWrapper.h"

typedef struct
{
	rdpContext context;
	pOnImageUpdated OnImageUpdatedPtr;
	pOnStopped OnStoppedPtr;
	HANDLE transportStopEvent;
} wrapper_context;

void on_stopped(wrapper_context* wcontext)
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
	Log(level, (wchar_t*)wszMsgBuff);
	free(wszMsgBuff);

	if (wcontext->OnStoppedPtr)
	{
		pOnStopped callback = wcontext->OnStoppedPtr;
		if (!callback)
		{
			return;
		}

		callback(rdpError, rdpErrorString);
	}
}

wrapper_context* create_free_rdp_instance(void)
{
	freerdp* instance = freerdp_new();
	if (instance == NULL)
	{
		Log(WLOG_ERROR, L"Failed create the rdp instance");
		return NULL;
	}

	instance->ContextSize = sizeof(wrapper_context);

	if (freerdp_context_new(instance) == FALSE)
	{
		freerdp_free(instance);
		Log(WLOG_ERROR, L"Failed create the rdp context");
		return NULL;
	}

	rdpContext* context = instance->context;
	context->instance = instance;
	wrapper_context* wcontext = (wrapper_context*)context;

	return wcontext;
}

BOOL on_begin_paint(rdpContext* context)
{
	rdpGdi* gdi = context->gdi;
	gdi->primary->hdc->hwnd->invalid->null = TRUE;
	gdi->primary->hdc->hwnd->ninvalid = 0;
	return TRUE;
}

BOOL on_end_paint(rdpContext* context)
{
	DesktopImageData imageData;
	pOnImageUpdated callback = ((wrapper_context*)context)->OnImageUpdatedPtr;

	if (!callback || !context->instance)
	{
		return FALSE;
	}

	rdpGdi* gdi = context->gdi;
	if (!gdi || !gdi->width || !gdi->height || !gdi->stride || !gdi->primary_buffer)
	{
		return FALSE;
	}

	size_t imageByteCount = (size_t)gdi->stride * gdi->height;
	imageData.PixelArray = (BYTE*)calloc(imageByteCount, sizeof(BYTE));

	if (!imageData.PixelArray)
		return FALSE;

	imageData.Width = gdi->width;
	imageData.Height = gdi->height;
	imageData.Stride = gdi->stride;
	imageData.Format = gdi->dstFormat;
	memcpy(imageData.PixelArray, gdi->primary_buffer, imageByteCount);
	callback(&imageData);
	free(imageData.PixelArray);

	return TRUE;
}

BOOL on_post_connect(freerdp* instance)
{
	if (!instance || !instance->context || !instance->context->settings || !instance->context->update)
		return FALSE;

	if (!gdi_init(instance, PIXEL_FORMAT_BGRA32))
		return FALSE;

	instance->context->update->BeginPaint = on_begin_paint;
	instance->context->update->EndPaint = on_end_paint;

	return TRUE;
}

void on_post_disconnect(freerdp* instance)
{
	gdi_free(instance);
}

void prepare_rdp_context(rdpContext* context, const ConnectOptions* rdpOptions)
{
    freerdp_settings_set_string(context->settings, FreeRDP_ServerHostname, ConvertWCharToUtf8Alloc(rdpOptions->HostName, NULL));
	if (rdpOptions->Port)
	{
        freerdp_settings_set_uint32(context->settings, FreeRDP_ServerPort, rdpOptions->Port);
	}

    freerdp_settings_set_string(context->settings, FreeRDP_Domain, ConvertWCharToUtf8Alloc(rdpOptions->Domain, NULL));
    freerdp_settings_set_string(context->settings, FreeRDP_Username, ConvertWCharToUtf8Alloc(rdpOptions->User, NULL));
    freerdp_settings_set_string(context->settings, FreeRDP_Password, ConvertWCharToUtf8Alloc(rdpOptions->Pass, NULL));

	if (rdpOptions->ClientName)
	{
        freerdp_settings_set_string(context->settings, FreeRDP_ClientHostname, ConvertWCharToUtf8Alloc(rdpOptions->ClientName, NULL));
	}

    freerdp_settings_set_bool(context->settings, FreeRDP_SoftwareGdi, TRUE);
    freerdp_settings_set_bool(context->settings, FreeRDP_LocalConnection, TRUE);
    freerdp_settings_set_uint32(context->settings, FreeRDP_ProxyType, PROXY_TYPE_IGNORE);

	// Without this setting the RDP session getting disconnected unexpectedly after a time
	// This issue can be reproduced using 2.5.0 freerdp version
	// (https://uipath.atlassian.net/browse/ROBO-2607) and seems to be introduced by this
	// commit:
	// https://github.com/FreeRDP/FreeRDP/pull/5151/commits/7610917a48e2ea4f1e1065bd226643120cbce4e5
    freerdp_settings_set_bool(context->settings, FreeRDP_BitmapCacheEnabled, TRUE);

	// Increase the TcpAckTimeout to 60 seconds (default is 9 seconds). Used to wait for an
	// active tcp connection (CONNECTION_STATE_ACTIVE)
	// https://github.com/FreeRDP/FreeRDP/blob/fa3cf9417ffb67a3433ecb48d18a1c2b3190a03e/libfreerdp/core/connection.c#L380
    freerdp_settings_set_uint32(context->settings, FreeRDP_TcpAckTimeout, 60000);

    freerdp_settings_set_bool(context->settings, FreeRDP_IgnoreCertificate, TRUE);

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

    freerdp_settings_set_bool(context->settings, FreeRDP_AllowFontSmoothing, rdpOptions->FontSmoothing);

	if (rdpOptions->OnImageUpdatedPtr)
	{
		((wrapper_context*)context)->OnImageUpdatedPtr = rdpOptions->OnImageUpdatedPtr;
		context->instance->PostConnect = on_post_connect;
		context->instance->PostDisconnect = on_post_disconnect;
	}

	if (rdpOptions->OnStoppedPtr)
	{
		((wrapper_context*)context)->OnStoppedPtr = rdpOptions->OnStoppedPtr;
	}
}

DWORD release_all(wrapper_context* wcontext)
{
	if (wcontext == NULL)
	{
		Log(WLOG_ERROR, L"RdpRelease: Invalid wrapper context");
		return ERROR_INVALID_PARAMETER;
	}

	freerdp* instance = ((rdpContext*)wcontext)->instance;
	freerdp_disconnect(instance);
	CloseHandle(wcontext->transportStopEvent);
	wcontext->transportStopEvent = NULL;
	freerdp_context_free(instance);
	freerdp_free(instance);

	return ERROR_SUCCESS;
}

void register_thread_scope(const char *hostName, const char *userName)
{
	if (hostName == NULL || userName == NULL)
	{
		return;
	}

	size_t scopeLen = strlen(hostName) + strlen(userName) + 14;
	char *scope = calloc(1, scopeLen);
	if (scope)
	{
		sprintf_s(scope, scopeLen, "host=%s, user=%s", hostName, userName);
		RegisterCurrentThreadScope(scope);
		free(scope);
	}
}

// Freerdp async transport implementation
// Was removed from freerdp core (https://github.com/FreeRDP/FreeRDP/pull/4815), and remains
// only on freerdp clients Seems to still needed for Windows7 disconnected session
// (https://github.com/UiPath/Driver/commit/dbc3ea9009b988471eee124ed379b02a63b993eb)
DWORD WINAPI transport_thread(LPVOID pData)
{
	HANDLE handles[64] = {0};

	wrapper_context* wcontext = pData;
	if (wcontext == NULL || wcontext->transportStopEvent == NULL)
	{
		Log(WLOG_ERROR, L"Invalid wrapper context");
		return 1;
	}

	rdpContext* context = (rdpContext*)wcontext;
	register_thread_scope(freerdp_settings_get_server_name(context->settings), freerdp_settings_get_string(context->settings, FreeRDP_Username));
	handles[0] = wcontext->transportStopEvent;

	while (1)
	{
		DWORD nCount = 1; // transportStopEvent
		DWORD nCountTmp = freerdp_get_event_handles(context, &handles[nCount], 64 - nCount);
		if (nCountTmp == 0)
		{
			Log(WLOG_ERROR, L"freerdp_get_event_handles failed");
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
			Log(WLOG_ERROR, L"WaitForMultipleObjects returned 0x%08x", status);
			break;
		}
	}

	on_stopped(wcontext);
	return release_all(wcontext);
}

BOOL transport_start(wrapper_context* wcontext)
{
	wcontext->transportStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (!wcontext->transportStopEvent)
	{
		Log(WLOG_ERROR, L"Failed to create freerdp transport stop event, error: %u", GetLastError());
		return FALSE;
	}

	HANDLE transportThreadHandle = CreateThread(NULL, 0, transport_thread, wcontext, 0, NULL);
	if (!transportThreadHandle)
	{
		Log(WLOG_ERROR, L"Failed to create freerdp transport client thread, error: %u", GetLastError());
		CloseHandle(wcontext->transportStopEvent);
		return FALSE;
	}

	CloseHandle(transportThreadHandle);

	return TRUE;
}

DWORD RdpStart(ConnectOptions* rdpOptions, HANDLE* wcontextPtr)
{
	*wcontextPtr = NULL;
	wrapper_context* wcontext = create_free_rdp_instance();
	if (!wcontext)
	{
		return E_OUTOFMEMORY;
	}

	rdpContext* context = (rdpContext*)wcontext;
	freerdp* instance = context->instance;
	prepare_rdp_context(context, rdpOptions);
	BOOL connectResult = freerdp_connect(instance);
	if (connectResult)
	{
		WLog_INFO("Wrapper", "connected");
		BOOL started = transport_start(wcontext);
		if (started)
		{
			*wcontextPtr = wcontext;
			return S_OK;
		}

		Log(WLOG_ERROR, L"Failed start the freerdp transport thread");
	}

	on_stopped(wcontext);
	release_all(wcontext);
	return E_FAIL;
}

DWORD RdpStop(HANDLE wcontextPtr)
{
	const wrapper_context* wcontext = wcontextPtr;

	DWORD result = 0;
	if (!SetEvent(wcontext->transportStopEvent))
	{
		result = GetLastError();
	}

	return result;
}
