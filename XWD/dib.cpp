#include "dib.h"

using namespace Gdiplus;

CDIB::CDIB()
{
	bmp = NULL;
	bitmap = NULL;
	scan0 = NULL;
	dc = CreateCompatibleDC(0);
	bi = new BITMAPINFO;
	memset(bi, 0, sizeof(BITMAPINFO));
}

CDIB::~CDIB()
{
	FreeImage();
	DeleteObject(dc);
	delete bi;
}

bool CDIB::Ready() 
{
	return ((bitmap != NULL) && (bmp != NULL));
}

int CDIB::Width() 
{ 
	return Ready() ? bi->bmiHeader.biWidth : NULL; 
}

int CDIB::Height() 
{ 
	return Ready() ? bi->bmiHeader.biHeight : NULL; 
}

CRect CDIB::Rect()
{
	return Ready() ? CRect(0, 0, bi->bmiHeader.biWidth, bi->bmiHeader.biHeight) : CRect(0, 0, 0, 0);
}

int CDIB::Size()
{
	return Ready() ? bi->bmiHeader.biSizeImage : NULL; 
}

void* CDIB::Pixels(int x, int y)
{
	return Ready() ? (void*)((int)scan0 + ((bi->bmiHeader.biHeight - 1 - y) * bi->bmiHeader.biWidth + x) * 4) : NULL;
}

void CDIB::Assign(CDIB *dib)
{
	FreeImage();
	if(dib && dib->Ready())
	{
		Assign(dib->bmp);
	}
}

void CDIB::Assign(Gdiplus::Bitmap *dib)
{
	FreeImage();
	if(dib)
	{
		BitmapData bmpData;
		if(dib->LockBits(NULL, ImageLockModeRead, PixelFormat32bppARGB, &bmpData) == Ok)
		{
			Assign(bmpData.Width, bmpData.Height, bmpData.Scan0);
			dib->UnlockBits(&bmpData);
		}
	}
}

void CDIB::Assign(int width, int height, void *dib)
{
	FreeImage();
	if(dib)
	{
		Resize(width, height);
		if(Ready())
		{
			DIB_ARGB *pd = scan0;
			DIB_ARGB *ps = (DIB_ARGB*)dib;
			for(int i = 0; i < bi->bmiHeader.biWidth * bi->bmiHeader.biHeight; i++, pd++, ps++)
			{
				pd->c = ps->c;
			}
		}
	}
}

void CDIB::Resize(int width, int height)
{
	if((width <= 0) || (height <= 0))
	{
		return;
	}

	if(Ready() && (bi->bmiHeader.biWidth == width) && (bi->bmiHeader.biHeight == height))
	{
		Fill();
		return;
	}

	FreeImage();

	bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi->bmiHeader.biBitCount = 32;
	bi->bmiHeader.biCompression = BI_RGB;
	bi->bmiHeader.biHeight = height;
	bi->bmiHeader.biWidth = width;
	bi->bmiHeader.biPlanes = 1;
	bi->bmiHeader.biSizeImage = bi->bmiHeader.biWidth * bi->bmiHeader.biHeight * 4;

	bitmap = CreateDIBSection(0, bi, DIB_RGB_COLORS, (void**)&scan0, 0, 0);
	if(!bitmap)
	{
		FreeImage();
		return;
	}
	oldBitmap = (HBITMAP)SelectObject(dc, bitmap);

	bmp = new Bitmap(bi->bmiHeader.biWidth, bi->bmiHeader.biHeight, bi->bmiHeader.biWidth * 4, 
		PixelFormat32bppARGB, (BYTE*)scan0);

	if(!bmp || (bmp->GetLastStatus() != Ok))
	{
		FreeImage();
		return;
	}
}

void CDIB::FreeImage()
{
	if(bmp)
	{
		delete bmp;
		bmp = NULL;
	}
	if(bitmap)
	{
		SelectObject(dc, oldBitmap);
		memset(bi, 0, sizeof(BITMAPINFO));
		DeleteObject(bitmap);
		bitmap = NULL;
		scan0 = NULL;
	}
}

