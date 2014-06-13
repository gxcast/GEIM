#include "ICP.h"

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
bool ICP::ICPMain(ST_MTPAIR&& stMtPair)
{
	// image pair
	m_stParamA = stMtPair.first;
	m_stParamB = stMtPair.second;

	Init();
	do
	{
		Distance();
		Correspond();
		Transform();
	}
	while(Mahalnobis());

	return true;
}

bool ICP::Init()
{
	VT_ATTR* pvtA = m_stParamA.pvtAttr;
	VT_ATTR* pvtB = m_stParamB.pvtAttr;

	m_mtxDist.Destroy();
	int iNumA = (int)pvtA->size();
	int iNumB = (int)pvtB->size();
	m_mtxDist.Create(iNumB, iNumA);

	iNumA = 0;
	for (auto itA = pvtA->begin(); itA != pvtA->end(); ++itA)
	{
		if (itA->bInvalid)
			continue;
		ST_SPOT_CHARACT& crtA = *(itA->pCrt);

		ST_POINT_TRANS& tran = m_mtxDist.AxisY(iNumA);
		tran.x = crtA.x;
		tran.y = crtA.y;
		tran.tx = crtA.x;
		tran.ty = crtA.y;

		++iNumA;
	}
	iNumB = 0;
	for (auto itB = pvtB->begin(); itB != pvtB->end(); ++itB)
	{
		if (itB->bInvalid)
			continue;
		ST_SPOT_CHARACT& crtB = *(itB->pCrt);

		ST_POINT_TRANS& tran = m_mtxDist.AxisX(iNumB);
		tran.x = crtB.x;
		tran.y = crtB.y;
		tran.tx = crtB.x;
		tran.ty = crtB.y;

		++iNumB;
	}
	// for some spot is invalid, so static real number
	m_mtxDist.Resize(iNumB, iNumA, true);

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
			dLine[x] = hypot(ptA.tx - ptB.tx, ptA.ty - ptB.ty);
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
		double* dLine = m_mtxDist[y];
		// traverse the line, find minmium item's index
		// minvalue change to 1, otherwise to 0;
		double dMin = dLine[0];
		int iMin = 0;
		dLine[0] = 1.0;
		for (int x = 1; x < iW; ++x)
		{
			if (dLine[x] < dMin)
			{
				dLine[iMin] = 0.0;
				dMin = dLine[x];
				iMin = x;
				dLine[x] = 1.0;
			}
		}
	}
	return true;
}

bool ICP::Transform()
{
	return true;
}

bool ICP::Mahalnobis()
{
	return false;
}
