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
	int fData;     //数据
	int fPriority;   //优先级
	int fOrder;    //顺序
};

/************优先级队列的数据节点优先条件：大值优先**************/
struct node_LowestPriorityFirst_cmp
{
	bool operator()(const node &a, const node &b)
	{
		//如果a的优先级大于b的优先级，则返回true;
		if (a.fPriority > b.fPriority)
			return true;
		//否则，如果a的优先级小于b的优先级，则返回false
		else if (a.fPriority < b.fPriority)
			return false;
		//如果a的优先级等于b的优先级，且a的顺序大于b的顺序，返回true
		else
			return  a.fOrder > b.fOrder;
	}
};

/************优先级队列的数据节点优先条件：小值优先**************/
struct node_HighestPriorityFirst_cmp
{
	bool operator()(const node &a, const node &b)
	{
		//如果a的优先级小于b的优先级，则返回true;
		if (a.fPriority < b.fPriority)
			return true;
		//否则，如果a的优先级大于b的优先级，则返回false
		else if (a.fPriority > b.fPriority)
			return false;
		//如果a的优先级等于b的优先级，且a的顺序大于b的顺序，返回true
		else
			return  a.fOrder > b.fOrder;
	}
};

/*****************************************************************************
   WaterShed - 分水岭检测类
*****************************************************************************/
class WaterShed
{
private:

	/*****************************原图像的信息****************************/
	/**< cache management */
	EffectPar* m_pParEft;
	/**< the inpute image */
	unsigned char* m_pImgIn;
	/**< is modify the in image */
	bool m_bMdfyIn;

	int nPixels;			// 图像像素个数
	int pixHeight;	// 图像高度
	int pixWidth; 		// 图像宽度

	/***************************图像指针*********************************/
	//    short * pre_pixdat;	// 原始灰度图像数据。
	short *filted_pixdat;		// 滤波后图像数据。
	short *adjustedBG_pixdat;  //去背景后图像数据。
	short *grad_pixdat;  //梯度图像数据。

	/****************************标记指针*******************************/
	bool *internal_label;  //内部标记标签矩阵。
	bool *external_label;  //外部标记标签矩阵。
	//    bool * watershed_label; //分水岭标签矩阵。

	/*************************输入的参数变量***************************/
	int faintX, faintY; //最弱点中心坐标
	//float  faintThreshold;    // 最弱点内外阈值
	float  spotRxRyRatio;       // 蛋白点的长宽比
	bool isMedianFilter;        // 是否中值滤波
	bool isGaussFilter;         // 是否高斯滤波
	int medianModeType;         // 中值滤波类型 1 ：3X3
	int gaussModeType;          // 高斯滤波类型 1 ：3X3
	int gradModeType;           // 形态学梯度类型 半径大小
	bool isBackgroud;           // 是否进行背景削减
	int maxSpotRadius;          // 最大点半径
	int minSpotRadius;          // 最小点半径
	int labelModeType;          // 标记类型  4：4领域   8：8领域
	bool isMorphological;       // 是否在预处理时先进行形态学闭运算
	/*****************************邻阈信息****************************/
	CNeighborhoodpixels nghpixs;

	/**************************蛋白点链表信息****************************/
	//SpotList * ws_spotList;   // 蛋白点链表
	int spot_ID;                // 蛋白点编号
public:
	short *pre_pixdat;      // 原始灰度图像数据。
	bool *watershed_label;  // 分水岭标签矩阵
	SpotList *ws_spotList;  // 蛋白点链表
	float  faintThreshold;  // 最弱点内外阈值

public:
	/********************************************************************
	    函数：WaterShed  -  构造函数
	    参数：EffectPar* pParEft：输入图像32bpp；
	*******************************************************************/
	WaterShed(EffectPar* pParEft);
	/********************************************************************
	    函数：~WaterShed  - 析构函数
	    参数：无
	********************************************************************/
	~WaterShed();
	/********************************************************************
	    函数：WSTmain  -  分水岭检测蛋白点程序的主函数
	    参数：无
	    返回：如正确执行，则返回true，否则返回false
	********************************************************************/
	bool WSTmain();
	/********************************************************************
	    函数：FreeMain  -  释放主函数中分配的空间
	    参数：无
	    返回：如正确执行，则返回true，否则返回false
	********************************************************************/
	bool FreeMain();
	bool setDefaultPar(PST_DTPARAM pDtParam);

