#pragma once
#ifndef TPS_2D_H
#define TPS_2D_H

#include "GEIMDef.h"

typedef struct Coorder_diff
{
	double x;	//源点坐标
	double y;
	double dx;	//变换后目标点坐标
	double dy;
} Coorder;

typedef std::vector<Coorder> VT_TPS_PAIRS;

//设计思路：
//构造函数输入控制点对和图像指针
//main函数中调用CreateEquation(),调用LU_Solve(),然后调用get_X()求得变换系数矩阵VM；
//main函数最后调用morph（），变形后的图像存在out_pixdat中，可通过bBitmap显示
class CTPS_2d_Morpher
{
public:
	CTPS_2d_Morpher();
	~CTPS_2d_Morpher();

	/** \brief 计算一次TPS变换参数
	 *
	 * \param pPairs VT_TPS_PAIRS*	[IN] 控制点对，需要在*堆上*创建，该类在不用时会自己释放
	 * \param regularization double	[IN] 松弛度
	 * \return bool true:成功 false:出错
	 *
	 */
	bool TPSMain(VT_TPS_PAIRS* pPairs, double regularization);
    /** \brief get current transform's smoothness: integral bending norm
     *
     * \return double the IBN, if < 0, it's invalid
     *
     */
	double Ibn()
	{
		return m_dIbn;
	}
	/** \brief 根据变换参数进行变换
	 *
	 * \param Pts Coorder*	[IN] a point to transform
	 * \return bool true:success false:failed
	 *
	 */
	bool morph(Coorder* Pts);
	/** \brief 根据变换参数变换一幅图像
	 *
	 * \param pSrc ST_RGB*	[IN] a image data to transform
	 * \param iW int		[IN] input image's width
	 * \param iH int		[IN] input image's height
	 * \param pDes ST_RGB*	[OUT] the result image, need alloc memory before invoke
	 * \param oW int		[IN] the output image's width
	 * \param oH int		[IN] the output image's height
	 * \return bool	true:suceess false:failed
	 *
	 */
	bool TransImg(ST_RGB* pSrc, int iW, int iH, ST_RGB* pDes, int oW, int oH);
    /** \brief release last transform result
     *
     * \return 	true:suceess false:failed
     *
     */
	bool Destroy();
    /** \brief is current has a valid transform parameter
     *
     * \return bool true:yes false:no
     *
     */
	bool IsOK()
	{
		return m_pSamples != nullptr;
	}

private:
	// 通过控制点对构造系数矩阵，正则化系数对应扭曲平滑程度
	int CreateEquation(double **mtx_l, double **mtx_v);
	// 线性方程左边系数a进行LU分解，输出结果保存在L,U当中
	int LU_Solve(double **a, double **L, double **U);
	// 线性方程左边系数a进行LU分解，输出结果保存在L,U当中
	int LU_Solve_Ou(double **matrix, double **L_matrix, double **U_matrix);
	// 根据LU矩阵解现行方程组，得到仿射变换系数（1，ax，ay）和弹性变形系数（wi_x ,wi_y），并存到b中输出
	int get_X(double **a, double **L, double **U, double **b, int n);
	// calculate the smoothness of th tansformation
	double calcu_ibn(double** mtxK, double** mtxW, int iDimen);
	// 创建动态数组
	double **create_Array(int n, int m);
	// 释放动态数组
	void free_Array(double **a, int n);

	inline double SQR( double x )
	{
		return x * x;    //平方函数
	}
	// 基函数，r2为待变形点到控制点的距离平方
	inline double base_func(double r2)
	{
		// same as r*r * log(r), but for r^2:
		return ( r2 == 0 )
		       ? 0.0						// function limit at 0
		       : r2 * log(r2) * 0.217147241; // 0.217147241= 1/(2*log(10))
	}

public:

private:
	// 控制点对
	VT_TPS_PAIRS* m_pSamples = nullptr;
	// 松弛度lambda
	double m_regularization = 0.0;

	// 变换参数保存在m_ParaMatrix当中
	double **m_ParaMatrix = nullptr;
	// 矩阵维度（控制点对数目）
	int m_iParaDimen = 0;
	// 弯曲能量integral bending norm >= 0
	double m_dIbn = -1.0;
};
#endif	// #ifndef TPS_2D_H
