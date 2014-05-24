#include "SpotDt.h"
#include "Graying.h"
#include "MedianFilter.h"
#include "GaussFilter.h"
#include "Morphology.h"

#include <deque>
#include <stack>

/**< initial label id */
#define LABEL_INITID 1u

SpotDt::SpotDt()
{
}

SpotDt::~SpotDt()
{
}

/**< detect main function */
bool SpotDt::DtMain(PST_DTPARAM pDtParam, EffectPar* pParEft)
{
	// check the parameter
	if (pDtParam == nullptr || pParEft == nullptr)
	{
		wxASSERT_MSG(false, _T("DtMain param is null"));
		return false;
	}
	// log the parameter
	m_pDtParam = pDtParam;
	m_pParEft = pParEft;
	m_pImgIn = m_pParEft->Input();
	m_bMdfyIn = m_pParEft->Modify();
	// if can't  modify in image, create a copy, and, can modify the copy
	if (!m_bMdfyIn)
	{
		unsigned char* pImgCah = m_pParEft->GetCache();
		memcpy(pImgCah, m_pImgIn, (size_t)m_pParEft->PixNum()*3);
		m_pParEft->Input(pImgCah, true);	// next modify the input
	}

	// --------------------image graylise--------------------
	Graying::Gray(*m_pParEft);

	// --------------------iamge filter--------------------
	// median filter
	if (m_pDtParam->iMedianFlt >= 0)
		MedianFilter::Do(*m_pParEft, m_pDtParam->iMedianFlt);
	// Gaussian filter
	if (m_pDtParam->iGaussFlt >= 0)
		GaussFilter::Do(*m_pParEft, m_pDtParam->iGaussFlt);

	// --------------------background remove--------------------
	// none

	// --------------------Water Shed--------------------
	unsigned char* pImgO = m_pParEft->Input();
	WaterShed(pImgO);

	return true;
}

/**< perform the water shed algorithm */
bool SpotDt::WaterShed(unsigned char* pImg)
{
	// --------------------Morphological Degree--------------------
	m_pParEft->Input(pImg, false);
	m_pParEft->Modify(false);
	Morphology::Grading(*m_pParEft, 1);
	unsigned char* pImgMD = m_pParEft->Output();

	// --------------------WS of Min & distance--------------------
	unsigned char* pLabel = m_pParEft->GetCache();
	unsigned char* pShed = m_pParEft->GetCache();
	memset(pLabel, 0, m_pParEft->PixNum()*3);
	memset(pShed, 0, m_pParEft->PixNum()*3);
	MinDistWS(pImgMD, pLabel, pShed);

	return true;
}

/**<  */
bool SpotDt::MinDistWS(unsigned char* pImg,
                       unsigned char* pLabel,
                       unsigned char* pShed)
{
	bool bRet = true;
	int iDeep = -2;	// extended min-value deepth
	int iBWType = 8;	// connected-domain type: 8 or 4

	unsigned int* pRegin = nullptr;
	int* pDist = nullptr;

	if (pImg == nullptr || pLabel == nullptr || pShed == nullptr)
	{
		wxASSERT_MSG(false, _T("SpotDt::MinDistWS parameter is nullptr."));
		return false;
	}

	// extended min-value, that is, inner label
	if (!ExtendedMin(pImg, pLabel, iDeep))
		return false;

	// inner label number, i.e., connected-domain number, ensure the inner-label is valid
	int iN = m_pParEft->PixNum();
	pRegin = new unsigned int[iN];
	memset(pRegin, 0, sizeof(unsigned int)*iN);
	unsigned int nBWNum = 0;
	if (!Bwlabel(pLabel, iBWType, pRegin, &nBWNum) || nBWNum == 0)
	{
		bRet = false;
		goto _MinDistWS_end;
	}

	// inner-label distance
	pDist = (int*)pRegin;
	if (!Bwdist(pLabel, pDist))
	{
		bRet = false;
		goto _MinDistWS_end;
	}

	// distance water shed
	if (!Bwshed(pDist, pShed))
	{
		bRet = false;
		goto _MinDistWS_end;
	}

_MinDistWS_end:
	if (pRegin != nullptr)
		delete [] pRegin;
	return bRet;
}

