/**
 *
 * \file GEIMDef.h
 * \author Author:ZHD
 * \date Date:26/03/2014
 *
 * generic define
 *
 */
#pragma once
#ifndef GEIMDEF_H_INCLUDED
#define GEIMDEF_H_INCLUDED

/**< 3x3 neighborhood pixels */
typedef struct _ST_NEIBOR3
{
    int va[9] = { 0 };	//	pix value
    int dx[9] = { 0 };	//	pix index base 0 in full image
    int num = 0;			//	pix in the 3x3 neighborhood rect
} ST_NEIBOR3, *PST_NEIBOR3;

/**< spot detect param */
typedef struct _ST_DTPARAM_
{
	/**< faint spot position */
	wxPoint ptFaint = {0, 0};
	/**< faint spot streshold */
	wxDouble dFaint = 0.0;

	/**< minimum radius of spots, must > 0 */
	wxInt32 iMinRad = 0;
	/**< maximum radius of spots, must > 0 */
	wxInt32 iMaxRad = 0;

	/**< minimum aspect ratio */
	wxDouble dAspect = 0.0;

	/**< Median filter param: -1 0 1 2*/
	wxInt32 iMedianFlt = -1;
	/**< Gauss filter param: -1 0 1 2 */
	wxInt32 iGaussFlt = -1;

}ST_DTPARAM, *PST_DTPARAM;

#endif // GEIMDEF_H_INCLUDED
