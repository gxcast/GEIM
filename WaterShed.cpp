//---------------------------------------------------------------------------
#include "WaterShed.h"
#include <math.h>
#include <queue>
#include <stack>
#include <vector>
#include <algorithm>

#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a, b)  (((a) < (b)) ? (a) : (b))

#define  LABEL  1   //标记初始值 默认从1开始标记
#define MASK -2     // initial value of a threshold level
#define WSHED 0     // value of the pixels belongs to the watersheds
#define INIT -1     // initial value of im0
#define FICTITIOUS -1
#define MWSIZE_MAX 10000
//---------------------------------------------------------------------------


/********************************************************************
    函数：WaterShed  -  构造函数
    参数：EffectPar* pParEft：输入图像32bpp；
*******************************************************************/
WaterShed::WaterShed(EffectPar* pParEft)
{
	/*********图像信息*************/
	m_pParEft = pParEft;
	m_pImgIn = m_pParEft->Input();
	m_bMdfyIn = m_pParEft->Modify();
	// if can't  modify in image, create a copy, and, can modify the copy
	if (!m_bMdfyIn)
	{
		unsigned char* pImgCah = m_pParEft->GetCache();
		memcpy(pImgCah, m_pImgIn, (size_t)m_pParEft->PixNum()*3);
		m_pImgIn = pImgCah;
	}

	pixWidth = m_pParEft->Width(); 		// 获取图像宽
	pixHeight = m_pParEft->Height(); 	// 获取图像高
	nPixels = pixWidth * pixHeight; 			// 获取图像总像素数
	/*********图像指针*************/
	pre_pixdat = nullptr; 	//原始灰度图像数据。
	filted_pixdat = nullptr; //滤波后图像数据。
	adjustedBG_pixdat = nullptr; //去背景后图像数据。
	grad_pixdat = nullptr; //梯度图像数据。
	internal_label = nullptr; //内部标记标签矩阵。
	external_label = nullptr; //外部标记标签矩阵。
	watershed_label = nullptr; //分水岭标签矩阵。

	/*********输入参数*************/
	isMedianFilter = true; //是否中值滤波
	isGaussFilter = true; //是否高斯滤波
	medianModeType = 1;   //中值滤波类型 1 ：3X3
	gaussModeType = 1;   //高斯滤波类型 1 ：3X3

	isBackgroud = false; //是否进行背景削减
	maxSpotRadius = 30; //  最大点半径

	gradModeType = 1; //形态学梯度类型 半径大小
	labelModeType = 8; //标记类型  4：4领域   8：8领域

	faintThreshold = -1 ; //最弱点内外阈值
	spotRxRyRatio = 3.0 ; //蛋白点的长宽比

	minSpotRadius = 2; //最小点半径
	faintX = 0;
	faintY = 0;

	isMorphological = true; //是否在预处理时先进行形态学闭运算
	/********蛋白点链表信息**************/
	ws_spotList = nullptr; //蛋白点链表
	spot_ID = 0;        //蛋白点编号
}
/********************************************************************
    函数：~WaterShed  - 析构函数
    参数：无
********************************************************************/
WaterShed::~WaterShed()
{
	if (!m_bMdfyIn)
	{
		m_pParEft->Recycle(m_pImgIn);
		m_pImgIn = nullptr;
	}
}

/********************************************************************
    函数：WSTmain  -  分水岭检测蛋白点程序的主函数
    参数：无
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::WSTmain()
{
	/*********************导入实验参数***********************************/
	gradModeType = 1; //形态学梯度类型 半径大小
	labelModeType = 8; //标记类型  4：4领域   8：8领域
	/*********************导入图像数据***********************************/

	if (pre_pixdat)
	{
		delete[] pre_pixdat ;
		pre_pixdat = nullptr ;
	}
	pre_pixdat = new short[nPixels]; //原图像
	memset(pre_pixdat, 0, sizeof(short)*nPixels);
	unsigned char *Ptr = m_pImgIn;
	for (int n = 0; n < nPixels; n++)
	{
		pre_pixdat[n] = Ptr[0]; 	//并将像素值保存在pixdat中
		Ptr += 3;
	}

	/********************** 1 对输入图像滤波处理*****************************/
	if (filted_pixdat)
	{
		delete[] filted_pixdat ;
		filted_pixdat = nullptr ;
	}
	filted_pixdat = new short[nPixels]; //滤波后图像数据
	memset(filted_pixdat, 0, sizeof(short)*nPixels);
	memcpy(filted_pixdat, pre_pixdat, nPixels * sizeof(short)); //默认数据为原图

	//滤波处理
	if (!ImageFilter(pre_pixdat , filted_pixdat , isMedianFilter , medianModeType,
	                 isGaussFilter, gaussModeType) )
		return false;

	/********************** 2 去除背景后的图像*******************************/
	if (adjustedBG_pixdat)
	{
		delete[] adjustedBG_pixdat ;
		adjustedBG_pixdat = nullptr ;
	}
	adjustedBG_pixdat = new short[nPixels]; //去背景后图像数据
	memset(adjustedBG_pixdat, 0, sizeof(short)*nPixels);
	memcpy(adjustedBG_pixdat, filted_pixdat, nPixels * sizeof(short)); //默认数据为滤波后的图像

	// 背景消减
	if (!AdjustBackgroud(filted_pixdat, adjustedBG_pixdat, isBackgroud, maxSpotRadius))
		return false;

	/************************* 3 计算形态学梯度*****************************/
	if (grad_pixdat)
	{
		delete[] grad_pixdat ;
		grad_pixdat = nullptr ;
	}
	grad_pixdat = new short[nPixels]; //去背景后图像数据
	memset(grad_pixdat, 0, sizeof(short)*nPixels);

	//计算形态学梯度
	if (!GradImage(adjustedBG_pixdat, grad_pixdat, gradModeType))
		return false;

	/******* 4 第一次分水岭  内标记（扩展极小值）、外标记**********************/
	//内标图 二值图像
	if (internal_label)
	{
		delete[] internal_label ;
		internal_label = nullptr ;
	}
	internal_label = new bool[nPixels];
	memset(internal_label, 0, sizeof(bool)*nPixels);

	//外标记数组emlabel，分水岭边界图 1
	if (external_label)
	{
		delete[] external_label ;
		external_label = nullptr ;
	}
	external_label = new bool [nPixels];
	memset(external_label, 0, sizeof(bool)*nPixels);

	//    int labelModeType=8; //标记类型  4：4领域   8：8领域
	int h = 2; //扩展极小值深度

	short *exPix_ws;     //watershedlabel   gradPix_ws
	exPix_ws = new short [nPixels];
	memset(exPix_ws, 0, sizeof(short)*nPixels);

	// 极小值扩展（内标）、分水岭（外标记）、涨水标注
	if (!ExtendMinAndDistanceWatershed(adjustedBG_pixdat, internal_label, external_label,
	                                   exPix_ws, h , labelModeType) )
		return false;

	/********************* 5 极小值强加修改梯度图像*****************************/
	short *regradPix;     //watershedlabel   gradPix_ws
	regradPix = new short [nPixels];
	memset(regradPix, 0, sizeof(short)*nPixels);
	memcpy(regradPix, grad_pixdat, sizeof(short)*nPixels);

	//极小值强加
	if (!Imimposemin(grad_pixdat, internal_label, external_label, regradPix))
		return false;

	/*********** 6 第二次分水岭  对修改后的梯度图像进行分水岭******************/
	short *gradPix_ws;     //watershedlabel   gradPix_ws
	gradPix_ws = new short [nPixels];
	memset(gradPix_ws, 0, sizeof(short)*nPixels);
	//梯度图像分水岭后的脊线标记数组watershedlabel，分水岭边界图 1
	if (watershed_label)
	{
		delete[] watershed_label ;
		watershed_label = nullptr ;
	}
	watershed_label = new bool [nPixels];
	memset(watershed_label, 0, sizeof(bool)*nPixels);

	//meyer分水岭
	if (!watershed(regradPix, gradPix_ws))
		return false;

	for (int k = 0; k < nPixels; k++)
	{
		// 分水岭边界置1
		if (gradPix_ws[k] < 1)
			watershed_label[k] = true;
	}

	/**************************后处理 *****************************/
	if (!CalculateLabel(internal_label, external_label, watershed_label, gradPix_ws) )
		return false;

	/************************释放分配的指针空间*****************************/
	if (gradPix_ws)
	{
		//释放修改后的梯度图像的分水岭指针空间
		delete[] gradPix_ws;
		gradPix_ws = nullptr ;
	}
	if (regradPix)
	{
		//释放修改后的梯度图像指针空间
		delete[] regradPix;
		regradPix = nullptr ;
	}
	if (exPix_ws)
	{
		//释放内标记图像极小值扩展的指针空间
		delete[] exPix_ws;
		exPix_ws = nullptr ;
	}

	return true;
}

bool WaterShed::setDefaultPar(PST_DTPARAM pDtParam)
{
	// check the parameter
	if (pDtParam == nullptr)
	{
		wxASSERT_MSG(false, _T("setDefaultPar param is null"));
		return false;
	}

	//获得参数值
	faintX = pDtParam->ptFaint.x;
	faintY = pDtParam->ptFaint.y;
	faintThreshold = (float)(pDtParam->dFaint);
	isMedianFilter = (pDtParam->iMedianFlt >= 0); //是否中值滤波
	medianModeType = pDtParam->iMedianFlt + 1;   //中值滤波类型 1 ：3X3

	isGaussFilter = (pDtParam->iGaussFlt >= 0);  //是否高斯滤波
	gaussModeType = pDtParam->iGaussFlt + 1;   //高斯滤波类型 1 ：3X3

	isBackgroud = false; 										// 是否进行背景削减
	maxSpotRadius = pDtParam->iMaxRad; //  最大点半径

	//   faintThreshold=faintTh; //最弱点内外阈值
	spotRxRyRatio = pDtParam->dAspect; //蛋白点的长宽比
	if (spotRxRyRatio < 0)
		spotRxRyRatio = -spotRxRyRatio;
	if (spotRxRyRatio < 1 && spotRxRyRatio > 0)
		spotRxRyRatio = 1/spotRxRyRatio;

	minSpotRadius = pDtParam->iMinRad;

	isMorphological = false;
	return true;
}

bool WaterShed::FreeMain()
{
	// 释放蛋白点链表空间
	if (!DestroyList(ws_spotList))
		return false ;
	//释放图像指针空间
	if (watershed_label)
	{
		delete[] watershed_label;
		watershed_label = nullptr;
	}
	if (external_label)
	{
		delete[] external_label;
		external_label = nullptr;
	}
	if (internal_label)
	{
		delete[] internal_label;
		internal_label = nullptr;
	}
	if (grad_pixdat)
	{
		delete[] grad_pixdat;
		grad_pixdat = nullptr;
	}
	if (adjustedBG_pixdat)
	{
		delete[] adjustedBG_pixdat;
		adjustedBG_pixdat = nullptr;
	}
	if (filted_pixdat)
	{
		delete[] filted_pixdat;
		filted_pixdat = nullptr;
	}
	if (pre_pixdat)
	{
		delete[] pre_pixdat;
		pre_pixdat = nullptr;
	}

	return true;
}


bool WaterShed::ImageFilter(short *fin, short *fout, bool isMedianFilter,
                            int medianModeType, bool isGaussFilter, int gaussModeType)
{
	//图像滤波执行函数
	if (fin == nullptr || fout == nullptr || (isMedianFilter && medianModeType < 0) || (isGaussFilter && gaussModeType < 0))
		return false;
	/********************** 1 先开运算图像 HF ******************************/
	short *temp_erodepixdat;  //临时数据
	temp_erodepixdat = nullptr;
	temp_erodepixdat = new short[nPixels];
	memset(temp_erodepixdat, 0, sizeof(short)*nPixels);
	memcpy(temp_erodepixdat, fin, nPixels * sizeof(short)); //默认数据为原图

	short *temp_dilatepixdat;  //临时数据
	temp_dilatepixdat = nullptr;
	temp_dilatepixdat = new short[nPixels];
	memset(temp_dilatepixdat, 0, sizeof(short)*nPixels);
	memcpy(temp_erodepixdat, fin, nPixels * sizeof(short)); //默认数据为原图


	//    bool isMorphological=true;
	if (isMorphological)
	{
		int seR = 1;

		if (!DoImdilate(fin, temp_dilatepixdat, seR)) //膨胀  SE半径为2
			return false;

		if (!DoImerode(temp_dilatepixdat, temp_erodepixdat, seR)) //腐蚀  SE半径为2
			return false;
		/*

		       if(!DoImerode(temp_erodepixdat,temp_dilatepixdat,seR))  //腐蚀  SE半径为2
		           return false;
		       if(!DoImdilate(temp_dilatepixdat,temp_erodepixdat,seR))  //膨胀  SE半径为2
		           return false;
		*/
	}
	if (temp_dilatepixdat)
	{
		delete[] temp_dilatepixdat;
		temp_dilatepixdat = nullptr;
	}
	//    bool isMedianFilter=true;
	short *temp_pixdat;  //临时数据
	temp_pixdat = nullptr;
	temp_pixdat = new short[nPixels]; //滤波后图像数据
	memset(temp_pixdat, 0, sizeof(short)*nPixels);
	memcpy(temp_pixdat, temp_erodepixdat, nPixels * sizeof(short)); //默认数据为原图
	/********************** 2 中值滤波去椒盐噪声******************************/
	if (isMedianFilter)
	{
		//进行中值滤波
		if (!MedianFilter(temp_erodepixdat, temp_pixdat, medianModeType) )
			return false;
	}
	else
	{
		//不进行中值滤波
		memcpy(temp_pixdat, temp_erodepixdat, nPixels * sizeof(short));
	}

	if (temp_erodepixdat)
	{
		delete[] temp_erodepixdat;
		temp_erodepixdat = nullptr;
	}
	/********************** 3 高斯滤波去白噪声******************************/

	//    bool isGaussFilter=true;
	if (isGaussFilter)
	{
		//进行高斯滤波
		if (!GaussFilter(temp_pixdat, fout, gaussModeType) )
			return false;
	}
	else
	{
		//不进行高斯滤波
		memcpy(fout, temp_pixdat, nPixels * sizeof(short));
	}

	//释放临时图像数据空间
	if (temp_pixdat)
	{
		delete[] temp_pixdat;
		temp_pixdat = nullptr;
	}

	return true;
}


