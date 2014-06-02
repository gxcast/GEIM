#include "SpotDtThread.h"

#include "EffectPar.h"
#include "Graying.h"
#include "WaterShed.h"

/**< event id */
const long SpotDtThread::ID = wxNewId();

/**< construct */
SpotDtThread::SpotDtThread(wxEvtHandler* host, ST_DTPARAM& param, wxArrayPtrVoid& imgs, wxArrayPtrVoid& disps, LS_DTRESULT& sets)
	: wxThread(wxTHREAD_DETACHED)	// 分离线程，自动释放自己的资源
	, m_pHost(host)
	, m_stDtParam(param)
	, m_aryImgs(imgs)
	, m_aryImgsDisp(disps)
	, m_lsDtResult(sets)
{
}

SpotDtThread::~SpotDtThread()
{
}

/**< thread's entry */
void* SpotDtThread::Entry()
{
	bool bRet = SpotDtBatch(m_stDtParam);
	// notition host, pEvt's memory auto release
	wxThreadEvent* pEvt = new wxThreadEvent(wxEVT_THREAD, SpotDtThread::ID);
	pEvt->SetInt(bRet?1:0);
	m_pHost->QueueEvent(pEvt);

	return nullptr;
}

/**< destroy the detect result list */
bool SpotDtThread::DestroyDtResult(LS_DTRESULT* pLs)
{
	if (pLs == nullptr)
	{
		wxASSERT_MSG(false, _T("GEIMFrame::DestroyDtResult parameter is nullptr."));
		return false;
	}

	size_t nNum = pLs->size();
	if (nNum <= 0)
		return true;

	// traverse all the image's result
	for (auto it = pLs->begin(); it != pLs->end(); ++it)
	{
		ST_DTRESULT& ret = *it;
		// release flag image
		if (ret.pData != nullptr)
		{
			delete [] ret.pData;
			ret.pData = nullptr;
		}
		// release spots-list
		if (ret.pLs != nullptr)
		{
			WaterShed::ClearSpots(ret.pLs);
			delete ret.pLs;
			ret.pLs = nullptr;
		}
	}
	pLs->clear();

	return true;
}

/**< perform the detection algorithm */
bool SpotDtThread::SpotDtBatch(ST_DTPARAM& stDtParam)
{
	bool bRet = true;
	WaterShed wst;
	EffectPar parEft;
	unsigned char* pIn = nullptr;

	// traverse all the image
	size_t nNum = m_aryImgs.size();
	for (size_t i = 0; i < nNum && bRet; ++i)
	{
		ST_DTRESULT stResult;
		wxImage* pImg = nullptr;
		wxImage* pImgDisp = nullptr;
		// get the origin image
		pImg = static_cast<wxImage*>(m_aryImgs.Item(i));
		if (pImg == nullptr)
		{
			wxASSERT_MSG(pImg != nullptr, _T("get origin image error"));
			bRet = false;
			break;
		}
		// dtecte parameter
		parEft.SetImage(pImg);
		pIn = parEft.GetCache();
		memcpy(pIn, pImg->GetData(), parEft.PixNum()*3);
		parEft.Input(pIn, true);
		stResult.pData = (PST_RGB)pIn;
		stResult.pLs = new LS_SPOTS;

		// Graying switch
		Graying::Gray(parEft);
		// clp detect
		if (!wst.WSTmain(&parEft, &stDtParam, stResult.pLs))
		{
			bRet = false;
			goto SpotDtBatch_for_end;
		}

		// get the diplay image
		pImgDisp = static_cast<wxImage*>(m_aryImgsDisp.Item(i));
		if (pImgDisp == nullptr)
		{
			wxASSERT_MSG(pImgDisp != nullptr, _T("get display image error"));
			bRet = false;
			goto SpotDtBatch_for_end;
		}
		// display the result
		if (!DispDtResult(pImgDisp, stResult))
		{
			bRet = false;
			goto SpotDtBatch_for_end;
		}

SpotDtBatch_for_end:
		// add in result set
		m_lsDtResult.push_back(stResult);
	}

	// failed, destroy resource
	if (!bRet)
		SpotDtThread::DestroyDtResult(&m_lsDtResult);
	return bRet;
}

/**< display the detect result */
bool SpotDtThread::DispDtResult(wxImage* pImg, ST_DTRESULT& ret)
{
	if (pImg == nullptr)
	{
		wxASSERT_MSG(false, _T("SpotDtThread::DispDtResult parameter is nullptr"));
		return false;
	}
	int iSpotN = (int)ret.pLs->size();
	if (iSpotN <= 0)
		return false;

	unsigned char* pDest = pImg->GetData();
	int iW = pImg->GetWidth();
	int iWb = iW*3;
	int iH = pImg->GetHeight();
	int iN = iW*iH;
	PST_RGB pFlag = ret.pData;
	// draw the spot edge
	for (int i = 0; i < iN; ++i)
	{
		if(pFlag->g > 0)
		{
			pDest[0] = 0u;
			pDest[1] = 255u;
			pDest[2] = 0u;
		}
		else
		{
			pDest[0] = pFlag->r;
			pDest[1] = pFlag->r;
			pDest[2] = pFlag->r;
		}
		pDest += 3;
		++pFlag;
	}
	// draw the spot center
	pDest = pImg->GetData();
	pFlag = ret.pData;
	for (auto it = ret.pLs->begin(); it != ret.pLs->end(); ++it)
	{
		ST_SPOT_NODE& spot = *it;
		// 绘制蛋白点+
		int x = spot.x;
		int y = spot.y;
		unsigned char* pPix = pDest + (y*iW + x)*3;
		// -
		for (int i = -3; i <= 3; ++i)
		{
			if (x+i < 0 || x+i >= iW)
				continue;
			unsigned char* pT = pPix + i*3;
			pT[0] = 255;
			pT[1] = 0;
			pT[2] = 0;
		}
		// |
		for (int i = -3; i <= 3; ++i)
		{
			if (y+i < 0 || y+i >= iH)
				continue;
			unsigned char* pT = pPix + i*iWb;
			pT[0] = 255;
			pT[1] = 0;
			pT[2] = 0;
		}
	}

	return true;
}
