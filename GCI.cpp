#include "GCI.h"

#include <math.h>
#include <float.h>
#include <wx/file.h>

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
	stratify_one(m_stParamA, m_aScaleA, m_vtGciA);
	stratify_one(m_stParamB, m_aScaleB, m_vtGciB);
	return true;
}

bool GCI::stratify_one(ST_MTPARAM &param, int *scale, VT_GCI &vt_gci)
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
	vt_gci.resize(cnt_nd);
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
				ST_GCINODE& nd_gci = vt_gci[idx];
				// init gci-node
				nd_gci.iOrder = cnt_nd;
				nd_gci.level = lv;
				nd_gci.tran_x = it->pNode->x;
				nd_gci.tran_y = it->pNode->y;

				cnt_lv[lv] += 1;
				break;
			}
		}
	}

	return true;
}

bool GCI::CoarseMT()
{
	int num_trd = 0;
	// least pair number thresold
	num_trd = least_pair_num();
	// similary thresold
	InitThresold();

	// only match level 0, get the initial mean-center
	m_nPair = 0;
	GetCenter(true);
	LayerMT(0x02);	// 0-layer, mean-center
	// depend on first match result, calculate center
	if (m_nPair <= 0)
		return false;
	GetCenter(false);

	// clean first match result
	clean_mt_result();

	// iterate match
	do
	{
		// do match for every level, use mean-center
		LayerMT(0x03);

		// whether pair number enough, else, decrease thresold
		if (m_nPair >= num_trd)
			break;
		m_trdOvlp -= 0.05;
		m_trdItst -= 0.05;
		m_trdSimi = m_dFactor*m_trdItst + (1.0 - m_dFactor)*m_trdOvlp;
	}
	while(m_trdSimi > 0.60);

	cull_mt_result();

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

int GCI::least_pair_num()
{
	int num_b = 0, num_trd = 0;

	num_trd = (int)m_vtGciA.size();
	num_b = (int)m_vtGciB.size();
	if (num_b < num_trd)
		num_trd = num_b;
	num_trd /= 4;
	if (num_trd < 10)
		num_trd = 10;

	return num_trd;
}

/** \brief match use clp method,
 *
 * \param mode int	[IN] bit-filed, 0x01 multi-layer, 0x02 mean-center
 * \return bool true ; false;
 *
 * pair-number is the m_nPair.
 */
bool GCI::LayerMT(int mode)
{
	bool multi_layer = ((mode & 0x01) != 0);
	bool mean_center = ((mode & 0x02) != 0);

	// calculate all pots' minmium distance to their neighbor
	adjoining_distance();

	// do match for every level
	for (int lv = 0; lv < NUM_STRATIFY; ++lv)
	{
		// use has matched pair to calculate center
		if (mean_center && m_nPair > 0)
			GetCenter(false);

		// one-to-one match
		for (auto it_a = m_vtGciA.begin(); (it_a != m_vtGciA.end())&&(it_a->level <= lv); ++it_a)
		{
			if (it_a->match)
				continue;
			for (auto it_b = m_vtGciB.begin(); (it_b != m_vtGciB.end())&&(it_b->level <= lv); ++it_b)
			{
				if (!it_b->match)
					Similary(*it_a, *it_b, mean_center);
			}
		}

		// verify match result
		for (auto it_a = m_vtGciA.begin(); (it_a != m_vtGciA.end())&&(it_a->level <= lv); ++it_a)
		{
			if (it_a->match)
				continue;

			ST_GCINODE *part = it_a->partner;
			if (it_a->ovlp > m_trdOvlp && it_a->simi > m_trdSimi &&
			        !part->match && part->partner != nullptr && it_a->iOrder == part->partner->iOrder)
			{
				bool oneone = true;
				if (multi_layer)	// if only layer0, not check
				{
					for (auto it_b = m_vtGciB.begin(); (it_b != m_vtGciB.end())&&(it_b->level <= lv); ++it_b)
					{
						if (it_b->partner != nullptr &&
						        it_b->partner->iOrder == it_a->iOrder &&
						        it_b->iOrder != part->iOrder)
						{
							oneone = false;
							break;
						}
					}
				}
				if (oneone)
				{
					it_a->match = true;
					part->match = true;
					++m_nPair;
				}
			}
		}	// verify

		if (!multi_layer)
			break;
	}	// every level

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
		center_mp(m_ptCenterA, m_stParamA, m_vtGciA);
		center_mp(m_ptCenterB, m_stParamB, m_vtGciB);
	}
	return true;
}

bool GCI::center_global(wxPoint2DDouble &pt, ST_MTPARAM &param)
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

	pt.m_x = cent_x;
	pt.m_y = cent_y;

	return true;
}