bool WaterShed::AdjustBackgroud(short *fin, short *fout, bool isBackgroud, int ModeType)
{
	if (fin == nullptr || fout == nullptr || (isBackgroud && ModeType < 0))
		return false;

	if (isBackgroud)
	{
		//进行背景消减
		short *temp_pixdat1;  //预处理后的图像的反色图像
		temp_pixdat1 = nullptr;
		temp_pixdat1 = new short[nPixels];
		memset(temp_pixdat1, 0, sizeof(short)*nPixels);
		//3.1 对滤波后的图像，求反
		if (!Imcomplement(fin, temp_pixdat1))
			return false;

		/*3.2（白）高帽变换   temp_pixdat1 - Imdilate(Imerode)开 */

		//开运算  先腐蚀，后膨胀
		short *temp_pixdat1_erode;  //腐蚀输出图像
		temp_pixdat1_erode = nullptr;
		temp_pixdat1_erode = new short[nPixels];
		memset(temp_pixdat1_erode, 0, sizeof(short)*nPixels);
		//3.2.1  先腐蚀，
		if (!DoImerode(temp_pixdat1, temp_pixdat1_erode, ModeType)) //maxRadius
			return false;

		short *open_pixdat;  //开运算图像
		open_pixdat = nullptr;
		open_pixdat = new short[nPixels];
		memset(open_pixdat, 0, sizeof(short)*nPixels);
		//3.2.2 后膨胀
		if (!DoImdilate(temp_pixdat1_erode, open_pixdat, ModeType))
			return false;

		short *top_hat_pixdat;  //原图(预处理图像的反色图像)减去开运算图
		top_hat_pixdat = nullptr;
		top_hat_pixdat = new short[nPixels];
		memset(top_hat_pixdat, 0, sizeof(short)*nPixels);
		//3.2.3 top-hat  背景消减
		if (!Imsubtract(temp_pixdat1, open_pixdat, top_hat_pixdat))
			return false;

		//3.2.4 输出背景消减后的图像的反色图像
		if (!Imcomplement(top_hat_pixdat, fout))
			return false;

		//释放临时图像数据空间
		if (top_hat_pixdat)
		{
			delete[] top_hat_pixdat;
			top_hat_pixdat = nullptr;
		}
		if (open_pixdat)
		{
			delete[] open_pixdat;
			open_pixdat = nullptr;
		}
		if (temp_pixdat1_erode)
		{
			delete[] temp_pixdat1_erode;
			temp_pixdat1_erode = nullptr;
		}
		if (temp_pixdat1)
		{
			delete[] temp_pixdat1;
			temp_pixdat1 = nullptr;
		}
	}
	else
	{
		//未进行背景消减，则使用滤波后的图像
		memcpy(fout, fin, nPixels * sizeof(short));
	}

	return true;
}



bool WaterShed::GradImage(short *fin, short *fout, int ModeType)
{
	if (!fin || !fout || ModeType < 1)
		return false;

	short *pixdilate;  // 膨胀图像
	pixdilate = nullptr;
	pixdilate = new short[nPixels];
	memset(pixdilate, 0, sizeof(short)*nPixels);

	short *pixerode;  // 腐蚀图像
	pixerode = nullptr;
	pixerode = new short[nPixels];
	memset(pixerode, 0, sizeof(short)*nPixels);

	// 1、 膨胀
	if (!DoImdilate(fin, pixdilate, ModeType))
		return false;

	// 2、腐蚀，
	if (!DoImerode(fin, pixerode, ModeType))
		return false;

	// 3、膨胀减腐蚀
	if (!Imsubtract(pixdilate, pixerode, fout))
		return false;

	//释放临时图像数据空间
	if (pixdilate)
	{
		delete[] pixdilate;
		pixdilate = nullptr;
	}
	if (pixerode)
	{
		delete[] pixerode;
		pixerode = nullptr;
	}

	return true;
}



bool WaterShed::ExtendMinAndDistanceWatershed(short *fin1, bool *fout1, bool *fout2,
        short *fout3, int h , int labelMode)
{
	if (fin1 == nullptr || h < 0 || (labelMode != 4 && labelMode != 8)) //如果没有输入图像，则返回false
		return false;

	/********************** step1 内标记 极小值扩展**************************/
	// 极小值扩展求内标记，去除一些不符合要求的极小值区域
	short *ilpix;  // 内标记输入的灰度图像
	ilpix = nullptr;
	ilpix = new short[nPixels];
	memset(ilpix, 0, sizeof(short)*nPixels);
	memcpy(ilpix, fin1, nPixels * sizeof(short)); //默认数据为滤波后的图像

	//内标图 二值图像
	bool *imlabel;
	imlabel = new bool[nPixels];

	//求内标记    扩展极小值
	if (!Imextendedmin(ilpix, h, imlabel))
		return false;

	/***************** step2 对内标记二值图像进行标注**************************/
	int *labelpix;
	labelpix = new int [nPixels]; //输出的内标记的标注图像
	memset(labelpix, 0, sizeof(int)*nPixels);

	int label_num = 0; //连通域数
	//对内标记图像的连通阈进行标注labelpix，并返回连通域数
	label_num = Bwlabel(imlabel , labelpix, labelMode);
	if (label_num == 0)
		return false;

	/***** step3 外标记（计算内部标记（二值）图像的距离变换的分水岭变换）****/
	//3.1 计算内标记间的距离图
	float *fbwdist = new float[nPixels];
	memset(fbwdist, 0.0, sizeof(float)*nPixels);
	//求内标记间的距离  fbwdist
	if (!Bwdist(imlabel, fbwdist))
		return false;

	//优化距离图像，8领域的距离相等
	short* pbwdist = (short*)fbwdist;
	for (int i = 0; i < nPixels; i++)
		pbwdist[i] = (short)(fbwdist[i] + 0.5);

	//3.2对内标记的距离变换图进行分水岭变换
	//分水岭 求外标记
	//分水岭标记图
	short *exLabel_ws ;  //分水岭区域图
	exLabel_ws = new short [nPixels];
	memset(exLabel_ws, 0, sizeof(short)*nPixels);

	if (!watershed(pbwdist, exLabel_ws))
		return false;  //meyer分水岭

	//外标记数组emlabel，分水岭边界图 1
	bool *emlabel;
	emlabel = new bool [nPixels];
	memset(emlabel, 0, sizeof(bool)*nPixels);

	for (int k = 0; k < nPixels; k++)
	{
		if (exLabel_ws[k] < 1)
			emlabel[k] = true; 		//分水岭边界置1
	}

	if (fout1)  //如果需要输出内标记
		memcpy(fout1, imlabel, nPixels * sizeof(bool)); //输出内标记

	if (fout2)  //如果需要输出外标记脊线
		memcpy(fout2, emlabel, nPixels * sizeof(bool)); //输出外标记

	memcpy(fout3, exLabel_ws, nPixels * sizeof(short)); //输出标记区域

	if (emlabel)
	{
		delete[] emlabel;
		emlabel = nullptr;
	}
	if (exLabel_ws)
	{
		delete[] exLabel_ws;
		exLabel_ws = nullptr;
	}
	if (fbwdist)
	{
		delete[] fbwdist;
		fbwdist = nullptr;
	}
	if (labelpix)
	{
		delete[] labelpix;
		labelpix = nullptr;
	}
	if (imlabel)
	{
		delete[] imlabel;
		imlabel = nullptr;
	}
	if (ilpix)
	{
		delete[] ilpix;
		ilpix = nullptr;
	}
	return true;
}


bool WaterShed::MedianFilter(short *fin, short *fout, int modeType)
{
	if (fin == nullptr || fout == nullptr) //如果没有输入图像，则返回false
		return false;
	short *p;  //模板对应图像中的灰度值
	short *mode;   //模板
	p = mode = 0;
	int modeSize = 0;
	int edge = modeType;

	//默认数据为输入图像
	memcpy(fout, fin, nPixels * sizeof(short));

	switch (modeType)
	{
	//选择模板
	case 2:
	{
		//半径为2
		/*
		1 1 1 1 1
		1 1 1 1 1
		1 1 1 1 1
		1 1 1 1 1
		1 1 1 1 1
		*/
		//结构元素大小
		modeSize = 25;

		//存储结构元素对应点的灰度值
		p = new short[modeSize];
		memset(p, 0, sizeof(short)*modeSize);
		//存储结构元素
		mode = new short[modeSize];
		memset(mode, 0, sizeof(short)*modeSize);
		//结构元素
		mode[0] = 1;
		mode[1] = 1;
		mode[2] = 1;
		mode[3] = 1;
		mode[4] = 1;
		mode[5] = 1;
		mode[6] = 1;
		mode[7] = 1;
		mode[8] = 1;
		mode[9] = 1;
		mode[10] = 1;
		mode[11] = 1;
		mode[12] = 1;
		mode[13] = 1;
		mode[14] = 1;
		mode[15] = 1;
		mode[16] = 1;
		mode[17] = 1;
		mode[18] = 1;
		mode[19] = 1;
		mode[20] = 1;
		mode[21] = 1;
		mode[22] = 1;
		mode[23] = 1;
		mode[24] = 1;

		break;
	}
	case 3:
	{
		//半径为3
		/*
		 1 1 1 1 1 1 1
		 1 1 1 1 1 1 1
		 1 1 1 1 1 1 1
		 1 1 1 1 1 1 1
		 1 1 1 1 1 1 1
		 1 1 1 1 1 1 1
		 1 1 1 1 1 1 1
		*/
		//结构元素大小
		modeSize = 49;

		//存储结构元素对应点的灰度值
		p = new short[modeSize];
		memset(p, 0, sizeof(short)*modeSize);
		//存储结构元素
		mode = new short[modeSize];
		memset(mode, 0, sizeof(short)*modeSize);
		//结构元素
		mode[0] = 1;
		mode[1] = 1;
		mode[2] = 1;
		mode[3] = 1;
		mode[4] = 1;
		mode[5] = 1;
		mode[6] = 1;
		mode[7] = 1;
		mode[8] = 1;
		mode[9] = 1;
		mode[10] = 1;
		mode[11] = 1;
		mode[12] = 1;
		mode[13] = 1;
		mode[14] = 1;
		mode[15] = 1;
		mode[16] = 1;
		mode[17] = 1;
		mode[18] = 1;
		mode[19] = 1;
		mode[20] = 1;
		mode[21] = 1;
		mode[22] = 1;
		mode[23] = 1;
		mode[24] = 1;
		mode[25] = 1;
		mode[26] = 1;
		mode[27] = 1;
		mode[28] = 1;
		mode[29] = 1;
		mode[30] = 1;
		mode[31] = 1;
		mode[32] = 1;
		mode[33] = 1;
		mode[34] = 1;
		mode[35] = 1;
		mode[36] = 1;
		mode[37] = 1;
		mode[38] = 1;
		mode[39] = 1;
		mode[40] = 1;
		mode[41] = 1;
		mode[42] = 1;
		mode[43] = 1;
		mode[44] = 1;
		mode[45] = 1;
		mode[46] = 1;
		mode[47] = 1;
		mode[48] = 1;

		break;
	}
	case 1:
	default:
	{
		//半径为1
		/*
		1 1 1
		1 1 1
		1 1 1
		*/
		//模板大小
		modeSize = 9;

		//存储结构元素对应点的灰度值
		p = new short[modeSize];
		memset(p, 0, sizeof(short)*modeSize);
		//存储结构元素
		mode = new short[modeSize];
		memset(mode, 0, sizeof(short)*modeSize);
		//结构元素
		mode[0] = 1;
		mode[1] = 1;
		mode[2] = 1;
		mode[3] = 1;
		mode[4] = 1;
		mode[5] = 1;
		mode[6] = 1;
		mode[7] = 1;
		mode[8] = 1;

		break;
	}
	}
	//进行中值滤波
	for (int y = edge; y < pixHeight - edge; y++)
	{
		for (int x = edge; x < pixWidth - edge; x++)
		{

			int num = 0; //标号
			int cur = 0; //处理后，当前点的灰度值
			int temp = 0;
			for (int m = -edge; m <= edge && num < modeSize; m++)
			{
				//获取结构元素对应点的灰度值
				for (int n = -edge; n <= edge && num < modeSize; n++)
				{
					p[num] = fin[(y + m) * pixWidth + x + n] * mode[num];
					num++;
				}
			}
			//把p[k]的内容按下降顺序冒泡法分类
			for (int i = 0; i < modeSize / 2 + 1; i++)
			{
				for (int j = i + 1; j < modeSize; j++)
				{
					if (p[j] < p[i])
					{
						temp = p[i];
						p[i] = p[j];
						p[j] = temp;
					}
				}
			}
			int median = 0;
			median = floor(modeSize / 2); //中间点标号
			cur = p[median]; //中间点的灰度值
			fout[y * pixWidth + x] = cur; //将当前点处理后的灰度值赋给输出图像

		}
	}

	if (mode)
		delete[] mode;
	if (p)
		delete[] p;

	return true;
}


