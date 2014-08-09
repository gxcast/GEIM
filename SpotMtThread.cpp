#include "SpotMtThread.h"

// random color
#include "Graying.h"
// match class
#include "CharactVect.h"

/**< event id */
const long SpotMtThread::ID = wxNewId();

/**< construct */
SpotMtThread::SpotMtThread(wxEvtHandler* host, LS_DTRESULT& dt, wxArrayPtrVoid& imgs, wxArrayPtrVoid& disps, ST_MTRESULT& mt)
	: wxThread(wxTHREAD_DETACHED)	// 分离线程，自动释放自己的资源
	, m_pHost(host)
	, m_lsDtResult(dt)
	, m_aryImgs(imgs)
	, m_aryImgsDisp(disps)
	, m_stMtResult(mt)
{
}

SpotMtThread::~SpotMtThread()
{
}

/**< thread's entry */
void* SpotMtThread::Entry()
{
	bool bRet = SpotMatch();
	// notition host, pEvt's memory auto release
	wxThreadEvent* pEvt = new wxThreadEvent(wxEVT_THREAD, SpotMtThread::ID);
	pEvt->SetInt(bRet?1:0);
	m_pHost->QueueEvent(pEvt);

	return nullptr;
}

/**< detroy the mt param */
bool SpotMtThread::DestroyMTParam(ST_MTPARAM* pstParm)
{
	if (pstParm == nullptr)
		return true;

	// origin image
	if (pstParm->pImg != nullptr)
	{
		delete [] pstParm->pImg;
		pstParm->pImg = nullptr;
	}

	// detect result maybe come from result list
	pstParm->pData = nullptr;

	// attrib vector
	if (pstParm->pvtAttr != nullptr)
	{
		for (auto it = pstParm->pvtAttr->begin(); it != pstParm->pvtAttr->end(); ++it)
		{
			ST_SPOT_ATTR& attr = *it;

			// detect result maybe come from result list
			attr.pNode = nullptr;
			// release charact
			if (attr.pCrt != nullptr)
			{
				delete attr.pCrt;
				attr.pCrt = nullptr;
			}
		}
		if (!pstParm->pvtAttr->empty())
			pstParm->pvtAttr->clear();

		delete pstParm->pvtAttr;
		pstParm->pvtAttr = nullptr;
	}

	return true;
}

/**< destroy the match result */
bool SpotMtThread::DestroyMtResult(ST_MTRESULT* pRst)
{
	if (pRst == nullptr)
		return true;
	// destroy union image
	if (pRst->pImgUnion != nullptr)
	{
		wxImage *pImg = (wxImage *)pRst->pImgUnion;
		if (pImg->IsOk())
			pImg->Destroy();
		delete pImg;
		pRst->pImgUnion = nullptr;
	}
	// destroy match-pair
	return CharactVect::DestroyResult(pRst);
}

bool SpotMtThread::SpotMatch()
{
	bool bRet = false;
	ST_MTPARAM stParamA;
	ST_MTPARAM stParamB;

	// release old match result
	SpotMtThread::DestroyMtResult(&m_stMtResult);

	// generate the parameter
	InitParam(stParamA, 0);
	InitParam(stParamB, 1);

	// match
	CharactVect cvt;
	if (cvt.CVMain(std::make_pair(stParamA, stParamB), &m_stMtResult))
	{
		//DispMtResult_icp(stParamA, 0);
		//DispMtResult_icp(stParamB, 1);
		DispMtResult_vec(stParamA, stParamB);

		bRet = true;
	}

	// release param
	SpotMtThread::DestroyMTParam(&stParamA);
	SpotMtThread::DestroyMTParam(&stParamB);

	return bRet;
}

/**< fill a match param */
bool SpotMtThread::InitParam(ST_MTPARAM& stParam, int id)
{
	// get the specify image
	wxImage* pImg = static_cast<wxImage*>(m_aryImgs.Item(id));
	unsigned char* pTmp = pImg->GetData();
	// get the corresponding detection result
	auto it = m_lsDtResult.begin();
	for (int i = 0; i < id; ++i)
		++it;
	ST_DTRESULT& rst = *it;

	// image's infomation
	stParam.iW = pImg->GetWidth();
	stParam.iH = pImg->GetHeight();
	stParam.iWb = stParam.iW*3;
	stParam.iN = stParam.iW*stParam.iH;
	// image raw data
	stParam.pImg = new ST_RGB[stParam.iN];
	memcpy(stParam.pImg, pTmp, stParam.iN*3);
	// detection marks
	stParam.pData = rst.pData;
	// spots' infomation
	size_t nC = rst.pLs->size();
	stParam.pvtAttr = new VT_ATTR(nC);
	nC = 0u;
	for(auto ir = rst.pLs->begin(); ir != rst.pLs->end(); ++ir)
	{
		ST_SPOT_NODE& stNode = *ir;
		ST_SPOT_ATTR& attr = stParam.pvtAttr->at(nC++);
		attr.pNode = &stNode;
		attr.pCrt = new ST_SPOT_CHARACT;
	}
	wxASSERT_MSG(nC == rst.pLs->size(), _T("Fill match param's pNode failed."));

	return true;
}

