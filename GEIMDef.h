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
#include <vector>
#include <utility>

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
	/**< r:预处理后的灰度图像 g:蛋白点边缘 b:蛋白点内部 */
	PST_RGB pData = nullptr;
	/**< 蛋白点信息链表 */
	LS_SPOTS* pLs = nullptr;
} ST_DTRESULT, *PST_DTRESULT;

// 所有图像的检测结果
typedef std::list<ST_DTRESULT> LS_DTRESULT;

// 蛋白点表征向量
typedef struct _ST_SPOT_CHARACT_
{
	double x = 0.0;			// 座标
	double y = 0.0;
	double area = 0.0;		// 面积
	double mean = 0.0;		// 平均灰度
	double base = 0.0;		// 背景灰度
	double deep = 0.0;		// 深度
	double plump = 0.0;		// 饱满度
} ST_SPOT_CHARACT, *PST_SPOT_CHARACT;

// 蛋白点原始属性+表征向量
typedef struct _ST_SPOT_ATTR_
{
	// 原始属性
	PST_SPOT_NODE pNode = nullptr;
	// 表征向量
	PST_SPOT_CHARACT pCrt = nullptr;
	// 是否无效
	bool bInvalid = false;
} ST_SPOT_ATTR, *PST_SPOT_ATTR;

// 蛋白点属性链表
typedef std::vector<ST_SPOT_ATTR> VT_ATTR;

// 匹配输入参数
typedef struct _ST_MTPARAM_
{
	int iW = 0;		// 图像宽度
	int iWb = 0;	// 图像行字节数
	int iH = 0;		// 图像高度
	int iN = 0;		// 图像像素数

	PST_RGB pImg = nullptr;		// 图像数据
	PST_RGB pData = nullptr;	// 检测结果r:预处理后的灰度图像 g:蛋白点边缘 b:蛋白点内部

	VT_ATTR* pvtAttr = nullptr;	// 蛋白点属性集
} ST_MTPARAM, *PST_MTPARAM;

// 待匹配的一对图像
typedef std::pair<ST_MTPARAM,ST_MTPARAM> ST_MTPAIR;

// 匹配结果
typedef struct _ST_MTRESULT_
{
	int id = 0;
} ST_MTRESULT, *PST_MTRESULT;

#endif // GEIMDEF_H_INCLUDED