bool WaterShed::GaussFilter(short *fin, short *fout, int modeType)
{
	if (fin == nullptr || fout == nullptr) //如果没有输入图像，则返回false
		return false;
	short *p;  //模板对应图像中的灰度值
	short *mode;   //模板
	p = mode = 0;
	int modeSize = 0;
	int edge = modeType;
	int tb = 0; //模板内的总和

	//默认数据为输入图像
	memcpy(fout, fin, nPixels * sizeof(short));

	switch (modeType)
	{
	//选择模板
	case 2:
	{
		//半径为2
		/*
		1 1 2 1 1
		1 2 4 2 1
		2 4 8 4 2       /52
		1 2 4 2 1
		1 1 2 1 1
		*/
		//结构元素大小
		modeSize = 25;
		tb = 52;
		//存储结构元素对应点的灰度值
		p = new short[modeSize];
		memset(p, 0, sizeof(short)*modeSize);
		//存储结构元素
		mode = new short[modeSize];
		memset(mode, 0, sizeof(short)*modeSize);
		//结构元素
		mode[0] = 1;
		mode[1] = 1;
		mode[2] = 2;
		mode[3] = 1;
		mode[4] = 1;
		mode[5] = 1;
		mode[6] = 2;
		mode[7] = 4;
		mode[8] = 2;
		mode[9] = 1;
		mode[10] = 2;
		mode[11] = 4;
		mode[12] = 8;
		mode[13] = 4;
		mode[14] = 2;
		mode[15] = 1;
		mode[16] = 2;
		mode[17] = 4;
		mode[18] = 2;
		mode[19] = 1;
		mode[20] = 1;
		mode[21] = 1;
		mode[22] = 2;
		mode[23] = 1;
		mode[24] = 1;

		break;
	}
	case 3:
	{
		//半径为3
		/*
		 -4 -6 -12 -14 -12 -6 -4
		 -6  9  18  21  18  9 -6
		-12 18  36  42  36 18 -12
		-14 21  42  49  42 21 -14        /441
		-12 18  36  42  36 18 -12
		 -6  9  18  21  18  9 -6
		 -4 -6 -12 -14 -12 -6 -4
		*/
		//结构元素大小
		modeSize = 49;
		tb = 441;
		//存储结构元素对应点的灰度值
		p = new short[modeSize];
		memset(p, 0, sizeof(short)*modeSize);
		//存储结构元素
		mode = new short[modeSize];
		memset(mode, 0, sizeof(short)*modeSize);
		//结构元素
		mode[0] = -4;
		mode[1] = -6;
		mode[2] = -12;
		mode[3] = -14;
		mode[4] = -12;
		mode[5] = -6;
		mode[6] = -4;
		mode[7] = -6;
		mode[8] = 9;
		mode[9] = 18;
		mode[10] = 21;
		mode[11] = 18;
		mode[12] = 9;
		mode[13] = -6;
		mode[14] = -12;
		mode[15] = 18;
		mode[16] = 36;
		mode[17] = 42;
		mode[18] = 36;
		mode[19] = 18;
		mode[20] = -12;
		mode[21] = -14;
		mode[22] = 21;
		mode[23] = 42;
		mode[24] = 49;
		mode[25] = 42;
		mode[26] = 21;
		mode[27] = -14;
		mode[28] = -12;
		mode[29] = 18;
		mode[30] = 36;
		mode[31] = 42;
		mode[32] = 36;
		mode[33] = 18;
		mode[34] = -12;
		mode[35] = -6;
		mode[36] = 9;
		mode[37] = 18;
		mode[38] = 21;
		mode[39] = 18;
		mode[40] = 9;
		mode[41] = -6;
		mode[42] = -4;
		mode[43] = -6;
		mode[44] = -12;
		mode[45] = -14;
		mode[46] = -12;
		mode[47] = -6;
		mode[48] = -4;

		break;
	}
	case 1:
	default:
	{
		//半径为1
		/*
		1 2 1
		2 4 2       /16
		1 2 1
		*/
		//模板大小
		modeSize = 9;
		tb = 16;
		//存储结构元素对应点的灰度值
		p = new short[modeSize];
		memset(p, 0, sizeof(short)*modeSize);
		//存储结构元素
		mode = new short[modeSize];
		memset(mode, 0, sizeof(short)*modeSize);
		//结构元素
		mode[0] = 1;
		mode[1] = 2;
		mode[2] = 1;
		mode[3] = 2;
		mode[4] = 4;
		mode[5] = 2;
		mode[6] = 1;
		mode[7] = 2;
		mode[8] = 1;

		break;
	}
	}
	//进行高斯滤波
	for (int y = edge; y < pixHeight - edge; y++)
	{
		for (int x = edge; x < pixWidth - edge; x++)
		{

			int num = 0; //标号
			int total = 0; //总和
			int cur = 0; //处理后，当前点的灰度值
			for (int m = -edge; m <= edge && num < modeSize; m++)
			{
				//获取结构元素对应点的灰度值
				for (int n = -edge; n <= edge && num < modeSize; n++)
				{
					p[num] = fin[(y + m) * pixWidth + x + n] * mode[num];
					num++;
				}
			}
			for (int k = 0; k < modeSize; k++)
			{
				//求结构元素内的灰度值总和
				total += p[k];
			}
			cur = total / tb;
			fout[y * pixWidth + x] = cur; //将当前点处理后的灰度值赋给输出图像

		}
	}

	if (mode)
		delete[] mode;
	if (p)
		delete[] p;

	return true;
}


/********************************************************************
    函数：DoImdilate  -  图像灰度膨胀
    参数：fin：输入图像
          fout：输出图像
          radius：结构元素半径
    返回：如正确执行，则返回true，否则返回false
********************************************************************/

bool WaterShed::DoImdilate(short *fin, short *fout, int radius)
{
	//执行膨胀
	if (fin == nullptr) //如果没有输入图像，则返回false
		return false;
	short *inpix;
	inpix = nullptr;
	inpix = new short[nPixels];
	memset(inpix, 0, sizeof(short)*nPixels);
	memcpy(inpix, fin, nPixels * sizeof(short));

	short *outpix;
	outpix = nullptr;
	outpix = new short[nPixels];
	memset(outpix, 0, sizeof(short)*nPixels);
	memcpy(outpix, fin, nPixels * sizeof(short));
	if (radius > 9)
	{
		//如果半径大于等于10，则使用八边形模板分解的线性结构元素 反复迭代
		for (int i = 1; i <= radius / 10; i++)
		{
			if (!Imdilate(inpix, outpix, 1))
				return false;
			memcpy(inpix, outpix, nPixels * sizeof(short));

			if (!Imdilate(inpix, outpix, 2))
				return false;
			memcpy(inpix, outpix, nPixels * sizeof(short));


			if (!Imdilate(inpix, outpix, 3))
				return false;
			memcpy(inpix, outpix, nPixels * sizeof(short));

			if (!Imdilate(inpix, outpix, 4))
				return false;
			memcpy(inpix, outpix, nPixels * sizeof(short));

		}
		if (radius % 10 > 0)
		{
			for (int i = 1; i <= radius % 10; i++)
			{
				if (!Imdilate(inpix, outpix, 5))
					return false;
				memcpy(inpix, outpix, nPixels * sizeof(short));

				if (!Imdilate(inpix, outpix, 6))
					return false;
				memcpy(inpix, outpix, nPixels * sizeof(short));
			}
		}
	}
	else
	{
		if (radius == 1 || radius == 2)
		{
			//如果半径为1或2 ，则只要使用基本菱形就可以
			for (int i = 1; i <= radius; i++)
			{
				if (!Imdilate(inpix, outpix, 9))
					return false;
				memcpy(inpix, outpix, nPixels * sizeof(short));
			}
		}
		else
		{
			//如果半径大于等于3，则交替使3X3菱形和正方形进行膨胀，将3X3菱形和正方形分解为线性结构元素
			for (int i = 1; i < radius; i++)
			{
				if (i % 2 == 1) //奇数
				{
					if (!Imdilate(inpix, outpix, 9))
						return false;
					memcpy(inpix, outpix, nPixels * sizeof(short));

					//      if(!Imdilate(inpix, outpix,8))
					//         return false;
					//      memcpy(inpix,outpix,nPixels*sizeof(short));
				}
				if (i % 2 == 0) //偶数
				{
					if (!Imdilate(inpix, outpix, 5))
						return false;
					memcpy(inpix, outpix, nPixels * sizeof(short));

					if (!Imdilate(inpix, outpix, 6))
						return false;
					memcpy(inpix, outpix, nPixels * sizeof(short));
				}

			}
		}
	}
	memcpy(fout, outpix, nPixels * sizeof(short)); //输出图像

	if (inpix)
		delete[] inpix;
	if (outpix)
		delete[] outpix;

	return true;
}



bool WaterShed::DoImerode(short *fin, short *fout, int radius)
{
	//执行腐蚀
	if (fin == nullptr) //如果没有输入图像，则返回false
		return false;
	short *inpix;
	inpix = nullptr;
	inpix = new short[nPixels];
	memset(inpix, 0, sizeof(short)*nPixels);
	memcpy(inpix, fin, nPixels * sizeof(short));

	short *outpix;
	outpix = nullptr;
	outpix = new short[nPixels];
	memset(outpix, 0, sizeof(short)*nPixels);
	memcpy(outpix, fin, nPixels * sizeof(short));
	if (radius > 9)
	{
		//如果半径大于等于10，则使用八边形模板分解成线性结构反复迭代
		for (int i = 1; i <= radius / 10; i++)
		{
			if (!Imerode(inpix, outpix, 1))       //9*1的垂直线性结构
				return false;
			memcpy(inpix, outpix, nPixels * sizeof(short));

			if (!Imerode(inpix, outpix, 2))       // 7*1的左上到右下结构
				return false;
			memcpy(inpix, outpix, nPixels * sizeof(short));


			if (!Imerode(inpix, outpix, 3))       //9*1的水平线性结构
				return false;
			memcpy(inpix, outpix, nPixels * sizeof(short));

			if (!Imerode(inpix, outpix, 4))       //7*1的右上到左下结构
				return false;
			memcpy(inpix, outpix, nPixels * sizeof(short));

		}
		if (radius % 10 > 0)
		{
			for (int i = 1; i <= radius % 10; i++)
			{
				if (!Imerode(inpix, outpix, 5))      //3*1的水平线性结构
					return false;
				memcpy(inpix, outpix, nPixels * sizeof(short));

				if (!Imerode(inpix, outpix, 6))      //3*1的垂直线性结构
					return false;
				memcpy(inpix, outpix, nPixels * sizeof(short));
			}
		}
	}
	else
	{
		if (radius == 1 || radius == 2)
		{
			//如果半径为1或2 ，则只要使用基本菱形就可以
			for (int i = 1; i <= radius; i++)
			{
				if (!Imerode(inpix, outpix, 9))
					return false;
				memcpy(inpix, outpix, nPixels * sizeof(short));
			}
		}
		else
		{
			//如果半径大于等于3小于10，则交替使3X3菱形和正方形进行膨胀，将3X3菱形和正方形分解为线性结构元素
			for (int i = 1; i < radius; i++)
			{
				if (i % 2 == 1) //奇数
				{
					if (!Imerode(inpix, outpix, 9))              //3*1右上到左下
						return false;
					memcpy(inpix, outpix, nPixels * sizeof(short));

					//     if(!Imerode(inpix, outpix,8))               //3*1左上到右下
					//        return false;
					//     memcpy(inpix,outpix,nPixels*sizeof(short));
				}
				if (i % 2 == 0) //偶数
				{
					if (!Imerode(inpix, outpix, 5))             //3*1水平线性结构
						return false;
					memcpy(inpix, outpix, nPixels * sizeof(short));

					if (!Imerode(inpix, outpix, 6))            //3*1垂直线性结构
						return false;
					memcpy(inpix, outpix, nPixels * sizeof(short));
				}

			}
		}
	}
	memcpy(fout, outpix, nPixels * sizeof(short)); //输出图像

	if (inpix)
		delete[] inpix;
	if (outpix)
		delete[] outpix;

	return true;
}


