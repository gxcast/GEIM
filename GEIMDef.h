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

//landmark标记
typedef struct LandMarkLink
{
	int x;      // 标记点的水平坐标点
	int y;      // 标记点的垂直坐标点
	char sign;  // 标记点的字母序号
	struct LandMarkLink *next;
} LandMarkLink;
#define LMLENTH sizeof(struct LandMarkLink)    // landmark结构体的长度

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

// 进行匹配用的窗口
typedef struct CW              //   要进行匹配的窗口大小
{
	int cwx1;                   //   窗口的左边框的x坐标
	int cwx2;                   //   窗口的右边框的x坐标
	int cwy1;                   //   窗口的上顶的y坐标
	int cwy2;                   //   窗口的下底的y坐标
} CW;
#define CWLENTH sizeof(struct CW)    //  CW结构体的长度

// 蛋白点检测参数结构体
typedef struct SegParameters
{
	int fx;                //最弱点X坐标
	int fy;                //最弱点Y坐标
	float faintStu;        //最弱点阈值
	int minSpotR;          //最小点半径
	int maxSpotR;          //最大点半径
	float spotRxRy;        //长宽比阈值
	bool isMedian;         //是否中值滤波
	int medianType;        //中值滤波模板半径
	bool isGauss;          //是否高斯滤波
	int gaussType;         //高斯滤波模板半径
	bool isBackgroudA;     //是否背景校正
	bool isMoClose;
} SegParameters;

// 蛋白点信息结构体
typedef struct GelSpot
{
	int CenterX ;                // 变换后的中心x坐标
	int CenterY ;                // 变换后的中心y坐标
	int origX ;                  //  原始的中心x坐标
	int origY ;                  //  原始的中心y坐标
	int ID;                      //  蛋白点的ID号
	int lmNbrI;                  //  离蛋白点最近的landmark 号 即蛋白点所归属的区域号: 0~25 间  初始化为-1
	int nbrAltLMs;               //  匹配的次数，包括第一次比较  初始化为 0

	int altLMset[4] ;             //  分别基于landmark匹配的次序landmark号，记录离蛋白点远近的landmark号，从次近记录起  初始化为-1

	float altLMsetDistSq[4];      //   蛋白点到标号为altLMset[4]的landmark的距离  初始化为 0.0f

	GelSpot *nextLMspot;          //   指向归属于同一landmark的下一个蛋白点   初始化为 null

	GelSpot *bestPairedSpot;      //  指向另一张凝胶中与该蛋白点最佳的配对点，也可能为NULL  初始化为 null
	int code;                   // 蛋白点的标记号，取值为0、1、2、3；分别对应US ，SP，PP，AP；
	// "SP" "PP"表示两蛋白点之间互相对应，但距离前者比后者近，
	//"AP"表示单向对应（即主胶中蛋白点在另副胶中找到对应的蛋白点，但副胶中该蛋白点在主胶中对应另一蛋白点）,
	//"UP"表示蛋白点没有在另张胶中找到对应蛋白点

	float  dP;                   // 蛋白点到最近蛋白点bestPairedSpot之间的距离

	float dL;                   // 蛋白点到对应landmark的距离；

	double aa[3];                   //  该Spot坐标变换到对应胶坐标系下的变换系数的 aa[0]、aa[1]、aa[2]  匹配胶要用上
	double bb[3];                   //  该Spot坐标变换到对应胶坐标系下的变换系数 bb[0]、bb[1]、bb[2]    匹配胶要用上
	GelSpot *next;              // 同一张凝胶中指向下一蛋白点地址指针

	//添加蛋白点的其他信息  如：面积、体积...
} GelSpot;

//实验分析用的地址索引号
typedef struct AddressIndex
{
	short int GroupNo;   // 虚拟胶号
	int SpotAdress;      // 蛋白点的编号
	AddressIndex *next;
} AddressIndex;
#define ADDINDEXLENTH sizeof(struct AddressIndex)    //  AddressIndex结构体的长度
// CLP define
// -------------------------------------------

#endif // GEIMDEF_H_INCLUDED
