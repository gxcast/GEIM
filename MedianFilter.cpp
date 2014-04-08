#include "MedianFilter.h"

/////////////////////////////////////////////////////////
/**< perform filter */
bool MedianFilter::Do(wxImage& imgSrc, wxImage& imgDes,  int iType)
{
	// two image must be valid and has the same size
	if (!imgSrc.IsOk() || !imgDes.IsOk() || imgSrc.GetSize() != imgDes.GetSize())
		return false;

	int iW = imgSrc.GetWidth();
	int iH = imgSrc.GetHeight();

	// Create Template
	int iWidth = 0;	// template width
	int iSize = 0;		// template size = width^2
	unsigned char* pTmpl = nullptr;	// template
	unsigned char* pBlock = nullptr;	// obtain block
	switch(iType)
	{
	case 0:	// 3X3
	{
		iWidth = 3;
		iSize = 9;
		// image's validion
		if (iW < iWidth || iH< iWidth)
		{
			wxASSERT_MSG(false, _T("MedianFilter, image's size is too small."));
			return false;
		}
		pTmpl = new unsigned char [iSize];
		memset(pTmpl, 0xff, iSize);	// filled with 1
		pBlock = new unsigned char [iSize];
		memset(pBlock, 0, iSize);
	}
	break;
	case 1:	// 5X5
	{
		iWidth = 5;
		iSize = 25;
		// image's validion
		if (iW < iWidth || iH< iWidth)
		{
			wxASSERT_MSG(false, _T("MedianFilter, image's size is too small."));
			return false;
		}
		pTmpl = new unsigned char [iSize];
		memset(pTmpl, 0xff, iSize);	// filled with 1
		pBlock = new unsigned char [iSize];
		memset(pBlock, 0, iSize);
	}
	break;
	case 2:	// 7X7
	{
		iWidth = 7;
		iSize = 49;
		// image's validion
		if (iW < iWidth || iH< iWidth)
		{
			wxASSERT_MSG(false, _T("MedianFilter, image's size is too small."));
			return false;
		}
		pTmpl = new unsigned char [iSize];
		memset(pTmpl, 0xff, iSize);	// filled with 1
		pBlock = new unsigned char [iSize];
		memset(pBlock, 0, iSize);
	}
	break;
	default:
		wxASSERT_MSG(false, _T("Median Filter type invalide."));
		return false;
	}

	// filter
	unsigned char* pSrc = imgSrc.GetData();
	unsigned char* pDes = imgDes.GetData();
	int iPadding = iWidth/2;

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
					pBlock[iBdex] = pBP[0] & pTmpl[iBdex];
					++iBdex;
					pBP += 3;
				}
				pBL += iW*3;
			}

			// obtai block's median
			for (int i = 0; i < iSize/2 + 1; ++i)
			{
				// get max item index
				unsigned char tmp = i;
				for (int j = i + 1; j < iSize; ++j)
				{
					if (pBlock[tmp] < pBlock[j])
						tmp = j;
				}
				// switch with max
				if (tmp != i)
				{
					unsigned char t = pBlock[tmp];
					pBlock[tmp] = pBlock[i];
					pBlock[i] = t;
				}
			}
			unsigned char median = pBlock[iSize/2];

			// filter
			pPix[0] = median;
			pPix[1] = median;
			pPix[2] = median;

			// next pix
			pPix += 3;
		}

		// next line
		pLine += iW*3;
	}

	// release mem
	if (pTmpl != nullptr)
	{
		delete [] pTmpl;
		pTmpl = nullptr;
	}
	if (pBlock != nullptr)
	{
		delete [] pBlock;
		pBlock = nullptr;
	}
	return true;
}

