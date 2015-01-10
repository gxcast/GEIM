#include "GCI.h"

#include <math.h>
#include <float.h>
#include <stack>
#include <wx/ffile.h>

#include "GEIMMatrix.h"
#include "GEIMMatrix.cpp"
#include "DispDlg.h"

GCI::GCI()
{
}

GCI::~GCI()
{
	// release temporary resources
	Release();
}

bool GCI::Main(ST_MTPAIR&& stMtPair, ST_MTRESULT* pstMtRet)
{
	bool bRet = true;

	if (pstMtRet == nullptr)
	{
		wxASSERT_MSG(false, _T("GrayStratify::GSMain parameter is nullptr."));
		return false;
	}
	m_stParamA = stMtPair.first;
	m_stParamB = stMtPair.second;
	m_pstMtRet = pstMtRet;

	// gray normalization
	bRet = Normalize(m_stParamA); if (!bRet) goto _tag_mail_end;
	bRet = Normalize(m_stParamB); if (!bRet) goto _tag_mail_end;

	// do match
	bRet = Match(); if (!bRet) goto _tag_mail_end;

	// release match temporary resources
	bRet = Release(); if (!bRet) goto _tag_mail_end;

_tag_mail_end:

	return bRet;
}

/**< image normalization */
bool GCI::Normalize(ST_MTPARAM& stParam)
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
bool GCI::CalcuPattern(ST_MTPARAM& stParam)
{
	VT_ATTR* pvtAttr = stParam.pvtAttr;

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
			dNum += 1.0;
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
		if (pCrt->deep == 0)
			pCrt->plump = -1024.0;
		else
			pCrt->plump = (pCrt->base - pCrt->mean)/pCrt->deep;
//		// invalid spot
//		if (pCrt->deep < 0 || pCrt->plump < 0)
//		{
//			// flag
//			it->bInvalid = true;
//			// abandon it's edge
//			for (int i = 0; i < pNode->perimeter; ++i)
//			{
//				int id = pNode->edge[i];
//				stParam.pData[id].g = 0;
//			}
//			continue;
//		}

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
		if (pCrt->plump != -1024.0 && pCrt->plump < dPlump0) dPlump0 = pCrt->plump;
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

		pCrt->area = (pCrt->area - dArea0)/dArea1;
		pCrt->mean = (pCrt->mean - dMean0)/dMean1;
		pCrt->base = (pCrt->base - dBase0)/dBase1;
		pCrt->deep = (pCrt->deep - dDeep0)/dDeep1;
		if (pCrt->plump == -1024.0)
			pCrt->plump = 1;
		else
			pCrt->plump = (pCrt->plump - dPlump0)/dPlump1;
	}

	return true;
}

/**< traverse one spot's pixels */
template<typename _Func>
bool GCI::SpotPixs(ST_MTPARAM& stParam, ST_SPOT_NODE& spot, _Func __f)
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

/**< calculate charact-vector for the image's spotd */
bool GCI::CalcuLevel(ST_MTPARAM& stParam)
{
	// level step and level thresshold value
	const double lv_trd[] = {0.0, 1.0/3.0, 2.0/3.0, 1.1};
	int lv_area = -1, lv_deep = -1, lv_com = -1;
	for (auto it_spot = stParam.pvtAttr->begin(); it_spot != stParam.pvtAttr->end(); ++it_spot)
	{
		ST_SPOT_ATTR& spot_attr = *it_spot;
		if (spot_attr.bInvalid)
			continue;

		lv_area = -1;
		for (int it_lv = 1; it_lv <= m_iLevelNum; ++it_lv)
		{
			if (spot_attr.pCrt->area <= lv_trd[it_lv])
			{
				lv_area = it_lv;
				break;
			}
		}
		lv_deep = -1;
		for (int it_lv = 1; it_lv <= m_iLevelNum; ++it_lv)
		{
			if (spot_attr.pCrt->deep <= lv_trd[it_lv])
			{
				lv_deep = it_lv;
				break;
			}
		}
		lv_com = (lv_area >= lv_deep)?lv_area:lv_deep;
		if (lv_com == -1)
			lv_com = m_iLevelNum - 1;
		else
			lv_com = m_iLevelNum - lv_com;
	}
	return true;
}

