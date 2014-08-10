#include "GCI.h"

GCI::GCI()
{
}

GCI::~GCI()
{
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

	bRet = bRet && Stratify();
	bRet = bRet && CoarseMT();
	bRet = bRet && Release();

	return bRet;
}

bool GCI::Stratify()
{
	stratify_one(m_stParamA, m_aScaleA, m_vtGsA);
	stratify_one(m_stParamB, m_aScaleB, m_vtGsB);
	return true;
}

bool GCI::stratify_one(ST_MTPARAM &param, int *scale, VT_GS &vt_gs)
{
	// statistics min and max gray value
	unsigned char gray_min = 255, gray_max = 0;
	ST_RGB *pImg = param.pImg;
	int num_pix = param.iW * param.iH;
	for (int i = 0; i < num_pix; ++i)
	{
		unsigned char pix = pImg[i].r;
		if (pix < gray_min)
			gray_min = pix;
		if (pix > gray_max)
			gray_max = pix;
	}

	// level-step
	double step = ((double)gray_max - gray_min)/NUM_STRATIFY;
	unsigned char level[NUM_STRATIFY];
	for (int i = 1; i < NUM_STRATIFY; ++i)
		level[i-1] = gray_min + (unsigned char)(step*i);
	level[NUM_STRATIFY-1] = gray_max;

	// statistics num of every level
	int cnt_nd = 0;
	int cnt_lv[NUM_STRATIFY] = {0};
	for (int lv = 0; lv < NUM_STRATIFY; ++lv)
		cnt_lv[lv] = 0;
	for (auto it = param.pvtAttr->begin(); it != param.pvtAttr->end(); ++it)
	{
		if (it->bInvalid)
			continue;
		// spot center's pix
		int idx = param.iW*it->pNode->y + it->pNode->x;
		unsigned char pix = pImg[idx].r;
		// locate in which level
		for (int lv = 0; lv < NUM_STRATIFY; ++lv)
		{
			if (pix <= level[lv])
			{
				cnt_lv[lv] += 1;
				break;
			}
		}
		++cnt_nd;
	}

	// pre-alloc memory
	vt_gs.resize(cnt_nd);
	cnt_nd = 0;
	for (int lv = 0; lv < NUM_STRATIFY; ++lv)
	{
		cnt_nd += cnt_lv[lv];
		scale[lv] = cnt_nd;
		cnt_lv[lv] = 0;
	}

	// get spot-node for every level
	cnt_nd = -1;
	for (auto it = param.pvtAttr->begin(); it != param.pvtAttr->end(); ++it)
	{
		++cnt_nd;
		if (it->bInvalid)
			continue;
		// spot center's pix
		int idx = param.iW*it->pNode->y + it->pNode->x;
		unsigned char pix = pImg[idx].r;
		// locate in which level
		for (int lv = 0; lv < NUM_STRATIFY; ++lv)
		{
			if (pix <= level[lv])
			{
				// add a spot in lv level
				if (lv == 0)
					idx = cnt_lv[0];
				else
					idx = scale[lv-1] + cnt_lv[lv];
				ST_GSNODE& nd_gs = vt_gs[idx];

				nd_gs.iOrder = cnt_nd;
				nd_gs.level = lv;

				cnt_lv[lv] += 1;
				break;
			}
		}
	}

	return true;
}