bool GCI::center_mp(wxPoint2DDouble &pt, ST_MTPARAM &param, VT_GCI &vt_gci)
{
	double cent_x = 0.0, cent_y = 0.0;
	int num = 0;

	for (auto it = vt_gci.begin(); it != vt_gci.end(); ++it)
	{
		if (!it->match)
			continue;
		cent_x += it->tran_x;
		cent_y += it->tran_y;
		++num;
	}
	cent_x /= num;
	cent_y /= num;

	pt.m_x = cent_x;
	pt.m_y = cent_y;

	return true;
}

double GCI::Similary(ST_GCINODE &gci_a, ST_GCINODE &gci_b, bool mean_center)
{
	double sm_ovlp = 0.0, sm_itst = 0.0, sm_syn = 0.0;

	// similary of position
	sm_ovlp = simi_overlap(gci_a, gci_b, mean_center);
	if (sm_ovlp < m_trdOvlp - 0.05)
		return false;

	// similary of intensity
	sm_itst = simi_intensity(gci_a, gci_b);

	// synthetical
	sm_syn = m_dFactor*sm_itst + (1 - m_dFactor)*sm_ovlp;
	if (sm_syn < m_trdSimi - 0.05)
		return false;

	// record most match
	if (sm_ovlp > gci_a.ovlp && sm_syn > gci_a.simi)
	{
		gci_a.ovlp = sm_ovlp;
		gci_a.itst = sm_itst;
		gci_a.simi = sm_syn;
		gci_a.partner = &gci_b;
	}
	if (sm_ovlp > gci_b.ovlp && sm_syn > gci_b.simi)
	{
		gci_b.ovlp = sm_ovlp;
		gci_b.itst = sm_itst;
		gci_b.simi = sm_syn;
		gci_b.partner = &gci_a;
	}

	return true;
}

double GCI::simi_overlap(ST_GCINODE &gci_a, ST_GCINODE &gci_b, bool mean_center)
{
	double simi = 0.0;
	double a_x = gci_a.tran_x;
	double a_y = gci_a.tran_y;
	double b_x = gci_b.tran_x;
	double b_y = gci_b.tran_y;
	double rt_x = 0.0, rt_y = 0.0;

	if (mean_center)
	{
		rt_x = fabs((a_x - m_ptCenterA.m_x)/(double)m_stParamA.iW - (b_x - m_ptCenterB.m_x)/(double)m_stParamB.iW);
		rt_y = fabs((a_y - m_ptCenterA.m_y)/(double)m_stParamA.iH - (b_y - m_ptCenterB.m_y)/(double)m_stParamB.iH);
		if (rt_x > 1.0)
			rt_x = 1.0;
		if (rt_y > 1.0)
			rt_y = 1.0;
		simi = (1.0 - rt_x)*(1.0 - rt_y);
	}
	else
	{
		rt_x = fabs((a_x - b_x)/(double)m_stParamB.iW);
		rt_y = fabs((a_y - b_y)/(double)m_stParamB.iH);
		if (rt_x > 1.0)
			rt_x = 1.0;
		if (rt_y > 1.0)
			rt_y = 1.0;
		simi = (1.0 - rt_x)*(1.0 - rt_y);
	}
	return simi;
}