/**< calculate spots normalize coordnation by all spots */
bool GCI::CalcuCoord_all(ST_MTPARAM& stParam)
{
	int counter = 0;
	double meanx = 0, meany = 0;
	int img_w = stParam.iW, img_h = stParam.iH;
	// image valid region
	double rc_left = img_w/6.0, rc_top = img_h/6.0, rc_right = img_w*5.0/6.0, rc_bottom = img_h*5.0/6.0;

	// mean coord of spots located in 4/9 of image
	for (auto it_spot = stParam.pvtAttr->begin(); it_spot != stParam.pvtAttr->end(); ++it_spot)
	{
		ST_SPOT_ATTR& spot_attr = *it_spot;
		if (spot_attr.bInvalid)
			continue;

		// spot locate in 4/9 of image
		if (spot_attr.pNode->x >= rc_left && spot_attr.pNode->x <= rc_right
			&& spot_attr.pNode->y >= rc_top && spot_attr.pNode->y <= rc_bottom)
		{
			meanx += spot_attr.pNode->x;
			meany += spot_attr.pNode->y;
			++counter;
		}
	}
	meanx /= counter;
	meany /= counter;

	// normalize coord
	for (auto it_spot = stParam.pvtAttr->begin(); it_spot != stParam.pvtAttr->end(); ++it_spot)
	{
		ST_SPOT_ATTR& spot_attr = *it_spot;
		if (spot_attr.bInvalid)
			continue;

		// spot locate in 4/9 of image
		spot_attr.pCrt->x = (spot_attr.pNode->x - meanx)/img_w;
		spot_attr.pCrt->y = (spot_attr.pNode->y - meany)/img_h;
	}
	return true;
}

/**< calculate spots normalize coordnation by mathed spots */
bool GCI::CalcuCoord_match(ST_MTPARAM& stParam)
{
	return true;
}

/**< calculate shpae-contex similarity */
bool GCI::calcu_sc(const int * pt_set, int pt_num, double &mean, int bins_radius, int bins_theta, double *const histogram)
{
	bool bRet = false;
	if (pt_set == nullptr || pt_num <= 0 || histogram == nullptr)
		return false;
	// distance radius
	int * distgram = new int[pt_num*pt_num];
	bRet = calcu_sc_radiu(pt_set, pt_num, mean, bins_radius, distgram);
	if (!bRet)
	{
		delete distgram;
		return false;
	}
	// theta angle
	int * thetagram = new int[pt_num*pt_num];
	bRet = calcu_sc_theta(pt_set, pt_num, bins_theta, thetagram);
	if (!bRet)
	{
		delete distgram;
		delete thetagram;
		return false;
	}

	// statistic the histogram
	int *gram_theta = thetagram, *gram_dist = distgram;
	double *gram_data = histogram;
	int gram_bins = bins_theta*bins_radius;
	memset(gram_data, 0, pt_num*gram_bins*sizeof(double));
	for (int gram_y = 0; gram_y < pt_num; ++gram_y)
	{
		for (int gram_x = 0; gram_x < pt_num; ++gram_x)
		{
			if (gram_dist[gram_x] >= 0 && gram_theta[gram_x] >= 0)
				gram_data[gram_theta[gram_x]*bins_radius + gram_dist[gram_x]] += 1.0;
		}
		gram_theta += pt_num;
		gram_dist += pt_num;
		gram_data += gram_bins;
	}

	// normalize histogram
	gram_data = histogram;
	for (int gram_y = 0; gram_y < pt_num; ++gram_y)
	{
		double sum = 0.0;
		for (int gram_x = 0; gram_x < gram_bins; ++gram_x)
			sum += gram_data[gram_x];
		for (int gram_x = 0; gram_x < gram_bins; ++gram_x)
			gram_data[gram_x] /= sum;

		gram_data += gram_bins;
	}

	delete distgram;
	delete thetagram;
	return true;
}

