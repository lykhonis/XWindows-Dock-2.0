#include "video.h"

bool Video::GetFrame(CString fileName, CDIB *dib, double delta)
{
	IMediaDet *mediaDet;
	HRESULT hRes;

	hRes = CoInitialize(NULL);
	if(FAILED(hRes))
	{
		return false;
	}

	hRes = CoCreateInstance(CLSID_MediaDet, NULL, CLSCTX_INPROC_SERVER, IID_IMediaDet, (void**)&mediaDet);
	if(FAILED(hRes))
	{
		CoUninitialize();
		return false;
	}

	hRes = mediaDet->put_Filename(fileName.GetBuffer());
	if(FAILED(hRes))
	{
		mediaDet->Release();
		CoUninitialize();
		return false;
	}

	AM_MEDIA_TYPE mediaType;

	hRes = mediaDet->get_StreamMediaType(&mediaType);
	if(FAILED(hRes))
	{
		mediaDet->Release();
		CoUninitialize();
		return false;
	}

	if(!IsEqualGUID(mediaType.majortype, MEDIATYPE_Video))
	{
		mediaDet->Release();
		CoUninitialize();
		return false;
	}

	if(!IsEqualGUID(mediaType.formattype, FORMAT_VideoInfo))
	{
		mediaDet->Release();
		CoUninitialize();
		return false;
	}

	double position;

	hRes = mediaDet->get_StreamLength(&position);
	if(FAILED(hRes))
	{
		mediaDet->Release();
		CoUninitialize();
		return false;
	}

	position *= delta;

	BITMAPINFOHEADER *bmpInfo = &((VIDEOINFOHEADER*)mediaType.pbFormat)->bmiHeader;
	long size;

	hRes = mediaDet->GetBitmapBits(position, &size, NULL, bmpInfo->biWidth, bmpInfo->biHeight);
	if(FAILED(hRes))
	{
		mediaDet->Release();
		CoUninitialize();
		return false;
	}

	void *buf = malloc(size);
	hRes = mediaDet->GetBitmapBits(position, &size, (char*)buf, bmpInfo->biWidth, bmpInfo->biHeight);
	if(FAILED(hRes))
	{
		free(buf);
		mediaDet->Release();
		CoUninitialize();
		return false;
	}

	dib->Resize(bmpInfo->biWidth, bmpInfo->biHeight);

	DIB_AARGB *pd = (DIB_AARGB*)dib->scan0;
	RGBTRIPLE *ps = (RGBTRIPLE*)buf;
	ps = &ps[(bmpInfo->biHeight - 1) * bmpInfo->biWidth];

	for(int y = 0; y < bmpInfo->biHeight; y++)
	{
		for(int x = 0; x < bmpInfo->biWidth; x++)
		{
			pd[x]->a = 255;
			pd[x]->r = ps[x].rgbtBlue;
			pd[x]->g = ps[x].rgbtRed;
			pd[x]->b = ps[x].rgbtGreen;
		}
		pd = &pd[bmpInfo->biWidth];
		ps = &ps[-bmpInfo->biWidth];
	}

	free(buf);
	mediaDet->Release();

	CoUninitialize();

	return true;
}