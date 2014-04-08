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

class Graying
{
public:

    /** \brief Abandon image's color
     *
     * \param img wxImage& [INOUT] image be muaniputed
     * \return bool true:success false:failed
     *
     */
	static bool Do(wxImage& img);

protected:

private:
};

#endif // GRAYING_H
