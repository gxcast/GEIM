/** \brief the matrix class
 *
 * \file ICP.h
 * \author Author:ZHD
 * \date Date:12/06/2014
 *
 * matrix calss, create, destroy, access [], et.
 *
 */

#pragma once
#ifndef ICP_H
#define ICP_H

#include "GEIMDef.h"
// my matrix
#include "GEIMMatrix.h"
// TPS
#include "TPS_2D.h"

class ICP
{
public:
	ICP();
	virtual ~ICP();

	/** \brief icp entry
	 *
	 * \param stMtPair ST_MTPAIR&&	[IN] two gel images to match, and their sopt infomation
	 * \param pstMtRet ST_MTRESULT*	[IN] match result
	 * \return bool true:sucess false:failed
	 *
	 */
	bool ICPMain(ST_MTPAIR&& stMtPair, ST_MTRESULT* pstMtRet);

protected:

private:
	bool Init();
	bool Distance();
	bool Correspond0();
	bool Correspond1();
	bool Transform();
	bool Mahalanobis();

	bool MtResult();
	bool TransImg();

	/**< param for image A */
	ST_MTPARAM m_stParamA;
	/**< param for image B */
	ST_MTPARAM m_stParamB;
	/**< match result */
	ST_MTRESULT* m_pstMtRet = nullptr;

	/**< distance and correspondence matrix */
	GEIMMatrix<double, ST_POINT_TRANS, ST_POINT_TRANS> m_mtxDist;
	GEIMMatrix<unsigned char, char, char> m_mtxCorre;

	/**< TPS transform */
	CTPS_2d_Morpher m_TPS;

	/**< lambda： TPS's looseness */
	static constexpr double m_dLambda = 0.5;
	/**< J_T: TPS's smooness */
	double m_dJt = 0.0;
	/**< iteration's num */
	int m_iIter = -1;
};

#endif // ICP_H