bool WaterShed::Imerode(short *fin, short *fout, int setype)
{
	//灰度腐蚀
	if (fin == nullptr) //如果没有输入图像，则返回false
		return false;

	int p, pl, pr, pu, pd; //当前点的八邻阈值
	p = pl = pr = pu = pd = 0;
	//    int p=0;
	//默认数据为输入图像
	memcpy(fout, fin, nPixels * sizeof(short));

	short *inpix;
	inpix = nullptr;
	inpix = new short[nPixels];
	memset(inpix, 0, sizeof(short)*nPixels);
	memcpy(inpix, fin, nPixels * sizeof(short));

	short *outpix;
	outpix = nullptr;
	outpix = new short[nPixels];
	memset(outpix, 0, sizeof(short)*nPixels);
	memcpy(outpix, fin, nPixels * sizeof(short));

	int minP = 255;
	switch (setype)
	{
	//选择结构元素
	case 1:
	{
		//9*1的垂直结构元素
		for (int y = 0; y < pixHeight; y++)
		{
			for (int x = 0; x < pixWidth; x++)
			{
				minP = inpix[y * pixWidth + x];
				for (int i = 1; i < 5; i++)
				{
					if ((y + i) <= (pixHeight - 1))
					{
						p = inpix[(y + i) * pixWidth + x];
						minP = min(minP, p) ;
					}
					if ((y - i) >= 0)
					{
						p = inpix[(y - i) * pixWidth + x];
						minP = min(minP, p) ;
					}
				}
				outpix[y * pixWidth + x] = minP; //将结构元素内的极小值值赋给当前点

			}
		}
		memcpy(fout, outpix, nPixels * sizeof(short));
		break;
	}

	case 2:
	{
		//7*1左上到右下 线性结构元素
		for (int y = 0; y < pixHeight; y++)
		{
			for (int x = 0; x < pixWidth; x++)
			{
				minP = inpix[y * pixWidth + x];
				for (int i = 1; i < 4; i++)
				{
					if (((x + i) <= (pixWidth - 1)) && ((y + i) <= (pixHeight - 1)))
					{
						p = inpix[(y + i) * pixWidth + (x + i)];
						minP = min(minP, p) ;
					}
					if (((x - i) >= 0) && ((y - i) >= 0))
					{
						p = inpix[(y - i) * pixWidth + (x - i)];
						minP = min(minP, p) ;
					}
				}
				outpix[y * pixWidth + x] = minP; //将结构元素内的极小值值赋给当前点

			}

		}
		memcpy(fout, outpix, nPixels * sizeof(short));

		break;
	}
	case 3:
	{
		//9*1水平 线性 结构元素
		for (int y = 0; y < pixHeight; y++)
		{
			for (int x = 0; x < pixWidth; x++)
			{
				minP = inpix[y * pixWidth + x];
				for (int i = 1; i < 5; i++)
				{
					if ((x + i) <= (pixWidth - 1))
					{
						p = inpix[y * pixWidth + (x + i)];
						minP = min(minP, p) ;
					}
					if ((x - i) >= 0)
					{
						p = inpix[y * pixWidth + (x - i)];
						minP = min(minP, p) ;
					}
				}
				outpix[y * pixWidth + x] = minP; //将结构元素内的极小值值赋给当前点

			}
		}
		memcpy(fout, outpix, nPixels * sizeof(short));
		break;
	}

	case 4:
	{
		//7*1的右上到左下 线性 结构元素
		for (int y = 0; y < pixHeight; y++)
		{
			for (int x = 0; x < pixWidth; x++)
			{
				minP = inpix[y * pixWidth + x];
				for (int i = 1; i < 4; i++)
				{
					if (((x + i) <= (pixWidth - 1)) && ((y - i) >= 0))
					{
						p = inpix[(y - i) * pixWidth + (x + i)];
						minP = min(minP, p) ;
					}
					if (((x - i) >= 0) && ((y + i) <= (pixHeight - 1)))
					{
						p = inpix[(y + i) * pixWidth + (x - i)];
						minP = min(minP, p) ;
					}
				}
				outpix[y * pixWidth + x] = minP; //将结构元素内的极小值值赋给当前点

			}

		}

		memcpy(fout, outpix, nPixels * sizeof(short));
		break;
	}
	case 5:
	{
		//3*1的水平线性结构元素
		for (int y = 0; y < pixHeight; y++)
		{
			for (int x = 0; x < pixWidth; x++)
			{
				minP = inpix[y * pixWidth + x];

				if ((x - 1) >= 0)
				{
					p = inpix[y * pixWidth + (x - 1)];
					minP = min(minP, p) ;
				}
				if ((x + 1) <= (pixWidth - 1))
				{
					p = inpix[y * pixWidth + (x + 1)];
					minP = min(minP, p) ;
				}

				outpix[y * pixWidth + x] = minP; //将结构元素内的极小值值赋给当前点

			}
		}
		memcpy(fout, outpix, nPixels * sizeof(short));
		break;
	}
	case 6:
	{
		//3*1的垂直 线性结构元素
		for (int y = 0; y < pixHeight; y++)
		{
			for (int x = 0; x < pixWidth; x++)
			{
				minP = inpix[y * pixWidth + x];

				if ((y - 1) >= 0)
				{
					p = inpix[(y - 1) * pixWidth + x];
					minP = min(minP, p) ;
				}
				if ((y + 1) <= (pixHeight - 1))
				{
					p = inpix[(y + 1) * pixWidth + x];
					minP = min(minP, p) ;
				}

				outpix[y * pixWidth + x] = minP; //将结构元素内的极小值值赋给当前点

			}
		}
		memcpy(fout, outpix, nPixels * sizeof(short));
		break;
	}
	case 7:
	{
		//3*1的右上到左下 线性结构元素
		for (int y = 0; y < pixHeight; y++)
		{
			for (int x = 0; x < pixWidth; x++)
			{
				minP = inpix[y * pixWidth + x];

				if ((x + 1) <= (pixWidth - 1) && (y - 1) >= 0)
				{
					p = inpix[(y - 1) * pixWidth + (x + 1)];
					minP = min(minP, p) ;
				}
				if ((x - 1 >= 0) && (y + 1 <= pixHeight - 1))
				{
					p = inpix[(y + 1) * pixWidth + (x - 1)];
					minP = min(minP, p) ;
				}
				outpix[y * pixWidth + x] = minP; //将结构元素内的极小值值赋给当前点

			}
		}
		memcpy(fout, outpix, nPixels * sizeof(short));
		break;
	}
	case 8:
	{
		//3*1的左上到右下 线性结构元素
		for (int y = 0; y < pixHeight; y++)
		{
			for (int x = 0; x < pixWidth; x++)
			{
				int minP = inpix[y * pixWidth + x];
				{
					if ((x - 1) >= 0 && (y - 1) >= 0)
					{
						p = inpix[(y - 1) * pixWidth + (x - 1)];
						minP = min(minP, p) ;
					}
					if ((x + 1 <= pixWidth - 1) && (y + 1 <= pixHeight - 1))
					{
						p = inpix[(y + 1) * pixWidth + (x + 1)];
						minP = min(minP, p) ;
					}
				}
				outpix[y * pixWidth + x] = minP; //将结构元素内的极小值值赋给当前点

			}
		}
		memcpy(fout, outpix, nPixels * sizeof(short));
		break;
	}

	case 9:
	default:
	{
		//   菱形结构元素
		/*
		0 1 0
		1 1 1
		0 1 0
		*/

		for (int y = 0; y < pixHeight; y++)
		{
			for (int x = 0; x < pixWidth; x++)
			{
				minP = inpix[y * pixWidth + x] ; //当前点

				if (x != 0)
				{
					pl = inpix[y * pixWidth + x - 1]; //左
					minP = min(minP, pl) ;
				}
				if (x != (pixWidth - 1))
				{
					pr = inpix[y * pixWidth + x + 1]; //右
					minP = min(minP, pr) ;
				}
				if (y != 0)
				{
					pu = inpix[(y - 1) * pixWidth + x]; //上
					minP = min(minP, pu) ;
				}
				if (y != (pixHeight - 1))
				{
					pd = inpix[(y + 1) * pixWidth + x]; //下
					minP = min(minP, pd) ;
				}
				outpix[y * pixWidth + x] = minP; //将结构元素内的极小值值赋给当前点

			}
		}
		memcpy(fout, outpix, nPixels * sizeof(short));
		break;
	}
	}

	if (inpix)
		delete[] inpix;
	if (outpix)
		delete[] outpix;

	return true;
}



bool WaterShed::Imdilate(short *fin, short *fout, int setype)
{
	//灰度膨胀
	if (fin == nullptr) //如果没有输入图像，则返回false
		return false;

	int p, pl, pr, pu, pd; //当前点的八邻阈值
	p = pl = pr = pu = pd = 0;
	//    int p=0;
	//默认数据为输入图像
	memcpy(fout, fin, nPixels * sizeof(short));

	short *inpix;
	inpix = nullptr;
	inpix = new short[nPixels];
	memset(inpix, 0, sizeof(short)*nPixels);
	memcpy(inpix, fin, nPixels * sizeof(short));

	short *outpix;
	outpix = nullptr;
	outpix = new short[nPixels];
	memset(outpix, 0, sizeof(short)*nPixels);
	memcpy(outpix, fin, nPixels * sizeof(short));

	int maxP = 0;
	switch (setype)
	{
	//选择结构元素

	case 1:
	{
		//9*1垂直 结构元素
		for (int y = 0; y < pixHeight; y++)
		{
			for (int x = 0; x < pixWidth; x++)
			{
				maxP = inpix[y * pixWidth + x];
				for (int i = 1; i < 5; i++)
				{
					if ((y + i) <= (pixHeight - 1))
					{
						p = inpix[(y + i) * pixWidth + x];
						maxP = max(maxP, p) ;
					}
					if ((y - i) >= 0)
					{
						p = inpix[(y - i) * pixWidth + x];
						maxP = max(maxP, p) ;
					}
				}
				outpix[y * pixWidth + x] = maxP; //将结构元素内的极小值值赋给当前点

			}
		}
		memcpy(fout, outpix, nPixels * sizeof(short));
		break;
	}

	case 2:
	{
		//7*1左上到右下 1 1 1 结构元素 1 1 1 结构元素
		for (int y = 0; y < pixHeight; y++)
		{
			for (int x = 0; x < pixWidth; x++)
			{
				maxP = inpix[y * pixWidth + x];
				for (int i = 1; i < 4; i++)
				{
					if (((x + i) <= (pixWidth - 1)) && ((y + i) <= (pixHeight - 1)))
					{
						p = inpix[(y + i) * pixWidth + (x + i)];
						maxP = max(maxP, p) ;
					}
					if (((x - i) >= 0) && ((y - i) >= 0))
					{
						p = inpix[(y - i) * pixWidth + (x - i)];
						maxP = max(maxP, p) ;
					}
				}
				outpix[y * pixWidth + x] = maxP; //将结构元素内的极小值值赋给当前点

			}

		}
		memcpy(fout, outpix, nPixels * sizeof(short));

		break;
	}
	case 3:
	{
		//9*1水平 1 1 1 结构元素
		for (int y = 0; y < pixHeight; y++)
		{
			for (int x = 0; x < pixWidth; x++)
			{
				maxP = inpix[y * pixWidth + x];
				for (int i = 1; i < 5; i++)
				{
					if ((x + i) <= (pixWidth - 1))
					{
						p = inpix[y * pixWidth + (x + i)];
						maxP = max(maxP, p) ;
					}
					if ((x - i) >= 0)
					{
						p = inpix[y * pixWidth + (x - i)];
						maxP = max(maxP, p) ;
					}
				}
				outpix[y * pixWidth + x] = maxP; //将结构元素内的极小值值赋给当前点

			}
		}
		memcpy(fout, outpix, nPixels * sizeof(short));
		break;
	}
	case 4:
	{
		//7*1右上到左下 1 1 1 结构元素
		for (int y = 0; y < pixHeight; y++)
		{
			for (int x = 0; x < pixWidth; x++)
			{
				maxP = inpix[y * pixWidth + x];
				for (int i = 1; i < 4; i++)
				{
					if (((x + i) <= (pixWidth - 1)) && ((y - i) >= 0))
					{
						p = inpix[(y - i) * pixWidth + (x + i)];
						maxP = max(maxP, p) ;
					}
					if (((x - i) >= 0) && ((y + i) <= (pixHeight - 1)))
					{
						p = inpix[(y + i) * pixWidth + (x - i)];
						maxP = max(maxP, p) ;
					}
				}
				outpix[y * pixWidth + x] = maxP; //将结构元素内的极小值值赋给当前点

			}

		}

		memcpy(fout, outpix, nPixels * sizeof(short));
		break;
	}
	case 5:
	{
		//3*1水平 1 1 1 结构元素
		for (int y = 0; y < pixHeight; y++)
		{
			for (int x = 0; x < pixWidth; x++)
			{
				maxP = inpix[y * pixWidth + x];
				{
					if ((x - 1) >= 0)
					{
						p = inpix[y * pixWidth + (x - 1)];
						maxP = max(maxP, p) ;
					}
					if ((x + 1) <= (pixWidth - 1))
					{
						p = inpix[y * pixWidth + (x + 1)];
						maxP = min(maxP, p) ;
					}
				}
				outpix[y * pixWidth + x] = maxP; //将结构元素内的极小值值赋给当前点

			}
		}
		memcpy(fout, outpix, nPixels * sizeof(short));
		break;
	}
	case 6:
	{
		//3*1垂直 1 1 1 结构元素
		for (int y = 0; y < pixHeight; y++)
		{
			for (int x = 0; x < pixWidth; x++)
			{
				int maxP = inpix[y * pixWidth + x];
				{
					if ((y - 1) >= 0)
					{
						p = inpix[(y - 1) * pixWidth + x];
						maxP = max(maxP, p) ;
					}
					if ((y + 1) <= (pixHeight - 1))
					{
						p = inpix[(y + 1) * pixWidth + x];
						maxP = max(maxP, p) ;
					}
				}
				outpix[y * pixWidth + x] = maxP; //将结构元素内的极小值值赋给当前点

			}
		}
		memcpy(fout, outpix, nPixels * sizeof(short));
		break;
	}
	case 7:
	{
		//3*1右上到左下 1 1 1 结构元素
		for (int y = 0; y < pixHeight; y++)
		{
			for (int x = 0; x < pixWidth; x++)
			{
				maxP = inpix[y * pixWidth + x];
				{
					if ((x + 1) <= (pixWidth - 1) && (y - 1) >= 0)
					{
						p = inpix[(y - 1) * pixWidth + (x - 1)];
						maxP = max(maxP, p) ;
					}
					if ((x - 1 >= 0) && (y + 1 <= pixHeight - 1))
					{
						p = inpix[(y + 1) * pixWidth + (x - 1)];
						maxP = max(maxP, p) ;
					}
				}
				outpix[y * pixWidth + x] = maxP; //将结构元素内的极小值值赋给当前点

			}
		}
		memcpy(fout, outpix, nPixels * sizeof(short));
		break;
	}
	case 8:
	{
		//3*1左上到右下 1 1 1 结构元素
		for (int y = 0; y < pixHeight; y++)
		{
			for (int x = 0; x < pixWidth; x++)
			{
				maxP = inpix[y * pixWidth + x];
				{
					if ((x - 1) >= 0 && (y - 1) >= 0)
					{
						p = inpix[(y - 1) * pixWidth + (x - 1)];
						maxP = max(maxP, p) ;
					}
					if ((x + 1 <= pixWidth - 1) && (y + 1 <= pixHeight - 1))
					{
						p = inpix[(y + 1) * pixWidth + (x + 1)];
						maxP = max(maxP, p) ;
					}
				}
				outpix[y * pixWidth + x] = maxP; //将结构元素内的极小值值赋给当前点

			}
		}
		memcpy(fout, outpix, nPixels * sizeof(short));
		break;
	}

	case 9:
	default:
	{
		//   菱形结构元素
		/*
		0 1 0
		1 1 1
		0 1 0
		*/

		for (int y = 0; y < pixHeight; y++)
		{
			for (int x = 0; x < pixWidth; x++)
			{
				maxP = inpix[y * pixWidth + x] ; //当前点

				if (x != 0)
				{
					pl = inpix[y * pixWidth + x - 1]; //左
					maxP = max(maxP, pl) ;
				}
				if (x != (pixWidth - 1))
				{
					pr = inpix[y * pixWidth + x + 1]; //右
					maxP = max(maxP, pr) ;
				}
				if (y != 0)
				{
					pu = inpix[(y - 1) * pixWidth + x]; //上
					maxP = max(maxP, pu) ;
				}
				if (y != (pixHeight - 1))
				{
					pd = inpix[(y + 1) * pixWidth + x]; //下
					maxP = max(maxP, pd) ;
				}
				outpix[y * pixWidth + x] = maxP; //将结构元素内的极大值值赋给当前点

			}
		}
		memcpy(fout, outpix, nPixels * sizeof(short));
		break;
	}
	}

	if (inpix)
		delete[] inpix;
	if (outpix)
		delete[] outpix;

	return true;
}