bool GCI::CoarseMT()
{
	int num_pair = 0, num_trd = 0;

	// least spot number
	num_trd = (int)m_vtGsA.size();
	num_pair = (int)m_vtGsB.size();
	if (num_pair < num_trd)
		num_trd = num_pair;
	num_trd /= 4;
	if (num_trd < 10)
		num_trd = 10;
	num_pair = 0;

	InitThresold();

	// only match level 0
	GetCenter(true);
	for (auto it_a = m_vtGsA.begin(); (it_a != m_vtGsA.end())&&(it_a->level == 0); ++it_a)
	{
		for (auto it_b = m_vtGsB.begin(); (it_b != m_vtGsB.end())&&(it_b->level == 0); ++it_b)
			Similary(*it_a, *it_b);
	}
	// verify one-to-one match
	for (auto it_a = m_vtGsA.begin(); (it_a != m_vtGsA.end())&&(it_a->level == 0); ++it_a)
	{
		if (it_a->ovlp > m_trdOvlp && it_a->simi > m_trdSimi &&
		        it_a->partner->partner != nullptr && it_a->iOrder == it_a->partner->partner->iOrder)
		{
			it_a->match = true;
			it_a->partner->match = true;
			++num_pair;
		}
	}

	// depend on first match result, calculate center
	if (num_pair <= 0)
		return false;
	GetCenter(false);

	// clean first match result
	for (auto it_a = m_vtGsA.begin(); (it_a != m_vtGsA.end())&&(it_a->level == 0); ++it_a)
	{
		it_a->match = false;
		it_a->partner = nullptr;
		it_a->ovlp = 0.0;
		it_a->itst = 0.0;
		it_a->simi = 0.0;
	}
	for (auto it_b = m_vtGsB.begin(); (it_b != m_vtGsB.end())&&(it_b->level == 0); ++it_b)
	{
		it_b->match = false;
		it_b->partner = nullptr;
		it_b->ovlp = 0.0;
		it_b->itst = 0.0;
		it_b->simi = 0.0;
	}
	num_pair = 0;

	// iterate match
	do
	{
		// do match for every level
		for (int lv = 0; lv < NUM_STRATIFY; ++lv)
		{
			// use has matched pair to calculate center
			if (num_pair > 0)
				GetCenter(false);

			// one-to-one match
			for (auto it_a = m_vtGsA.begin(); (it_a != m_vtGsA.end())&&(it_a->level <= lv); ++it_a)
			{
				if (it_a->match)
					continue;
				for (auto it_b = m_vtGsB.begin(); (it_b != m_vtGsB.end())&&(it_b->level <= lv); ++it_b)
				{
					if (!it_b->match)
						Similary(*it_a, *it_b);
				}
			}

			// verify match result
			for (auto it_a = m_vtGsA.begin(); (it_a != m_vtGsA.end())&&(it_a->level <= lv); ++it_a)
			{
				if (it_a->match)
					continue;

				ST_GSNODE *part = it_a->partner;
				if (it_a->ovlp > m_trdOvlp && it_a->simi > m_trdSimi &&
				        !part->match && part->partner != nullptr && it_a->iOrder == part->partner->iOrder)
				{
					bool oneone = true;
					for (auto it_b = m_vtGsB.begin(); (it_b != m_vtGsB.end())&&(it_b->level <= lv); ++it_b)
					{
						if (it_b->partner != nullptr &&
						        it_b->partner->iOrder == it_a->iOrder &&
						        it_b->iOrder != part->iOrder)
						{
							oneone = false;
							break;
						}
					}
					if (oneone)
					{
						it_a->match = true;
						part->match = true;
						++num_pair;
					}
				}
			}	// verify
		}	// every level

		// whether pair number enough, else, decrease thresold
		if (num_pair >= num_trd)
			break;
		m_trdOvlp -= 0.05;
		m_trdItst -= 0.05;
		m_trdSimi = m_dFactor*m_trdItst + (1.0 - m_dFactor)*m_trdOvlp;
	}
	while(m_trdSimi > 0.60);

	// record pairs' number
	m_nPair = num_pair;

	return true;
}

bool GCI::InitThresold()
{
	double rat = (m_stParamB.iH*m_stParamB.iW)/(double)(m_stParamA.iH*m_stParamA.iW);
	if (rat > 1.0)
		rat = 1/rat;
	double alpha = 0.5*rat;
	if (rat >= 0.9)
	{
		m_trdOvlp = 0.95;
		m_trdItst = 0.90;
	}
	else if (0.75 <= rat && rat < 0.90)
	{
		m_trdOvlp = 0.85;
		m_trdItst = 0.80;
	}
	else if (0.50 <= rat && rat < 0.75)
	{
		m_trdOvlp = 0.75;
		m_trdItst = 0.70;
	}
	m_trdSimi = alpha*m_trdItst + (1.0 - alpha)*m_trdOvlp;
	m_dFactor = alpha;
	return true;
}

bool GCI::GetCenter(bool global)
{
	if (global)
	{
		center_global(m_ptCenterA, m_stParamA);
		center_global(m_ptCenterB, m_stParamB);
	}
	else
	{
		center_mp(m_ptCenterA, m_stParamA, m_vtGsA);
		center_mp(m_ptCenterB, m_stParamB, m_vtGsB);
	}
	return true;
}

bool GCI::center_global(wxPoint &pt, ST_MTPARAM &param)
{
	double cent_x = 0.0, cent_y = 0.0;
	int nd_x = 0, nd_y = 0;
	int num = 0;
	int left = 0, right = 0, top = 0, bottom = 0;

	// center 2/3 raea of image
	left  = param.iW/6;
	right = param.iW - left;
	top = param.iH/6;
	bottom = param.iH - top;

	for (auto it = param.pvtAttr->begin(); it != param.pvtAttr->end(); ++it)
	{
		if (it->bInvalid)
			continue;
		nd_x = it->pNode->x;
		nd_y = it->pNode->y;
		if (left <= nd_x && nd_x < right && top <= nd_y && nd_y < bottom)
		{
			cent_x += nd_x;
			cent_y += nd_y;
			++num;
		}
	}
	cent_x /= num;
	cent_y /= num;

	pt.x = (int)cent_x;
	pt.y = (int)cent_y;

	return true;
}

