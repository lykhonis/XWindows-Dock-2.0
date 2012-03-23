#include <Windows.h>
#include "stdafx.h"
#include "atlbase.h"
#include <dshow.h>
#include <objbase.h>

#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__

#include "qedit.h"

BOOL WINAPI DllMain(__in HINSTANCE hinstDLL, __in DWORD fdwReason,__in LPVOID lpvReserved)
{
    return TRUE;
}

BOOL WINAPI GetVideoFrame(wchar_t *FileName, double Delta, void **Bits, int *Width, int *Height)
{
	IMediaDet *mediaDet;
	HRESULT hRes;

	hRes = CoCreateInstance(CLSID_MediaDet, NULL, CLSCTX_INPROC_SERVER, IID_IMediaDet, (void**)&mediaDet);
	if(FAILED(hRes))
	{
		return FALSE;
	}

	hRes = mediaDet->put_Filename(FileName);
	if(FAILED(hRes))
	{
		mediaDet->Release();
		return FALSE;
	}

	AM_MEDIA_TYPE mediaType;

	hRes = mediaDet->get_StreamMediaType(&mediaType);
	if(FAILED(hRes))
	{
		mediaDet->Release();
		return FALSE;
	}

	if(!IsEqualGUID(mediaType.majortype, MEDIATYPE_Video))
	{
		mediaDet->Release();
		return FALSE;
	}

	if(!IsEqualGUID(mediaType.formattype, FORMAT_VideoInfo))
	{
		mediaDet->Release();
		return FALSE;
	}

	double position;

	hRes = mediaDet->get_StreamLength(&position);
	if(FAILED(hRes))
	{
		mediaDet->Release();
		return FALSE;
	}

	position *= Delta;

	BITMAPINFOHEADER *bmpInfo = &((VIDEOINFOHEADER*)mediaType.pbFormat)->bmiHeader;
	long size;

	hRes = mediaDet->GetBitmapBits(position, &size, NULL, bmpInfo->biWidth, bmpInfo->biHeight);
	if(FAILED(hRes))
	{
		mediaDet->Release();
		return FALSE;
	}

	void *buf = malloc(size);
	hRes = mediaDet->GetBitmapBits(position, &size, (char*)buf, bmpInfo->biWidth, bmpInfo->biHeight);
	if(FAILED(hRes))
	{
		free(buf);
		mediaDet->Release();
		return FALSE;
	}

	HGLOBAL buf2 = GlobalAllocPtr(GMEM_MOVEABLE | GMEM_ZEROINIT, size);

	RGBTRIPLE *pd = (RGBTRIPLE*)buf2;
	RGBTRIPLE *ps = (RGBTRIPLE*)buf;
	ps = &ps[(bmpInfo->biHeight - 1) * bmpInfo->biWidth];

	for(int y = 0; y < bmpInfo->biHeight; y++)
	{
		for(int x = 0; x < bmpInfo->biWidth; x++)
		{
			pd[x].rgbtRed = ps[x].rgbtBlue;
			pd[x].rgbtGreen = ps[x].rgbtRed;
			pd[x].rgbtBlue = ps[x].rgbtGreen;
		}
		pd = &pd[bmpInfo->biWidth];
		ps = &ps[-bmpInfo->biWidth];
	}

	free(buf);

	*Width = bmpInfo->biWidth;
	*Height = bmpInfo->biHeight;
	*Bits = buf2;

	mediaDet->Release();

	return TRUE;
}
