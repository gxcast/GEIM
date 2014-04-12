/** \brief image processing function param
 *
 * \file EffectPar.h
 * \author Author: ZHD
 * \date Date: 08/04/2014
 *
 * used for image processing function, include cache manage
 *
 */

#pragma once
#ifndef EFFECTPAR_H
#define EFFECTPAR_H

#include <wx/wx.h>
#include <deque>

class EffectPar
{
public:
	/** \brief construc
	 *
	 * \param wxImage* pImg	[IN] nullptr:not init memver
	 *				other:if the image is ok, init member
	 *
	 */
	EffectPar(wxImage* pImg = nullptr);
	virtual ~EffectPar();

	/** \brief set the origin image
	 *
	 * \param pImg wxImage*	[IN] the origin image's pointer
	 * \return bool	true:success false:failed
	 *
	 * this operation will clear all the cache memory
	 * use as initialize
	 *
	 */
	bool SetImage(wxImage* pImg);
	/** \brief release all the cache
	 *
	 * \return bool	true:success false:failed
	 *
	 */
	bool Release();
	/** \brief return a cache memory
	 *
	 * \return unsigned char*	nullptr:failed other:the cache pointer
	 *
	 * if exist no-used cache, will return a no-used cache
	 * if no cache, will new a cache
	 *
	 */
	unsigned char* GetCache();
	/** \brief recycle a cache
	 *
	 * \param pCache unsigned char*	[IN] the cache recycled
	 * \return bool	true:success false:failed
	 *
	 */
	bool Recycle(unsigned char* pCache);

	/** \brief set the input image
	 *
	 * \param pIn unsigned char* [IN] input image
	 * \param bool 			 				[IN] true:modify input image false:not modify
	 * \return bool	true:success false:failed
	 *
	 */
	bool Input(unsigned char* pIn, bool bMdfy);
	inline unsigned char* Input()
	{
		return m_pIn;
	}
	bool Input2(unsigned char* pIn, bool bMdfy);
	inline unsigned char* Input2()
	{
		return m_pIn2;
	}
	/** \brief set the result, get the result
	 *
	 * \return unsigned char* the result, if nullptr failed
	 * \note set the result should used only in the process function
	 *
	 */
	inline void Output(unsigned char* pOut)
	{
		m_pOut = pOut;
	}
	inline unsigned char* Output()
	{
		return m_pOut;
	}
	/** \brief set or get if modify the input image
	 *
	 * \param bool	[IN] true:modify input image
	 * \return void
	 *
	 */
	inline void Modify(bool b)
	{
		m_bModify = b;
	}
	inline bool Modify() const
	{
		return m_bModify;
	}
	inline void Modify2(bool b)
	{
		m_bModify2 = b;
	}
	inline bool Modify2()
	{
		return m_bModify2;
	}

	/** \brief get the image size
	 *
	 * \see Width() width
	 * \see Height() height
	 * \see PixNum() pixel's number
	 *
	 */
	inline int Width() const
	{
		return m_iW;
	};
	inline int Height() const
	{
		return m_iH;
	}
	inline int PixNum() const
	{
		return m_iN;
	}

protected:

private:
	/**< origin image, not modified */
	wxImage* m_pImg = nullptr;
	/**< image info */
	int m_iW = 0;	// image's width
	int m_iH = 0;	// image's height
	int m_iN = 0;	// image's pixel number

	/**< cache deque */
	std::deque<unsigned char*> m_dqCache;

	/**< input image data */
	unsigned char* m_pIn = nullptr;
	/**< the second input iamge, used in such as subtract */
	unsigned char* m_pIn2 = nullptr;
	/**< output iamge data, the result */
	unsigned char* m_pOut = nullptr;
	/**< if modify the input image */
	bool m_bModify = false;
	bool m_bModify2 = false;
};

#endif // EFFECTPAR_H
