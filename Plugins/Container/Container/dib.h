#ifndef DIB_H
#define DIB_H

#include <afxwin.h>
#include <gdiplus.h>

union DIB_ARGB
{
	unsigned int c;
	struct 
	{
		unsigned char b;
		unsigned char g;
		unsigned char r;
		unsigned char a;
	};
};
typedef union DIB_ARGB DIB_ARGB;
typedef DIB_ARGB DIB_AARGB[1];

enum DrawFlags
{
	DrawFlagsPaint			= 1 << 0,
	DrawFlagsReflectDest	= 1 << 1,
	DrawFlagsReflectSrc		= 1 << 2
};

class CDIB
{
public:
	CDIB();
	~CDIB();

	void Assign(CDIB *dib);
	void Assign(Gdiplus::Bitmap *dib);
	void Assign(int width, int height, void *dib);
	void Resize(int width, int height);
	void Load(CString fileName);
	void FreeImage();

	void Fill();
	void Fill(int x, int y, int width, int height);

	bool Ready();

	int Width();
	int Height();
	int Size();
	CRect Rect();

	void* Pixels(int x, int y);

	void ReflectVertical();

	void Blur(CRect workRect, CRect ignoreRect, int shadowSize);
	static void Blur(void *dst, int width, int height, CRect workRect, CRect ignoreRect, int shadowSize);

	void Draw(CRect rect, int srcX, int srcY, CDIB *dib, unsigned int flags = 0);
	void Draw(CRect dstRect, CRect srcRect, CDIB *dib, unsigned int flags = 0);
	void DrawBlend(CRect dstRect, CRect srcRect, CDIB *dib, unsigned char alpha, unsigned int flags = 0);
	void DrawTail(CRect dstRect, CRect srcRect, CDIB *dib, unsigned int flags = 0);

	static void Draw(int xdst, int ydst, int widthdst, int heightdst, 
		int xsrc, int ysrc, int widthsrc, int heightsrc,
		void *dst, int dstWidth, int dstHeight, 
		void *src, int srcWidth, int srcHeight, 
		unsigned int flags = 0);

	static void DrawBlend(int xdst, int ydst, int widthdst, int heightdst, 
		int xsrc, int ysrc, int widthsrc, int heightsrc,
		void *dst, int dstWidth, int dstHeight, 
		void *src, int srcWidth, int srcHeight, 
		unsigned char alpha, unsigned int flags = 0);

	void AlphaBlend(CRect rect, unsigned char alpha, unsigned int flags = 0);

	static void AlphaBlend(int xdst, int ydst, int widthdst, int heightdst,
		void *scan0, int width, int height, unsigned char alpha, unsigned int flags = 0);

public:
	HDC dc;
	BITMAPINFO *bi;
	DIB_ARGB *scan0;
	HBITMAP bitmap;
	Gdiplus::Bitmap *bmp;

private:
	HBITMAP oldBitmap;
};

#endif /* DIB_H */