/********************************************************************
    函数：Imadd  -  图像加常数h    fout = fin +h
    参数：fin：输入图像
          h：常数
          fout：输出图像
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::Imadd(short *fin, int h, short *fout)
{
	//图像像素值加上常h
	if (fin == nullptr || fout == nullptr)
		return false;
	for (int y = 0; y < pixHeight; y++)
	{
		for (int x = 0; x < pixWidth; x++)
		{
			if (fin[y * pixWidth + x] + h > 255)
				fout[y * pixWidth + x] = 255;
			fout[y * pixWidth + x] = fin[y * pixWidth + x] + h;
		}
	}
	return true;
}
/********************************************************************
    函数：Imsubtract  -  图像减常数h，fout = fin -h
    参数：fin：输入图像
      h：常数
      fout：输出图像
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::Imsubtract(short *fin, int h, short *fout)
{
	//图像像素值减去常h
	if (fin == nullptr || fout == nullptr)
		return false;

	for (int y = 0; y < pixHeight; y++)
	{
		for (int x = 0; x < pixWidth; x++)
		{
			if (fin[y * pixWidth + x] - h < 0)
				fout[y * pixWidth + x] = 0;
			else
				fout[y * pixWidth + x] = fin[y * pixWidth + x] - h;
		}
	}
	return true;
}
/********************************************************************
    函数：Imsubtract -  图像相减，fout = fin1 - fin2
    参数：fin1：输入图像1
          Fin2：输入图像2
          fout：输出图像
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::Imsubtract(short *fin1, short *fin2, short *fout)
{
	//两幅图像相减
	if (fin1 == nullptr || fin2 == nullptr || fout == nullptr) //如果没有输入图像，则返回false
		return false;

	for (int y = 0; y < pixHeight; y++)
	{
		for (int x = 0; x < pixWidth; x++)
		{
			if ((fin1[y * pixWidth + x] - fin2[y * pixWidth + x]) < 0)
				fout[y * pixWidth + x] = 0;
			else
				fout[y * pixWidth + x] = fin1[y * pixWidth + x] - fin2[y * pixWidth + x];
		}
	}
	return true;
}
/********************************************************************
    函数：Imor  -  图像逐点或运算，fout = fin1| fin2*
    参数：fin1：输入图像1
          Fin2：输入图像2
          fout：输出图像
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::Imor(short *fin1 , short *fin2, short *fout)
{
	//图像逐点或运算
	if (fin1 == nullptr || fin2 == nullptr || fout == nullptr) //如果没有输入图像，则返回false
		return false;
	for (int y = 0; y < pixHeight; y++)
	{
		for (int x = 0; x < pixWidth; x++)
		{
			if ((fin1[y * pixWidth + x] == 0 || fin2[y * pixWidth + x]) == 0)
				fout[y * pixWidth + x] = 0;
			else
				fout[y * pixWidth + x] = 255;
		}
	}
	return true;
}
/********************************************************************
    函数：Immin  -  图像逐点求最小值，fout = min(fin1,fin2)
    参数：fin1：输入图像1
          Fin2：输入图像2
          fout：输出图像
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::Immin(short *fin1 , short *fin2, short *fout)
{
	//图像逐点求最小值
	if (fin1 == nullptr || fin2 == nullptr || fout == nullptr) //如果没有输入图像，则返回false
		return false;

	for (int y = 0; y < pixHeight; y++)
	{
		for (int x = 0; x < pixWidth; x++)
		{
			fout[y * pixWidth + x] = min(fin1[y * pixWidth + x], fin2[y * pixWidth + x]);
		}
	}

	return true;
}
/********************************************************************
    函数：Imcomplement  -  图像求补，fout = 255- fin
    参数：fin：输入图像
          fout：输出图像
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::Imcomplement(short *fin , short *fout)
{
	//图像求补
	if (fin == nullptr || fout == nullptr)
		return false;
	for (int y = 0; y < pixHeight; y++)
	{
		for (int x = 0; x < pixWidth; x++)
		{
			fout[y * pixWidth + x] = 255 - fin[y * pixWidth + x];
		}
	}
	return true;
}
/********************************************************************
    函数：Imextendedmin  -  极小值扩展
    参数：fin：输入图像
          h：扩展深度
          bw：输出的二值图像，1：极小值点(内标记)，0：非极小值点
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::Imextendedmin (short *fin, int h, bool *bw)
{
	//扩散极小值
	if (fin == nullptr || bw == nullptr)
		return false;

	short *I, *J;
	I = new short[nPixels];
	J = new short[nPixels];
	memset(I, 0, sizeof(short)*nPixels);
	memset(J, 0, sizeof(short)*nPixels);

	memcpy(I, fin, nPixels * sizeof(short));
	//输入图像求反I  掩模图像
	if (!Imcomplement(I, I))
		return false;
	//J=I-h   //求反后的图像I减h，输出J   J<I
	if (!Imsubtract(I, h, J))
		return false;
	//重构
	if (!Imreconstruct(J, I))
		return false;
	//区域极大值
	if (!Imregionalmax(J, bw))
		return false;

	if (I != nullptr)
	{
		delete [] I;
		I = nullptr;
	}
	if (J != nullptr)
	{
		delete [] J;
		J = nullptr;
	}

	return true;
}

bool WaterShed::Imreconstruct (short *marker,  short *mask)
{
	//重构
	if (marker == nullptr || mask == nullptr)
		return false;
	// PixnWrite(J,"J.txt");
	int maxp;
	int p, q;
	int y, x, Xp, Yp;

	for (int k = 0; k < nPixels; k++)
	{
		if (marker[k] > mask[k])
		{
			return false;
		}
	}

	std::queue<int> Fifo;  //队列

	int edge = 0;

	for (y = edge; y < pixHeight - edge; y++)
	{
		for (x = edge; x < pixWidth - edge; x++)
		{
			p = y * pixWidth + x;
			maxp = marker[p];

			// 得到当前点的3*3邻域信息。去除中心点(自身)
			if (!GetNgh3x3MinusCenterPix(marker, x, y))
				return false;

			for (int j = 0; j < nghpixs.Nhnlength; j++)
				// 获取标记图像中3X3领域内的灰度极大值
				maxp = max(maxp, nghpixs.Nhn_g[j]);

			// 将标记图像当前点的领域的灰度极大值和掩模图像的当前点的之前的极小值赋给标记图像
			marker[p] = min(maxp, mask[p]);
		}
	}

	for (y = pixHeight - edge - 1; y > edge - 1; y--)
	{
		for (x = pixWidth - edge - 1; x > edge - 1; x--)
		{
			p = y * pixWidth + x;
			maxp = marker[p];
			if (!GetNgh3x3MinusCenterPix(marker, x, y))
				return false;

			for (int j = 0; j < nghpixs.Nhnlength; j++)
				maxp = max(maxp, nghpixs.Nhn_g[j]);

			marker[p] = min(maxp, mask[p]);

			for (int k = 0; k < nghpixs.Nhnlength; k++)
			{
				if (nghpixs.Nhn_g[k] < marker[p])
				{
					q = nghpixs.Nhn_ax[k]; //得到绝对偏移量q(q为绝对坐标值)
					if (marker[q] < marker[p] && marker[q] < mask[q])
					{
						Fifo.push(p);
						break;
					}
				}
			}
		}
	}

	while (Fifo.size() > 0)
	{
		p = Fifo.front(); //p为绝对坐标值  p=y*pixWidth+x;
		Fifo.pop();
		//绝对坐标p=y*pixWidth+x;转相对坐标[x,y]
		Yp = (int)p / pixWidth;
		Xp = p - Yp * pixWidth;
		if (!GetNgh3x3MinusCenterPix(marker, Xp, Yp))
			return false;
		for (int k = 0; k < nghpixs.Nhnlength; k++)
		{
			q = nghpixs.Nhn_ax[k]; //得到绝对偏移量q(q为绝对坐标值)
			if (marker[q] < marker[p] && mask[q] != marker[q])
			{
				marker[q] = min(marker[p], mask[q]);
				Fifo.push(q);
			}
		}
	}
	return true;
}

/********************************************************************
    函数：GetNgh3x3MinusCenterPix  -  获得3X3邻阈的像素信息
    参数：pix：输入图像
          x：当前像元的X坐标
          y：当前像元的Y坐标
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
template<typename _T>
bool WaterShed::GetNgh3x3MinusCenterPix(_T  pix[], int x, int y)
{
	/* getNgh3x3Pix */
	if (!pix)
		return false;
	//获取邻域信息
	int im, ip, jm, jp; //邻域坐标
	int j = 0;
	int id = y * pixWidth + x; //当前像素点的编号

	im = x - 1;
	ip = x + 1; //当前点的邻域坐标
	jm = y - 1;
	jp = y + 1;

	if (Is_inbounds_neighbor(im, jm))
	{
		//左上
		id = jm * pixWidth + im; //当前像素点的编号
		nghpixs.Nhn_ax[j] = id; //存储像素点编号
		nghpixs.Nhn_g[j++] = pix[id]; //存储当前像素点像素值
	}
	if (Is_inbounds_neighbor(x, jm))
	{
		//上
		id = jm * pixWidth + x; //当前像素点的编号
		nghpixs.Nhn_ax[j] = id; //存储像素点编号
		nghpixs.Nhn_g[j++] = pix[id]; //存储当前像素点像素值
	}
	if (Is_inbounds_neighbor(ip, jm))
	{
		//右上
		id = jm * pixWidth + ip; //当前像素点的编号
		nghpixs.Nhn_ax[j] = id; //存储像素点编号
		nghpixs.Nhn_g[j++] = pix[id]; //存储当前像素点像素值
	}
	if (Is_inbounds_neighbor(im, y))
	{
		//左
		id = y * pixWidth + im; //当前像素点的编号
		nghpixs.Nhn_ax[j] = id; //存储像素点编号
		nghpixs.Nhn_g[j++] = pix[id]; //存储当前像素点像素值
	}
	if (Is_inbounds_neighbor(ip, y))
	{
		//右
		id = y * pixWidth + ip; //当前像素点的编号
		nghpixs.Nhn_ax[j] = id; //存储像素点编号
		nghpixs.Nhn_g[j++] = pix[id]; //存储当前像素点像素值
	}
	if (Is_inbounds_neighbor(im, jp))
	{
		//左下
		id = jp * pixWidth + im; //当前像素点的编号
		nghpixs.Nhn_ax[j] = id; //存储像素点编号
		nghpixs.Nhn_g[j++] = pix[id]; //存储当前像素点像素值
	}
	if (Is_inbounds_neighbor(x, jp))
	{
		//下
		id = jp * pixWidth + x; //当前像素点的编号
		nghpixs.Nhn_ax[j] = id; //存储像素点编号
		nghpixs.Nhn_g[j++] = pix[id]; //存储当前像素点像素值
	}
	if (Is_inbounds_neighbor(ip, jp))
	{
		//右下
		id = jp * pixWidth + ip; //当前像素点的编号
		nghpixs.Nhn_ax[j] = id; //存储像素点编号
		nghpixs.Nhn_g[j++] = pix[id]; //存储当前像素点像素值
	}
	nghpixs.Nhnlength = j;  //领域个数
	return (true);

}

/********************************************************************
    函数：Is_inbounds_neighbor  -  判断邻阈是否超边界
    参数：x：当前像元的X坐标
          y：当前像元的Y坐标
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::Is_inbounds_neighbor(int x , int y)
{
	//判断领域是否出界
	bool flag = false;
	if (x >= 0 && x < pixWidth && y >= 0 && y < pixHeight)
		flag = true;
	return flag;
}

/********************************************************************
    函数：Imregionalmax  -  区域极大值
    参数：marker：标记图像
          BW：内标记图像
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::Imregionalmax (short *fin , bool *BW)
{
	//区域极大值
	if (!fin || !BW)
		return false;

	int p, q;
	int pp, qq;
	bool found;
	int edge = 0;
	int val;
	int Ypp, Xpp;

	std::stack<int> mstack;

	for (p = 0; p < nPixels; p++)
		BW[p] = 1;


	for (int y = edge; y < pixHeight - edge; y++)
	{
		for (int x = edge; x < pixWidth - edge; x++)
		{
			p = y * pixWidth + x;
			if (BW[p] != 0)
			{

				found = false;

				if (!GetNgh3x3MinusCenterPix(fin, x, y))
					return false;

				for (int k = 0; k < nghpixs.Nhnlength; k++)
				{
					q = nghpixs.Nhn_ax[k];
					if (fin[q] > fin[p])
					{
						found = true;
						break;
					}
				}

				if (found)
				{

					val = fin[p];
					mstack.push(p);
					BW[p] = 0;
					while (mstack.size() > 0)
					{
						pp = mstack.top();
						mstack.pop();
						Ypp = (int)pp / pixWidth;
						Xpp = (int)pp % pixWidth;
						if (!GetNgh3x3MinusCenterPix(fin, Xpp, Ypp))
							return false;
						for (int kk = 0; kk < nghpixs.Nhnlength; kk++)
						{
							qq = nghpixs.Nhn_ax[kk];
							if ((BW[qq] != 0) && (fin[qq] == val))
							{
								mstack.push(qq);
								BW[qq] = 0;
							}
						}
					}

				}
			}
		}
	}
	return true;
}



/********************************************************************
    函数：Bwdist  -  内标记图像的距离变换
    参数：bw：内标记图像
          fdist：距离变换后的图像
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::Bwdist(bool *bw , float *fdist)
{
	//求内标记间的距离
	if (!bw || !fdist)
		return false;

	int M = pixWidth;
	int N = pixHeight;
	short *col_values;
	pixel_T *candidates;//二维  保存每一行中具有内标记的像素点的坐标
	pixel_T *ell;//二维
	float *Out = fdist;
	int num_candidates;
	int num_cells;
	float *pr;
	float *nn = nullptr;
	int k;
	int c;
	int c_up;

	pr = fdist;

	nn = new float[nPixels];

	for (k = 0; k < M * N; k++)
	{
		pr[k] = 4.2950e+009;
	}

	candidates = new  pixel_T[M];
	ell = new  pixel_T[M];
	col_values = new  short[M]; //索引矩阵


	for (k = 0; k < M; k++)
		col_values[k] = FAKE_COL_VAL;
	for (c = 0; c < N; c++)
	{
		if (!Update_candidates(candidates, &num_candidates, col_values, c, bw, M))
			return false;
		if (!Compute_ell(ell, &num_cells, candidates, num_candidates, c))
			return false;
		if (!Assign_distances(Out, M, c, ell, num_cells, nn))
			return false;
	}

	for (k = 0; k < M; k++)
		col_values[k] = FAKE_COL_VAL;
	for (c_up = 0; c_up < N; c_up++)
	{
		//反向扫描
		c = N - 1 - c_up;

		if (!Update_candidates(candidates, &num_candidates, col_values, c, bw, M))
			return false;
		if (!Compute_ell(ell, &num_cells, candidates, num_candidates, c))
			return false;
		if (!Assign_distances(pr, M, c, ell, num_cells, nn))
			return false;
	}

	for (k = 0; k < M * N; k++)
		pr[k] = sqrt(pr[k]);

	if (nn)
		delete[] nn;
	if (candidates)
		delete[] candidates;
	if (ell)
		delete[] ell;
	if (col_values)
		delete[] col_values;

	return true;
}

/********************************************************************
    函数：Update_candidates  -  更新第C列的参数矩阵
    参数：candidates：当前像元（参数点）
          num_candidates：待处理像素点数
          col_values：像素在每行中的最大值的列编号
          c：当前像元的列编号
          bw：二值图像矩阵
          M：矩阵的行编号
    返回：如正确执行，则返回true，否则返回false
********************************************************************/

