/** \brief detect spot algorithm
 *
 * \file SpotDt.h
 * \author Author: ZHD
 * \date Date:10/04/2014
 *
 */
#pragma once
#ifndef SPOTDT_H
#define SPOTDT_H

#include <wx/wx.h>
#include "GEIMDef.h"
#include "EffectPar.h"

class SpotDt
{
	public:
		SpotDt();
		virtual ~SpotDt();

        /** \brief detect main function
         *
         * \param pDtParam PST_DTPARAM	[IN] cache management
         * \param pParEft EffectPar*				[IN] detection param
         * \return bool true:success false:failed
         *
         */
		bool DtMain(PST_DTPARAM pDtParam, EffectPar* pParEft);

	protected:

	private:
        /** \brief perform the water shed algorithm
         *
         * \return bool true:success false:failed
         *
         */
		bool WaterShed(unsigned char* pImg);

        /** \brief
         *
         * \param
         * \param
         * \return
         *
         */
		bool MinDistWS(unsigned char* pImg,
						unsigned char* pLabel, unsigned char* pShed);

		bool ExtendedMin(unsigned char* pImg,
						unsigned char* pLabelIn,
						int iDeep);

		bool ReConstruct(unsigned char* pImg,
						unsigned char* pMask);

		bool GetNeibor3(unsigned char* pImg, int x, int y,
						PST_NEIBOR3 pNeiB);

		bool LocalMax(unsigned char* pImg,
						unsigned char* pMinLab);

        /** \brief get min-value number, i.e., the connected-domain number
         *
         * \param pMinLab unsigned char*	[IN] min-value image
         * \param iMode int							[IN] mode 4 or 8
         * \param pRegion unsigned char*	[OUT] connected-domain label image
         * \param nNum unsigned int*			[OUT] connected-domain number
         * \return bool	true:sucess false:failed
         *
         */
		bool Bwlabel(unsigned char* pMinLab, int iMode,
					unsigned int* pRegion, unsigned int* pnNum = nullptr);
		bool BwlabeNeibor(unsigned char* pMinLab, int iMode, int dx, unsigned int nID,
					unsigned int* pRegion);

		bool Bwdist(unsigned char* pMinLab, float* pDist);

		/**< detection param */
		PST_DTPARAM m_pDtParam;
		/**< cache management */
		EffectPar* m_pParEft;

		/**< the inpute image */
        unsigned char* m_pImgIn;
        /**< is modify the in image */
        bool m_bMdfyIn;
};

#endif // SPOTDT_H
