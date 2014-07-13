//---------------------------------------------------------------------------

#ifndef WATERSHED_H
#define WATERSHED_H

#include <wx/wx.h>
#include "GEIMDef.h"
#include "EffectPar.h"

#define  FAKE_COL_VAL -1
//---------------------------------------------------------------------------
/*******************当前像元8 -邻阈的像素信息****************/
typedef struct _CNeighborhoodpixels
{
	int Nhn_g[8];    //当前像元近邻的像素值矩阵
	int Nhn_ax[8];   //当前像元近邻的像素编号矩阵
	int Nhnlength;   //最邻近的包含在图片里的像素数
} CNeighborhoodpixels;

/******************当前像元的数据结构***********************/
typedef struct _pixel_T
{
	int x;    //当前像元X坐标
	int y;    //当前像元Y坐标
} pixel_T;

/*****************优先级队列的数据节点*********************/
struct node
{
	int iData;		// 数据
	int iPriority;	// 优先级
	int iOrder;		// 顺序
};

/************优先级队列的数据节点优先条件：大值优先**************/
struct node_LowestPriorityFirst_cmp
{
	bool operator()(const node &a, const node &b)
	{
		//如果a的优先级大于b的优先级，则返回true;
		if (a.iPriority > b.iPriority)
			return true;
		//否则，如果a的优先级小于b的优先级，则返回false
		else if (a.iPriority < b.iPriority)
			return false;
		//如果a的优先级等于b的优先级，且a的顺序大于b的顺序，返回true
		else
			return  a.iOrder > b.iOrder;
	}
};

/************优先级队列的数据节点优先条件：小值优先**************/
struct node_HighestPriorityFirst_cmp
{
	bool operator()(const node &a, const node &b)
	{
		//如果a的优先级小于b的优先级，则返回true;
		if (a.iPriority < b.iPriority)
			return true;
		//否则，如果a的优先级大于b的优先级，则返回false
		else if (a.iPriority > b.iPriority)
			return false;
		//如果a的优先级等于b的优先级，且a的顺序大于b的顺序，返回true
		else
			return  a.iOrder > b.iOrder;
	}
};

/*****************************************************************************
   WaterShed - 分水岭检测类
*****************************************************************************/
class WaterShed
{
	// 图像信息
	int nPixels = 0;		// 图像像素个数
	int pixHeight = 0;		// 图像高度
	int pixWidth = 0;		// 图像宽度

	// 检测参数
	int faintX = 0;					// 最弱点中心坐标
	int faintY = 0;
	float faintThreshold = 0.0f;	// 最弱点内外阈值
	float spotRxRyRatio = 3.0f;		// 蛋白点的长宽比
	bool isMedianFilter = true;		// 是否中值滤波
	int medianModeType = 1;			// 中值滤波类型 1 ：3X3
	bool isGaussFilter = true;		// 是否高斯滤波
	int gaussModeType = 1;			// 高斯滤波类型 1 ：3X3
	bool isBackgroud = false;		// 是否进行背景削减
	int maxSpotRadius = 30;			// 最大点半径
	int minSpotRadius = 2;			// 最小点半径
	bool isMorphological = false;	// 是否在预处理时先进行形态学闭运算
	int gradModeType = 1;			// 形态学梯度类型 半径大小
	int labelModeType = 8;			// 标记类型  4：4领域   8：8领域

	// 用于获取像素的领域
	CNeighborhoodpixels nghpixs;
	// 用于计数蛋白点编号
	int spot_ID = 0;
	// 预处理后的图像
	short* m_psImgAdjust = nullptr;
	// 蛋白点链表
	LS_SPOTS* m_plsSpots = nullptr;

public:
	WaterShed();
	~WaterShed();

    /** \brief 标记控制分水岭蛋白点检测算法
     *
     * \param pParEft EffectPar*	[INOUT] 图像信息
     * \param pDtParam PST_DTPARAM	[INOUT] 输入检测参数，输出自动弱点阈值
     * \param pList LS_SPOTS*		[INOUT] 蛋白点链表
     * \return bool true:成功 false:失败
     *
     */
	bool WSTmain(EffectPar* pParEft, PST_DTPARAM pDtParam, LS_SPOTS* pList);