bool WaterShed::Update_candidates(pixel_T *candidates, int *num_candidates,
                                  short *col_values, int c, bool *bw, int M)
{

	bool *pr; //输入的二值图像
	int r;


	pr = bw + M * c; //获取输入图像第C行首地址
	for (r = 0; r < M; r++)
	{
		if (pr[r] != 0)
		{
			//第c行的内标记图像如果不为零，则将其行值col_values设为C
			col_values[r] = c;   //默认为FAKE_COL_VAL：-1
		}
	}


	*num_candidates = 0;
	for (r = 0; r < M; r++)
	{
		if (col_values[r] != FAKE_COL_VAL)  //FAKE_COL_VAL=-1
		{
			//保存有标记点的像素点的行和外标号
			candidates[*num_candidates].x = col_values[r];
			candidates[*num_candidates].y = r;
			(*num_candidates)++;
		}
	}
	return true;
}

/********************************************************************
    函数：Compute_ell  -  创建参数点
    参数：ell：泰森多边形法的单元阵列
          num_cells：泰森多边形法的单元阵列数
          candidates：参数像素点矩阵
          num_candidates：参数点数
          c：当前处理像元的列编号
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::Compute_ell(pixel_T *ell, int *num_cells, pixel_T *candidates,
                            int num_candidates, int c)
{
	int k;
	int p;
	pixel_T w;

	if (num_candidates == 0)
	{
		*num_cells = 0;
	}
	else if (num_candidates == 1)
	{
		*num_cells = 1;
		ell[0] = candidates[0];
	}
	else
	{
		ell[0] = candidates[0];
		ell[1] = candidates[1];
		k = 1;
		p = 2;
		while (p < num_candidates)
		{
			w = candidates[p];
			while ((k >= 1) && Remove_candidate(ell[k - 1], ell[k], w, c))
				k--;
			k++;
			p++;
			ell[k] = w;
		}
		*num_cells = k + 1;
	}

	return true;
}

/********************************************************************
	函数：Assign_distances  -  计算像元离Voronoi中心最近的距离
	参数：Out：输出距离图像；
		M：行编号
		column：列编号
		ell：泰森多边形法的单元阵列
		num_cells：泰森多边形法的单元阵列数
	返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::Assign_distances(float *Out, int M, int column, pixel_T *ell,
                                 int num_cells, float *nn)
{
	int c = column;  //注意：第C行
	int r;
	int current_cell = 0;
	float *pr;
	float *pr_nn = nullptr;
	float sq_dist;
	float temp_sq_dist;
	float dx;
	float dy;
	int k;
	bool do_labels = nn != nullptr;

	if (num_cells > 0)
	{
		pr = Out + M * c;
		if (do_labels)
			pr_nn = nn + M * c;

		for (r = 0; r <  M; r++)
		{
			/*
			 * Compute squared distance to current cell.
			 */
			dx = (float) (c - ell[current_cell].x);
			dy = (float) (r - ell[current_cell].y);
			sq_dist = dx * dx + dy * dy;

			/*
			 * See if we need to update the current cell.
			 */
			for (k = current_cell + 1; k < num_cells; k++)
			{
				dx = (float) (c - ell[k].x);
				dy = (float) (r - ell[k].y);
				temp_sq_dist = dx * dx + dy * dy;
				if (temp_sq_dist < sq_dist)
				{
					current_cell = k;
					sq_dist = temp_sq_dist;
				}
				else
				{
					break;
				}
			}

			if (sq_dist < pr[r])
			{
				pr[r] = sq_dist;
				if (do_labels)
					pr_nn[r] = M * ell[current_cell].x + ell[current_cell].y + 1.0;
			}
		}
	}
	return true;
}

/********************************************************************
    函数：Remove_candidate  -  反转方程中的X和Y
    参数：c；列编号
          u、v、w：像元
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::Remove_candidate(pixel_T u, pixel_T v, pixel_T w, int c)
{
	float term1;
	float term2;
	float ux = (float) u.x;
	float uy = (float) u.y;
	float vx = (float) v.x;
	float vy = (float) v.y;
	float wx = (float) w.x;
	float wy = (float) w.y;


	term1 = (wy - vy) * (vy * vy - uy * uy - 2 * c * (vx - ux) + vx * vx - ux * ux);

	term2 = (vy - uy) * (wy * wy - vy * vy - 2 * c * (wx - vx) + wx * wx - vx * vx);

	// u.y >= w.y
	return term1 >= term2;
}


/********************************************************************
    函数：Bwlabel  -  快速标记二值图像的连通区域
    参数：image_in；输入二值图像
          image_out：输出标记图像
          mode：计算类型 4：4-连通   8：8-连通
    返回：返回连通区域数
********************************************************************/
int WaterShed::Bwlabel(bool *image_in , int *image_out, int mode )
{
	if (!image_in || (mode != 4 && mode != 8))
		return 0;

	int label;
	int num = 0;

	label = LABEL;

	int *tempout;  //标记图像，判断是否标记
	tempout = new int[nPixels];
	memset(tempout, 0, sizeof(int)*nPixels);

	for (int j = 0; j < pixHeight; j++)
	{
		for (int i = 0; i < pixWidth; i++)
		{
			//判断输入图像中的当前点是否为1,并且在输出标记图像中是否没有标记
			if (*(image_in + j * pixWidth + i) == 1 && *(tempout + j * pixWidth + i) == 0)
			{
				//如果当前点为1,并且没有在输出标记图像中标记数字 (除0外)
				//则从当前点开始向其邻阈寻找未标记的，在输入图像中为1的点，并对其标记
				if (!Labelset(image_in , tempout, i , j , label , mode))
					return 0;
				label++;  //标记序号加1
			}
		}
	}


	memcpy(image_out, tempout, sizeof(int)*nPixels); //输出图像

	if (tempout)
	{
		delete [] tempout;
		tempout = nullptr;
	}

	num = label - LABEL;
	return num;
}