bool GCI::center_mp(wxPoint &pt, ST_MTPARAM &param, VT_GS &vtgs)
{
	double cent_x = 0.0, cent_y = 0.0;
	int num = 0;

	for (auto it = vtgs.begin(); it != vtgs.end(); ++it)
	{
		if (!it->match)
			continue;
		ST_SPOT_NODE *nd_spot = param.pvtAttr->at(it->iOrder).pNode;
		cent_x += nd_spot->x;
		cent_y += nd_spot->y;
		++num;
	}
	cent_x /= num;
	cent_y /= num;

	pt.x = (int)cent_x;
	pt.y = (int)cent_y;

	return true;
}

double GCI::Similary(ST_GSNODE &gs_a, ST_GSNODE &gs_b)
{
	double sm_ovlp = 0.0, sm_itst = 0.0, sm_syn;
	ST_SPOT_NODE *nd_a = m_stParamA.pvtAttr->at(gs_a.iOrder).pNode;
	ST_SPOT_NODE *nd_b = m_stParamB.pvtAttr->at(gs_b.iOrder).pNode;

	// similary of position
	sm_ovlp = simi_overlap(nd_a->x, nd_a->y, nd_b->x, nd_b->y);
	if (sm_ovlp < m_trdOvlp - 0.05)
		return false;

	// similary of intensity
	sm_itst = simi_intensity(*nd_a, *nd_b);

	// synthetical
	sm_syn = m_dFactor*sm_itst + (1 - m_dFactor)*sm_ovlp;
	if (sm_syn < m_trdSimi - 0.05)
		return false;

	// record most match
	if (sm_ovlp > gs_a.ovlp && sm_syn > gs_a.simi)
	{
		gs_a.ovlp = sm_ovlp;
		gs_a.itst = sm_itst;
		gs_a.simi = sm_syn;
		gs_a.partner = &gs_b;
	}
	if (sm_ovlp > gs_b.ovlp && sm_syn > gs_b.simi)
	{
		gs_b.ovlp = sm_ovlp;
		gs_b.itst = sm_itst;
		gs_b.simi = sm_syn;
		gs_b.partner = &gs_a;
	}

	return true;
}

double GCI::simi_overlap(int a_x, int a_y, int b_x, int b_y)
{
	double rt_x = 0.0, rt_y = 0.0;
	rt_x = fabs((a_x - m_ptCenterA.x)/(double)m_stParamA.iW - (b_x - m_ptCenterB.x)/(double)m_stParamB.iW);
	rt_y = fabs((a_y - m_ptCenterA.y)/(double)m_stParamA.iH - (b_y - m_ptCenterB.y)/(double)m_stParamB.iH);
	if (rt_x > 1.0)
		rt_x = 1.0;
	if (rt_y > 1.0)
		rt_y = 1.0;
	return (1.0 - rt_x)*(1.0 - rt_y);
}