    /** \brief 释放蛋白点链表
     *
     * \param pList LS_SPOTS*	[IN] 待释放的蛋白点链表
     * \return bool	true
     *
     */
	static bool ClearSpots(LS_SPOTS* pList);

private:
	/*-------------------------灰度图像预处理--------------------------------*/
	bool ImageFilter(short *fin, short *fout, bool isMedian, int medianModeType,
	                 bool isGaussFilter, int gaussModeType);
	bool AdjustBackgroud(short *fin, short *fout, bool isBackgroud, int ModeType);
	bool GradImage(short *fin, short *fout, int ModeType);
	bool ExtendMinAndDistanceWatershed(short *fin1, bool *fout1, bool *fout2,
	                                   short *fout3, int h , int labelMode);
	bool MedianFilter(short *fin, short *fout, int modeType);
	bool GaussFilter(short *fin, short *fout, int modeType);
	/*-------------------------灰度图像形态学变换--------------------------------*/
	bool Imdilate(short *fin, short *fout, int setype);
	bool DoImdilate(short *fin, short *fout, int radius);
	bool Imerode(short *fin, short *fout, int setype);
	bool DoImerode(short *fin, short *fout, int radius);

	/*---------------------------图像算术运算----------------------------------*/
	bool Imadd(short *fin, int h, short *fout);
	bool Imsubtract(short *fin, int h, short *fout);
	bool Imsubtract(short *fin1, short *fin2, short *fout);
	bool Imor(short *fin1 , short *fin2, short *fout);
	bool Immin(short *fin1 , short *fin2, short *fout);
	bool Imcomplement(short *fin , short *fout);

	/*--------------求内标记：扩展极小值、灰度重建、区域极大值--------------------*/
	bool Imextendedmin (short *fin,  int h, bool *bw); //极小值扩展
	/********************************************************************
	    函数：Imreconstruct  -  灰度重建
	    参数：marker：标记图像
	          mask：掩模图像
	    返回：如正确执行，则返回true，否则返回false
	********************************************************************/
	bool Imreconstruct (short *marker,  short *mask);
	template<typename _T>
	bool GetNgh3x3MinusCenterPix(_T  pix[], int x, int y);
	bool Is_inbounds_neighbor(int x , int y);
	bool Imregionalmax (short *marker, bool *BW);

	/*------------------------二值图像距离变换--------------------------------*/
	bool Bwdist(bool *bw , float *fdist);
	bool Update_candidates(pixel_T *candidates, int *num_candidates,
	                       short *col_values, int c, bool *bw, int M)  ;
	bool Compute_ell(pixel_T *ell, int *num_cells, pixel_T *candidates,
	                 int num_candidates, int c) ;
	bool Assign_distances(float *Out, int M, int column, pixel_T *ell,
	                      int num_cells, float *nn);
	bool Remove_candidate(pixel_T u, pixel_T v, pixel_T w, int c);

	/*------------------------标记二值图像连通区域--------------------------------*/
	int Bwlabel(bool *image_in , int *image_out, int mode );
	bool Labelset(bool *image_in, int *image_out, int xx , int yy , int label , int mode);

	/*---------------------------基于距离分水岭*----------------------------------*/
	bool watershed_meyer(short *fin, int *fmask, short *fout);
	bool watershed(short *fin, short *fout);
	bool Imimposemin(short *inPix, bool *inLabel, bool *exLabel , short *outPix);

	/*-----------------------------后处理-------------------------------------*/
	bool CalculateLabel(bool *inImg, bool *exImg , bool *wsImg , short *ws_Img);
	bool CalculateLabel(bool *inImg, bool *exImg , bool *wsImg, short *ws_Img,
	                    short *image_out, bool *ws_out,
	                    int xx , int yy , short label);
	/********************************************************************
	    函数：CalculateFaintRatio  -  计算最弱点阈值
	    参数：image_in：输入标记图像
	          image_out：输出标记图像
	          xx：当前点的X坐标
	          yy：当前点的Y坐标
	          label：当前区域的标记
	    返回：返回最弱点阈值
	********************************************************************/
	float CalculateFaintRatio(short *image_in, int *image_out,
							int xx, int yy, int label);
	float CalculateFaintRatio(bool *inImg, bool *exImg , bool *wsImg, short *ws_Img,
							int *image_out,
							int xx, int yy, int label);

	/*-----------------------保存数据到蛋白点链表---------------------------------*/
	bool AddSpot(int spotID,
				int centerX, int centerY, int centroidX, int centroidY,
				int spotArea, int spotVolume, int spotRx, int SpotRy, int spotPerimeter,
				float spotAveGray, float spotBgAveGray, float spotInAveGray, int *pWatershedLine );
};

#endif	// #ifdef WATERSHED_H
