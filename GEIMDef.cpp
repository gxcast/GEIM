#include "GEIMDef.h"
#include <cfloat>

/** \brief Calculates the squared Euclidian distance between two feature descriptors.
 *
 * \param f1 ST_FEATURE*	first feature
 * \param f2 ST_FEATURE*	second feature
 * \return double	Returns the squared Euclidian distance between the descriptors of
 *    \a f1 and \a f2.
 *
 */
double feat_dist_sq( ST_FEATURE *f1, ST_FEATURE *f2 )
{
	static double WEIGHT[FEATURE_MAX_D] = {0.2, 0.2, 0.1, 0.15, 0.05, 0.1, 0.2};
	double diff, dsq = 0;
	double* descr1, * descr2;
	int i, n;

	n = f1->n;
	if( f2->n != n )
		return DBL_MAX;
	descr1 = f1->descr;
	descr2 = f2->descr;

	for( i = 0; i < n; i++ )
	{
		diff = descr1[i] - descr2[i];
		dsq += WEIGHT[i]*diff*diff;
	}
	return dsq;
}

bool img_draw_line(wxImage &img, const wxRect &rc, const ST_RGB &clr)
{
	int iW = 0, iH = 0;
	ST_RGB *pImg = nullptr;
	double slope = 0.0;

	if (!img.IsOk() || (rc.width == 0 && rc.height == 0))
		return false;

	iW = img.GetWidth();
	iH = img.GetHeight();
	pImg = (ST_RGB *)img.GetData();

	if (abs(rc.width) >= abs(rc.height))	// width priority
	{
		int x = rc.x;
		int y = 0;
		slope = rc.height/(double)rc.width;
		if (rc.width < 0)	// --
		{
			for (int tx = 0; tx > rc.width; --tx)
			{
				y = (int)(tx*slope + rc.y);
				if (x >= 0 && x < iW && y >= 0 && y < iH)
					pImg[y*iW+x] = clr;
				--x;
			}
		}
		else	// ++
		{
			for (int tx = 0; tx < rc.width; ++tx)
			{
				y = (int)(tx*slope + rc.y);
				if (x >= 0 && x < iW && y >= 0 && y < iH)
					pImg[y*iW+x] = clr;
				++x;
			}
		}
	}
	else	// height priority
	{
		int x = 0;
		int y = rc.y;
		slope = rc.width/(double)rc.height;
		if (rc.height < 0)	// --
		{
			for (int ty = 0; ty > rc.height; --ty)
			{
				x = (int)(ty*slope + rc.x);
				if (x >= 0 && x < iW && y >= 0 && y < iH)
					pImg[y*iW+x] = clr;
				--y;
			}
		}
		else	// ++
		{
			for (int ty = 0; ty < rc.height; ++ty)
			{
				x = (int)(ty*slope + rc.x);
				if (x >= 0 && x < iW && y >= 0 && y < iH)
					pImg[y*iW+x] = clr;
				++y;
			}
		}
	}
	return true;
}
