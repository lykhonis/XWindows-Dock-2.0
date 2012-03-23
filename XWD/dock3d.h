#ifndef DOCK3D_H
#define DOCK3D_H

#include <afxwin.h>
#include <math.h>
#include <algorithm>
#include "consts.h"
#include "DIB.h"

namespace Dock3D
{
	bool DrawImage(CDIB *dst, CRect rect, int offsetX, CDIB *src);
}

#endif /* DOCK3D_H */