/**< calculate distance-diagram of the points' set */
bool GCI::calcu_sc_radiu(const int * pt_set, int pt_num, double &mean, int bins_radius, int *const distgram)
{
	if (pt_set == nullptr || pt_num <= 0 || bins_radius <= 2 || distgram == nullptr)
		return false;

	double * bins_trd = nullptr;
	// function: calculate split threshold
	auto _func_bins_trd = [bins_radius, &bins_trd]() -> void {
		constexpr double radius_0 = 0.125, radius_1 = 2.0;

		double bins_begin = log10(radius_0);
		double bins_end = log10(radius_1);
		bins_end = (bins_end - bins_begin)/(bins_radius - 1);	// log step
		for (int i = 2; i < bins_radius; ++i)
		{
			bins_begin += bins_end;
			bins_trd[i] = pow(10, bins_begin);
		}
		bins_trd[0] = 0.0;
		bins_trd[1] = radius_0;
		bins_trd[bins_radius] = radius_1;
	};
	// function: calculate which bins the point located in
	auto _func_bins_q = [bins_radius, &bins_trd](double dist) -> int {
		int locate = -1;
		for (int i = 1; i <= bins_radius; ++i)
		{
			if (dist < bins_trd[i])
				++locate;
		}
		return locate;
	};

	// step1, get bins-edges
	bins_trd = new double[bins_radius + 1];
	_func_bins_trd();

	// step 2, calculate the mean distance
	if (mean <= 0.0)
	{
		mean = 0.0;
		int num = 0;
		for (int it_y = 0; it_y < pt_num; ++it_y)
		{
			for (int it_x = it_y + 1; it_x < pt_num; ++it_x)
			{
				mean += hypot(
				    pt_set[it_y<<1] - pt_set[it_x<<1],
				    pt_set[(it_y<<1) + 1] - pt_set[(it_x<<1) + 1]
				);
				++num;
			}
		}
		num = (num<<1) + pt_num;
		mean /= num;
	}

	// step3, get distance and then get it's bins-number(quatize)
	memset(distgram, 0, pt_num*pt_num*sizeof(int));
	for (int it_y = 0; it_y < pt_num; ++it_y)
	{
		for (int it_x = it_y + 1; it_x < pt_num; ++it_x)
		{
			int locate = _func_bins_q(
			    hypot(
			        pt_set[it_y<<1] - pt_set[it_x<<1],
			        pt_set[(it_y<<1) + 1] - pt_set[(it_x<<1) + 1]
			    )/mean
			);
			distgram[it_y*pt_num + it_x] = locate;
			distgram[it_x*pt_num + it_y] = locate;
		}
	}
	// distance to itself is -1
	for (int it_x = 0; it_x < pt_num; ++it_x)
		distgram[it_x*pt_num + it_x] = -1;

	delete bins_trd;
	return true;
}

/**< calculate theta-diagram of the points' set */
bool GCI::calcu_sc_theta(const int * pt_set, int pt_num, int bins_theta, int *const thetagram)
{
	if (pt_set == nullptr || pt_num <= 0 || bins_theta < 4 || thetagram == nullptr)
		return false;

	double bins_step = 2*pi/bins_theta;
	auto _func_bins_q = [bins_step](double theta) -> int {
		// theta should be in [-pi, pi]
		if (theta < 0.0)
			theta += 2*pi;
		return (int)(theta/bins_step);
	};

	// without itselft, the angle to itself default is -1
	memset(thetagram, 0, pt_num*pt_num*sizeof(int));
	for (int it_y = 0; it_y < pt_num; ++it_y)
	{
		for (int it_x = it_y + 1; it_x < pt_num; ++it_x)
		{
			double theta = atan2(
			    pt_set[(it_x<<1) + 1] - pt_set[(it_y<<1) + 1],
			    pt_set[it_x<<1] - pt_set[it_y<<1]
			);
			thetagram[it_y*pt_num + it_x] = _func_bins_q(theta);

			// the opposite theta
			if (theta > 0)
				theta -= pi;
			else
				theta += pi;
			thetagram[it_x*pt_num + it_y] = _func_bins_q(theta);
		}
	}
	for (int it_x = 0; it_x < pt_num; ++it_x)
		thetagram[it_x*pt_num + it_x] = -1;

	return true;
}