void CDIB::Load(CString fileName)
{
	FreeImage();
	if(fileName.IsEmpty())
	{
		return;
	}
	Bitmap *tmp = new Bitmap(fileName.GetBuffer());
	if(tmp)
	{
		if(tmp->GetLastStatus() == Ok)
		{
			Assign(tmp);
		}
		delete tmp;
	}
}

void CDIB::Fill()
{
	if(Ready())
	{
		unsigned int *p = (unsigned int*)scan0;
		int size = bi->bmiHeader.biWidth * bi->bmiHeader.biHeight;
		for(int y = 0; y < size; y++, p++)
		{
			(*p) = 0;
		}
	}
}

void CDIB::Fill(int x, int y, int width, int height)
{
	if(Ready())
	{
		for(int iy = y; iy < y + height; iy++)
		{
			if((iy >= 0) && (iy < bi->bmiHeader.biHeight))
			{
				int *p = (int*)Pixels(x, iy);
				for(int ix = x; ix < x + width; ix++, p++)
				{
					if((ix >= 0) && (ix < bi->bmiHeader.biWidth))
					{
						(*p) = 0;
					}
				}
			}
		}
	}
}

void CDIB::DecodeAlpha()
{
	if(Ready())
	{
		DIB_ARGB *p = scan0;
		int size = bi->bmiHeader.biWidth * bi->bmiHeader.biHeight;
		for (int i = 0; i < size; i++, p++)
		{
			if (p->a)
			{
				p->r = (unsigned char)(p->r * 255 / p->a);
				p->g = (unsigned char)(p->g * 255 / p->a);
				p->b = (unsigned char)(p->b * 255 / p->a);
			}
		}
	}
}

void CDIB::ReflectVertical()
{
	if(Ready())
	{
		int count = bi->bmiHeader.biWidth;
		int stride = count * 4;
		for(int i = 0; i < bi->bmiHeader.biHeight / 2; i++)
		{
			void *pd = (void*)((int)scan0 + i * stride);
			void *ps = (void*)((int)scan0 + (bi->bmiHeader.biHeight - 1 - i) * stride);
			__asm
			{
				push ecx
				push edi
				push esi

				mov ecx,count
				mov esi,ps
				mov edi,pd

			l1:
				mov eax,[esi]
				xchg [edi],eax
				mov [esi],eax
				lea esi,[esi+4]
				lea edi,[edi+4]
				loop l1

				pop esi
				pop edi
				pop ecx
			}
		}
	}
}

void CDIB::Blur(CRect workRect, CRect ignoreRect, int shadowSize)
{
	if(Ready())
	{
		Blur(scan0, bi->bmiHeader.biWidth, bi->bmiHeader.biHeight, workRect, ignoreRect, shadowSize);
	}
}

void CDIB::Blur(void *dst, int width, int height, CRect workRect, CRect ignoreRect, int shadowSize)
{
	unsigned int rs, gs, bs, as, cs;
	int sz = width * height;
	void *src = malloc(sz * 4);

	//memcpy(src, dst, sz);
	__asm
	{
		push esi
		push edi
		push ecx
		mov ecx,sz
		mov esi,dst
		mov edi,src
l1:
		mov eax,[esi]
		xchg eax,[edi]
		mov [esi],eax
		lea esi,[esi+4]
		lea edi,[edi+4]
		loop l1
		pop ecx
		pop edi
		pop esi
	}

	DIB_ARGB *pd = (DIB_ARGB*)((int)dst + ((height - 1 - workRect.top) * width + workRect.left) * 4);
	int pdi = width * 2;
	for(int y = workRect.top; y < workRect.bottom; y++, pd -= pdi)
	{
		for(int x = workRect.left; x < workRect.right; x++, pd++)
		{
			if((y < ignoreRect.top) || (y >= ignoreRect.bottom) || (x < ignoreRect.left) || (x >= ignoreRect.right))
			{
				rs = 0;
				gs = 0;
				bs = 0;
				as = 0;
				cs = 0;
				DIB_ARGB *p0 = (DIB_ARGB*)((int)src + ((height - 1 - (y - shadowSize)) * width) * 4);
				for(int yy = y - shadowSize; yy < y + shadowSize; yy++, p0 -= width)
				{
					if((yy >= 0) && (yy < height))
					{
						DIB_ARGB *p = p0 + x - shadowSize;
						for(int xx = x - shadowSize; xx < x + shadowSize; xx++, cs++, p++)
						{
							if((xx >= 0) && (xx < width))
							{
								rs += p->r;
								gs += p->g;
								bs += p->b;
								as += p->a;
							}
						}
					}
				}
				if(!cs)
				{
					cs = 1;
				}
				pd->a = (unsigned char)(as / cs);
				pd->r = (unsigned char)(rs / cs);
				pd->g = (unsigned char)(gs / cs);
				pd->b = (unsigned char)(bs / cs);
				/*pd->r = (unsigned char)(rs * pd->a / cs / 255);
				pd->g = (unsigned char)(gs * pd->a / cs / 255);
				pd->b = (unsigned char)(bs * pd->a / cs / 255);*/
			}
		}
	}
	free(src);
}

