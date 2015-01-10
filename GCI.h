#ifndef GCI_H
#define GCI_H

#include "GEIMDef.h"

template<typename _T, typename _Tx, typename _Ty> class GEIMMatrix;

/**< GCI Similarity Matric Index */
struct ST_GCI_SMD
{
	int order = -1;
	ST_SPOT_ATTR * spot_attr = nullptr;
};

/**< GCI Similarity Matrix Item*/
struct ST_GCI_SMI
{
	double ovlp = 0.0;	/**< overlap */
	double patn = 0.0;	/**< pattern */
	double stct = 0.0;	/**< structral */
	double spct = 0.0;	/**< shape contex */
};

// similarity function format
class GCI;
typedef bool (GCI::*_func_simi)(ST_SPOT_ATTR* spot_a, int id_a, ST_SPOT_ATTR* spot_b, int id_b);

class GCI
{
public:
	GCI();
	virtual ~GCI();

	bool Main(ST_MTPAIR&& stMtPair, ST_MTRESULT* pstMtRet);

protected:

private:
	/** \brief normalization the image
	 *
	 * \param stParam ST_MTPARAM&	[IN] the image's param
	 * \return bool	true:success false:failed
	 *
	 */
	bool Normalize(ST_MTPARAM& stParam);
	/** \brief calculate charact-vector for the image's spotd
	 *
	 * \param stParam ST_MTPARAM&
	 * \return bool
	 *
	 * level: 0强 1中 2弱
	 */
	bool CalcuPattern(ST_MTPARAM& stParam);
	/** \brief traverse one spot's pixels
	 *
	 * \param stParam ST_MTPARAM&	[IN] image
	 * \param spot ST_SPOT_NODE&	[IN] spot
	 * \param __f _Func				[IN] the callback function
	 * \return bool
	 *
	 * _Func: bool XXX(int id);
	 *
	 */
	template<typename _Func> bool SpotPixs(ST_MTPARAM& stParam, ST_SPOT_NODE& spot, _Func __f);
    /** \brief calculate which level spot located in
     *
     * \param stParam ST_MTPARAM&
     * \return bool
     *
     */
	bool CalcuLevel(ST_MTPARAM& stParam);
    /** \brief calculate spots normalize coordnation by all spots
     *
     * \param stParam ST_MTPARAM&
     * \return bool
     *
     */
	bool CalcuCoord_all(ST_MTPARAM& stParam);
    /** \brief calculate spots normalize coordnation by mathed spots
     *
     * \param stParam ST_MTPARAM&
     * \return bool
     *
     */
	bool CalcuCoord_match(ST_MTPARAM& stParam);
    /** \brief calculate shpae-contex similarity
     *
     * \param pt_set const int*		[IN] points set
     * \param pt_num int			[IN] points' count
     * \param mean double&			[INOUT] mean distance
     * \param bins_radius int		[IN] radius merotomy
     * \param bins_theta int		[IN] theta-angled merotomy
     * \param histogram double *const	[OUT] the shape-contex histogram
     * \return bool
     *
     */
	bool calcu_sc(const int * pt_set, int pt_num, double &mean, int bins_radius, int bins_theta, double *const histogram);
    /** \brief calculate distance-diagram of the points' set
     *
     * \param pt_set const int*		[IN] points set
     * \param pt_num int			[IN] points' count
     * \param mean double&			[INOUT] mean distance
     * \param bins_radius int		[IN] radius merotomy
     * \param distgram int *const	[OUT] the distance diagram
     * \return bool
     *
     */
	bool calcu_sc_radiu(const int * pt_set, int pt_num, double &mean, int bins_radius, int *const distgram);
    /** \brief calculate theta-diagram of the points' set
     *
     * \param pt_set const int*		[IN] points set
     * \param pt_num int			[IN] points' count
     * \param bins_theta int		[IN] theta-angled merotomy
     * \param thetagram int *const	[OUT] the theta-angle diagram
     * \return bool
     *
     */
	bool calcu_sc_theta(const int * pt_set, int pt_num, int bins_theta, int *const thetagram);


    /** \brief initial the similarity matrix
     *
     * \return bool
     *
     */
	bool simi_init();
    /** \brief iterate all spots in two images, invoke __f do process
     *
     * \param __f _Func		[IN] two spots process function
     * \return bool
     *
     * __f: bool xxx(ST_SPOT_ATTR* spot_a, int id_a, ST_SPOT_ATTR* spot_b, int id_b);
     *      if __f return true, continue ietral, else terminate
     */
	bool simi_it(_func_simi __f);
    /** \brief claculate spots overlap similarity
     *
     * \param spot_a ST_SPOT_ATTR*	[IN] attribution of a spot in image A
     * \param id_a int				[IN] the index(order) of  a spot in image A
     * \param spot_b ST_SPOT_ATTR*	[IN] attribution of a spot in image B
     * \param id_b int				[IN] the index(order) of  a spot in image B
     * \return bool
     *
     */
	bool simi_overlap();
	bool simi_overlap(ST_SPOT_ATTR* spot_a, int id_a, ST_SPOT_ATTR* spot_b, int id_b);
    /** \brief claculate spots pattern  similarity
     *
     */
	bool simi_pattern();
	bool simi_pattern(ST_SPOT_ATTR* spot_a, int id_a, ST_SPOT_ATTR* spot_b, int id_b);
    /** \brief claculate spots structural similarity
     *
     */
	bool simi_structral();
	bool simi_structral(ST_SPOT_ATTR* spot_a, int id_a, ST_SPOT_ATTR* spot_b, int id_b);
    /** \brief claculate spots shpe-contex similarity
     *
     */
	bool simi_shapecontex();
	bool simi_shapecontex(ST_SPOT_ATTR* spot_a, int id_a, ST_SPOT_ATTR* spot_b, int id_b);

	bool Match();
	bool mt_cull();

	bool InitThresold();


	/** \brief export the last result, release temporary match result
	 *
	 * \return bool true; false;
	 *
	 */
	bool Release();

	/**< level count */
	static constexpr int m_iLevelNum = 3;

	/**< similarity matrix, w:num_b h:num_a */
	GEIMMatrix<ST_GCI_SMI, ST_GCI_SMD, ST_GCI_SMD> * m_pmxSimi = nullptr;

	/**< param for image A */
	ST_MTPARAM m_stParamA;
	/**< param for image B */
	ST_MTPARAM m_stParamB;
	/**< shap context array */
	int m_sc_num = 0;
	double * m_sc_histogram[2] = {nullptr, nullptr};

	/**< match result */
	ST_MTRESULT* m_pstMtRet = nullptr;
};

#endif // GCI_H
