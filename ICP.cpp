#include "ICP.h"

// math calculate header
#include <math.h>

// implement matrix template
#include "GEIMMatrix.cpp"


ICP::ICP()
{
}

ICP::~ICP()
{
}

/**< ICP entry */
bool ICP::ICPMain(ST_MTPAIR&& stMtPair, ST_MTRESULT* pstMtRet)
{
	bool bRet = true;
	if (pstMtRet == nullptr)
	{
		wxASSERT_MSG(false, _T("ICP::ICPMain parameter is nullptr."));
		return false;
	}
	// image pair
	m_stParamA = stMtPair.first;
	m_stParamB = stMtPair.second;
	m_pstMtRet = pstMtRet;

	m_iIter = -1;
	Init();
	do
	{
		++m_iIter;
		bRet = Distance();
		bRet = bRet && Correspond();
		bRet = bRet && Transform();
	}
	while(bRet && Mahalanobis());

	// use the tps parameter
	MtResult();
	TransImg();

	// release last transform data
	m_TPS.Destroy();

	return bRet;
}

bool ICP::Init()
{
	VT_ATTR* pvtA = m_stParamA.pvtAttr;
	VT_ATTR* pvtB = m_stParamB.pvtAttr;

	int iNumA = (int)pvtA->size();
	int iNumB = (int)pvtB->size();
	m_mtxDist.Resize(iNumB, iNumA);

	// ---------A---------
	int iOrder = -1;
	double mx = 0.0, my = 0.0, dx = 0.0, dy = 0.0;
	iNumA = 0;
	for (auto itA = pvtA->begin(); itA != pvtA->end(); ++itA)
	{
		++iOrder;
		if (itA->bInvalid)
			continue;
		ST_SPOT_NODE& nodeA = *(itA->pNode);
		//ST_SPOT_CHARACT& crtA = *(itA->pCrt);

		ST_POINT_TRANS& stPTA = m_mtxDist.AxisY(iNumA);
		stPTA.iOrder = iOrder;
		stPTA.x = nodeA.x;
		stPTA.y = nodeA.y;
		stPTA.tx = nodeA.x;
		stPTA.ty = nodeA.y;

		// mean
		mx += stPTA.x;
		my += stPTA.y;

		++iNumA;
	}
	mx /= iNumA;
	my /= iNumA;
	// variance
	for (int y = 0; y < iNumA; ++y)
	{
		ST_POINT_TRANS& stPTA = m_mtxDist.AxisY(y);
		double dT = stPTA.x - mx;
		dx += dT*dT;
		dT = stPTA.y - my;
		dy += dT*dT;
	}
	dx = sqrt(dx/iNumA);
	dy = sqrt(dy/iNumA);
	// deviation
	for (int y = 0; y < iNumA; ++y)
	{
		ST_POINT_TRANS& stPTA = m_mtxDist.AxisY(y);
		stPTA.sx = (stPTA.x - mx)/dx;
		stPTA.sy = (stPTA.y - my)/dy;
	}
	// ---------B---------
	iOrder = -1;
	mx = 0.0;
	my = 0.0;
	dx = 0.0;
	dy = 0.0;
	iNumB = 0;
	for (auto itB = pvtB->begin(); itB != pvtB->end(); ++itB)
	{
		++iOrder;
		if (itB->bInvalid)
			continue;
		ST_SPOT_NODE& nodeB = *(itB->pNode);
		//ST_SPOT_CHARACT& crtB = *(itB->pCrt);

		ST_POINT_TRANS& stPTB = m_mtxDist.AxisX(iNumB);
		stPTB.iOrder = iOrder;
		stPTB.x = nodeB.x;
		stPTB.y = nodeB.y;
		stPTB.tx = nodeB.x;
		stPTB.ty = nodeB.y;

		// mean
		mx += stPTB.x;
		my += stPTB.y;

		++iNumB;
	}
	mx /= iNumB;
	my /= iNumB;
	// variance
	for (int x = 0; x < iNumB; ++x)
	{
		ST_POINT_TRANS& stPTB = m_mtxDist.AxisX(x);
		double dT = stPTB.x - mx;
		dx += dT*dT;
		dT = stPTB.y - my;
		dy += dT*dT;
	}
	dx = sqrt(dx/iNumB);
	dy = sqrt(dy/iNumB);
	// deviation
	for (int x = 0; x < iNumB; ++x)
	{
		ST_POINT_TRANS& stPTB = m_mtxDist.AxisX(x);
		stPTB.sx = (stPTB.x - mx)/dx;
		stPTB.sy = (stPTB.y - my)/dy;
	}
	// for some spot is invalid, so static real number
	m_mtxDist.Resize(iNumB, iNumA, true);
	m_mtxCorre.Resize(iNumB, iNumA);
	int iNum = iNumA*iNumB;
	unsigned char* pCorre = m_mtxCorre[0];
	memset(pCorre, 0, iNum);

	return true;
}