/**< display the match result */
bool SpotMtThread::DispMtResult_icp(ST_MTPARAM& stParam, int id)
{
	wxImage* pImg = static_cast<wxImage*>(m_aryImgsDisp.Item(id));
	PST_RGB pDes = (PST_RGB)pImg->GetData();
	int iW = stParam.iW;
	int iH = stParam.iH;
	int iN = stParam.iN;
	// copy the image
	if (id == 1)
		memcpy(pDes, stParam.pImg, iN*3);
	/*// draw spot's character
	for (auto it = stParam.pvtAttr->begin(); it != stParam.pvtAttr->end(); ++it)
	{
		if (it->bInvalid)
			continue;
		ST_SPOT_NODE& spot = *(it->pNode);
		ST_SPOT_CHARACT& crt = *(it->pCrt);
		// gel spot coordinate
		int x = spot.x;
		int y = spot.y;
		// character
		ST_RGB clr;
		Graying::ColorMap(crt.deep, &clr);	// charact's value color-map
		for (int j = -3; j <= 3; ++j)
		{
			if (y+j < 0 || y+j >= iH)
				continue;
			PST_RGB pLine = pDes + (y+j)*iW;
			for (int i = -3; i <=3; ++i)
			{
				if (x+i < 0 || x+i >= iW)
					continue;
				PST_RGB pPix = pLine + (x+i);
				*pPix = clr;
			}
		}
	}*/
	// draw match pair
	if (id == 0)
	{
		VT_SPAIR* pvtPairs = m_stMtResult.pvtSpair;
		for (auto it = pvtPairs->begin(); it != pvtPairs->end(); ++it)
		{
			ST_SPOT_ATTR& spot = stParam.pvtAttr->at(it->iOdA);
			// gel spot coordinate
			int x = spot.pNode->x;
			int y = spot.pNode->y;
			// character
			ST_RGB clr;
			clr.b = 255;
			for (int j = -2; j <= 2; ++j)
			{
				if (y+j < 0 || y+j >= iH)
					continue;
				PST_RGB pLine = pDes + (y+j)*iW;
				for (int i = -2; i <=2; ++i)
				{
					if (x+i < 0 || x+i >= iW)
						continue;
					PST_RGB pPix = pLine + (x+i);
					*pPix = clr;
				}
			}
		}
	}
	else
	{
		VT_SPAIR* pvtPairs = m_stMtResult.pvtSpair;
		for (auto it = pvtPairs->begin(); it != pvtPairs->end(); ++it)
		{
			ST_SPOT_ATTR& spot = stParam.pvtAttr->at(it->iOdB);
			// gel spot coordinate
			int x = spot.pNode->x;
			int y = spot.pNode->y;
			// character
			ST_RGB clr;
			clr.b = 255;
			for (int j = -2; j <= 2; ++j)
			{
				if (y+j < 0 || y+j >= iH)
					continue;
				PST_RGB pLine = pDes + (y+j)*iW;
				for (int i = -2; i <=2; ++i)
				{
					if (x+i < 0 || x+i >= iW)
						continue;
					PST_RGB pPix = pLine + (x+i);
					*pPix = clr;
				}
			}
		}
	}

	return true;
}

bool SpotMtThread::DispMtResult_vec(ST_MTPARAM& paramA, ST_MTPARAM& paramB)
{
	int iTemp = 0;
	int iW = paramA.iW + paramB.iW;
	int iWb = iW*3;
	int iH = (paramA.iH > paramB.iH)?paramA.iH:paramB.iH;
	wxImage *pImgUnion = nullptr;
	unsigned char *pImg = nullptr;
	unsigned char *pImgA = nullptr;
	unsigned char *pImgB = nullptr;
	ST_RGB clr;
	wxRect rcl;

	// ensure image is suitable
	pImgUnion = (wxImage *)m_stMtResult.pImgUnion;
	if (pImgUnion == nullptr)
		pImgUnion = new wxImage(iW, iH);
	else if (!pImgUnion->IsOk())
		pImgUnion->Create(iW, iH);
	else if (pImgUnion->GetWidth() != iW || pImgUnion->GetHeight() != iH)
	{
		pImgUnion->Destroy();
		pImgUnion->Create(iW, iH);
	}

	// union image
	pImg = pImgUnion->GetData();
	pImgA = (unsigned char *)paramA.pImg;
	pImgB = (unsigned char *)paramB.pImg;
	iTemp = paramA.iWb;
	for (int y = 0; y < paramA.iH; ++y)
	{
		memcpy(pImg, pImgA, (size_t)iTemp);
		pImg += iWb;
		pImgA += iTemp;
	}
	pImg = pImgUnion->GetData() + iTemp;	// offset image A
	iTemp = paramB.iWb;
	for (int y = 0; y < paramB.iH; ++y)
	{
		memcpy(pImg, pImgB, (size_t)iTemp);
		pImg += iWb;
		pImgB += iTemp;
	}

	// maker one pair
	for (auto it =  m_stMtResult.pvtSpair->begin(); it != m_stMtResult.pvtSpair->end(); ++it)
	{
		ST_SPOT_ATTR& spotA = paramA.pvtAttr->at(it->iOdA);
		ST_SPOT_ATTR& spotB = paramB.pvtAttr->at(it->iOdB);
		// gel spot coordinate
		rcl.x = spotA.pNode->x;
		rcl.y = spotA.pNode->y;
		rcl.width = spotB.pNode->x + paramA.iW - rcl.x;
		rcl.height = spotB.pNode->y - rcl.y;
		if (rcl.width > 0) rcl.width += 1;
		else if (rcl.width < 0) rcl.width -= 1;
		if (rcl.height > 0) rcl.height += 1;
		else if (rcl.height < 0) rcl.height -= 1;
		// random color
		Graying::RandColor(&clr);
		// draw pair line
		img_draw_line(*pImgUnion, rcl, clr);
	}

	m_stMtResult.pImgUnion = pImgUnion;
	return true;
}
