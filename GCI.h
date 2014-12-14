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
	bool stratify_one(ST_MTPARAM &param, int *scale, VT_GCI &vt_gci);

	bool CoarseMT();

	bool InitThresold();
	int least_pair_num();

	/** \brief match use clp method,
	 *
	 * \param mode int	[IN] bit-filed, 0x01 multi-layer, 0x02 mean-center
	 * \return bool true ; false;
	 *
	 * pair-number is the m_nPair.
	 */
	bool LayerMT(int mode);

	/** \brief get a mean center positon in the specified method
	 *
	 * \param global bool true: all the spots in the center 2/3 of the iamge; false: the matched pair
	 * \return bool true
	 *
	 */
	bool GetCenter(bool global);
	bool center_global(wxPoint2DDouble &pt, ST_MTPARAM &param);
	bool center_mp(wxPoint2DDouble &pt, ST_MTPARAM &param, VT_GCI &vt_gci);

	double Similary(ST_GCINODE &gci_a, ST_GCINODE &gci_b, bool mean_center);
	double simi_overlap(ST_GCINODE &gci_a, ST_GCINODE &gci_b, bool mean_center);
	double simi_intensity(ST_GCINODE &gci_a, ST_GCINODE &gci_b);

	bool adjoining_distance();

	/** \brief clear all the match-flag in gci-vector and the pair-num
	 *
	 * \return bool true;
	 *
	 */
	bool clean_mt_result();

	bool cull_mt_result();

	/** \brief export the last result, release temporary match result
	 *
	 * \return bool true; false;
	 *
	 */
	bool Release();

	/**< number of match pair */
	int m_nPair = 0;

	/**< center of image */
	wxPoint2DDouble m_ptCenterA;
	wxPoint2DDouble m_ptCenterB;

	/**< thresold of every character */
	double m_dFactor = 0.0;
	double m_trdOvlp = 0.0;
	double m_trdItst = 0.0;
	double m_trdSimi = 0.0;

	/**< stratified infomation */
	int m_aScaleA[NUM_STRATIFY] = {0};
	VT_GCI m_vtGciA;
	int m_aScaleB[NUM_STRATIFY] = {0};
	VT_GCI m_vtGciB;

	/**< param for image A */
	ST_MTPARAM m_stParamA;
	/**< param for image B */
	ST_MTPARAM m_stParamB;

	/**< match result */
	ST_MTRESULT* m_pstMtRet = nullptr;
};

#endif // GCI_H
