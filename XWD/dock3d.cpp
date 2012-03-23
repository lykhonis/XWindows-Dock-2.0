#include "dock3d.h"

void DrawDockD3Image(CDIB *dst, CRect rect, int offsetX, CDIB *src)
{
	int x, y, index, x1, x2;
	float y_ratio = (float)(src->Height() - 1) / rect.Height();
	float x_ratio, x_diff, y_diff;
	DIB_ARGB *pd, *psa, *psb, *psc, *psd;

	for(int i = 0; i < rect.Height(); i++)
	{
		x1 = (int)(rect.left + 1 + offsetX * (1 - (1.0f + i) / rect.Height()));
		x2 = (int)(rect.right - 1 - offsetX * (1 - (1.0f + i) / rect.Height()));

		x_ratio = (float)src->Width() / (x2 - x1);
		pd = (DIB_ARGB*)((int)dst->scan0 + ((rect.bottom - 1 - i) * dst->Width() + x1) * 4);

		for(int j = 0; j < x2 - x1; j++, pd++)
		{
			x = (int)(x_ratio * j);
			y = (int)(y_ratio * i);
			x_diff = (x_ratio * j) - x;
			y_diff = (y_ratio * i) - y;
			index = y * src->Width() + x;
			psa = (DIB_ARGB*)((int)src->scan0 + (index) * 4);
			psb = (DIB_ARGB*)((int)src->scan0 + (index + 1) * 4);
			psc = (DIB_ARGB*)((int)src->scan0 + (index + src->Width()) * 4);
			psd = (DIB_ARGB*)((int)src->scan0 + (index + src->Width() + 1) * 4);

			pd->a = (unsigned char)(
				psa->a * (1 - x_diff) * (1 - y_diff) +
				psb->a * x_diff * (1 - y_diff) +
				psc->a * y_diff * (1 - x_diff) +
				psd->a * x_diff * y_diff);

			pd->b = (unsigned char)(
				psa->b * (1 - x_diff) * (1 - y_diff) +
				psb->b * x_diff * (1 - y_diff) +
				psc->b * y_diff * (1 - x_diff) +
				psd->b * x_diff * y_diff) * pd->a / 255;

			pd->g = (unsigned char)(
				psa->g * (1 - x_diff) * (1 - y_diff) +
				psb->g * x_diff * (1 - y_diff) +
				psc->g * y_diff * (1 - x_diff) +
				psd->g * x_diff * y_diff) * pd->a / 255;

			pd->r = (unsigned char)(
				psa->r * (1 - x_diff) * (1 - y_diff) +
				psb->r * x_diff * (1 - y_diff) +
				psc->r * y_diff * (1 - x_diff) +
				psd->r * x_diff * y_diff) * pd->a / 255;
		}
	}
}