bool ICP::Distance()
{
	int iNumA = m_mtxDist.Height();
	int iNumB = m_mtxDist.Width();
	for (int y = 0; y < iNumA; ++y)
	{
		ST_POINT_TRANS& ptA = m_mtxDist.AxisY(y);
		double* dLine = m_mtxDist[y];
		for (int x = 0; x < iNumB; ++x)
		{
			ST_POINT_TRANS& ptB = m_mtxDist.AxisX(x);
			// distance
			dLine[x] = hypot(ptA.x - ptB.tx, ptA.y - ptB.ty);
		}
	}

	return true;
}

bool ICP::Correspond()
{
	int iW = m_mtxDist.Width();
	int iH = m_mtxDist.Height();
	for (int y = 0; y < iH; ++y)
	{
		double* dLD = m_mtxDist[y];
		unsigned char* dLC = m_mtxCorre[y];

		// traverse the line, find minmium item's index
		// minvalue change to 1, otherwise to 0;
		double dMin = dLD[0];
		int iMin = 0;
		dLC[0] = 1u;
		for (int x = 1; x < iW; ++x)
		{
			if (dLD[x] < dMin)
			{
				dLC[iMin] = 0u;
				dMin = dLD[x];
				iMin = x;
				dLC[x] = 1u;
			}
		}
	}
	// ensure one-to-one match
	for (int x = 0; x < iW; ++x)
	{
		unsigned char* pCol = m_mtxCorre[0] + x;
		bool bFind = false;
		for (int y = 0; y < iH; ++y)
		{
			if (*pCol == 1)
			{
				if (!bFind)			// the first reserve
					bFind = true;
				else				// erase all other in the column
					*pCol = 0u;
			}

			pCol += iW;
		}
	}
	return true;
}