void CDIB::Draw(CRect rect, int srcX, int srcY, CDIB *dib, unsigned int flags)
{
	if(!Ready() || !dib->Ready())
	{
		return;
	}

	int w = rect.Width();
	int h = rect.Height();

	if((w <= 0) || (h <= 0) || (srcX < 0) || (srcY < 0))
	{
		return;
	}

	DIB_AARGB *ps, *pd;
	int psi, pdi;

	for(int y = 0; y < h; y++)
	{
		if(flags & DrawFlagsReflectSrc)
		{
			psi = dib->bi->bmiHeader.biHeight - 1 - srcY - y;
			if(psi < 0)
			{
				return;
			}
		}
		else
		{
			psi = srcY + y;
			if(psi >= dib->Height())
			{
				return;
			}
		}
		if(flags & DrawFlagsReflectDest)
		{
			pdi = bi->bmiHeader.biHeight - 1 - y - rect.top;
			if(pdi < 0)
			{
				return;
			}
		}
		else
		{
			pdi = y + rect.top;
			if(pdi >= rect.bottom)
			{
				return;
			}
		}
		ps = (DIB_AARGB*)((int)dib->scan0 + (psi * dib->bi->bmiHeader.biWidth + srcX) * 4);
		pd = (DIB_AARGB*)((int)scan0 + (pdi * bi->bmiHeader.biWidth + rect.left) * 4);
		for(int x = 0; x < w; x++)
		{
			if((srcX + x >= 0) && (srcX + x < dib->bi->bmiHeader.biWidth) &&
				(rect.left + x >= 0) && (rect.left + x < bi->bmiHeader.biWidth))
			{
				if(flags & DrawFlagsPaint)
				{
					pd[x]->a = ps[x]->a;
					float k = pd[x]->a / 255.0f;
					pd[x]->r = (unsigned char)(ps[x]->r * k);
					pd[x]->g = (unsigned char)(ps[x]->g * k);
					pd[x]->b = (unsigned char)(ps[x]->b * k);
				}
				else
				{
					pd[x]->c = ps[x]->c;
				}
			}
		}
	}
}

void CDIB::DrawTail(CRect dstRect, CRect srcRect, CDIB *dib, unsigned int flags)
{
	if(!Ready() || !dib->Ready())
	{
		return;
	}

	int sw = srcRect.Width();
	int sh = srcRect.Height();
	int dw = dstRect.Width();
	int dh = dstRect.Height();
	int dy;
	int dx;
	int y;
	int x;

	if((sw <= 0) || (sh <= 0) || (dw <= 0) || (dh <= 0))
	{
		return;
	}

	/*if((dh < sh) || (dw < sw))
	{
		Draw(dstRect, dib->Rect(), dib, flags);
		return;
	}*/
	if(dh < sh)
	{
		sh = dh;
	}
	if(dw < sw)
	{
		sw = dw;
	}

	for(y = dstRect.top; y <= dstRect.bottom - sh; y += sh)
	{
		for(x = dstRect.left; x <= dstRect.right - sw; x += sw)
		{
			CRect rect;
			rect.left = x;
			rect.top = y;
			rect.right = rect.left + sw;
			rect.bottom = rect.top + sh;

			Draw(rect, srcRect.left, srcRect.top, dib, flags);
		}
		dx = dstRect.right - x;
		if(dx > 0)
		{
			CRect rect;
			rect.left = dstRect.right - dx;
			rect.top = y;
			rect.right = dstRect.right;
			rect.bottom = rect.top + sh;

			Draw(rect, srcRect.left, srcRect.top, dib, flags);
		}
	}

	dy = dstRect.bottom - y;
	if(dy > 0)
	{
		for(x = dstRect.left; x <= dstRect.right - sw; x += sw)
		{
			CRect rect;
			rect.left = x;
			rect.top = dstRect.bottom - dy;
			rect.right = rect.left + sw;
			rect.bottom = dstRect.bottom;

			Draw(rect, srcRect.left, srcRect.top, dib, flags);
		}
		dx = dstRect.right - x;
		if(dx > 0)
		{
			CRect rect;
			rect.left = dstRect.right - dx;
			rect.top = dstRect.bottom - dy;
			rect.right = dstRect.right;
			rect.bottom = dstRect.bottom;

			Draw(rect, srcRect.left, srcRect.top, dib, flags);
		}
	}
}