bool Dock3D::DrawImage(CDIB *dst, CRect rect, int offsetX, CDIB *src)
{
	const int scale = 2;

	CDIB tmp;
	tmp.Resize(rect.Width() * scale, rect.Height() * scale);
	if (!tmp.Ready())
	{
		return false;
	}

	DrawDockD3Image(&tmp, tmp.Rect(), offsetX * scale, src);

	rect.left++;
	//rect.right--;

	dst->Draw(rect, tmp.Rect(), &tmp);

	/*int x, y, index, x1, x2;
	float y_ratio = (float)(src->Height() - 1) / rect.Height();
	float x_ratio, x_diff, y_diff;
	DIB_ARGB *pd, *psa, *psb, *psc, *psd;

	for(int i = 0; i < rect.Height(); i++)
	{
		x1 = (int)(rect.left + 1 + offsetX * (1 - (1.0f + i) / rect.Height()));
		x2 = (int)(rect.right - 1 - offsetX * (1 - (1.0f + i) / rect.Height()));

		x_ratio = (float)src->Width() / (x2 - x1);
		pd = (DIB_ARGB*)((int)dst->scan0 + ((rect.bottom - 1 - i) * dst->Width() + x1) * 4);

		for(int j = 0; j < x2 - x1; j++, pd++)
		{
			x = (int)(x_ratio * j);
			y = (int)(y_ratio * i);
			x_diff = (x_ratio * j) - x;
			y_diff = (y_ratio * i) - y;
			index = y * src->Width() + x;
			psa = (DIB_ARGB*)((int)src->scan0 + (index) * 4);
			psb = (DIB_ARGB*)((int)src->scan0 + (index + 1) * 4);
			psc = (DIB_ARGB*)((int)src->scan0 + (index + src->Width()) * 4);
			psd = (DIB_ARGB*)((int)src->scan0 + (index + src->Width() + 1) * 4);

			pd->a = (unsigned char)(
				psa->a * (1 - x_diff) * (1 - y_diff) +
				psb->a * x_diff * (1 - y_diff) +
				psc->a * y_diff * (1 - x_diff) +
				psd->a * x_diff * y_diff);

			pd->b = (unsigned char)(
				psa->b * (1 - x_diff) * (1 - y_diff) +
				psb->b * x_diff * (1 - y_diff) +
				psc->b * y_diff * (1 - x_diff) +
				psd->b * x_diff * y_diff) * pd->a / 255;

			pd->g = (unsigned char)(
				psa->g * (1 - x_diff) * (1 - y_diff) +
				psb->g * x_diff * (1 - y_diff) +
				psc->g * y_diff * (1 - x_diff) +
				psd->g * x_diff * y_diff) * pd->a / 255;

			pd->r = (unsigned char)(
				psa->r * (1 - x_diff) * (1 - y_diff) +
				psb->r * x_diff * (1 - y_diff) +
				psc->r * y_diff * (1 - x_diff) +
				psd->r * x_diff * y_diff) * pd->a / 255;
		}
	}

	// Border's antialiasing
	DIB_ARGB *pd1, *pd2;
	int px2 = 0, py2 = 0, px1 = 0, py1 = 0;

	for(int i = 0; i <= rect.Height(); i++)
	{
		if(i == 0)
		{
			px1 = (int)(rect.left + 1 + offsetX * (1 - (1.0f + i) / rect.Height()));
			py1 = i;
			px2 = (int)(rect.right - 1 - offsetX * (1 - (1.0f + i) / rect.Height()));
			py2 = i;
		}
		else
		{
			x1 = (int)(rect.left + 1 + offsetX * (1 - (1.0f + i) / rect.Height()));
			if(x1 < px1)
			{
				pd1 = (DIB_ARGB*)((int)dst->scan0 + ((rect.bottom - 1 - py1) * dst->Width() + px1 + 1) * 4);
				pd2 = (DIB_ARGB*)((int)dst->scan0 + ((rect.bottom - 1 - py1 + 1) * dst->Width() + px1) * 4);
				pd = (DIB_ARGB*)((int)dst->scan0 + ((rect.bottom - 1 - py1) * dst->Width() + px1) * 4);

				for(int y = py1; y < i; y++, pd1 -= dst->Width(), pd2 -= dst->Width(), pd -= dst->Width())
				{
					pd->a = (unsigned char)((pd1->a + pd2->a) / 2 * (1 + y - py1) / (i - py1));
					pd->r = (pd1->r * 255 / max(pd1->a, 1) + pd2->r * 255 / max(pd2->a, 1)) / 2 * pd->a / 255;
					pd->g = (pd1->g * 255 / max(pd1->a, 1) + pd2->g * 255 / max(pd2->a, 1)) / 2 * pd->a / 255;
					pd->b = (pd1->b * 255 / max(pd1->a, 1) + pd2->b * 255 / max(pd2->a, 1)) / 2 * pd->a / 255;
				}
				px1 = x1;
				py1 = i;
			}

			x2 = (int)(rect.right - 1 - offsetX * (1 - (1.0f + i) / rect.Height()));
			if(x2 > px2)
			{
				pd1 = (DIB_ARGB*)((int)dst->scan0 + ((rect.bottom - 1 - py2) * dst->Width() + px2 - 1) * 4);
				pd2 = (DIB_ARGB*)((int)dst->scan0 + ((rect.bottom - 1 - py2 + 1) * dst->Width() + px2) * 4);
				pd = (DIB_ARGB*)((int)dst->scan0 + ((rect.bottom - 1 - py2) * dst->Width() + px2) * 4);

				for(int y = py2; y < i; y++, pd1 -= dst->Width(), pd2 -= dst->Width(), pd -= dst->Width())
				{
					pd->a = (unsigned char)((pd1->a + pd2->a) / 2 * (1 + y - py2) / (i - py2));
					pd->r = (pd1->r * 255 / max(pd1->a, 1) + pd2->r * 255 / max(pd2->a, 1)) / 2 * pd->a / 255;
					pd->g = (pd1->g * 255 / max(pd1->a, 1) + pd2->g * 255 / max(pd2->a, 1)) / 2 * pd->a / 255;
					pd->b = (pd1->b * 255 / max(pd1->a, 1) + pd2->b * 255 / max(pd2->a, 1)) / 2 * pd->a / 255;
				}
				px2 = x2;
				py2 = i;
			}
		}
	}*/

	return true;
}
