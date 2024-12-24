#pragma once

typedef struct
{
	BYTE* PixelArray;
	UINT32 Width;
	UINT32 Height;
	UINT32 Stride;
	UINT32 Format;
} DesktopImageData;

typedef void (*pOnImageUpdated)(DesktopImageData* data);
typedef void (*pOnStopped)(UINT32 code, const char* message);

typedef struct
{
	int32_t Width;
	int32_t Height;
	int32_t Depth;
	int32_t FontSmoothing;
	WCHAR* User;
	WCHAR* Domain;
	WCHAR* Pass;
	WCHAR* ClientName;
	WCHAR* HostName;
	int32_t Port;
	pOnImageUpdated OnImageUpdatedPtr;
	pOnStopped OnStoppedPtr;
	int32_t AutoReconnectionEnabled;
	int32_t AutoReconnectMaxRetries;
	int32_t TcpAckTimeout;
	int32_t TcpConnectTimeout;
} ConnectOptions;

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32
	__declspec(dllexport)
#endif
	DWORD RdpStart(const ConnectOptions* rdpOptions, HANDLE* wcontextPtr);

#ifdef _WIN32
	__declspec(dllexport)
#endif
	DWORD RdpStop(HANDLE dataPrt);

#ifdef _WIN32
	__declspec(dllexport)
#endif
	BOOL RdpSendKeyboardEvent(HANDLE wcontextPtr, UINT16 flags, UINT8 code);

#ifdef _WIN32
	__declspec(dllexport)
#endif
	BOOL RdpSendMouseEvent(HANDLE wcontextPtr, UINT16 flags, UINT16 x, UINT16 y);

#ifdef _WIN32
	__declspec(dllexport)
#endif
	VOID SetPaintEnabled(HANDLE wcontextPtr, BOOL enabled);

#ifdef __cplusplus
}
#endif
