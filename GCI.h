#ifndef GCI_H
#define GCI_H

#include "GEIMDef.h"

// the number of stratification
#define NUM_STRATIFY	3

class GCI
{
public:
	GCI();
	virtual ~GCI();

	bool Main(ST_MTPAIR&& stMtPair, ST_MTRESULT* pstMtRet);

protected:

private:
	bool Stratify();
	bool stratify_one(ST_MTPARAM &param, int *scale, VT_GS &vt_gs);

	bool CoarseMT();

	bool InitThresold();

	bool GetCenter(bool global);
	bool center_global(wxPoint &pt, ST_MTPARAM &param);
	bool center_mp(wxPoint &pt, ST_MTPARAM &param, VT_GS &vtgs);

	double Similary(ST_GSNODE &gs_a, ST_GSNODE &gs_b);
	double simi_overlap(int a_x, int a_y, int b_x, int b_y);
	double simi_intensity(ST_SPOT_NODE &nd_a, ST_SPOT_NODE &nd_b);

	bool Release();

	/**< number of match pair */
	int m_nPair = 0;
	/**< center of image */
	wxPoint m_ptCenterA;
	wxPoint m_ptCenterB;

	/**< thresold of every character */
	double m_dFactor = 0.0;
	double m_trdOvlp = 0.0;
	double m_trdItst = 0.0;
	double m_trdSimi = 0.0;

	/**< stratified infomation */
	int m_aScaleA[NUM_STRATIFY] = {0};
	VT_GS m_vtGsA;
	int m_aScaleB[NUM_STRATIFY] = {0};
	VT_GS m_vtGsB;

	/**< param for image A */
	ST_MTPARAM m_stParamA;
	/**< param for image B */
	ST_MTPARAM m_stParamB;

	/**< match result */
	ST_MTRESULT* m_pstMtRet = nullptr;
};

#endif // GCI_H