void CDIB::Draw(CRect dstRect, CRect srcRect, CDIB *dib, unsigned int flags)
{
	if(Ready())
	{
		Draw(dstRect.left, dstRect.top, dstRect.Width(), dstRect.Height(),
			srcRect.left, srcRect.top, srcRect.Width(), srcRect.Height(),
			scan0, bi->bmiHeader.biWidth, bi->bmiHeader.biHeight,
			dib->scan0, dib->Width(), dib->Height(), flags);
	}
}

void CDIB::DrawBlend(CRect dstRect, CRect srcRect, CDIB *dib, unsigned char alpha, unsigned int flags)
{
	if(Ready())
	{
		DrawBlend(dstRect.left, dstRect.top, dstRect.Width(), dstRect.Height(),
			srcRect.left, srcRect.top, srcRect.Width(), srcRect.Height(),
			scan0, bi->bmiHeader.biWidth, bi->bmiHeader.biHeight,
			dib->scan0, dib->Width(), dib->Height(), alpha, flags);
	}
}

void CDIB::Draw(int xdst, int ydst, int widthdst, int heightdst, 
		int xsrc, int ysrc, int widthsrc, int heightsrc,
		void *dst, int dstWidth, int dstHeight, 
		void *src, int srcWidth, int srcHeight, 
		unsigned int flags)
{
	DrawBlend(xdst, ydst, widthdst, heightdst, xsrc, ysrc, widthsrc, heightsrc, dst, dstWidth, dstHeight,
		src, srcWidth, srcHeight, 255, flags);
}

