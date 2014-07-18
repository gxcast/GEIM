/** \brief base class for manipulate iamge
 *
 * \file Graying.h
 * \author Author: zhd
 * \date Date: 05/04/2014
 *
 */
#pragma once
#ifndef GRAYING_H
#define GRAYING_H

#include "GEIMDef.h"
#include "EffectPar.h"

class Graying
{
public:
	/** \brief jet color map generic
	 *
	 * \param d double  [IN] index [0-1]
	 * \return ST_RGB   the pseudo color
	 *
	 */
	static bool ColorMap(double d, PST_RGB pClr);
	static ST_RGB ColorMap(double d);

    /** \brief prseudo-random color generic
     *
     * \param pClr PST_RGB	the random color
	 * \return ST_RGB		the random color
     *
     */
	static bool RandColor(PST_RGB pClr);
	static ST_RGB RandColor();

	/** \brief Abandon image's color
	 *
	 * \param EffectPar&		[INOUT] process data
	 * \return bool true:success false:failed
	 *
	 */
	static bool Gray(EffectPar& parEft);
	/** \brief invert gray
	 *
	 * \param EffectPar&		[INOUT] process data
	 * \return bool true:success false:failed
	 *
	 */
	static bool Invert(EffectPar& parEft);
	/** \brief image1 - image2
	 *
	 * \param EffectPar&		[INOUT] process data
	 * \return bool true:success false:failed
	 *
	 */
	static bool Subtract(EffectPar& parEft);
	/** \brief image1 + iChange
	 *
	 * \param parEft EffectPar&		[INOUT] process data
	 * \param iCg int							[IN] alter degree
	 * \return bool true:success false:failed
	 *
	 */
	static bool Subtract(EffectPar& parEft, int iCg);
	/** \brief gray normalization
	 *
	 * \param parEft EffectPar&	[INOUT] process data
	 * \param dE0=125.0 double 	[IN] expectation
	 * \param dV0=125.0 double 	[IN] variance
	 * \return bool
	 *
	 */
	static bool Normalize(EffectPar& parEft, double dE0 = 125.0, double dV0 = 90.0);

protected:

private:
};

#endif // GRAYING_H
