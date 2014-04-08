/** \brief Gaussian Filter
 *
 * \file GaussFilter.h
 * \author Author: ZHD
 * \date Date: 07/04/2014
 *
 */
#pragma once
#ifndef GAUSSFILTER_H
#define GAUSSFILTER_H

#include <wx/wx.h>

class GaussFilter
{
public:
	/** \brief perform the gaussfilter
	 *
	 * \param imgSrc wxImage&	[IN] source image
	 * \param imgDes wxImage&	[OUT] result
	 * \param iType int					[IN] template size
	 * \return bool		true:success false:failed
	 *
	 */
	static bool Do(wxImage& imgSrc, wxImage& imgDes, int iType);

protected:
	// template and it's value sum
	static constexpr int TMPL_3_V = 16;
	static const int TMPL_3 [9];
	static constexpr int TMPL_5_V = 52;
	static const int TMPL_5[25];
	static constexpr int TMPL_7_V = 409;
	static const int TMPL_7[49];
private:
};

#endif // GAUSSFILTER_H
