/** \brief NLM(Non-Local Means)
 *
 * \file NLM.h
 * \author Author: ZHD
 * \date Date: 05/04/2014
 *
 */
#pragma once
#ifndef NLM_H
#define NLM_H

#include <wx/wx.h>

class NLM
{
	public:
		NLM();
		virtual ~NLM();

        /** \brief nlm filter on image
         *
         * \param img wxImage& [INOUT] image
         * \param 1 int iIter=1	[IN] iterations
         * \return bool	true:success false:failed
         *
         */
		bool Do(wxImage& img, int iIter = 1);

	protected:

	private:
		/**< neighbour window radius */
		int m_iRdWin = 3;
		/**< window area */
		int m_iAeWin = (2*m_iRdWin + 1)*(2*m_iRdWin + 1);
		/**< block radius */
		int m_iRdBlock = 10;
		/**< block diameter */
		int m_iDiaBlock = 2*m_iRdBlock + 1;
		/**< Gaussian std-divetion */
		double m_dSigma = 20.0;
		/**<  */
		double m_dStrong = 0.1;
};

#endif // NLM_H
