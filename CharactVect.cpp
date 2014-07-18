#include "CharactVect.h"

#include <stack>

// base image manipulate interface
#include "EffectPar.h"
// image's gray operation
#include "Graying.h"
// icp
#include "ICP.h"
// kdtree-bbf
#include "Kdtree.h"

CharactVect::CharactVect()
{
}

CharactVect::~CharactVect()
{
}

/**< match main entry */
bool CharactVect::CVMain(ST_MTPAIR&& stMtPair, ST_MTRESULT* pstMtRet)
{
	bool bRet = true;
	if (pstMtRet == nullptr)
	{
		wxASSERT_MSG(false, _T("CharactVect::CVMain parameter is nullptr."));
		return false;
	}
	m_stParamA = stMtPair.first;
	m_stParamB = stMtPair.second;
	m_pstMtRet = pstMtRet;

	// normalization
	bRet = bRet && Normalize(m_stParamA);
	bRet = bRet && Normalize(m_stParamB);

	// calculate the charact vector
	bRet = bRet && CalcuCharacr(m_stParamA);
	bRet = bRet && CalcuCharacr(m_stParamB);

	// icp
	//ICP icp;
	//bRet = bRet && icp.ICPMain(std::make_pair(m_stParamA, m_stParamB), pstMtRet);
	bRet = bRet && VectMatch();

	return bRet;
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

/** \brief match use the vector-bbf method
 *
 ** \return bool true:success
 *
 * match result in m_pstMtRet
 */
bool CharactVect::VectMatch()
{
	bool bRet = false;
	int iNumB = 0;
	ST_FEATURE *pFeatB = nullptr;
	VT_ATTR *pVtB = m_stParamB.pvtAttr, *pVtA = m_stParamA.pvtAttr;

	// generate ST_FEATURE array for image B
	bRet = feat_generate(pFeatB, iNumB, pVtB);
	if (!bRet)
		goto VectMatch_end;

	// bbf
	if (m_pstMtRet->pvtSpair == nullptr)
		m_pstMtRet->pvtSpair = new VT_SPAIR;
	if (m_pstMtRet->pvtSpair->size() > 0)
		m_pstMtRet->pvtSpair->clear();
	bRet = feat_bbf(m_pstMtRet->pvtSpair, pFeatB, iNumB, pVtA);
	//if (!bRet)
	//	goto VectMatch_end;

VectMatch_end:
	// release ST_FEATURE array of image B
	feat_release(pFeatB);
	return bRet;
}

/** \brief transform my spot attrib format to feature array
 *
 * \param feats ST_FEATURE *&	[OUT] the feature array, alloc memory inside
 * \param num int&				[OUT] items' number in feature array
 * \param attr const VT_ATTR*	[IN] my spot attribute format: vector
 * \return bool true:success
 *
 */
bool CharactVect::feat_generate(ST_FEATURE *& feats, int &num, const VT_ATTR *attr)
{
	int num_node = 0, n_temp = 0;

	if (attr == nullptr)
		return false;

	if (feats != nullptr)
	{
		delete [] feats;
		feats = nullptr;
	}
	num = 0;

	// get the number of the feature nodes
	for (auto it = attr->begin(); it != attr->end(); ++it)
	{
		if (!it->bInvalid)
			++num_node;
	}
	if (num_node <= 0)
		return false;

	// alloc memory for feature nodes
	feats = new ST_FEATURE[num_node];
	if (feats == nullptr)
		return false;

	// fill feature nodes
	num_node = 0;
	n_temp = 0;
	for (auto it = attr->begin(); it != attr->end(); ++it)
	{
		if (!it->bInvalid)
		{
			PST_SPOT_CHARACT pCrt = it->pCrt;
			ST_FEATURE& feat = feats[num_node];

			feat.iOrder = n_temp;
			feat.n = 7;	// <= FEATURE_MAX_D
			feat.descr[0] = pCrt->x;
			feat.descr[1] = pCrt->y;
			feat.descr[2] = pCrt->area;
			feat.descr[3] = pCrt->mean;
			feat.descr[4] = pCrt->base;
			feat.descr[5] = pCrt->deep;
			feat.descr[6] = pCrt->plump;
			feat.feature_data = nullptr;

			++num_node;
		}
		++n_temp;
	}

	num = num_node;
	return true;
}

/** \brief find another set spots' match part in feature array
 *
 * \param pairs VT_SPAIR*		[OUT] match result
 * \param feats ST_FEATURE*		[IN] feature array generated by feat_generate
 * \param num int				[IN] items' number of feature array
 * \param attr const VT_ATTR*	[IN] another spot attribute set
 * \return bool	true:success
 *
 */
bool CharactVect::feat_bbf(VT_SPAIR* pairs, ST_FEATURE *feats, int num, const VT_ATTR *attr)
{
	bool bRet = false;
	int num_pair = 0, n_temp = 0;
	ST_FEATURE **nbrs = nullptr;
	Kdtree kdt;

	if (pairs == nullptr || feats == nullptr || attr == nullptr)
		return false;

	// build kd-tree
	bRet = kdt.build(feats, num);
	if (!bRet)
		return false;

	// find every node's parts, put the match pair into match-result
	n_temp = 0;
	for (auto it = attr->begin(); it != attr->end(); ++it)
	{
		if (!it->bInvalid)
		{
			int knn = 0;
			PST_SPOT_CHARACT pCrt = it->pCrt;
			ST_FEATURE feat;
			// fill a feature
			feat.iOrder = n_temp;
			feat.n = 7;	// <= FEATURE_MAX_D
			feat.descr[0] = pCrt->x;
			feat.descr[1] = pCrt->y;
			feat.descr[2] = pCrt->area;
			feat.descr[3] = pCrt->mean;
			feat.descr[4] = pCrt->base;
			feat.descr[5] = pCrt->deep;
			feat.descr[6] = pCrt->plump;
			feat.feature_data = nullptr;
			// bbf
			knn = kdt.bbf_knn(&feat, 2, &nbrs, KDTREE_BBF_MAX_NN_CHKS);
			if (knn == 2)
			{
				double d0 = feat_dist_sq(&feat, nbrs[0]);
				double d1 = feat_dist_sq(&feat, nbrs[1]);
				if ( d0 < d1 * NN_SQ_DIST_RATIO_THR )
				{
					// add match pair
					ST_SPAIR cp;
					cp.iOdA = feat.iOrder;
					cp.iOdB = nbrs[0]->iOrder;
					pairs->push_back(cp);
					++num_pair;
				}
			}
			// release bbf reuslt
			if (nbrs != nullptr)
			{
				free(nbrs);
				nbrs = nullptr;
			}
		}
		++n_temp;
	}

	// release kd-tree
	kdt.release();

	return true;
}

/** \brief release memory allocated in feat_generate
 *
 * \param feats ST_FEATURE *&	[INOUT] the feature array to be release
 * \return void
 *
 */
void CharactVect::feat_release(ST_FEATURE *& feats)
{
	if (feats != nullptr)
	{
		delete [] feats;
		feats = nullptr;
	}
}