double GCI::simi_intensity(ST_GCINODE &gci_a, ST_GCINODE &gci_b)
{
	ST_SPOT_NODE *nd_a = m_stParamA.pvtAttr->at(gci_a.iOrder).pNode;
	ST_SPOT_NODE *nd_b = m_stParamB.pvtAttr->at(gci_b.iOrder).pNode;

	int a_l = 0, a_t = 0, a_r = 0, a_b = 0;
	int b_l = 0, b_t = 0, b_r = 0, b_b = 0;
	int a_x0 = 0, a_x1 = 0, a_y0 = 0, a_y1 = 0;
	int b_x0 = 0, b_x1 = 0, b_y0 = 0, b_y1 = 0;
	double rat_w = 0.0, rat_h = 0.0;
	int a_np = 0, b_np = 0;
	int a_x = 0, a_y = 0, b_x = 0, b_y = 0;
	ST_RGB *a_img = nullptr, *b_img = nullptr;
	double mean_a = 0.0, mean_b = 0.0;
	unsigned char pix_a = 0u, pix_b = 0u;
	double rab = 0.0, lab = 0.0, laa = 0.0, lbb = 0.0;

	// calculate extend radius
	/*a_r = m_stParamA.iW/50 + 20;
	if (a_r < nd_a->rx) a_r = nd_a->rx;
	a_r = a_r/2 + nd_a->rx;
	a_b = m_stParamA.iH/50 + 20;
	if (a_b < nd_a->ry) a_b = nd_a->ry;
	a_b = a_b/2 + nd_a->ry;
	b_r = m_stParamB.iW/50 + 20;
	if (b_r < nd_b->rx) b_r = nd_b->rx;
	b_r = b_r/2 + nd_b->rx;
	b_b = m_stParamB.iH/50 + 20;
	if (b_b < nd_b->ry) b_b = nd_b->ry;
	b_b = b_b/2 + nd_b->ry;
	// get spot rect in origin image
	a_l = nd_a->x - a_r;
	a_t = nd_a->y - a_b;
	a_r = nd_a->x + a_r + 1;
	a_b = nd_a->y + a_b + 1;
	b_l = nd_b->x - b_r;
	b_t = nd_b->y - b_b;
	b_r = nd_b->x + b_r + 1;
	b_b = nd_b->y + b_b + 1;*/
	rab = (gci_a.crct > gci_b.crct) ? gci_a.crct:gci_b.crct;
	lab = nd_a->rx;
	if (lab < nd_a->ry) lab = nd_a->ry;
	if (lab < nd_b->rx) lab = nd_b->rx;
	if (lab < nd_b->ry) lab = nd_b->ry;
	if (rab > 3*lab) rab = 2*lab;
	b_b = (int)rab;
	a_l = nd_a->x - b_b;
	a_t = nd_a->y - b_b;
	a_r = nd_a->x + b_b + 1;
	a_b = nd_a->y + b_b + 1;
	b_l = nd_b->x - b_b;
	b_t = nd_b->y - b_b;
	b_r = nd_b->x + b_b + 1;
	b_b = nd_b->y + b_b + 1;
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
	rab = lab = laa = lbb = 0.0;
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

bool GCI::adjoining_distance()
{
	// set all the distance to infinite
	auto fuc_cl = [](VT_GCI &gci)
	{
		for (auto it = gci.begin(); it != gci.end(); ++it)
			it->crct = DBL_MAX;
	};
	fuc_cl(m_vtGciA);
	fuc_cl(m_vtGciB);

	// statistics of minimium distance
	auto fuc_min = [](VT_GCI &gci)
	{
		auto it_end_2 = gci.end();
		auto it_end_1 = it_end_2 - 1;
		double d_x = 0.0, d_y = 0.0, d_dist = 0.0;
		for (auto it_1 = gci.begin(); it_1 != it_end_1; ++it_1)
		{
			for (auto it_2 = it_1 + 1; it_2 != it_end_2; ++it_2)
			{
				d_x = fabs(it_1->tran_x - it_2->tran_x);
				d_y = fabs(it_1->tran_y - it_2->tran_y);
				d_dist = sqrt(d_x*d_x + d_y*d_y);

				if (it_1->crct > d_dist)
					it_1->crct = d_dist;
				if (it_2->crct > d_dist)
					it_2->crct = d_dist;
			}
		}
	};
	fuc_min(m_vtGciA);
	fuc_min(m_vtGciB);

	return true;
}

bool GCI::clean_mt_result()
{
	auto cl_vt = [](VT_GCI &vt_gci)
	{
		for (auto it = vt_gci.begin(); it != vt_gci.end(); ++it)
		{
			it->match = false;
			it->partner = nullptr;
			it->ovlp = 0.0;
			it->itst = 0.0;
			it->simi = 0.0;
		}
	};
	cl_vt(m_vtGciA);
	cl_vt(m_vtGciB);

	m_nPair = 0;

	return true;
}

bool GCI::cull_mt_result()
{
	// write pair to file
	wxFile fl_out;
	fl_out.Open(_T("/home/nicolas/Desktop/out.txt"), wxFile::write);
	for (auto it_a = m_vtGciA.begin(); it_a != m_vtGciA.end(); ++it_a)
	{
		if (it_a->match)
		{
			wxString str_out;
			str_out.Printf(_T("%lf %lf %lf %lf\r\n"),
			               it_a->tran_x, it_a->tran_y,
			               it_a->partner->tran_x, it_a->partner->tran_y);
			fl_out.Write(str_out);
		}
	}
	fl_out.Close();

	return true;
}

bool GCI::Release()
{
	if (m_nPair > 0)
	{
		int cnt_p = 0;

		if (m_pstMtRet->pvtSpair == nullptr)
			m_pstMtRet->pvtSpair = new VT_SPAIR;
		m_pstMtRet->pvtSpair->resize(m_nPair);

		for (auto it_a = m_vtGciA.begin(); (it_a != m_vtGciA.end())&&(cnt_p <= m_nPair); ++it_a)
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
	m_vtGciA.clear();
	m_vtGciB.clear();

	return true;
}