/**< initial the similarity matrix */
bool GCI::simi_init()
{
	// create similarity matrix
	int num_a = (int)m_stParamA.pvtAttr->size();
	int num_b = (int)m_stParamB.pvtAttr->size();
	if (m_pmxSimi == nullptr)
		m_pmxSimi = new GEIMMatrix<ST_GCI_SMI, ST_GCI_SMD, ST_GCI_SMD>(num_b, num_a);
	else if (m_pmxSimi->Height() != num_a || m_pmxSimi->Width() != num_b)
		// matrix's size is not suitebale, resize
		m_pmxSimi->Resize(num_b, num_a);

	// set matrix's axises value
	int id_t = 0;
	for (auto it_attr = m_stParamA.pvtAttr->begin(); it_attr != m_stParamA.pvtAttr->end(); ++it_attr)
	{
		ST_GCI_SMD & axis = m_pmxSimi->AxisY(id_t);
		axis.order = id_t;
		axis.spot_attr = &*it_attr;
	}
	id_t = 0;
	for (auto it_attr = m_stParamB.pvtAttr->begin(); it_attr != m_stParamB.pvtAttr->end(); ++it_attr)
	{
		ST_GCI_SMD & axis = m_pmxSimi->AxisX(id_t);
		axis.order = id_t;
		axis.spot_attr = &*it_attr;
	}

	return true;
}

bool GCI::simi_it(_func_simi __f)
{
	bool bRet = true;
	int id_a = 0, id_b = 0;
    for (auto it_a = m_stParamA.pvtAttr->begin(); it_a != m_stParamA.pvtAttr->end(); ++it_a)
	{
		id_b = 0;
		for (auto it_b = m_stParamB.pvtAttr->begin(); it_b != m_stParamB.pvtAttr->end(); ++it_b)
		{
			bRet = (this->*__f)(&*it_a, id_a, &*it_b, id_b);
			if (!bRet)
				goto _tag_simi_it_end;
			++id_b;
		}
		++id_a;
	}
_tag_simi_it_end:
	return bRet;
}

/**< claculate spots overlap similarity */
bool GCI::simi_overlap()
{
	CalcuCoord_all(m_stParamA);
	CalcuCoord_all(m_stParamB);

	return simi_it(&GCI::simi_overlap);
}
bool GCI::simi_overlap(ST_SPOT_ATTR* spot_a, int id_a, ST_SPOT_ATTR* spot_b, int id_b)
{
	double simi = (1.0 - fabs(spot_a->pCrt->x - spot_b->pCrt->x))*
		(1.0 - fabs(spot_a->pCrt->y - spot_b->pCrt->y));
	if (simi < 0.0)
		simi = 0.0;
	else if (simi > 1.0)
		simi = 1.0;
	ST_GCI_SMI& smi = (*m_pmxSimi)[id_a][id_b];
	smi.ovlp = simi;
	return true;
}

/**< claculate spots pattern  similarity */
bool GCI::simi_pattern()
{
	bool bRet = true;

	// caculate spot pattern-parameter
	bRet = bRet && CalcuPattern(m_stParamA);
	bRet = bRet && CalcuPattern(m_stParamB);
	// calculate spot level
	bRet = bRet && CalcuLevel(m_stParamA);
	bRet = bRet && CalcuLevel(m_stParamB);

	bRet = bRet && simi_it(&GCI::simi_pattern);

	return bRet;
}
bool GCI::simi_pattern(ST_SPOT_ATTR* spot_a, int id_a, ST_SPOT_ATTR* spot_b, int id_b)
{
	static int debug_count = 0;
	++debug_count;

	double temp = 0.0;
	double simi = 0.0;
	temp = spot_a->pCrt->area - spot_b->pCrt->area;
	simi += temp*temp;
	temp = spot_a->pCrt->base - spot_b->pCrt->base;
	simi += temp*temp;
	temp = spot_a->pCrt->deep - spot_b->pCrt->deep;
	simi += temp*temp;
	temp = spot_a->pCrt->mean - spot_b->pCrt->mean;
	simi += temp*temp;
	temp = spot_a->pCrt->plump - spot_b->pCrt->plump;
	simi += temp*temp;
	simi = sqrt(simi);
	simi = 1.0 - 2*simi/(sqrt(5.0) + simi);
	ST_GCI_SMI& smi = (*m_pmxSimi)[id_a][id_b];
	smi.patn = simi;
	return true;
}