/**< extended min-value, that is, inner label */
bool SpotDt::ExtendedMin(unsigned char* pImg,
                         unsigned char* pLabelIn,
                         int iDeep)
{
	bool bRet = true;
	unsigned char* pImgDp = nullptr;

	// unvert
	m_pParEft->Input(pImg, true);
	Graying::Invert(*m_pParEft);

	// change deep
	m_pParEft->Input(pImg, false);
	Graying::Subtract(*m_pParEft, iDeep);
	pImgDp = m_pParEft->Output();

	// ReConstruct
	if (!ReConstruct(pImgDp, pImg))
	{
		bRet = false;
		goto _ExtendedMin_end;
	}

	// local max-value
	if (!LocalMax(pImgDp, pLabelIn))
	{
		bRet = false;
		goto _ExtendedMin_end;
	}

_ExtendedMin_end:
	// recycle the cache
	if (pImgDp != nullptr)
		m_pParEft->Recycle(pImgDp);
	return bRet;
}

/**<  */
bool SpotDt::ReConstruct(unsigned char* pImg,
                         unsigned char* pMask)
{
	if (pImg == nullptr || pMask == nullptr)
	{
		wxASSERT_MSG(false, _T("SpotDt::ReConstruct parameter is nullptr"));
		return false;
	}

	int iW = m_pParEft->Width();
	int iH = m_pParEft->Height();
	ST_NEIBOR3 stNeibor3;

	// forward
	for (int y = 0; y < iH; ++y)
	{
		for (int x = 0; x < iW; ++x)
		{
			// get the 3x3 neighbor, the first is current pix
			if(!GetNeibor3(pImg, x, y, &stNeibor3))
				return false;
			// get the max in 3x3
			int iMax = 0;
			for (int i = 0; i < stNeibor3.num; ++i)
			{
				if (iMax < stNeibor3.va[i])
					iMax = stNeibor3.va[i];
			}
			// alter current pix
			int iOffset = stNeibor3.dx[0];
			if (iMax > pMask[iOffset])	// iMax < Mask
				iMax = pMask[iOffset];
			unsigned char* pCur = pImg + stNeibor3.dx[0];
			pCur[0] = (unsigned char)iMax;
		}
	}

	// backward
	std::deque<int> dqIter;
	for (int y = iH -1; y >= 0; --y)
	{
		for (int x = iW -1; x >= 0; --x)
		{
			// get the 3x3 neighbor, the first is current pix
			if(!GetNeibor3(pImg, x, y, &stNeibor3))
				return false;
			// get the max in 3x3
			int iMax = 0;
			for (int i = 0; i < stNeibor3.num; ++i)
			{
				if (iMax < stNeibor3.va[i])
					iMax = stNeibor3.va[i];
			}
			// alter current pix
			int iOffset = stNeibor3.dx[0];
			if (iMax > pMask[iOffset])	// iMax < Mask
				iMax = pMask[iOffset];
			unsigned char* pCur = pImg + iOffset;
			pCur[0] = (unsigned char)iMax;
			pCur[1] = (unsigned char)iMax;
			pCur[2] = (unsigned char)iMax;

			// init iterater
			for (int i = 1; i < stNeibor3.num; ++i)
			{
				if (stNeibor3.va[i] < pCur[0])
				{
					int iT = stNeibor3.dx[i];
					if (pImg[iT] < pMask[iT])
					{
						dqIter.push_back(iOffset);
						break;
					}
				}
			}

		}
	}

	// iterater
	while (!dqIter.empty())
	{
		int iOffset = dqIter.front();
		dqIter.pop_front();
		// get 3x3 neighbor
		int y = iOffset/iW;
		int x = iOffset%iW;
		if(!GetNeibor3(pImg, x, y, &stNeibor3))
			return false;
		//
		for (int i = 1; i < stNeibor3.num; ++i)
		{
			int iT = stNeibor3.dx[i];
			if (pImg[iT] < pImg[iOffset] && pImg[iT] != pMask[iT])
			{
				if (pImg[iOffset] < pMask[iT])
					pImg[iT] = pImg[iOffset];
				else
					pImg[iT] = pMask[iT];
				pImg[iT+1] = pImg[iT];
				pImg[iT+2] = pImg[iT];

				dqIter.push_back(iT);
				break;
			}
		}
	}
	return true;
}

