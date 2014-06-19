#include "CharactVect.h"

#include <stack>

// base image manipulate interface
#include "EffectPar.h"
// image's gray operation
#include "Graying.h"
// icp
#include "ICP.h"

CharactVect::CharactVect()
{
}

CharactVect::~CharactVect()
{
}

/**< match main entry */
bool CharactVect::CVMain(ST_MTPAIR&& stMtPair, ST_MTRESULT* pstMtRet)
{
	if (pstMtRet == nullptr)
	{
		wxASSERT_MSG(false, _T("CharactVect::CVMain parameter is nullptr."));
		return false;
	}
	m_stParamA = stMtPair.first;
	m_stParamB = stMtPair.second;
	m_pstMtRet = pstMtRet;

	// normalization
	Normalize(m_stParamA);
	Normalize(m_stParamB);

	// calculate the charact vector
	CalcuCharacr(m_stParamA);
	CalcuCharacr(m_stParamB);

	// icp
	ICP icp;
	icp.ICPMain(std::make_pair(m_stParamA, m_stParamB), pstMtRet);

	return true;
}

bool CharactVect::DestroyResult(PST_MTRESULT pRst)
{
	if (pRst == nullptr)
		return false;
    if (pRst->pvtSpair != nullptr)
	{
		delete pRst->pvtSpair;
		pRst->pvtSpair = nullptr;
	}
	return true;
}

/**< image normalization */
bool CharactVect::Normalize(ST_MTPARAM& stParam)
{
	PST_RGB pSrc = stParam.pData;
	PST_RGB pDes = stParam.pImg;
	if (pSrc == nullptr || pDes == nullptr)
	{
		wxASSERT_MSG(false, _T("match param's origin image is nullptr. "));
		return false;
	}

	// pixel number
	int iN = stParam.iN;

	// caculate ecpectation and variance
	double dE0 = 125.0, dE = 0.0;
	double dV0 = 40.0, dV = 0.0;
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

	return true;
}

