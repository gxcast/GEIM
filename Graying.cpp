#include "Graying.h"

/////////////////////////////////////////////////////////
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