bool SpotDt::GetNeibor3(unsigned char* pImg, int x, int y,
                        PST_NEIBOR3 pNeiB)
{
	if (pImg == nullptr || pNeiB == nullptr)
	{
		wxASSERT_MSG(false, _T("SpotDt::GetBeibor3 parameter nullptr."));
		return false;
	}

	int iW = m_pParEft->Width();
	int iH = m_pParEft->Height();
	int iC = 1;
	for (int j = -1; j < 2; ++j)
	{
		int iy = y + j;
		if (iy < 0 || iy > iH)
			continue;

		for (int i = -1; i < 2; ++i)
		{
			int ix = x +i;
			if (ix < 0 || ix > iW)
				continue;
			// current pix
			int iOffset =  (iy*iW + ix)*3;
			if (j == 0 && i == 0)
			{
				pNeiB->dx[0] = iOffset;
				pNeiB->va[0] = *(pImg + iOffset);
			}
			// get the neighbor
			else
			{
				pNeiB->dx[iC] = iOffset;
				pNeiB->va[iC] = *(pImg + iOffset);
				++iC;
			}
		}
	}
	wxASSERT_MSG(iC > 3, _T("SpotDt::GetBeibor3 Neighbor Pixel num less 4"));
	pNeiB->num = iC;

	return true;
}

bool SpotDt::LocalMax(unsigned char* pImg,
                      unsigned char* pMinLab)
{
	if (pImg == nullptr || pMinLab == nullptr)
	{
		wxASSERT_MSG(false, _T("SpotDt::LocalMax parameter is nullptr."));
		return false;
	}

	std::stack<int> stkIter;

	// default all is inerlabel
	int iBytes = m_pParEft->PixNum()*3;
	for (int i = 0; i < iBytes; i += 3)
		pMinLab[i] = 1;

	bool bFound = false;
	ST_NEIBOR3 stNeibor3;
	// iterater
	int iW = m_pParEft->Width();
	int iH = m_pParEft->Height();
	for (int y = 0; y < iH; ++y)
	{
		for (int x = 0; x < iW; ++x)
		{
			int iOffset = (y*iW+x)*3;
			if (pMinLab[iOffset] == 0)
				continue;

			// find neighbor pix that greate than current
			bFound = false;
			if (!GetNeibor3(pImg, x, y, &stNeibor3))
				return false;
			for(int i = 1; i < stNeibor3.num; ++i)
			{
				if (stNeibor3.va[0] < stNeibor3.va[i])
				{
					bFound = true;
					break;
				}
			}

			// has the same value in current pix neighbor will not the max
			if (bFound)
			{
				unsigned char val = stNeibor3.va[0];
				stkIter.push(iOffset);
				pMinLab[iOffset] = 0;
				while (!stkIter.empty())
				{
					int iS = stkIter.top();
					stkIter.pop();
					int tx = iS%iW;
					int ty = iS/iW;
					if (!GetNeibor3(pImg, tx, ty, &stNeibor3))
						return false;
					for(int i = 1; i < stNeibor3.num; ++i)
					{
						int iT = stNeibor3.dx[i];
						if (pMinLab[iT] != 0 && stNeibor3.va[i] == val)
						{
							stkIter.push(iT);
							pMinLab[iT] = 0;
						}
					}
				}
			}	// if (bFound)
		}
	}
	return true;
}

