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

#include <wx/wx.h>
#include "EffectPar.h"

class Graying
{
public:

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

protected:

private:
};

#endif // GRAYING_H