	/*-------------------------灰度图像预处理--------------------------------*/
	bool ImageFilter(short *fin, short *fout, bool isMedian, int medianModeType,
	                 bool isGaussFilter, int gaussModeType);
	bool AdjustBackgroud(short *fin, short *fout, bool isBackgroud, int ModeType);
	bool GradImage(short *fin, short *fout, int ModeType);
	bool ExtendMinAndDistanceWatershed(short *fin1, bool *fout1, bool *fout2,
	                                   short *fout3, int h , int labelMode);
	bool MedianFilter(short *fin, short *fout, int modeType);
	bool GaussFilter(short *fin, short *fout, int modeType);
	//    bool GenerateSE(int seType,int radius) ;//自动产生结构元素  type：类型  radius：半径
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
	template<typename _T>
	bool watershed_meyer(_T *fin, int *fmask, short *fout);
	template<typename _T>
	bool watershed(_T *fin, short *fout);
	bool Imimposemin(short *inPix, bool *inLabel, bool *exLabel , short *outPix);

	/*-----------------------------后处理-------------------------------------*/
	//   bool CalculateLabel(bool *inImg,short *exImg ,short *wsImg );
	bool CalculateLabel(bool *inImg, bool *exImg , bool *wsImg , short *ws_Img);
	//    bool CalculateLabel(short *image_in,int *image_out,bool *ws_out, int xx ,
	//                        int yy ,int label);
	bool CalculateLabel(bool *inImg, bool *exImg , bool *wsImg, short *ws_Img,
	                    int *image_out, bool *ws_out, int xx , int yy , int label);
	/********************************************************************
	    函数：CalculateFaintRatio  -  计算最弱点阈值
	    参数：image_in：输入标记图像
	          image_out：输出标记图像
	          xx：当前点的X坐标
	          yy：当前点的Y坐标
	          label：当前区域的标记
	    返回：返回最弱点阈值
	********************************************************************/
	float CalculateFaintRatio(short *image_in, int *image_out, int xx, int yy, int label);
	float CalculateFaintRatio(bool *inImg, bool *exImg , bool *wsImg, short *ws_Img, int *image_out, int xx, int yy, int label);
	/*-----------------------保存数据到蛋白点链表---------------------------------*/

	/********************************************************************
	    函数：InitSpotList  -  初始化相同游程链表头节点
	    参数：list：蛋白点链表
	      total：当前蛋白点编号
	    返回：如正确执行，则返回true，否则返回false
	********************************************************************/
	bool InitSpotList(SpotList *list, int total);
	bool TailSpotToList(SpotList *list, int spotID, int centerX, int centerY,
	                    int centroidX, int centroidY, int spotArea, int spotVolume,
	                    int spotRx, int SpotRy , int spotPerimeter, float spotAveGray,
	                    float spotBgAveGray, float spotInAveGray, int *pWatershedLine );

	/********************************************************************
	    函数：DeleteHeadNode  -  删除链表头节点
	    参数：list：蛋白点链表
	    返回：如正确执行，则返回true，否则返回false
	********************************************************************/
	bool DeleteHeadNode(SpotList *list) ;

	/********************************************************************
	    函数：DestroyList -  删除链表
	    参数：list：蛋白点链表
	    返回：如正确执行，则返回true，否则返回false
	********************************************************************/
	bool DestroyList(SpotList *list);
};

#endif	// #ifdef WATERSHED_H
