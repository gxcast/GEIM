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

// 链表标准模板厍头文件
#include <list>

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

} ST_DTPARAM, *PST_DTPARAM;

// 蛋白点信息结构
typedef struct _ST_SPOT_NODE
{
	int id;				// 蛋白点标号

	int x;				// 蛋白点中心x坐标 几何
	int y;				// 蛋白点中心y坐标 几何
	int xm;				// 蛋白点质心x坐标 （匹配中用到）（显示中用到）
	int ym;				// 蛋白点质心x坐标 （匹配中用到）（显示中用到）

	int area;			// 蛋白点面积
	int volume;			// 蛋白点体积
	int perimeter;      // 蛋白点周长
	int* edge;			// 边缘点集 分水岭脊线
	int rx;				// 蛋白点椭圆拟合后的长轴半径
	int ry;				// 蛋白点椭圆拟合后的短轴半径

	float mean;			// 蛋白点盆地平均灰度
	float meanBK;		// 蛋白点盆地外围背景平均灰度
	float meanIN;		// 内标记的平均灰度

} ST_SPOT_NODE, *PST_SPOT_NODE;

// 蛋白点链表
typedef std::list<ST_SPOT_NODE> LS_SPOTS;

#endif // GEIMDEF_H_INCLUDED