/********************************************************************
    函数：Labelset  -  计算当前点的连通区域
    参数：image_in；输入二值图像
          image_out：输出标记图像
          xx：当前点的X坐标
          yy：当前点的Y坐标
          label：当前标记编号
          mode：计算类型 4：4-连通   8：8-连通
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::Labelset(bool *image_in, int *image_out, int xx , int yy
                         , int label , int mode)
{
	int im, ip, jm, jp; //邻域坐标
	*(image_out + yy * pixWidth + xx) = label; //符合条件，则在输出标记图像中标记当前点
	int xs, ys; //当前点的坐标

	std::stack<int> labelStack; //空堆栈  存储为标记点的标号

	int id = yy * pixWidth + xx; //当前像素点的编号
	labelStack.push(id);  //将当前点压入堆栈

	while (labelStack.empty() == false)
	{
		id = labelStack.top(); //获取栈顶值
		labelStack.pop();  //删除栈顶元素
		xs = id % pixWidth; //获取当前点的X坐标
		ys = id / pixWidth; //获取当前点的Y坐标
		im = xs - 1;
		ip = xs + 1; //当前点的邻域坐标
		jm = ys - 1;
		jp = ys + 1;
		//保护边界
		if (im < 0) im = 0;
		if (ip >= pixWidth) ip = pixWidth - 1;
		if (jm < 0) jm = 0;
		if (jp >= pixHeight) jp = pixHeight - 1;

		if (*(image_in + jm * pixWidth + im) == 1 && *(image_out + jm * pixWidth + im) == 0 && mode == 8)
		{
			//左上
			*(image_out + jm * pixWidth + im) = label;
			id = jm * pixWidth + im; //当前像素点的编号
			labelStack.push(id);  //将编号压入堆栈
		}
		if (*(image_in + jm * pixWidth + xs) == 1 && *(image_out + jm * pixWidth + xs) == 0)
		{
			//上
			*(image_out + jm * pixWidth + xs) = label;
			id = jm * pixWidth + xs; //当前像素点的编号
			labelStack.push(id);  //将编号压入堆栈
		}
		if (*(image_in + jm * pixWidth + ip) == 1 && *(image_out + jm * pixWidth + ip) == 0 && mode == 8)
		{
			//右上
			*(image_out + jm * pixWidth + ip) = label;
			id = jm * pixWidth + ip; //当前像素点的编号
			labelStack.push(id);  //将编号压入堆栈
		}
		if (*(image_in + ys * pixWidth + im) == 1 && *(image_out + ys * pixWidth + im) == 0)
		{
			//左
			*(image_out + ys * pixWidth + im) = label;
			id = ys * pixWidth + im; //当前像素点的编号
			labelStack.push(id);  //将编号压入堆栈
		}
		if (*(image_in + ys * pixWidth + ip) == 1 && *(image_out + ys * pixWidth + ip) == 0)
		{
			//右
			*(image_out + ys * pixWidth + ip) = label;
			id = ys * pixWidth + ip; //当前像素点的编号
			labelStack.push(id); //将编号压入堆栈
		}
		if (*(image_in + jp * pixWidth + im) == 1 && *(image_out + jp * pixWidth + im) == 0 && mode == 8)
		{
			//左下
			*(image_out + jp * pixWidth + im) = label;
			id = jp * pixWidth + im; //当前像素点的编号
			labelStack.push(id);  //将编号压入堆栈
		}
		if (*(image_in + jp * pixWidth + xs) == 1 && *(image_out + jp * pixWidth + xs) == 0)
		{
			//下
			*(image_out + jp * pixWidth + xs) = label;
			id = jp * pixWidth + xs; //当前像素点的编号
			labelStack.push(id);   //将编号压入堆栈
		}
		if (*(image_in + jp * pixWidth + ip) == 1 && *(image_out + jp * pixWidth + ip) == 0 && mode == 8)
		{
			//右下
			*(image_out + jp * pixWidth + ip) = label;
			id = jp * pixWidth + ip; //当前像素点的编号
			labelStack.push(id); //将编号压入堆栈
		}
	}
	return true;
}


/********************************************************************
    函数：watershed_meyer  -  图像执行分水岭变换
    参数：fin：梯度指针
          fmask：标记矩形
          fout：输出分水岭数据指针
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::watershed_meyer(short *fin, int *fmask, short *fout)
{
	if (fin == nullptr || fmask == nullptr || fout == nullptr)
		return false;

	node mnode;
	std::priority_queue < node, std::vector<node>, node_LowestPriorityFirst_cmp > mqueue;

	int N = nPixels;
	bool *S = new bool[N];
	memset(S, false, sizeof(bool)*N);

	int Yp, Xp;

	int order = 0;

	for (int p = 0; p < N; p++)
	{
		fout[p] = fmask[p];
		if (fmask[p] != WSHED)
		{
			int q;
			S[p] = true;
			Yp = (int)p / pixWidth;
			Xp = p - Yp * pixWidth;
			GetNgh3x3MinusCenterPix(fmask, Xp, Yp);
			for (int k = 0; k < nghpixs.Nhnlength; k++)
			{
				q = nghpixs.Nhn_ax[k]; //得到绝对偏移量q(q为绝对坐标值)
				if ( (! S[q]) && (fmask[q] == WSHED) )	// 查找边缘，可能是岭
				{
					S[q] = true;
					mnode.iData = q;
					mnode.iPriority = fin[q];
					mnode.iOrder = order++;
					mqueue.push(mnode);
				}
			}
		}
	}

	while ( !mqueue.empty())
	{
		int q;
		int p = mqueue.top().iData;
		int v = mqueue.top().iPriority;
		mqueue.pop();

		double label = WSHED;
		bool watershed = false;

		Yp = (int)p / pixWidth;
		Xp = p - Yp * pixWidth;
		GetNgh3x3MinusCenterPix(fmask, Xp, Yp);
		// 该边缘点周围的标记点具有两种，该点就是分水岭
		for (int k = 0; k < nghpixs.Nhnlength; k++)
		{
			q = nghpixs.Nhn_ax[k]; //得到绝对偏移量q(q为绝对坐标值)
			if ((fout[q] != WSHED) && !watershed)
			{
				if ((label != WSHED) && (fout[q] != label))
					watershed = true;
				else
					label = fout[q];
			}
		}

		// 岭的临点也可能使岭
		if (!watershed)
		{
			fout[p] = label;	// 两种标记值的第一种，涨水

			Yp = (int)p / pixWidth;
			Xp = p - Yp * pixWidth;
			GetNgh3x3MinusCenterPix(fmask, Xp, Yp);
			for (int k = 0; k < nghpixs.Nhnlength; k++)
			{
				q = nghpixs.Nhn_ax[k]; //得到绝对偏移量q(q为绝对坐标值)
				if (!S[q])
				{
					S[q] = true;
					mnode.iData = q;
					mnode.iPriority = max(fin[q], v);
					mnode.iOrder = order++;
					mqueue.push(mnode);
				}
			}
		}
	}

	if (S != nullptr)
		delete[] S;

	return true;
}

/********************************************************************
    函数：watershed  -  分水岭变换
    参数：fin：梯度指针
          fout：输出分水岭数据指针
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::watershed(short *fin, short *fout)
{
	if (fin == nullptr || fout == nullptr)
		return false;

	short *ImcomplementInpix;
	ImcomplementInpix = new short [nPixels];
	for (int i = 0; i < nPixels; i++)
		ImcomplementInpix[i] = fin[i];

	short *Imcomplementpix;
	Imcomplementpix = new short [nPixels];
	Imcomplement(ImcomplementInpix, Imcomplementpix); // 输入的反色图像

	bool *areaExmaxPix;
	areaExmaxPix = new bool [nPixels];
	Imregionalmax(Imcomplementpix, areaExmaxPix); //区域极大值

	int *labelPix = new int [nPixels]; //标记图
	Bwlabel(areaExmaxPix, labelPix, 8);

	//执行MEYER分水岭   fin:梯度指针;labelPix:标记矩阵;L:输出
	if (!watershed_meyer(fin, labelPix, fout))
		return false;

	if (labelPix)
	{
		delete[] labelPix;
		labelPix = nullptr;
	}
	if (areaExmaxPix)
	{
		delete[] areaExmaxPix;
		areaExmaxPix = nullptr;
	}
	if (Imcomplementpix)
	{
		delete[] Imcomplementpix;
		Imcomplementpix = nullptr;
	}
	if (ImcomplementInpix)
	{
		delete[] ImcomplementInpix;
		ImcomplementInpix = nullptr;
	}

	return true;
}
/********************************************************************
    函数：Imimposemin  -  极小值强加，修改梯度图像
    参数：inPix：梯度图像
          inLabel：内标记矩阵
          exLabel：外标记矩阵
          outPix：修改后的梯度图像
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::Imimposemin(short *inPix, bool *inLabel, bool *exLabel, short *outPix)
{
	if (inPix == nullptr || inLabel == nullptr || exLabel == nullptr || outPix ==nullptr)
		return false;

	short *fm;	// 内标+外标 黑色
	fm = new short[nPixels];
	memset(fm, 0, sizeof(short)*nPixels);
	for (int i = 0; i < nPixels; i++)
	{
		if (inLabel[i] || exLabel[i])
			fm[i] = 0;
		else
			fm[i] = 255;
	}

	short *fp1 ;
	fp1 = new short[nPixels];
	memcpy(fp1, fm, sizeof(short)*nPixels);

	short *g ;
	g = new short[nPixels];
	memcpy(g, fm, sizeof(short)*nPixels);

	if (!Imadd(inPix , 1, fp1))
		return false;

	//(f+1)^fm
	if (!Immin(fp1 , fm, g))
		return false;

	//  ~(f+1)
	if (!Imcomplement(fm , fm))
		return false;

	//  ~fm
	if (!Imcomplement(g , g))
		return false;

	//对(f+1)^fm腐蚀重建（此函数为对~(f+1)^~fm膨胀重建）
	if (!Imreconstruct(fm , g))
		return false;

	//求反
	if (!Imcomplement(fm , fm))
		return false;

	memcpy(outPix, fm, sizeof(short)*nPixels);

	if (fm)
		delete[] fm;
	if (g)
		delete[] g;
	if (fp1)
		delete[] fp1;

	return true;
}


/********************************************************************
    函数：CalculateLabel  -  计算蛋白点区域
    参数：inImg：内标记矩阵
          exImg：外标记矩阵
          wsImg：分水岭标记矩阵
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::CalculateLabel(bool *inImg , bool *exImg , bool *wsImg , short *ws_Img)
{
	if (inImg == nullptr || exImg == nullptr || wsImg == nullptr || ws_Img == nullptr)
		return false;

	// 蛋白点标号初始, 从1开始
	spot_ID = 0;

	int *tempout;  //标记图像，判断是否标记
	tempout = new int[nPixels];
	memset(tempout, 0, sizeof(int)*nPixels);

	bool *wsout;  //修改后的分水岭脊线
	wsout = new bool[nPixels];
	memset(wsout, 0, sizeof(bool)*nPixels);

	int label;
	label = LABEL;
	if (faintThreshold <= 0)
	{
		// 如果最弱点阈值为负，则需要根据最弱点坐标重新计算阈值
		int *tempFaint;  //
		tempFaint = new int[nPixels];
		memset(tempFaint, 0, sizeof(int)*nPixels);

		// 计算最弱点阈值
		faintThreshold = CalculateFaintRatio(inImg, exImg, wsImg, ws_Img, tempFaint, faintX, faintY, LABEL);

		if (tempFaint)
			delete [] tempFaint;
	}

	bool* pTIn = inImg;
	int* pTOut = tempout;
	for (int j = 0; j < pixHeight; j++)
	{
		for (int i = 0; i < pixWidth; i++)
		{
			//判断输入图像中的当前点是否为内标记点,并且在输出标记图像中是否没有标记
			if (*pTIn && *pTOut == 0)
			{
				// 如果当前点为50,并且没有在输出标记图像中标记数字 (除0外)
				//  则从当前点开始向其邻阈寻找未标记的蛋白点区域，并对其标记
				if (!CalculateLabel(inImg, exImg, wsImg, ws_Img, tempout, wsout, i, j, label))
					return false;

				// 标记序号加1
				label++;
			}
			++pTIn;
			++pTOut;
		}
	}

	for (int i = 0; i < nPixels; ++i)
		wsImg[i] = wsout[i];

	if (ws_spotList)
	{
		if (wsout)
			memcpy(ws_spotList->watershed_Label, wsout, sizeof(bool)*nPixels);
		if (adjustedBG_pixdat)
			memcpy(ws_spotList->adjustedBG_pixdat, adjustedBG_pixdat, sizeof(short)*nPixels);
	}

	if (wsout)
		delete [] wsout;
	if (tempout)
		delete [] tempout;
	//  if(img)
	//      delete [] img;

	return true;
}


/********************************************************************
    函数：CalculateFaintRatio  -  计算最弱点阈值
    参数：image_in：输入标记图像
          image_out：输出标记图像
          xx：当前点的X坐标
          yy：当前点的Y坐标
          label：当前区域的标记
    返回：返回最弱点阈值
********************************************************************/
float WaterShed::CalculateFaintRatio(bool *inImg, bool *exImg , bool *wsImg, short *ws_Img, int *image_out, int xx, int yy, int label)
{
	if (!image_out)
		return 0;

	int inLabelSum = 0; //  内标记区域数
	int wsLabelSum = 0; //  分水岭区域数（不包含内标记）
	int exLabelSum = 0; //  外标记区域数(不包含分水岭区域和内标记区域)

	int wsLineSum = 0; //分水岭脊线所包含像素个数
	//int spotID;

	int inLabelValue = 0; //  内标记区域像素总和
	int wsLabelValue = 0; //  分水岭区域像素总和（不包含内标记）
	int exLabelValue = 0; //  外标记区域像素总和(不包含分水岭区域和内标记区域)

	int wsLineLabelValue = 0; //分水岭线的灰度值总和


	int im, ip, jm, jp; //邻域坐标
	*(image_out + yy * pixWidth + xx) = label; //符合条件，则在输出标记图像中标记当前点
	int xs, ys; //当前点的坐标


	inLabelSum++;  //内标记数自增
	inLabelValue += (255 - adjustedBG_pixdat[yy * pixWidth + xx]); //内标记总灰度值自增  255-value

	std::stack<int> exStack; //空堆栈  存储为外标记点的标号

	std::stack<int> ws_stack; //空堆栈  存储为分水岭标记点的标号

	int id = yy * pixWidth + xx; //当前像素点的编号

	exStack.push(id);  //将当前点压入堆栈

	//  int minArea=minSpotRadius*minSpotRadius;

	while (!exStack.empty())
	{
		//先统计外标记内部信息，同时将分水岭脊线、分水岭内部和内标信息保存到相应堆栈中

		id = exStack.top(); //获取栈顶值
		exStack.pop();  //删除栈顶元素
		xs = id % pixWidth; //获取当前点的X坐标
		ys = id / pixWidth; //获取当前点的Y坐标
		im = xs - 1;
		ip = xs + 1; //当前点的邻域坐标
		jm = ys - 1;
		jp = ys + 1;
		//保护边界
		if (im < 0) im = 0;
		if (ip >= pixWidth) ip = pixWidth - 1;
		if (jm < 0) jm = 0;
		if (jp >= pixHeight) jp = pixHeight - 1;



		if (*(exImg + jm * pixWidth + xs) != 1 && *(image_out + jm * pixWidth + xs) == 0)
		{
			//上   从内标记开始扩展 非外标记分水岭脊线，且没有在输出中标记
			id = jm * pixWidth + xs; //当前像素点的编号
			bool isStack = false;
			if (*(inImg + id) == 1)
			{
				//内标区域
				if (isStack == false)
				{
					exStack.push(id);  //将编号压入堆栈
					isStack = true;
				}


				inLabelSum++;
				inLabelValue += adjustedBG_pixdat[id];

			}

			if (*(wsImg + id) != 1 && *(ws_Img + id) > 2)
			{
				//分水岭盆地  蛋白点区域

				if (isStack == false)
				{
					exStack.push(id);  //将编号压入堆栈
					isStack = true;
				}

				wsLabelSum++;
				wsLabelValue += adjustedBG_pixdat[id];

			}

			if (*(wsImg + id) == 1)
			{
				//分水岭脊线  蛋白点边缘
				if (isStack == false)
				{
					exStack.push(id);  //将编号压入堆栈
					isStack = true;
				}
				ws_stack.push(id);  //将编号压入堆栈

				wsLineSum++; //分水岭脊线像素数
				wsLineLabelValue += adjustedBG_pixdat[id];


			}
			if (*(exImg + id) != 1)
			{
				//外标区域
				exStack.push(id);  //将编号压入堆栈

				exLabelSum++;
				exLabelValue += adjustedBG_pixdat[id];
			}

			*(image_out + id) = label;
		}


		if (*(exImg + ys * pixWidth + im) != 1 && *(image_out + ys * pixWidth + im) == 0)
		{
			//左   从内标记开始扩展 非外标记分水岭脊线，且没有在输出中标记
			id = ys * pixWidth + im; //当前像素点的编号
			bool isStack = false;
			if (*(inImg + id) == 1)
			{
				//内标区域
				if (isStack == false)
				{
					exStack.push(id);  //将编号压入堆栈
					isStack = true;
				}


				inLabelSum++;
				inLabelValue += adjustedBG_pixdat[id];

			}

			if (*(wsImg + id) != 1 && *(ws_Img + id) > 2)
			{
				//分水岭盆地  蛋白点区域

				if (isStack == false)
				{
					exStack.push(id);  //将编号压入堆栈
					isStack = true;
				}

				wsLabelSum++;
				wsLabelValue += adjustedBG_pixdat[id];

			}

			if (*(wsImg + id) == 1)
			{
				//分水岭脊线  蛋白点边缘
				if (isStack == false)
				{
					exStack.push(id);  //将编号压入堆栈
					isStack = true;
				}
				ws_stack.push(id);  //将编号压入堆栈

				wsLineSum++; //分水岭脊线像素数
				wsLineLabelValue += adjustedBG_pixdat[id];



			}
			if (*(exImg + id) != 1)
			{
				//外标区域
				exStack.push(id);  //将编号压入堆栈

				exLabelSum++;
				exLabelValue += adjustedBG_pixdat[id];
			}

			*(image_out + id) = label;
		}

		if (*(exImg + ys * pixWidth + ip) != 1 && *(image_out + ys * pixWidth + ip) == 0)
		{
			//右   从内标记开始扩展 非外标记分水岭脊线，且没有在输出中标记
			id = ys * pixWidth + ip; //当前像素点的编号
			bool isStack = false;
			if (*(inImg + id) == 1)
			{
				//内标区域
				if (isStack == false)
				{
					exStack.push(id);  //将编号压入堆栈
					isStack = true;
				}



				inLabelSum++;
				inLabelValue += adjustedBG_pixdat[id];

			}

			if (*(wsImg + id) != 1 && *(ws_Img + id) > 2)
			{
				//分水岭盆地  蛋白点区域

				if (isStack == false)
				{
					exStack.push(id);  //将编号压入堆栈
					isStack = true;
				}

				wsLabelSum++;
				wsLabelValue += adjustedBG_pixdat[id];

			}

			if (*(wsImg + id) == 1)
			{
				//分水岭脊线  蛋白点边缘
				if (isStack == false)
				{
					exStack.push(id);  //将编号压入堆栈
					isStack = true;
				}
				ws_stack.push(id);  //将编号压入堆栈

				wsLineSum++; //分水岭脊线像素数
				wsLineLabelValue += adjustedBG_pixdat[id];
			}
			if (*(exImg + id) != 1)
			{
				//外标区域
				exStack.push(id);  //将编号压入堆栈

				exLabelSum++;
				exLabelValue += adjustedBG_pixdat[id];
			}

			*(image_out + id) = label;
		}

		if (*(exImg + jp * pixWidth + xs) != 1 && *(image_out + jp * pixWidth + xs) == 0)
		{
			//下   从内标记开始扩展 非外标记分水岭脊线，且没有在输出中标记
			id = jp * pixWidth + xs; //当前像素点的编号
			bool isStack = false;
			if (*(inImg + id) == 1)
			{
				//内标区域
				if (isStack == false)
				{
					exStack.push(id);  //将编号压入堆栈
					isStack = true;
				}

				inLabelSum++;
				inLabelValue += adjustedBG_pixdat[id];
			}

			if (*(wsImg + id) != 1 && *(ws_Img + id) > 2)
			{
				//分水岭盆地  蛋白点区域

				if (isStack == false)
				{
					exStack.push(id);  //将编号压入堆栈
					isStack = true;
				}

				wsLabelSum++;
				wsLabelValue += adjustedBG_pixdat[id];
			}

			if (*(wsImg + id) == 1)
			{
				//分水岭脊线  蛋白点边缘
				if (isStack == false)
				{
					exStack.push(id);  //将编号压入堆栈
					isStack = true;
				}
				ws_stack.push(id);  //将编号压入堆栈

				wsLineSum++; //分水岭脊线像素数
				wsLineLabelValue += adjustedBG_pixdat[id];
			}
			if (*(exImg + id) != 1)
			{
				//外标区域
				exStack.push(id);  //将编号压入堆栈

				exLabelSum++;
				exLabelValue += adjustedBG_pixdat[id];
			}

			*(image_out + id) = label;
		}

	}

	float  imGray = 0;
	if (inLabelSum > 0)
		imGray = (float)inLabelValue / inLabelSum; //蛋白点内标记平均灰度
	//        float  wsGray=(float)wsLabelValue/wsLabelSum;  //蛋白点分水岭盆地（除内标记）平均灰度
	float  exGray = 0;
	if (exLabelSum > 0)
		exGray = (float)(exLabelValue - wsLabelValue) / (exLabelSum - wsLabelSum); //蛋白点外标记分水岭盆地（除蛋白点区域）平均灰度

	return  fabs(exGray - imGray);
}


