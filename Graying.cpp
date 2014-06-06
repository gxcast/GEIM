#include "Graying.h"

// math
#include <math.h>

/////////////////////////////////////////////////////////
/**< jet color map generic */
bool Graying::ColorMap(double d, PST_RGB pClr)
{
	if (pClr == nullptr)
	{
		wxASSERT_MSG(false, _T("ColorMap parameter is nullptr."));
		return false;
	}
	if (d < 0)
		d = -d;
	if (0 <= d && d <= 1.0)
		d *= 1000;
	else if(d > 1.0)
	{
		pClr->r = 255u;
		pClr->g = 255u;
		pClr->b = 255u;
		return false;
	}

	if (0 <= d && d <= 117.0 )
		pClr->b = (unsigned char)(d + 138);
	else if (117.0 < d && d <= 372.0)
	{
		pClr->g = (unsigned char)(d - 117);
		pClr->b = 255;
	}
	else if (372.0 < d && d <= 627.0)
	{
		pClr->r = (unsigned char)(d - 372);
		pClr->g = 255;
		pClr->b = (unsigned char)(627 - d);
	}
	else if (627.0 < d && d <= 882)
	{
		pClr->r = 255;
		pClr->g = (unsigned char)(882 - d);
	}
	else if (882.0 < d && d <= 1000)
		pClr->r = (unsigned char)(1137 - d);

	return true;
}
ST_RGB Graying::ColorMap(double d)
{
	ST_RGB rgb;
	ColorMap(d, &rgb);
	return rgb;
}

// Abandon image's color
bool Graying::Gray(EffectPar& parEft)
{
	unsigned char* pImgSrc = parEft.Input();
	// image valide
	if (pImgSrc == nullptr)
		return false;
	// image date
	int iN = parEft.PixNum();

	unsigned char*  pDes = nullptr;
	if (parEft.Modify())
		pDes = pImgSrc;
	else
	{
		pDes = parEft.GetCache();
		memcpy(pDes, pImgSrc, (size_t)iN*3);
	}
	unsigned char* pD = pDes;
	for (int n = 0; n < iN; ++n)
	{
		double dG = pD[0]*0.297 + pD[1]*0.589 + pD[2]*0.114;
		if (dG < 0.0)
			dG = 0.0;
		else if (dG > 255.0)
			dG = 255.0;
		pD[0] = (unsigned char)dG;
		pD[1] = (unsigned char)dG;
		pD[2] = (unsigned char)dG;
		pD += 3;
	}
	// the result
	parEft.Output(pDes);

	return true;
}

/**< invert gray */
bool Graying::Invert(EffectPar& parEft)
{
	unsigned char* pImgSrc = parEft.Input();
	// image valide
	if (pImgSrc == nullptr)
		return false;
	// image date
	int iN = parEft.PixNum();

	unsigned char*  pDes = nullptr;
	if (parEft.Modify())
		pDes = pImgSrc;
	else
	{
		pDes = parEft.GetCache();
		memcpy(pDes, pImgSrc, (size_t)iN*3);
	}
	unsigned char* pD = pDes;
	for (int n = 0; n < iN; ++n)
	{
		pD[0] ^= 0xff;
		pD[1] ^= 0xff;
		pD[2] ^= 0xff;
		pD += 3;
	}
	// the result
	parEft.Output(pDes);

	return true;
}

