#pragma once

#include "pch.h"

namespace FreeRdpClient
{
	typedef struct
	{
		BYTE* PixelArray;
		UINT32 Width;
		UINT32 Height;
		UINT32 Stride;
		UINT32 Format;
	} DesktopImageData;

	typedef void (*pOnImageUpdated)(DesktopImageData* data);

	typedef struct
	{
		long Width;
		long Height;
		long Depth;
		BOOL FontSmoothing;
		BSTR User;
		BSTR Domain;
		BSTR Pass;
		BSTR ClientName;
		BSTR HostName;
		long Port;
		pOnImageUpdated OnImageUpdatedPtr;
	} ConnectOptions;


	EXTERN_C __declspec(dllexport) HRESULT STDAPICALLTYPE RdpLogon(ConnectOptions* rdpOptions, BSTR& releaseEventName);
	EXTERN_C __declspec(dllexport) HRESULT STDAPICALLTYPE RdpRelease(BSTR releaseEventName);
}