/********************************************************************
    函数：CalculateLabel  -  计算蛋白点信息，保存合格蛋白点信息
    参数：image_in：输入标记图像
          image_out：输出标记图像
          ws_out,：分水岭标记矩阵
          xx：当前点的X坐标
          yy：当前点的Y坐标
          label：当前区域的标记
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::CalculateLabel(bool *inImg, bool *exImg , bool *wsImg, short *ws_Img,
                               int *image_out, bool *ws_out,
                               int xx , int yy , int label)
{
	// 计算有内标记的区域
	if (ws_out == nullptr || image_out == nullptr)
		return false;

	int inLabelSum = 0;	// 内标记区域数
	int wsLabelSum = 0;	// 分水岭区域数（不包含内标记）
	int wsLineSum = 0;	// 分水岭脊线所包含像素个数
	int exLabelSum = 0;	// 外标记区域数(不包含分水岭区域和内标记区域)

	int inLabelValue = 0; // 内标记区域像素总和
	int wsLabelValue = 0; // 分水岭区域像素总和（不包含内标记）
	int wsLineLabelValue = 0; // 分水岭线的灰度值总和
	int exLabelValue = 0; // 外标记区域像素总和(不包含分水岭区域和内标记区域)

	// 内标记的最大最小X、Y
	int minX = pixWidth - 1;
	int minY = pixHeight - 1;
	int maxX = 0;
	int maxY = 0;

	// 分水岭区域的最大最小X、Y
	int minrX = pixWidth - 1;
	int minrY = pixHeight - 1;
	int maxrX = 0;
	int maxrY = 0;

	minX = min(minX, xx);
	minY = min(minY, yy);
	maxX = max(maxX, xx);
	maxY = max(maxY, yy);

	inLabelSum++;  // 内标记数+1（当前点）
	inLabelValue += (255 - adjustedBG_pixdat[yy*pixWidth + xx]); // 内标记总灰度值+ （当前点）
	*(image_out + yy * pixWidth + xx) = label;	// 符合条件，则在输出标记图像中标记当前点

	std::stack<int> exStack;	// 空堆栈 存储为外标记点的标号
	std::stack<int> ws_stack;	// 空堆栈 存储为分水岭标记点的标号

	int id = yy*pixWidth + xx;	// 当前像素点的编号
	exStack.push(id);  			// 将当前点压入堆栈

	// 统计该蛋白点的信息：内标、内部、边缘、外标
	while (!exStack.empty())
	{
		// 迭代
		id = exStack.top();
		exStack.pop();
		int xs = id % pixWidth;
		int ys = id / pixWidth;

		// 上下左右四邻域扩散 从内标记开始扩展 非外标记分水岭脊线，且没有在输出中标记
		int iNB_x[4] = {-1, -1, -1, -1};
		int iNB_y[4] = {-1, -1, -1, -1};
		if (ys - 1 >= 0)
		{
			iNB_x[0] = xs;
			iNB_y[0] = ys - 1;
		}
		if (xs - 1 >= 0)
		{
			iNB_x[1] = xs - 1;
			iNB_y[1] = ys;
		}
		if (xs + 1 < pixWidth)
		{
			iNB_x[2] = xs + 1;
			iNB_y[2] = ys;
		}
		if (ys + 1 < pixHeight)
		{
			iNB_x[3] = xs;
			iNB_y[3] = ys + 1;
		}
		for (int i = 0; i < 4; ++i)
		{
			if (iNB_x[i] < 0)
				continue;
			int xm = iNB_x[i];
			int ym = iNB_y[i];
			id = ym*pixWidth + xm;

			if (exImg[id] || image_out[id] != 0)
				continue;

			bool isStack = false;
			image_out[id] = label;

			// 内标区域
			if (inImg[id])
			{
				if (!isStack)
				{
					exStack.push(id);
					isStack = true;
				}

				// 求内标记区域的最大最小坐标
				minX = min(minX, xm);
				minY = min(minY, ym);
				maxX = max(maxX, xm);
				maxY = max(maxY, ym);

				inLabelSum++;
				inLabelValue += (255 - adjustedBG_pixdat[id]);
			}

			// 蛋白点区域 内部
			if (!wsImg[id] && ws_Img[id] > 2)
			{
				if (!isStack)
				{
					exStack.push(id);
					isStack = true;
				}
				wsLabelSum++;
				wsLabelValue += (255 - adjustedBG_pixdat[id]);
			}

			// 分水岭脊线  蛋白点边缘
			if (wsImg[id])
			{
				if (!isStack)
				{
					exStack.push(id);
					isStack = true;
				}

				ws_stack.push(id);

				wsLineSum++;
				wsLineLabelValue += (255 - adjustedBG_pixdat[id]);

				// 求分水岭区域的最大最小坐标
				minrX = min(minrX, xm);
				minrY = min(minrY, ym);
				maxrX = max(maxrX, xm);
				maxrY = max(maxrY, ym);
			}

			// 外标区域
			if (!exImg[id])
			{
				exStack.push(id);

				exLabelSum++;
				exLabelValue += (255 - adjustedBG_pixdat[id]);
			}
		}	// for (int i = 0; i < 4; ++i)
	}

	// 输出符合要求的点
	if (wsLabelSum && exLabelSum && !ws_stack.empty())
	{
		// 蛋白点信息 内标记中心 质心
		int centroid_X = int((minX + maxX) / 2.0 + 0.5);
		int centroid_Y = int((minY + maxY) / 2.0 + 0.5);

		// 蛋白点几何中心
		int center_X = int((maxrX + minrX) / 2.0 + 0.5);
		int center_Y = int((maxrY + minrY) / 2.0 + 0.5);
		// 蛋白点区域半径
		int rx = int((maxrX - minrX) / 2.0 + 0.5);
		int ry = int((maxrY - minrY) / 2.0 + 0.5);

		int area = wsLabelSum;		// 蛋白点面积
		int volume = wsLabelValue;	// 蛋白点体积
		// 蛋白点平均灰度
		float aveGray = 0;
		if (area > 0)
			aveGray = 255 - (float)(volume / area);
		// 蛋白点内标记平均灰度
		float  imGray = 0;
		if (inLabelSum > 0)
			imGray = 255 - (float)(inLabelValue / inLabelSum);
		// 蛋白点外标记分水岭盆地（除蛋白点区域）平均灰度
		float  exGray = 0;
		if (exLabelSum > 0)
			exGray = 255 - (float)((exLabelValue - wsLabelValue) / (exLabelSum - wsLabelSum));

		// 去除不合格点
		if (fabs(exGray - imGray) <= faintThreshold ||	// 内外标记平均灰度差 小于 阈值
		        min(rx, ry) < minSpotRadius ||				// 小半径 小于 最小半径
		        area < minSpotRadius * minSpotRadius ||		// 面积 小于 最小面积
		        rx / float(ry + 0.5) >= spotRxRyRatio ||	// 宽高比
		        ry / float(rx + 0.5) >= spotRxRyRatio)
		{
			// 剔除
			while (!ws_stack.empty())
			{
				id = ws_stack.top();	// 获取栈顶值
				ws_stack.pop();			// 删除栈顶元素
				ws_out[id] = 0;			// 不是分水岭脊线
			}
		}
		else
		{
			//1 调整分水岭脊线标记数据
			int* pWatershedLineA = new int[wsLineSum];
			int kk = 0;
			while (!ws_stack.empty())
			{
				//分水岭脊线
				id = ws_stack.top();		// 获取栈顶值
				ws_stack.pop();				// 删除栈顶元素
				pWatershedLineA[kk++] = id;
				ws_out[id] = 1;
			}

			//2 新建蛋白点节点，并保存蛋白点信息
			spot_ID++; // 蛋白点编号
			if (ws_spotList == nullptr)
			{
				// 给蛋白点链表分配空间
				ws_spotList = new SpotList;
				if (!InitSpotList(ws_spotList, nPixels))
					return false;
			}
			// 添加蛋白点到链表尾
			if (!TailSpotToList(ws_spotList, spot_ID,
								center_X, center_Y, centroid_X, centroid_Y,
								area, volume, rx, ry, wsLineSum, aveGray, exGray, imGray, pWatershedLineA))
				return false;

			if (pWatershedLineA != nullptr)
				delete [] pWatershedLineA;
		}
	}
	else
	{
		while (!ws_stack.empty())
		{
			id = ws_stack.top();	// 获取栈顶值
			ws_stack.pop();			// 删除栈顶元素
		}
	}

	return true;
}

/****************************************************************************
   链表操作  存储蛋白点信息
*****************************************************************************/

/********************************************************************
    函数：InitSpotList  -  初始化相同游程链表头节点
    参数：list：蛋白点链表
        total：图像像素数
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::InitSpotList(SpotList* lsSpot, int total)
{
	lsSpot->first = nullptr;
	lsSpot->last = nullptr;

	lsSpot->watershed_Label = new bool[total];
	memset(lsSpot->watershed_Label, 0, sizeof(bool)*total);

	lsSpot->adjustedBG_pixdat = new short[total];
	memset(lsSpot->adjustedBG_pixdat, 0, sizeof(short)*total);

	lsSpot->spotTotal = 0;

	return true;
}

/********************************************************************
    函数：TailSpotToList  -  添加新节点到头节点
    参数：list：蛋白点链表
          spotID：蛋白点编号
          centerX：蛋白点中心X坐标
          centerY：蛋白点中心Y坐标
          centroidX：蛋白点质心X坐标
          centroidY：蛋白点质心Y坐标
          spotArea：蛋白点面积
          spotVolume：蛋白点体积
          spotRx：蛋白点X轴半径
          SpotRy：蛋白点Y轴半径
          spotPerimeter：蛋白点周长
          spotAveGray：蛋白点平均灰度
          spotBgAveGray：蛋白点背景平均灰度
          spotInAveGray：蛋白点内标记副平均灰度
          pWatershedLine：蛋白点分水岭脊线指针
    返回：如正确执行，则返回true，否则返回false
********************************************************************/

bool WaterShed::TailSpotToList(SpotList *list, int spotID, int centerX, int centerY,
                               int centroidX, int centroidY, int spotArea, int spotVolume,
                               int spotRx, int SpotRy , int spotPerimeter, float spotAveGray,
                               float spotBgAveGray, float spotInAveGray , int *pWatershedLine )
{
	//分配和添加新的节点到蛋白点链表的最后
	SpotNode *newNode = nullptr;

	newNode = new SpotNode[1];
	newNode->next = nullptr;
	newNode->spotID = spotID ;
	newNode->centerX = centerX ;
	newNode->centerY = centerY ;
	newNode->centroidX = centroidX ;
	newNode->centroidY = centroidY ;
	newNode->spotArea = spotArea ;
	newNode->spotVolume = spotVolume ;
	newNode->spotRx = spotRx ;
	newNode->spotRy = SpotRy ;
	newNode->spotPerimeter = spotPerimeter ;
	newNode->spotAveGray = spotAveGray ;
	newNode->spotBgAveGray = spotBgAveGray ;
	newNode->spotInAveGray = spotInAveGray ;

	int *pWatershedLineA;
	pWatershedLineA = new int[spotPerimeter];
	memcpy(pWatershedLineA, pWatershedLine, sizeof(int)*spotPerimeter);
	newNode->pWatershedLine = pWatershedLineA;

	if (list->first == nullptr)
	{
		//如果第一个蛋白点节点为空，则将新蛋白点保存在首节点
		list->first = newNode;
		list->last = newNode;
	}
	else
	{
		//否则保存到最后一个节点的后面
		list->last->next = newNode;
	}
	//使last指向最后一个新加入的节点
	list->last = newNode;

	//计算蛋白点数
	list->spotTotal++;


	return true;
}

/********************************************************************
    函数：DeleteHeadNode  -  删除链表头节点
    参数：list：蛋白点链表
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::DeleteHeadNode(SpotList *list)
{
	// 去除和释放头节点
	SpotNode *killNode;

	if (list->first != nullptr)
	{
		killNode = list->first;
		list->first = killNode->next;

		if (killNode->pWatershedLine)
		{
			delete[] killNode->pWatershedLine;
			killNode->pWatershedLine = nullptr;
		}

		delete [] killNode;
		killNode = nullptr;

		list->spotTotal--;
	}
	return true;
}

/********************************************************************
    函数：DestroyList -  删除链表
    参数：list：蛋白点链表
    返回：如正确执行，则返回true，否则返回false
********************************************************************/
bool WaterShed::DestroyList(SpotList *list)
{
	//释放整个链表
	if (list == nullptr)
		return true;

	while (list->first != nullptr)
	{
		//释放蛋白点节点
		if (!DeleteHeadNode(list))
			return false ;
	}

	if (list)
	{
		//释放蛋白点链表结构体对象
		if (list->watershed_Label)
		{
			delete[] list->watershed_Label;
			list->watershed_Label = nullptr;
		}
		if (list->adjustedBG_pixdat)
		{
			delete[] list->adjustedBG_pixdat;
			list->adjustedBG_pixdat = nullptr;
		}

		delete[] list;
		list = nullptr;
	}
	return true;
}