bool ICP::Transform()
{
	// release last data
	m_TPS.Destroy();
	// genetic the match pair
	VT_TPS_PAIRS* pvtPair = new VT_TPS_PAIRS;
	int iW = m_mtxDist.Width();
	int iH = m_mtxDist.Height();
	for (int y = 0; y < iH; ++y)
	{
		unsigned char* dLC = m_mtxCorre[y];
		for (int x = 0; x < iW; ++x)
		{
			if (*dLC == 1)
			{
				Coorder crr;
				ST_POINT_TRANS& stPTA = m_mtxDist.AxisY(y);
				ST_POINT_TRANS& stPTB = m_mtxDist.AxisX(x);
				crr.x = stPTB.x;	// B->A
				crr.y = stPTB.y;
				crr.dx = stPTA.x;
				crr.dy = stPTA.y;
				pvtPair->push_back(crr);
				break;
			}
			++dLC;
		}
	}

	// calculate TPS parameter
	if(m_TPS.TPSMain(pvtPair, m_dLambda))
	{
		// transform's smoothness
		m_dJt = m_TPS.Ibn();

		// re-calculate transforms of all the control point in B, used in next iteration
		Coorder crr;
		for (int x = 0; x < iW; ++x)
		{
			ST_POINT_TRANS& stPTB = m_mtxDist.AxisX(x);
			crr.x = stPTB.x;
			crr.y = stPTB.y;
			crr.dx = 0.0;
			crr.dy = 0.0;
			m_TPS.morph(&crr);
			stPTB.tx = crr.dx;
			stPTB.ty = crr.dy;
		}

		// calculate deviation
		double mx = 0.0, my = 0.0, dx = 0.0, dy = 0.0;
		for (int x = 0; x < iW; ++x)
		{
			ST_POINT_TRANS& stPTB = m_mtxDist.AxisX(x);
			mx += stPTB.tx;
			my += stPTB.ty;
		}
		mx /= iW;
		my /= iW;
		// variance
		for (int x = 0; x < iW; ++x)
		{
			ST_POINT_TRANS& stPTB = m_mtxDist.AxisX(x);
			double dT = stPTB.tx - mx;
			dx += dT*dT;
			dT = stPTB.ty - my;
			dy += dT*dT;
		}
		dx = sqrt(dx/iW);
		dy = sqrt(dy/iW);
		// deviation
		for (int x = 0; x < iW; ++x)
		{
			ST_POINT_TRANS& stPTB = m_mtxDist.AxisX(x);
			stPTB.sx = (stPTB.tx - mx)/dx;
			stPTB.sy = (stPTB.ty - my)/dy;
		}

		// statics mean correctence position of every point in A, used in converge function
		for (int y = 0; y < iH; ++y)
		{
			unsigned char* pLC = m_mtxCorre[y];
			double mm = 0.0, mx = 0.0, my = 0.0;
			for (int x = 0; x < iW; ++x)
			{
				mm += *pLC;
				ST_POINT_TRANS& stPTB = m_mtxDist.AxisX(x);
				mx += *pLC * stPTB.x;
				my += *pLC * stPTB.y;

				++pLC;
			}
			// no match pair
			if (fabs(mm) < 1e-6)
				continue;
			crr.x = mx/mm;
			crr.y = my/mm;
			crr.dx = 0.0;
			crr.dy = 0.0;
			m_TPS.morph(&crr);

			ST_POINT_TRANS& stPTA = m_mtxDist.AxisY(y);
			stPTA.tx = crr.dx;
			stPTA.ty = crr.dy;
		}

		return true;
	}

	return false;
}