/**< claculate spots structural similarity */
bool GCI::simi_structral()
{
	return simi_it(&GCI::simi_structral);
}
bool GCI::simi_structral(ST_SPOT_ATTR* spot_a, int id_a, ST_SPOT_ATTR* spot_b, int id_b)
{
	ST_GCI_SMI& smi = (*m_pmxSimi)[id_a][id_b];
	// spot valid rectangle in origin image: left top right bottom, and the wanted common rectangle
	int rc_a[4] = {0}, rc_b[4] = {0}, rc_t[4] = {0};
	double r_a[2] = {1.0, 1.0}, r_b[2] = {1.0, 1.0};	// zoom ratio (w, h)
	{
		ST_SPOT_NODE *node_a = spot_a->pNode, *node_b = spot_b->pNode;
		int rc1_a[4] = {0}, rc1_b[4] = {0};	// rect before zoom (l, t, r, b)
		double rc2_a[4] = {0}, rc2_b[4] = {0};	// rect after zoom (l, t, r, b)
		// zoom ratio
		rc_t[0] = node_a->rx*2 + 11; rc_t[1] = node_a->ry*2 + 11; rc_t[2] = node_b->rx*2 + 11; rc_t[3] = node_b->ry*2 + 11;
		if (rc_t[0] > rc_t[2]) r_a[0] = rc_t[2]/(double)rc_t[0]; else r_b[0] = rc_t[0]/(double)rc_t[2];
		if (rc_t[1] > rc_t[3]) r_a[1] = rc_t[3]/(double)rc_t[1]; else r_b[1] = rc_t[1]/(double)rc_t[3];
		// rect of a spot in idealy, maybe out of image
		rc_a[0] = node_a->x - node_a->rx - 5; rc_a[1] = node_a->y - node_a->ry - 5; rc_a[2] = node_a->x + node_a->rx + 5; rc_a[3] = node_a->y + node_a->ry + 5;
		rc_b[0] = node_b->x - node_b->rx - 5; rc_b[1] = node_b->y - node_b->ry - 5; rc_b[2] = node_b->x + node_b->rx + 5; rc_b[3] = node_b->y + node_b->ry + 5;
		// rect of a spot in image
		rc1_a[0] = (rc_a[0] >= 0)?rc_a[0]:0; rc1_a[1] = (rc_a[1] >= 0)?rc_a[1]:0; rc1_a[2] = (rc_a[2] < m_stParamA.iW-1)?rc_a[2]:m_stParamA.iW-1; rc1_a[3] = (rc_a[3] < m_stParamA.iH-1)?rc_a[3]:m_stParamA.iH-1;
		rc1_b[0] = (rc_b[0] >= 0)?rc_b[0]:0; rc1_b[1] = (rc_b[1] >= 0)?rc_b[1]:0; rc1_b[2] = (rc_b[2] < m_stParamB.iW-1)?rc_b[2]:m_stParamB.iW-1; rc1_b[3] = (rc_b[3] < m_stParamB.iH-1)?rc_b[3]:m_stParamB.iH-1;
		// rect remove spot-center to (0,0)
		rc1_a[0] -= node_a->x; rc1_a[1] -= node_a->y; rc1_a[2] -= node_a->x; rc1_a[3] -= node_a->y;
		rc1_b[0] -= node_b->x; rc1_b[1] -= node_b->y; rc1_b[2] -= node_b->x; rc1_b[3] -= node_b->y;
		// zoom rect
		rc2_a[0] = rc1_a[0]*r_a[0]; rc2_a[1] = rc1_a[1]*r_a[1]; rc2_a[2] = rc1_a[2]*r_a[0]; rc2_a[3] = rc1_a[3]*r_a[1];
		rc2_b[0] = rc1_b[0]*r_b[0]; rc2_b[1] = rc1_b[1]*r_b[1]; rc2_b[2] = rc1_b[2]*r_b[0]; rc2_b[3] = rc1_b[3]*r_b[1];
		// intersection
		if (rc2_a[0] >= rc2_b[2] || rc2_a[1] >= rc2_b[3] || rc2_a[2] <= rc2_b[0] || rc2_a[3] <= rc2_b[1])	// not intersecte
		{
			smi.stct = 0.0;
			return true;
		}
		if (rc2_a[0] < rc2_b[0]) rc2_a[0] = rc2_b[0];
		if (rc2_a[1] < rc2_b[1]) rc2_a[1] = rc2_b[1];
		if (rc2_a[2] > rc2_b[2]) rc2_a[2] = rc2_b[2];
		if (rc2_a[3] > rc2_b[3]) rc2_a[3] = rc2_b[3];
		rc_t[0] = (int)rc2_a[0]; rc_t[1] = (int)rc2_a[1]; rc_t[2] = (int)rc2_a[2]; rc_t[3] = (int)rc2_a[3];
		// inverse-zoom intersection
		rc_a[0] = node_a->x + (int)(rc2_a[0]/r_a[0]); rc_a[1] = node_a->y + (int)(rc2_a[1]/r_a[1]); rc_a[2] = node_a->x + (int)(rc2_a[2]/r_a[0]); rc_a[3] = node_a->y + (int)(rc2_a[3]/r_a[1]);
		rc_b[0] = node_b->x + (int)(rc2_a[0]/r_b[0]); rc_b[1] = node_b->y + (int)(rc2_a[1]/r_b[1]); rc_b[2] = node_b->x + (int)(rc2_a[2]/r_b[0]); rc_b[3] = node_b->y + (int)(rc2_a[3]/r_b[1]);
	}

	// cut out two spots' sub-image
	int subimg_w = rc_t[2] - rc_t[0] + 1, subimg_h = rc_t[3] - rc_t[1] + 1;
	int subimg_num = subimg_w*subimg_h;
	unsigned char *subimg_a = new unsigned char[subimg_num];
	unsigned char *subimg_b = new unsigned char[subimg_num];
	{
		ST_RGB *img_a = m_stParamA.pImg;
		ST_RGB *img_b = m_stParamB.pImg;
		int s_x[4], s_y[4], id_o, id_s;
		double o_x, o_y, s_r[4], r[2], temp;
		r[0] = (rc_a[2] - rc_a[0] + 1)/(double)subimg_w;
		r[1] = (rc_a[3] - rc_a[1] + 1)/(double)subimg_h;
		for (int y = 0; y < subimg_h; ++y)
		{
			o_y = rc_a[1] + y*r[1];
			s_y[0] = s_y[1] = (int)o_y; s_y[2] = s_y[3] = s_y[0] + 1;
			if (s_y[2] >= subimg_h)
				s_y[2] = s_y[3] = subimg_h - 1;
			for (int x = 0; x < subimg_w; ++x)
			{
				o_x = rc_a[0] + x*r[0];
				s_x[0] = s_x[2] = (int)o_x; s_x[1] = s_x[3] = s_x[0] + 1;
				if (s_x[1] >= subimg_w)
					s_x[1] = s_x[3] = subimg_w - 1;

				s_r[0] = ((int)o_x + 1 - o_x)*((int)o_y + 1 - o_y); s_r[1] = (o_x - (int)o_x)*((int)o_y + 1 - o_y);
				s_r[2] = ((int)o_x + 1 - o_x)*(o_y - (int)o_y); s_r[3] = (o_x - (int)o_x)*(o_y - (int)o_y);

				id_s = subimg_w*y + x;
				temp = 0.0;
				for (int i = 0; i < 4; ++i)
				{
					id_o = m_stParamA.iW*s_y[i] + s_x[i];
					temp += img_a[id_o].r * s_r[i];
				}
				if (temp < 0) temp = 0; else if (temp > 255.0) temp = 255.0;
				subimg_a[id_s] = (unsigned char)temp;
			}
		}
		r[0] = (rc_b[2] - rc_b[0] + 1)/(double)subimg_w;
		r[1] = (rc_b[3] - rc_b[1] + 1)/(double)subimg_h;
		for (int y = 0; y < subimg_h; ++y)
		{
			o_y = rc_b[1] + y*r[1];
			s_y[0] = s_y[1] = (int)o_y; s_y[2] = s_y[3] = s_y[0] + 1;
			if (s_y[2] >= subimg_h)
				s_y[2] = s_y[3] = subimg_h - 1;
			for (int x = 0; x < subimg_w; ++x)
			{
				o_x = rc_b[0] + x*r[0];
				s_x[0] = s_x[2] = (int)o_x; s_x[1] = s_x[3] = s_x[0] + 1;
				if (s_x[1] >= subimg_w)
					s_x[1] = s_x[3] = subimg_w - 1;

				s_r[0] = ((int)o_x + 1 - o_x)*((int)o_y + 1 - o_y); s_r[1] = (o_x - (int)o_x)*((int)o_y + 1 - o_y);
				s_r[2] = ((int)o_x + 1 - o_x)*(o_y - (int)o_y); s_r[3] = (o_x - (int)o_x)*(o_y - (int)o_y);

				id_s = subimg_w*y + x;
				temp = 0.0;
				for (int i = 0; i < 4; ++i)
				{
					id_o = m_stParamB.iW*s_y[i] + s_x[i];
					temp += img_b[id_o].r * s_r[i];
				}
				if (temp < 0) temp = 0; else if (temp > 255.0) temp = 255.0;
				subimg_b[id_s] = (unsigned char)temp;
			}
		}
	}
	// calculate structral similarity of two sub-images
	auto _func_mean = [](int img_num, unsigned char *img_data) -> double {
		double mean = 0.0;
		for (int i = 0; i < img_num; ++i)
			mean += *img_data++;
		mean /= img_num;
		return mean;
	};
	auto _func_varial = [](int img_num, unsigned char *img_data, double mean) -> double {
		double var = 0.0, temp = 0.0;
		for (int i = 0; i < img_num; ++i)
		{
			temp = *img_data++ - mean;
			var += temp * temp;
		}
		var /= img_num - 1;
		var = sqrt(var);
		return var;
	};
	auto _func_cov = [](int img_num, unsigned char *img_a, unsigned char *img_b, double mean[2]) -> double {
		double cov = 0.0;
		for (int i  = 0; i < img_num; ++i)
			cov += (*img_a++ - mean[0])*(*img_b++ - mean[1]);
		cov /= img_num -1;
		return cov;
	};
	double simi = 0.0, mean[2] = {0.0}, var[2] = {0.0}, cov = 0.0;
	constexpr double c_1 = (0.01*255)*(0.01*255), c_2 = (0.03*255)*(0.03*255);
	mean[0] = _func_mean(subimg_num, subimg_a);
	mean[1] = _func_mean(subimg_num, subimg_b);
	var[0] = _func_varial(subimg_num, subimg_a, mean[0]);
	var[1] = _func_varial(subimg_num, subimg_b, mean[1]);
	cov = _func_cov(subimg_num, subimg_a, subimg_b, mean);
	simi = 0.5 + (2*mean[0]*mean[1] + c_1)*(2*cov + c_2)/(2*(mean[0]*mean[0] + mean[1]*mean[1] + c_1)*(var[0]*var[0] + var[1]*var[1] + c_2));
	smi.stct = simi;
	return true;
}

