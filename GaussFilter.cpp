#include "GaussFilter.h"


/////////////////////////////////////////////////////////
// template and it's value sum
const int GaussFilter::TMPL_3 [9] =
{
	1, 2, 1,
	2, 4, 2,
	1, 2, 1
};
const int GaussFilter::TMPL_5[25] =
{
	1, 1, 2, 1, 1,
	1, 2, 4, 2, 1,
	2, 4, 8, 4, 2,
	1, 2, 4, 2, 1,
	1, 1, 2, 1, 1
};
const int GaussFilter::TMPL_7[49] =
{
	-4, -6, -12, -14, -12, -6, -4,
	-6, 9, 18, 21, 18, 9, -6,
	-12, 18, 36, 42, 36, 18, -12,
	-14, 21, 42, 49, 42, 21, -14,
	-12, 18, 36, 42, 36, 18, -12,
	-6, 9, 18, 21, 18, 9, -6,
	-4, -6, -12, -14, -12, -6, -4
};

/////////////////////////////////////////////////////////
/**< perform the gaussfilter */
bool GaussFilter::Do(EffectPar& parEft, int iType)
{
	unsigned char* pSrc = parEft.Input();
	// image valide
	if (pSrc == nullptr)
		return false;

	int iW = parEft.Width();
	int iH = parEft.Height();

	// Create Template
	int iWidth = 0;	// template width
	int iTV = 0;							// template value sum
	const int* pTmpl = nullptr;	// template
	switch(iType)
	{
	case 0:	// 3X3
		iWidth = 3;
		pTmpl = TMPL_3;
		iTV = TMPL_3_V;
		break;
	case 1:	// 5X5
		iWidth = 5;
		pTmpl = TMPL_5;
		iTV = TMPL_5_V;
		break;
	case 2:	// 7X7
		iWidth = 7;
		pTmpl = TMPL_7;
		iTV = TMPL_7_V;
		break;
	default:
		wxASSERT_MSG(false, _T("Median Filter type invalide."));
		return false;
	}
	// image's validion
	if (iW < iWidth || iH< iWidth)
	{
		wxASSERT_MSG(false, _T("MedianFilter, image's size is too small."));
		return false;
	}

	// filter
	unsigned char* pDes = parEft.GetCache();
	memcpy(pDes, pSrc, (size_t)parEft.PixNum()*3);	// image border wont be processed
	int iPadding = iWidth/2;
	int iSte = 0;			// 卷积加权和

	unsigned char* pLine = pDes + iPadding*iW*3;
	for (int y = iPadding; y < iH - iPadding; ++y)
	{
		unsigned char* pPix = pLine + iPadding*3;
		for (int x = iPadding; x < iW - iPadding; ++x)
		{
			// ===obtain block===
			unsigned char* pBL = pSrc + (y-iPadding)*iW*3;
			int iBdex = 0;
			for (int n = 0; n < iWidth; ++n)
			{
				unsigned char* pBP = pBL + (x-iPadding)*3;
				for (int m = 0; m < iWidth; ++m)
				{
					iSte += pBP[0] * pTmpl[iBdex];
					++iBdex;
					pBP += 3;
				}
				pBL += iW*3;
			}

			// weighted mean
			iSte /= iTV;
			if (iSte < 0)
				iSte = 0;
			else if (iSte > 255)
				iSte = 255;
			pPix[0] = (unsigned char)iSte;
			pPix[1] = (unsigned char)iSte;
			pPix[2] = (unsigned char)iSte;

			// next pix
			pPix += 3;
		}

		// next line
		pLine += iW*3;
	}

	// the result
	if (parEft.Modify())
	{
		// copy result to src
		memcpy(pSrc, pDes, (size_t)parEft.PixNum()*3);
		parEft.Output(pSrc);
		// recycle cache
		parEft.Recycle(pDes);
	}
	else
		parEft.Output(pDes);

	return true;
}