/**< calculate charact-vector for the image's spotd */
bool CharactVect::CalcuCharacr(ST_MTPARAM& stParam)
{
	VT_ATTR* pvtAttr = stParam.pvtAttr;

	// image's size
	double dW = stParam.iW;
	double dH = stParam.iH;
	// max and min value of characts
	double dArea1 = 0.0, dArea0 = 3.0e6;
	double dMean1 = 0.0, dMean0 = 4086.0;
	double dBase1 = 0.0, dBase0 = 4096.0;
	double dDeep1 = 0.0, dDeep0 = 4096.0;
	double dPlump1 = 0.0, dPlump0 = 2.0;

	// traverse all the spots, calculate their maxmium and minmium characts
	for (auto it = pvtAttr->begin(); it != pvtAttr->end(); ++it)
	{
		if (it->bInvalid)
			continue;
		PST_SPOT_NODE pNode = it->pNode;
		PST_SPOT_CHARACT pCrt = it->pCrt;

		double dMin = 4096.0;	// min gray value
		double dNum = 0.0;		// pixel's num
		double dVol = 0.0;		// gray's sum
		double dBk = 0.0;		// background's mean gray

		PST_RGB pTmp = stParam.pImg;
		SpotPixs(stParam, *pNode, [&dMin, &dNum, &dVol, pTmp](int id) -> bool
		{
			unsigned char pix = pTmp[id].r;
			dNum += 1;
			dVol += pix;
			if (pix < dMin)
				dMin = pix;
			return true;
		});
		for (int i = 0; i < pNode->perimeter; ++i)
		{
			int id = pNode->edge[i];
			unsigned char pix = pTmp[id].r;
			dBk += pix;
		}

		pCrt->x = pNode->x;
		pCrt->y = pNode->y;
		pCrt->area = dNum;
		pCrt->mean = dVol/dNum;
		pCrt->base = dBk/pNode->perimeter;
		pCrt->deep = pCrt->base - dMin;
		pCrt->plump = (pCrt->base - pCrt->mean)/pCrt->deep;
		// invalid spot
		if (pCrt->deep < 0 || pCrt->plump < 0)
		{
			// flag
			it->bInvalid = true;
			// abandon it's edge
			for (int i = 0; i < pNode->perimeter; ++i)
			{
				int id = pNode->edge[i];
				stParam.pData[id].g = 0;
			}
			continue;
		}

		// max value of characts
		if (pCrt->area > dArea1) dArea1 = pCrt->area;
		if (pCrt->mean > dMean1) dMean1 = pCrt->mean;
		if (pCrt->base > dBase1) dBase1 = pCrt->base;
		if (pCrt->deep > dDeep1) dDeep1 = pCrt->deep;
		if (pCrt->plump > dPlump1) dPlump1 = pCrt->plump;
		// min value of characts
		if (pCrt->area < dArea0) dArea0 = pCrt->area;
		if (pCrt->mean < dMean0) dMean0 = pCrt->mean;
		if (pCrt->base < dBase0) dBase0 = pCrt->base;
		if (pCrt->deep < dDeep0) dDeep0 = pCrt->deep;
		if (pCrt->plump < dPlump0) dPlump0 = pCrt->plump;
	}

	dArea1 -= dArea0;
	dMean1 -= dMean0;
	dBase1 -= dBase0;
	dDeep1 -= dDeep0;
	dPlump1 -= dPlump0;

	// traverse all the charact value, normalize characts
	for (auto it = pvtAttr->begin(); it != pvtAttr->end(); ++it)
	{
		if (it->bInvalid)
			continue;
		PST_SPOT_CHARACT pCrt = it->pCrt;

		pCrt->x /= dW;
		pCrt->y /= dH;
		pCrt->area = (pCrt->area - dArea0)/dArea1;
		pCrt->mean = (pCrt->mean - dMean0)/dMean1;
		pCrt->base = (pCrt->base - dBase0)/dBase1;
		pCrt->deep = (pCrt->deep - dDeep0)/dDeep1;
		pCrt->plump = (pCrt->plump - dPlump0)/dPlump1;
	}

	return true;
}

/**< traverse one spot's pixels */
template<typename _Func>
bool CharactVect::SpotPixs(ST_MTPARAM& stParam, ST_SPOT_NODE& spot, _Func __f)
{
	PST_RGB pImg = stParam.pData;
	int iW = stParam.iW;
	int iH = stParam.iH;

	int x = spot.x;
	int y = spot.y;
	int id = y*iW + x;

	// flag to indicate the pixel has been processed
	unsigned char label = pImg[id].b;
	unsigned char haspro = label ^ 0x55;

	// one pixel
	if(!__f(id))
		return false;
	pImg[id].b = haspro;

	std::stack<int> stkTra;
	stkTra.push(id);
	while (!stkTra.empty())
	{
		id = stkTra.top();
		stkTra.pop();
		x = id%iW;
		y = id/iW;

		// 4 neighbor: left top right bottom
		int iNB_x[4] = {-1, -1, -1, -1};
		int iNB_y[4] = {-1, -1, -1, -1};
		if (y - 1 >= 0)
		{
			iNB_x[0] = x;
			iNB_y[0] = y - 1;
		}
		if (x - 1 >= 0)
		{
			iNB_x[1] = x - 1;
			iNB_y[1] = y;
		}
		if (x + 1 < iW)
		{
			iNB_x[2] = x + 1;
			iNB_y[2] = y;
		}
		if (y + 1 < iH)
		{
			iNB_x[3] = x;
			iNB_y[3] = y + 1;
		}
		for (int i = 0; i < 4; ++i)
		{
			if (iNB_x[i] < 0)
				continue;
			x = iNB_x[i];
			y = iNB_y[i];
			id = y*iW + x;

			if (pImg[id].b == label)
			{
				// one pixel
				pImg[id].b = haspro;
				__f(id);
				// iteration
				stkTra.push(id);
			}
		}
	}	// while (!stkTra.empty())

	return true;
}