/**< claculate spots shpe-contex similarity */
bool GCI::simi_shapecontex()
{
	bool bRet = true;

	double dist_mean = -1.0;
	auto _func_sc_init = [&dist_mean, this](ST_MTPARAM& param, int id_img) -> bool{
		constexpr int bins_radius = 5, bins_theta = 12;
		constexpr int num_bins = bins_theta*bins_radius;

		bool bRet = true;
		int num_spot = (int)param.pvtAttr->size();
		// alloc histogram memory
		if (this->m_sc_histogram[id_img] != nullptr)
			delete this->m_sc_histogram[id_img];
		this->m_sc_histogram[id_img] = new double[num_spot*num_bins];

		// general spots set
		int * spot_set = new int[num_spot*2];
		int id_spot = 0;
		for (auto it_attr = param.pvtAttr->begin(); it_attr != param.pvtAttr->end(); ++it_attr)
		{
			spot_set[id_spot<<1] = it_attr->pNode->x;
			spot_set[(id_spot<<1) + 1] = it_attr->pNode->y;
			++id_spot;
		}

		// get histogram
		bRet = calcu_sc(spot_set, num_spot, dist_mean, bins_radius, bins_theta, this->m_sc_histogram[id_img]);
		this->m_sc_num = num_bins;

		delete spot_set;
		return bRet;
	};
	bRet = bRet && _func_sc_init(m_stParamA, 0);
	bRet = bRet && _func_sc_init(m_stParamB, 1);

	bRet = bRet && simi_it(&GCI::simi_shapecontex);

	m_sc_num = 0;
	delete m_sc_histogram[0];
	delete m_sc_histogram[1];
	return bRet;
}
bool GCI::simi_shapecontex(ST_SPOT_ATTR* spot_a, int id_a, ST_SPOT_ATTR* spot_b, int id_b)
{
	ST_GCI_SMI& smi = (*m_pmxSimi)[id_a][id_b];

	if (m_sc_num <= 0)
	{
		smi.spct = 0.0;
		return true;
	}

	double simi = 0.0, temp_0 = 0.0, temp_1 = 0.0;
	double *sc_a = m_sc_histogram[0] + id_a*m_sc_num;
	double *sc_b = m_sc_histogram[1] + id_b*m_sc_num;
	for (int it_bins = 0; it_bins < m_sc_num; ++it_bins)
	{
		temp_0 = sc_a[it_bins] - sc_b[it_bins];
		temp_1 = sc_a[it_bins] + sc_b[it_bins];
		if (temp_1 > 0.0)
			simi += (double)temp_0*temp_0/temp_1;
	}

	smi.spct = 1.0 - simi/2.0;
	return true;
}