void CDIB::DrawBlend(int xdst, int ydst, int widthdst, int heightdst, 
		int xsrc, int ysrc, int widthsrc, int heightsrc,
		void *dst, int dstWidth, int dstHeight, 
		void *src, int srcWidth, int srcHeight, 
		unsigned char alpha, unsigned int flags)
{
	/*if(flags & DrawFlagsPaint)
	{
		AfxDebugBreak();
		return;
	}*/
	if((widthdst == 0) || (heightdst == 0))
	{
		return;
	}
	int sx, sy, yp, xp, zy, izy, psi, psi2, pdi;
	DIB_AARGB *ps, *ps2, *pd;


	sx = (widthsrc << 16) / widthdst;
	sy = (heightsrc << 16) / heightdst;
	yp = (sy >> 1) - 0x8000;

	for(int y = 0; y < heightdst; y++, yp += sy)
	{
		if(flags & DrawFlagsReflectSrc)
		{
			psi = srcHeight - 1 - ysrc - (yp < 0 ? 0 : yp >> 16);
			if(psi < 0)
			{
				return;
			}
		}
		else
		{
			psi = ysrc + (yp < 0 ? 0 : yp >> 16);
			if(psi >= srcHeight)
			{
				return;
			}
		}
		psi2 = psi;
		if(psi2 < srcHeight - 1)
		{
			psi2++;
		}
		if(flags & DrawFlagsReflectDest)
		{
			pdi = dstHeight - 1 - ydst - y;
			if(pdi < 0)
			{
				return;
			}
		}
		else
		{
			pdi = ydst + y;
			if(pdi >= dstHeight)
			{
				return;
			}
		}

		ps = (DIB_AARGB*)((int)src + (psi * srcWidth + xsrc) * 4);
		ps2 = (DIB_AARGB*)((int)src + (psi2 * srcWidth + xsrc) * 4);
		pd = (DIB_AARGB*)((int)dst + (pdi * dstWidth + xdst) * 4);
		xp = (sx >> 1) - 0x8000;

		zy = yp < 0 ? 0 : yp & 0xffff;
		izy = yp < 0 ? 0x10000 : ((~yp) & 0xffff) + 1;

		for(int x = 0; x < widthdst; x++, xp += sx)
		{
			int dx = xp < 0 ? 0 : xp >> 16;
			int dx2 = dx;
			if(dx2 < srcWidth - 1)
			{
				dx2++;
			}
			int zx = xp < 0 ? 0 : xp & 0xffff;
			int w2 = (izy * zx) >> 16;
			int w1 = izy - w2;
			int w4 = (zy * zx) >> 16;
			int w3 = zy - w4;

			if(flags & DrawFlagsPaint)
			{	
				pd[x]->a = (unsigned char)((ps[dx]->a * w1 + ps[dx2]->a * w2 + ps2[dx]->a * w3 + ps2[dx2]->a * w4 + 0x8000) >> 16) * alpha / 255;
				pd[x]->r = (unsigned char)((ps[dx]->r * w1 + ps[dx2]->r * w2 + ps2[dx]->r * w3 + ps2[dx2]->r * w4 + 0x8000) >> 16) * pd[x]->a / 255;
				pd[x]->g = (unsigned char)((ps[dx]->g * w1 + ps[dx2]->g * w2 + ps2[dx]->g * w3 + ps2[dx2]->g * w4 + 0x8000) >> 16) * pd[x]->a / 255;
				pd[x]->b = (unsigned char)((ps[dx]->b * w1 + ps[dx2]->b * w2 + ps2[dx]->b * w3 + ps2[dx2]->b * w4 + 0x8000) >> 16) * pd[x]->a / 255;
			}
			else
			{
				pd[x]->a = (unsigned char)((ps[dx]->a * w1 + ps[dx2]->a * w2 + ps2[dx]->a * w3 + ps2[dx2]->a * w4 + 0x8000) >> 16) * alpha / 255;
				pd[x]->r = (unsigned char)((ps[dx]->r * w1 + ps[dx2]->r * w2 + ps2[dx]->r * w3 + ps2[dx2]->r * w4 + 0x8000) >> 16);
				pd[x]->g = (unsigned char)((ps[dx]->g * w1 + ps[dx2]->g * w2 + ps2[dx]->g * w3 + ps2[dx2]->g * w4 + 0x8000) >> 16);
				pd[x]->b = (unsigned char)((ps[dx]->b * w1 + ps[dx2]->b * w2 + ps2[dx]->b * w3 + ps2[dx2]->b * w4 + 0x8000) >> 16);
			}
		}
	}
}

void CDIB::AlphaBlend(CRect rect, unsigned char alpha, unsigned int flags)
{
	if(Ready())
	{
		AlphaBlend(rect.left, rect.top, rect.Width(), rect.Height(), 
			scan0, bi->bmiHeader.biWidth, bi->bmiHeader.biHeight, alpha, flags);
	}
}

void CDIB::AlphaBlend(int xdst, int ydst, int widthdst, int heightdst,
		void *scan0, int width, int, unsigned char alpha, unsigned int flags)
{
	for(int y = 0; y < heightdst; y++)
	{
		DIB_AARGB *p = (DIB_AARGB*)((int)scan0 + ((ydst + y) * width + xdst) * 4);
		for(int x = 0; x < widthdst; x++)
		{
			unsigned char a = p[x]->a * alpha / 255;
			if(flags & DrawFlagsPaint)
			{
				float k = (float)a / max(p[x]->a, 1);
				p[x]->r = (unsigned char)(p[x]->r * k);
				p[x]->g = (unsigned char)(p[x]->g * k);
				p[x]->b = (unsigned char)(p[x]->b * k);
			}
			p[x]->a = a;
		}
	}
}
