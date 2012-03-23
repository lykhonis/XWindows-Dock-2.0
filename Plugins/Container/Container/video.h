#ifndef VIDEO_H
#define VIDEO_H

#include <afxwin.h>
#include "atlbase.h"
#include <dshow.h>
#include <objbase.h>

#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__

#include "qedit.h"
#include "DIB.h"

namespace Video
{
	bool GetFrame(CString fileName, CDIB *dib, double delta);
};

#endif /* VIDEO_H */