/** \brief Morphological Algorithm
 *
 * \file Morphology.h
 * \author Author: ZHD
 * \date Date:08/04/2014
 *
 */
#pragma once
#ifndef MORPHOLOGY_H
#define MORPHOLOGY_H

#include "EffectPar.h"

class Morphology
{
	public:
        /** \brief opening operate
         *
         * \param parEft EffectPar&	[INOUT] process data
         * \param iType int					[IN] template radius
         * \return bool	true:success false:failed
         *
         */
		static bool Opening(EffectPar& parEft, int iType);
        /** \brief closeing operate
         *
         * \param parEft EffectPar&	[INOUT] process data
         * \param iType int					[IN] template radius
         * \return bool	true:success false:failed
         *
         */
		static bool Closeing(EffectPar& parEft, int iType);
        /** \brief dilate - erode, form a morphological grade iamge
         *
         * \param parEft EffectPar&	[INOUT] process data
         * \param iType int					[IN] template radius
         * \return bool	true:success false:failed
         *
         */
		static bool Grading(EffectPar& parEft, int iType);

        /** \brief dilate operation, invoke _Dilate to perform
         *
         * \param parEft EffectPar&	[INOUT] process data
         * \param iType int					[IN] template radius
         * \return bool	true:success false:failed
         * \see bool _Dilate(EffectPar& parEft, int iType)
		*
         */
		static bool Dilate(EffectPar& parEft, int iType);
        /** \brief erode operation, invoke _Erode to perform
         *
         * \param parEft EffectPar&	[INOUT] process data
         * \param iType int					[IN] template radius
         * \return bool	true:success false:failed
		 * \see bool _Erode(EffectPar& parEft, int iType)
         */
		static bool Erode(EffectPar& parEft, int iType);

	protected:
        /** \brief dilate, but the type only be 1-9, highlight diffusion
         *
         * \param parEft EffectPar&	[INOUT] process data
         * \param iType int					[IN] template type, see details in code
         * \return bool	true:success false:failed
         *
         */
		static bool _Dilate(EffectPar& parEft, int iType);
        /** \brief erode, but the type only be 1-9, shadow diffusion
         *
         * \param parEft EffectPar&	[INOUT] process data
         * \param iType int					[IN] template type, see details in code
         * \return bool	true:success false:failed
         *
         */
		static bool _Erode(EffectPar& parEft, int iType);

	private:

};

#endif // MORPHOLOGY_H