double GCI::simi_intensity(ST_SPOT_NODE &nd_a, ST_SPOT_NODE &nd_b)
{
	int a_l = 0, a_t = 0, a_r = 0, a_b = 0;
	int b_l = 0, b_t = 0, b_r = 0, b_b = 0;
	double rat_w = 0.0, rat_h = 0.0;
	int a_x = 0, a_y = 0, b_x = 0, b_y = 0;
	int a_x0 = 0, a_x1 = 0, a_y0 = 0, a_y1 = 0;
	int b_x0 = 0, b_x1 = 0, b_y0 = 0, b_y1 = 0;
	int a_np = 0, b_np = 0;
	ST_RGB *a_img = nullptr, *b_img = nullptr;
	double mean_a = 0.0, mean_b = 0.0;
	unsigned char pix_a = 0u, pix_b = 0u;
	double rab = 0.0, lab = 0.0, laa = 0.0, lbb = 0.0;

	// calculate extend radius
	a_r = m_stParamA.iW/50 + 20;
	if (a_r < nd_a.rx) a_r = nd_a.rx;
	a_r /= 2;
	a_b = m_stParamA.iH/50 + 20;
	if (a_b < nd_a.ry) a_b = nd_a.ry;
	a_b /= 2;
	b_r = m_stParamB.iW/50 + 20;
	if (b_r < nd_b.rx) b_r = nd_b.rx;
	b_r /= 2;
	b_b = m_stParamB.iH/50 + 20;
	if (b_b < nd_b.ry) b_b = nd_b.ry;
	// get spot rect in origin image
	a_l = nd_a.x - a_r;
	a_t = nd_a.y - a_b;
	a_r = nd_a.x + a_r + 1;
	a_b = nd_a.y + a_b + 1;
	b_l = nd_b.x - b_r;
	b_t = nd_b.y - b_b;
	b_r = nd_b.x + b_r + 1;
	b_b = nd_b.y + b_b + 1;
	// width ratio, height ratio: b/a
	rat_w = (b_r - b_l)/(double)(a_r - a_l);
	rat_h = (b_b - b_t)/(double)(a_b - a_t);
	// real rect in image
	a_x0 = (0 > a_l)?0:a_l;
	a_x1 = (a_r < m_stParamA.iW)?a_r:m_stParamA.iW;
	a_y0 = (0 > a_t)?0:a_t;
	a_y1 = (a_b < m_stParamA.iH)?a_b:m_stParamA.iH;
	a_np = (a_y1 - a_y0)*(a_x1 - a_x0);
	b_x0 = (0 > b_l)?0:b_l;
	b_x1 = (b_r < m_stParamB.iW)?b_r:m_stParamB.iW;
	b_y0 = (0 > b_t)?0:b_t;
	b_y1 = (b_b < m_stParamB.iH)?b_b:m_stParamB.iH;
	b_np = (b_y1 - b_y0)*(b_x1 - b_x0);

	// spot mean gray
	a_img = m_stParamA.pImg;
	for (a_y = a_y0; a_y < a_y1; ++a_y)
	{
		for (a_x = a_x0; a_x < a_x1; ++a_x)
		{
			pix_a = a_img[a_y*m_stParamA.iW + a_x].r;
			mean_a += pix_a;
		}
	}
	mean_a /= a_np;
	b_img = m_stParamB.pImg;
	for (b_y = b_y0; b_y < b_y1; ++b_y)
	{
		for (b_x = b_x0; b_x < b_x1; ++b_x)
		{
			pix_b = b_img[b_y*m_stParamB.iW + b_x].r;
			mean_b += pix_b;
		}
	}
	mean_b /= b_np;

	// calculate correlation between a and b, little image map to large image
	if (rat_w*rat_h > 1.0)	// image B large than A, A->B
	{
		for (a_y = a_y0; a_y < a_y1; ++a_y)
		{
			b_y = (int)((a_y - a_t)*rat_h + b_t);
			if (b_y < b_y0 || b_y >= b_y1) continue;

			for (a_x = a_x0; a_x < a_x1; ++a_x)
			{
				b_x = (int)((a_x - a_l)*rat_w + b_l);
				if (b_x < b_x0 || b_x >= b_x1) continue;

				pix_a = a_img[a_y*m_stParamA.iW + a_x].r;
				pix_b = b_img[b_y*m_stParamB.iW + b_x].r;
				laa += (pix_a - mean_a)*(pix_a - mean_a);
				lbb += (pix_b - mean_b)*(pix_b - mean_b);
				rab += (pix_a - mean_a)*(pix_b - mean_b);
			}
		}
	}
	else
	{
		for (b_y = b_y0; b_y < b_y1; ++b_y)
		{
			a_y = (int)((b_y - b_t)/rat_h + a_t);
			if (a_y < a_y0 || a_y >= a_y1) continue;

			for (b_x = b_x0; b_x < b_x1; ++b_x)
			{
				a_x = (int)((b_x - b_l)/rat_w + a_l);
				if (a_x < a_x0 || a_x >= a_x1) continue;

				pix_a = a_img[a_y*m_stParamA.iW + a_x].r;
				pix_b = b_img[b_y*m_stParamB.iW + b_x].r;
				laa += (pix_a - mean_a)*(pix_a - mean_a);
				lbb += (pix_b - mean_b)*(pix_b - mean_b);
				rab += (pix_a - mean_a)*(pix_b - mean_b);
			}
		}
	}
	lab = sqrt(laa)*sqrt(lbb);
	rab = fabs(0.5*(rab/lab + 1));

	return rab;
}

bool GCI::Release()
{
	if (m_nPair > 0)
	{
		int cnt_p = 0;

		if (m_pstMtRet->pvtSpair == nullptr)
			m_pstMtRet->pvtSpair = new VT_SPAIR;
		m_pstMtRet->pvtSpair->resize(m_nPair);

		for (auto it_a = m_vtGsA.begin(); (it_a != m_vtGsA.end())&&(cnt_p <= m_nPair); ++it_a)
		{
			if (it_a->match)
			{
				ST_SPAIR &spair = m_pstMtRet->pvtSpair->at(cnt_p);
				spair.iOdA = it_a->iOrder;
				spair.iOdB = it_a->partner->iOrder;

				++cnt_p;
			}
		}
		if (cnt_p != m_nPair)
		{
			wxASSERT_MSG(false, _T("GCI::Release. Spot pairs' number has error."));
			m_pstMtRet->pvtSpair->resize(cnt_p);
		}
	}
	m_vtGsA.clear();
	m_vtGsB.clear();

	return true;
}
