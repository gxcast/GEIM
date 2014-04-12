#include "EffectPar.h"

/**< construct and init the member */
EffectPar::EffectPar(wxImage* pImg /*= nullptr*/)
{
	SetImage(pImg);
}

EffectPar::~EffectPar()
{
	// release all the cache
	Release();
}

/**< set the origin image */
bool EffectPar::SetImage(wxImage* pImg)
{
	if (pImg != nullptr && pImg->IsOk())
	{
		Release();

		m_pImg = pImg;
		m_iW = m_pImg->GetWidth();
		m_iH = m_pImg->GetHeight();
		m_iN = m_iH*m_iW;
		return true;
	}
	return false;
}
/**< release all the cache */
bool EffectPar::Release()
{
	m_pImg = nullptr;
	m_pIn = nullptr;
	m_pOut = nullptr;
	m_iW = 0;
	m_iH = 0;
	m_iN = 0;
	// release all the cache
	size_t nNum = (size_t)m_dqCache.size();
	for(size_t i = 0; i < nNum; ++i)
	{
		const unsigned char* pT = m_dqCache.at(i);
		wxASSERT_MSG(pT != nullptr, _T("A image process cache is nullptr."));
		delete [] pT;
	}
	if (nNum > 0)
		m_dqCache.clear();

	return true;
}
/**< return a cache memory */
unsigned char* EffectPar::GetCache()
{
	if (m_pImg == nullptr)
	{
		wxASSERT_MSG(false, _T("Not set image but get a image process cache."));
		return nullptr;
	}

	unsigned char* pCache = nullptr;
	size_t nNum = (size_t)m_dqCache.size();
	if (nNum > 0)
	{
		pCache = m_dqCache.back();
		wxASSERT_MSG(pCache != nullptr, _T("Pop a image process cache failed."));
		m_dqCache.pop_back();
	}
	else
	{
		pCache = new unsigned char[m_iN*3];
		wxASSERT_MSG(pCache != nullptr, _T("New a image process cache failed."));
	}
	return pCache;
}
/**< recycle a cache */
bool EffectPar::Recycle(unsigned char* pCache)
{
	if (pCache == nullptr)
		return false;
	m_dqCache.push_back(pCache);
	return true;
}

/**< set the input image */
bool EffectPar::Input(unsigned char* pIn, bool bMdfy)
{
	if (pIn == nullptr)
		return false;
	m_pIn = pIn;
	m_pOut = nullptr;
	m_bModify = bMdfy;
	return true;
}
bool EffectPar::Input2(unsigned char* pIn, bool bMdfy)
{
	if (pIn == nullptr)
		return false;
	m_pIn2 = pIn;
	m_pOut = nullptr;
	m_bModify2 = bMdfy;
	return true;
}
