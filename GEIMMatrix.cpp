#include "GEIMMatrix.h"

/**< constuct the matrix */
template<typename _T, typename _Tx, typename _Ty>
GEIMMatrix<_T, _Tx, _Ty>::GEIMMatrix()
{
}

/**< construct the matrix, and alloc memory */
template<typename _T, typename _Tx, typename _Ty>
GEIMMatrix<_T, _Tx, _Ty>::GEIMMatrix(int w, int h)
	: GEIMMatrix()
{
	// delegation construct and create
	Create(w, h);
}

/**< deconstruct */
template<typename _T, typename _Tx, typename _Ty>
GEIMMatrix<_T, _Tx, _Ty>::~GEIMMatrix()
{
	// ensure memory be release
	Destroy();
}

/**< alloc memory */
template<typename _T, typename _Tx, typename _Ty>
bool GEIMMatrix<_T, _Tx, _Ty>::Create(int w, int h)
{
	if (m_pIndex != nullptr)
		return false;
	return Resize(w, h);
}

/**< resize the matrix */
template<typename _T, typename _Tx, typename _Ty>
bool GEIMMatrix<_T, _Tx, _Ty>::Resize(int w, int h, bool bCpy)
{
	if (w <= 0 || h <= 0)
		return false;

	// no change
	if (m_iW == w && m_iH == h)
		return true;

	_T** pIndex = new _T*[h];
	if (pIndex != nullptr)
	{
		// use currnet memory
		if (m_iCapacity >= h*w)
		{
			_T* pLine = m_pItems;
			for (int i = 0; i < h; ++i)
			{
				pIndex[i] = pLine;
				pLine += w;
			}

			// copy data
			if (bCpy && w < m_iW)	// line move before
			{
				int iMinh = (m_iH<h)?m_iH:h;
				for (int iy = 1; iy < iMinh; ++iy)
				{
					for (int ix = 0; ix < w; ++ix)
						pIndex[iy][ix] = m_pIndex[iy][ix];
				}
			}
			else if (bCpy && w > m_iW)	// line move behind
			{
				// h < m_iH
				for (int iy = h - 1; iy > 0; --iy)
				{
					for (int ix = m_iW - 1; ix >= 0; --ix)
						pIndex[iy][ix] = m_pIndex[iy][ix];
				}
			}

			// axis
			if (w > m_iAxisX)
			{
				_Tx* pAxisX = new _Tx[w];
				if (pAxisX != nullptr)
				{
					int iMin = (m_iW<w)?m_iW:w;
					for (int i = 0; bCpy && i < iMin; ++i)
						pAxisX[i] = m_pAxisX[i];

					if(m_pAxisX != nullptr)
						delete [] m_pAxisX;
					m_iAxisX = w;
					m_pAxisX = pAxisX;
				}
			}
			if (h > m_iAxisY)
			{
				_Ty* pAxisY = new _Ty[h];
				if (pAxisY != nullptr)
				{
					int iMin = (m_iH<h)?m_iH:h;
					for (int i = 0; bCpy && i < iMin; ++i)
						pAxisY[i] = m_pAxisY[i];

					if (m_pAxisY != nullptr)
						delete [] m_pAxisY;
					m_iAxisY = h;
					m_pAxisY = pAxisY;
				}
			}

			// release old index
			delete [] m_pIndex;
			m_pIndex = pIndex;
			m_iW = w;
			m_iH = h;

			return true;
		}
		// realloc the memory
		else
		{
			_T* pItems = new _T[h*w];
			if (pItems != nullptr)
			{
				_T* pLine = pItems;
				for (int i = 0; i < h; ++i)
				{
					pIndex[i] = pLine;
					pLine += w;
				}

				// copy data
				if (bCpy)
				{
					int iMinh = (m_iH<h)?m_iH:h;
					int iMinw = (m_iW<w)?m_iW:w;
					for (int iy = 0; iy < iMinh; ++iy)
					{
						for (int ix = 0; ix < iMinw; ++ix)
							pIndex[iy][ix] = m_pIndex[iy][ix];
					}
				}

				// axis
				if (w > m_iAxisX)
				{
					_Tx* pAxisX = new _Tx[w];
					if (pAxisX != nullptr)
					{
						int iMin = (m_iW<w)?m_iW:w;
						for (int i = 0; bCpy && i < iMin; ++i)
							pAxisX[i] = m_pAxisX[i];

						if (m_pAxisX != nullptr)
							delete [] m_pAxisX;
						m_iAxisX = w;
						m_pAxisX = pAxisX;
					}
				}
				if (h > m_iAxisY)
				{
					_Ty* pAxisY = new _Ty[h];
					if (pAxisY != nullptr)
					{
						int iMin = (m_iH<h)?m_iH:h;
						for (int i = 0; bCpy && i < iMin; ++i)
							pAxisY[i] = m_pAxisY[i];

						if (m_pAxisY != nullptr)
							delete [] m_pAxisY;
						m_iAxisY = h;
						m_pAxisY = pAxisY;
					}
				}

				// release pre data
				if (m_pItems != nullptr)
				{
					delete [] m_pItems;
					m_pItems = nullptr;
				}
				if (m_pIndex != nullptr)
				{
					delete [] m_pIndex;
					m_pIndex = nullptr;
				}
				m_pIndex = pIndex;
				m_pItems = pItems;
				m_iW = w;
				m_iH = h;
				m_iCapacity = h*w;

				return true;
			}

			delete [] pIndex;
			pIndex = nullptr;
			return false;
		} // realloc the memory
	}
	return false;
}

/**< relese the matrix's memory */
template<typename _T, typename _Tx, typename _Ty>
bool GEIMMatrix<_T, _Tx, _Ty>::Destroy()
{
	if (m_pItems != nullptr)
	{
		delete [] m_pItems;
		m_pItems = nullptr;
	}
	if (m_pIndex != nullptr)
	{
		delete [] m_pIndex;
		m_pIndex = nullptr;
	}
	if (m_pAxisX != nullptr)
	{
		delete [] m_pAxisX;
		m_pAxisX = nullptr;
	}
	if (m_pAxisY != nullptr)
	{
		delete [] m_pAxisY;
		m_pAxisY = nullptr;
	}
	m_iW = 0;
	m_iH = 0;
	m_iCapacity = 0;
	return true;
}

