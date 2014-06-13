/** \brief the matrix class
 *
 * \file ICP.h
 * \author Author:ZHD
 * \date Date:12/06/2014
 *
 * matrix calss, create, destroy, access [], et.
 *
 */

#ifndef ICP_H
#define ICP_H

#include <wx/wx.h>
#include "GEIMDef.h"
// my matrix
#include "GEIMMatrix.h"

class ICP
{
public:
	ICP();
	virtual ~ICP();

	/** \brief icp entry
	 *
	 * \param stMtPair ST_MTPAIR&&	[IN] two gel images to match, and their sopt infomation
	 * \return bool true:sucess false:failed
	 *
	 */
	bool ICPMain(ST_MTPAIR&& stMtPair);

protected:

private:
	bool Init();
	bool Distance();
	bool Correspond();
	bool Transform();
	bool Mahalnobis();


	/**< param for image A */
	ST_MTPARAM m_stParamA;
	/**< param for image B */
	ST_MTPARAM m_stParamB;

	/**< distance and correspondence matrix */
	GEIMMatrix<double, ST_POINT_TRANS, ST_POINT_TRANS> m_mtxDist;
};

#endif // ICP_H