bool ICP::Mahalanobis()
{
	/**< last energy */
	static double sEnergy = 0.0;

	int iW = m_mtxDist.Width();
	int iH = m_mtxDist.Height();

	// sum from i=1 to N %iomega_i %iDELTA^T S^{-1}_i %iDELTA + %ilambda J_T
	double dEnergy = 0.0;
	// sum from i=1 to N %iomega_i %iDELTA^T S^{-1}_i %iDELTA
	for (int y = 0; y < iH; ++y)
	{
		// %omega_i
		double wy = 0.0;
		// S_i = S_{Ai} + bar S_B, S^{-1}_i
		double sy[4] = {0.0}, sy_1[4] = {0.0};
		// sum from j m_{ij}
		double my = 0.0;

		double dT = 0.0;
		unsigned char* pLC = m_mtxCorre[y];
		for (int x = 0; x < iW; ++x)
		{
			ST_POINT_TRANS& stPTB = m_mtxDist.AxisX(x);
			// m_{ij}
			dT = *pLC;
			// sum from j m^2_{ij}
			wy += dT*dT;
			// sum from j S_Bj
			sy[0] += dT*stPTB.sx*stPTB.sx;
			sy[1] += dT*stPTB.sx*stPTB.sy;
			sy[2] += dT*stPTB.sy*stPTB.sx;
			sy[3] += dT*stPTB.sy*stPTB.sy;
			// sum from j m_{ij}
			my += dT;

			++pLC;
		}
		// no match pair
		if (fabs(my) < 1e-6)
			continue;

		ST_POINT_TRANS& stPTA = m_mtxDist.AxisY(y);
		wy = sqrt(wy);
		sy[0] = sy[0]/my + stPTA.sx*stPTA.sx;
		sy[1] = sy[1]/my + stPTA.sx*stPTA.sy;
		sy[2] = sy[2]/my + stPTA.sy*stPTA.sx;
		sy[3] = sy[3]/my + stPTA.sy*stPTA.sy;

		// 2D matrics inverse
		dT = sy[0]*sy[2] - sy[1]*sy[3];
		wxASSERT_MSG(fabs(dT) > 1e-15, _T("1/(ad-bc) infinity."));
		sy_1[0] = sy[3]/dT;
		sy_1[1] = -sy[1]/dT;
		sy_1[2] = -sy[2]/dT;
		sy_1[3] = sy[0]/dT;

		// %iDELTA
		sy[2] = stPTA.x - stPTA.tx;
		sy[3] = stPTA.y - stPTA.ty;

		// %iomega_i %iDELTA^T S^{-1}_i %iDELTA
		sy[0] = sy[2]*sy_1[0]*sy[2];
		sy[1] = sy[2]*sy_1[1]*sy[3];
		sy[2] = sy[3]*sy_1[2]*sy[2];
		sy[3] = sy[3]*sy_1[3]*sy[3];
		dEnergy += wy*(sy[0] + sy[1] + sy[2] + sy[3]);
	}	// sum energy

	// %lambda J_T
	dEnergy += m_dLambda*m_dJt;

	// converged ?
	if(m_iIter >= 10)
		return false;
	if (m_iIter > 0)	// not the fist iteration
	{
		// delta enough small
		if (fabs(dEnergy - sEnergy) < 1e-6)
			// stop iteration
			return false;
	}
	sEnergy = dEnergy;

	return true;
}

bool ICP::MtResult()
{
	if (m_pstMtRet->pvtSpair == nullptr)
		m_pstMtRet->pvtSpair = new VT_SPAIR;
	VT_SPAIR* pvtPair = m_pstMtRet->pvtSpair;
	if (pvtPair->size() > 0)
		pvtPair->clear();

	int iW = m_mtxDist.Width();
	int iH = m_mtxDist.Height();
	for (int y = 0; y < iH; ++y)
	{
		unsigned char* dLC = m_mtxCorre[y];
		for (int x = 0; x < iW; ++x)
		{
			if (*dLC == 1)
			{
				ST_SPAIR par;
				ST_POINT_TRANS& stPTA = m_mtxDist.AxisY(y);
				ST_POINT_TRANS& stPTB = m_mtxDist.AxisX(x);
				par.iOdA = stPTA.iOrder;
				par.iOdB = stPTB.iOrder;
				pvtPair->push_back(par);
				break;
			}
			++dLC;
		}
	}
	return true;
}

bool ICP::TransImg()
{
	int iW = m_stParamA.iW;
	int iH = m_stParamA.iH;
	int oW = m_stParamB.iW;
	int oH = m_stParamB.iH;
	PST_RGB pSrc = m_stParamA.pImg;

	PST_RGB pDes = new ST_RGB[oW*oH];
	memset(pDes, 0, oW*oH*sizeof(ST_RGB));
	m_TPS.TransImg(pSrc, iW, iH, pDes, oW, oH);

	int iN = m_stParamA.iN;
	PST_RGB pA = pDes;
	PST_RGB pB = m_stParamB.pImg;
	for (int i = 0; i < iN; ++i)
	{
		pB->g = pA->r;
		pB->b = 0u;
		++pA;
		++pB;
	}

	delete [] pDes;

	return true;
}

