#ifndef GEIMMATRIX_H
#define GEIMMATRIX_H

template<typename _T, typename _Tx, typename _Ty>
class GEIMMatrix
{
public:
	/** \brief constuct the matrix
	 *
	 * detemine items' type
	 */
	GEIMMatrix();
	/** \brief construct the matrix, and alloc memory
	 *
	 * \param w int [IN] matrix's width, X axis
	 * \param h int	[IN] matrix's height, Y axis
	 *
	 */
	GEIMMatrix(int w, int h);
	/** \brief deconstruct
	 *
	 * release memory
	 */
	virtual ~GEIMMatrix();

	/** \brief alloc memory
	 *
	 * \param w int	[IN] matrix's width, X axis
	 * \param h int [IN] matrix's height, Y axis
	 * \return bool	true:success false:memory error or matrix exist
	 *
	 */
	bool Create(int w, int h);
	/** \brief resize the matrix
	 *
	 * \param w int	[IN] matrix's new width, X axis
	 * \param h int [IN] matrix's new height, Y axis
	 * \param bCpy bool [IN] true:copy old data false:ignore old data
	 * \return bool	true:success false:memory error
	 *
	 * if the matrix not be create, create it, otherwise, recreate it
	 */
	bool Resize(int w, int h, bool bCpy = false);
	/** \brief relese the matrix's memory
	 *
	 * \return bool	true:success false:failed
	 *
	 */
	bool Destroy();


	inline int Width()
	{
		return m_iW;
	}
	inline int Height()
	{
		return m_iH;
	}
	inline _T* operator [](int y)
	{
		if (m_pIndex != nullptr && 0 <= y && y < m_iH)
			return m_pIndex[y];
		else
			return nullptr;
	}
	inline _Tx& AxisX(int x)
	{
		return m_pAxisX[x];
	}
	inline _Ty& AxisY(int y)
	{
		return m_pAxisY[y];
	}

protected:

private:
	/**< matrix's width */
	int m_iW = 0;
	/**< matrix's height */
	int m_iH = 0;
	/**< matrix line */
	_T** m_pIndex = nullptr;
	/**< matrix's item */
	_T* m_pItems = nullptr;
	/**< x and y axis */
	_Tx* m_pAxisX = nullptr;
	_Ty* m_pAxisY = nullptr;

	/**< memory size to hold items, optimaize the memory manage */
	int m_iCapacity = 0;
	/**< axis's capacity */
	int m_iAxisX = 0;
	int m_iAxisY = 0;
};

#endif // GEIMMATRIX_H