bool GCI::Match()
{
	simi_init();

	// similarity
	simi_overlap();
	simi_pattern();
	simi_structral();
	simi_shapecontex();

	// output similarity
	mt_cull();

	return true;
}

bool GCI::mt_cull()
{
	wxString str_out;
	wxFFile file_out;
	file_out.Open(_T("/home/nicolas/Desktop/out.txt"), _T("w"));

	int smi_a = m_pmxSimi->Height(), smi_b = m_pmxSimi->Width();
	int smi_num = smi_a*smi_b;
	str_out.Printf(_T("%d.0000 %d.0000 %lf %lf\r"), smi_a, smi_b, 0.0, 0.0);
	file_out.Write(str_out);

	ST_GCI_SMI *smi_ary = (*m_pmxSimi)[0];
	for (int smi_it = 0; smi_it < smi_num; ++smi_it)
	{
		str_out.Printf(_T("%lf %lf %lf %lf\r"), smi_ary->ovlp, smi_ary->patn, smi_ary->stct, smi_ary->spct);
		++smi_ary;
		file_out.Write(str_out);
	}
	file_out.Close();

	return true;
}

bool GCI::Release()
{
	// release similarity matrix
	if (m_pmxSimi != nullptr)
	{
		delete m_pmxSimi;
		m_pmxSimi = nullptr;
	}
	return true;
}
