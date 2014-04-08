/** \brief MedianFilter
 *
 * \file MedianFilter.h
 * \author AuthorL: ZHD
 * \date Date: 06/04/2014
 *
 */

#pragma once
#ifndef MEDIANFILTER_H
#define MEDIANFILTER_H

#include <wx/wx.h>

class MedianFilter
{
	public:
        /** \brief perform filter
         *
         * \param img wxImage&	[IN] image
         * \param img wxImage&	[OUT] image
         * \param iType int			[IN] template type 0:3X3 1:5X5 2:7X7
         * \return bool		true:success false:failed
         *
         */
		static bool Do(wxImage& imgSrc, wxImage& imgDes,  int iType);

	protected:
	private:
};

#endif // MEDIANFILTER_H