/**< image1 - image2 */
bool Graying::Subtract(EffectPar& parEft)
{
	unsigned char* pImgSrc1 = parEft.Input();
	unsigned char* pImgSrc2 = parEft.Input2();
	// image valide
	if (pImgSrc1 == nullptr || pImgSrc2 == nullptr)
		return false;
	// image date
	int iN = parEft.PixNum();

	unsigned char*  pDes = nullptr;
	if (parEft.Modify())
		pDes = pImgSrc1;
	else if(parEft.Modify2())
		pDes = pImgSrc2;
	else
		pDes = parEft.GetCache();
	// suctract and static the min/max value
	unsigned char* pD = pDes;
	unsigned char* pSrc1 = pImgSrc1;
	unsigned char* pSrc2 = pImgSrc2;
	short iMin = 0x7fff, iMax = -0x7fff;
	for (int n = 0; n < iN; ++n)
	{
		short* pDelta = (short*)pD;
		pDelta[0] = pSrc1[2] - pSrc2[2];
		if (pDelta[0] < iMin)
			iMin = pDelta[0];
		if (pDelta[0] > iMax)
			iMax = pDelta[0];

		pD += 3;
		pSrc1 += 3;
		pSrc2 += 3;
	}
	// modify the result
	double dScale = 255.0/(iMax - iMin);
	pD = pDes;
	for (int n = 0; n < iN; ++n)
	{
		short* pDelta = (short*)pD;
		double dDelta= (pDelta[0] - iMin)*dScale;
		if (dDelta < 0.0)
			dDelta = 0.0;
		else if (dDelta > 255.0)
			dDelta = 255.0;
		pD[0] = (unsigned char)dDelta;
		pD[1] = (unsigned char)dDelta;
		pD[2] = (unsigned char)dDelta;

		pD += 3;
	}
	// the result
	if (parEft.Modify() && parEft.Modify2())
	{
		memcpy(pImgSrc2, pImgSrc1, (size_t)iN*3);
	}
	parEft.Output(pDes);

	return true;
}

/**< image1 + iChange */
bool Graying::Subtract(EffectPar& parEft, int iCg)
{
	unsigned char* pImgSrc1 = parEft.Input();
	// image pixel num
	int iN = parEft.PixNum();
	// image valide
	if (pImgSrc1 == nullptr)
		return false;
	unsigned char*  pDes = nullptr;
	if (parEft.Modify())
		pDes = pImgSrc1;
	else
		pDes = parEft.GetCache();

	unsigned char* pD = pDes;
	unsigned char* pSrc1 = pImgSrc1;
	for (int n = 0; n < iN; ++n)
	{
		int iPix = pSrc1[0] + iCg;
		if (iPix < 0)
			iPix = 0;
		else if (iPix > 255)
			iPix = 255;
		pD[0] = (unsigned char)iPix;
		pD[1] = (unsigned char)iPix;
		pD[2] = (unsigned char)iPix;

		pSrc1 += 3;
		pD += 3;
	}

	parEft.Output(pDes);
	return true;
}

/**< gray normalization */
bool Graying::Normalize(EffectPar& parEft, double dE0, double dV0)
{
	PST_RGB pSrc = (PST_RGB)parEft.Input();
	if (pSrc == nullptr)
		return false;
	// pixel number
	int iN = parEft.PixNum();

	// dest image
	PST_RGB pDes = nullptr;
	if (parEft.Modify())
		pDes = pSrc;
	else
		pDes = (PST_RGB)parEft.GetCache();

	// caculate ecpectation and variance
	double dE = 0.0;
	double dV = 0.0;
	dV0 = dV0*dV0;
	for (int i = 0; i < iN; ++i)
		dE += pSrc[i].r;
	dE /= iN;
	for (int i = 0; i < iN; ++i)
	{
		double dT = pSrc[i].r - dE;
		dV += dT*dT;
	}
	dV /= iN;

	// normalization
	for (int i = 0; i < iN; ++i)
	{
		double dG = pSrc[i].r;
		double dT = dG - dE;
		dT = sqrt(dT*dT*dV0/dV);
		if (dG >= dE)
			dT = dE0 + dT;
		else
			dT = dE0 - dT;
		if (dT > 255.0)
			dT = 255.0;
		else if (dT < 0.0)
			dT = 0.0;

		pDes[i].r = (unsigned char)dT;
		pDes[i].g = (unsigned char)dT;
		pDes[i].b = (unsigned char)dT;
	}

	parEft.Output((unsigned char*)pDes);
	return true;
}
