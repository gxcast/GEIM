#ifndef CHARACTVECT_H
#define CHARACTVECT_H

#include <wx/wx.h>
// generic define
#include "GEIMDef.h"

class CharactVect
{
public:
	CharactVect();
	virtual ~CharactVect();

	/** \brief match main entry
	 *
	 * \param stMtPair ST_MTPAIR&&	[IN] two gel images to match, and their sopt infomation
	 * \param pstMtRet ST_MTRESULT*	[IN] match result
	 * \return bool true:success false:failed
	 *
	 */
	bool CVMain(ST_MTPAIR&& stMtPair, ST_MTRESULT* pstMtRet);

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
	 */
	bool CalcuCharacr(ST_MTPARAM& stParam);

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
	template<typename _Func>
	bool SpotPixs(ST_MTPARAM& stParam, ST_SPOT_NODE& spot, _Func __f);

	/**< param for image A */
	ST_MTPARAM m_stParamA;
	/**< param for image B */
	ST_MTPARAM m_stParamB;

	/**< match result */
	ST_MTRESULT* m_pstMtRet = nullptr;
};

#endif // CHARACTVECT_H
