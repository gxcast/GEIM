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

/**< pixel struct */
typedef struct _ST_RGB_
{
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;
} ST_RGB, *PST_RGB;

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
typedef struct _ST_SPOT_NODE_
{
	int id = 0;				// 蛋白点标号

	int x = 0;				// 蛋白点中心x坐标 几何
	int y = 0;				// 蛋白点中心y坐标 几何
	int xm = 0;				// 蛋白点质心x坐标 （匹配中用到）（显示中用到）
	int ym = 0;				// 蛋白点质心x坐标 （匹配中用到）（显示中用到）

	int area = 0;			// 蛋白点面积
	int volume = 0;			// 蛋白点体积
	int perimeter = 0;      // 蛋白点周长
	int* edge = nullptr;	// 边缘点集 分水岭脊线
	int rx = 0;				// 蛋白点椭圆拟合后的长轴半径
	int ry = 0;				// 蛋白点椭圆拟合后的短轴半径

	float mean = 0.0f;		// 蛋白点盆地平均灰度
	float meanBK = 0.0f;	// 蛋白点盆地外围背景平均灰度
	float meanIN = 0.0f;	// 内标记的平均灰度

} ST_SPOT_NODE, *PST_SPOT_NODE;

// 蛋白点链表
typedef std::list<ST_SPOT_NODE> LS_SPOTS;

// 一幅凝胶图像的检测结果
typedef struct _ST_DTRESULT_
{
	/**< r:预处理后的灰度图像 g:蛋白点边缘 b:外标记 */
	PST_RGB pData = nullptr;
	/**< 蛋白点信息链表 */
	LS_SPOTS* pLs = nullptr;
} ST_DTRESULT, *PST_DTRESULT;

// 所有图像的检测结果
typedef std::list<ST_DTRESULT> LS_DTRESULT;

#endif // GEIMDEF_H_INCLUDED
