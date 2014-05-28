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

// -------------------------------------------
// CLP define
#define INTLENTH sizeof(int)

// 蛋白点信息结构
typedef struct spotnode
{
	int spotID;             // 蛋白点标号
	int centerX;            // 蛋白点中心x坐标 几何
	int centerY;            // 蛋白点中心y坐标 几何
	int centroidX;          // 蛋白点质心x坐标 （匹配中用到） （显示中用到）
	int centroidY;          // 蛋白点质心x坐标 （匹配中用到） （显示中用到）
	int spotArea;           // 蛋白点面积
	int spotVolume;         // 蛋白点体积
	int spotRx;             // 蛋白点椭圆拟合后的长轴半径
	int spotRy;             // 蛋白点椭圆拟合后的短轴半径
	float spotAveGray;      // 蛋白点盆地平均灰度
	int spotPerimeter;      // 蛋白点周长
	int *pWatershedLine;    // 分水岭脊线一维坐标数组.

	float spotBgAveGray;    // 蛋白点盆地外围背景平均灰度
	float spotInAveGray;    // 内标记的平均灰度
	//int imid;             // 内部标记ID
	//int emid;             // 外部标记ID

	//int nWSDineLength;    // 分水岭脊线所含像素个数.
	struct spotnode *next;  // 下一个蛋白点
} SpotNode, Spot;
#define LEN sizeof(Spot)    // 蛋白点结构体的长度

// 蛋白点双向链表结构
typedef struct spotlist
{
	SpotNode *first;
	SpotNode *last;
	int spotTotal;
	bool *watershed_Label;
	short *adjustedBG_pixdat;
} SpotList;
// CLP define
// -------------------------------------------

#endif // GEIMDEF_H_INCLUDED
