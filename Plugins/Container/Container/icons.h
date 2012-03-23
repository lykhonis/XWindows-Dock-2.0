#ifndef ICONS_H
#define ICONS_H

#include <afxwin.h>
#include <shtypes.h>
#include <commoncontrols.h>
#include "DIB.h"
#include "ShellIO.h"

namespace Icons
{
	typedef struct {
		unsigned char bWidth;
		unsigned char bHeight;
		unsigned char bColorCount;
		unsigned char bReserved;
		short wPlanes;
		short wBitCount;
		unsigned int dwBytesInRes;
		unsigned int dwImageOffset;
	} ICONDIRENTRY;

	typedef struct {
		short idReserved; // = 0 ?
		short idType; // = 1 ?
		short idCount;
	} ICONDIR;

	int GetIconSize(CString fileName);
	HICON GetIcon(CString fileName);
	void GetDIBFromIcon(HICON icon, CDIB *dib);
	bool GetIcon(CString fileName, CDIB *dib);
	HICON GetIcon(HMODULE hModule, CString resName);
	HICON GetIcon(CString fileName, CString resName);
	HICON GetSystemIcon(UINT uType);
}

#endif /* ICONS_H */