/**< get min-value number, i.e., the connected-domain number */
bool SpotDt::Bwlabel(unsigned char* pMinLab, int iMode,
                     unsigned int* pRegion, unsigned int* pnNum)
{
	if (pMinLab == nullptr || pRegion == nullptr)
	{
		wxASSERT_MSG(false, _T("SpotDt::Bwlabel parameter is nullptr"));
		return false;
	}
	if (iMode != 4 && iMode != 8)
		iMode = 8;

	// initial label id
	unsigned int nLabel = LABEL_INITID;

	int iW = m_pParEft->Width();
	int iH = m_pParEft->Height();
	for (int y = 0; y < iH; ++y)
	{
		for (int x = 0; x < iW; ++x)
		{
			int iF = y*iW + x;
			if (pMinLab[iF*3] == 1 && pRegion[iF] == 0)
			{
				//labeling current pix
				pRegion[iF] = nLabel;
				// labeling the 3x3 neighbor
				if(!BwlabeNeibor(pMinLab, iMode, iF, nLabel, pRegion))
					return false;
				// increat label id
				++nLabel;
			}
		}
	}
	if (pnNum != nullptr)
		*pnNum = nLabel - LABEL_INITID;

	return true;
}

bool SpotDt::BwlabeNeibor(unsigned char* pMinLab, int iMode, int dx, unsigned int nID,
                          unsigned int* pRegion)
{
	int iW = m_pParEft->Width();
	int iH = m_pParEft->Height();

	std::stack<int> stkIter;
	stkIter.push(dx);
	// iterater
	while (!stkIter.empty())
	{
		int iS = stkIter.top();
		stkIter.pop();
		int xm = iS%iW;
		int ym = iS/iW;
		// 3x3 neighbor
		for (int j = -1; j < 2; ++j)
		{
			int yt = ym + j;
			if (yt < 0 || yt >= iH)
				continue;
			for (int i = -1; i < 2; ++i)
			{
				int xt = xm + i;
				if (xt < 0 || xt >= iW ||
				        (j == 0 && i == 0) ||
				        (iMode != 8 && j != 0 && i != 0))
					continue;

				int iT = yt*iW + xt;
				if (pMinLab[iT*3] == 1 && pRegion[iT] == 0)
				{
					pRegion[iT] = nID;
					stkIter.push(iT);
				}
			}
		}
	}	// stkIter.empty
	return true;
}

bool SpotDt::Bwdist(unsigned char* pMinLab, int* pDist)
{
	if (pMinLab == nullptr || pDist == nullptr)
	{
		wxASSERT_MSG(false, _T("SpotDt::Bwdist parameter is nullptr."));
		return false;
	}

	int iW = m_pParEft->Width();
	int iH = m_pParEft->Height();
	int iN = m_pParEft->PixNum();

	// initial the dist
	for (int i = 0; i < iN; ++i)
		pDist[i] = wxINT32_MAX;

	// assign distance
	unsigned char* pS = pMinLab;
	int* pT = pDist;
	int* pLab = new int[iW];
	for (int i = 0; i < iW; ++i)
		pLab[i] = -1;
	for (int y = 0; y < iH; ++y)
	{
		// scan min-value in this line
		int iCount = 0;
		for (int x = 0; x < iW; ++x)
		{
			if (pS[0] == 1)
				pLab[iCount++] = x;
			pS += 3;
		}
		if (iCount == 0)
			continue;

		// calculate distance
		int iDx = 0;
		int iDistMin = 0;
		for (int x = 0; x < iW; ++x)
		{
			iDistMin = abs(x - pLab[iDx]);
			for (int i = iDx + 1; i < iCount; ++i)
			{
				int iDistCur = abs(x - pLab[i]);
				if (iDistMin <= iDistCur)
					break;
				iDistMin = iDistCur;
				iDx = i;
			}
			if (pT[0] > iDistMin)
				pT[0] = iDistMin;
			++pT;
		}
	}

	if (pLab != nullptr)
		delete [] pLab;

	return true;
}

bool SpotDt::Bwshed(int* pDist, unsigned char* pShed)
{
	if (pDist == nullptr || pShed == nullptr)
	{
		wxASSERT_MSG(false, _T("SpotDt::Bwshed parameter is nullptr."));
		return false;
	}

	return true;
}
