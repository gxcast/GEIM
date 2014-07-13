#include "TPS_2D.h"

#include <math.h>


CTPS_2d_Morpher::CTPS_2d_Morpher()
{
}
CTPS_2d_Morpher::~CTPS_2d_Morpher()
{
	// destroy old data
	if (m_ParaMatrix != nullptr)
		Destroy();
}

/********************************************************************
    函数：morph  -  根据已计算得到的变换矩阵，计算(x,y)变换后的坐标(dx,dy)
    参数：Coorder* Pts：传入(x,y)，输出(dx,dy)
*******************************************************************/
bool CTPS_2d_Morpher::TPSMain(VT_TPS_PAIRS* pPairs, double regularization)
{
	bool bRet = true;
	if (pPairs == nullptr)
	{
		wxASSERT_MSG(false, _T("CTPS_2d_Morpher::TPSMain parameter is nullptrptr."));
		return false;
	}
	// destroy old data
	if (m_ParaMatrix != nullptr)
		Destroy();
	// 保存控制点对
	m_pSamples = pPairs;
	m_regularization = regularization;

	double **mtx_l = nullptr;
	double **mtx_v = nullptr;
	double **L = nullptr;
	double **U = nullptr;
	int iDimen = (int)m_pSamples->size() + 3;
	m_iParaDimen = iDimen;
	mtx_l = create_Array(iDimen, iDimen);	// 后三列为源点的齐次坐标矩阵
	if (mtx_l == nullptr)
	{
		bRet = false;
		goto TPSMain_end;
	}
	mtx_v = create_Array(iDimen, 2);	// 目标点的坐标矩阵,存放变换参数
	if (mtx_v == nullptr)
	{
		bRet = false;
		goto TPSMain_end;
	}

	// step1:创建线性方程组系数矩阵
	if (CreateEquation(mtx_l, mtx_v) == -1)
	{
		bRet = false;
		goto TPSMain_end;
	}

	// step2:求解L、U矩阵
	L = create_Array(iDimen, iDimen);
	if (L == nullptr)
	{
		bRet = false;
		goto TPSMain_end;
	}
	U = create_Array(iDimen, iDimen);
	if (U == nullptr)
	{
		bRet = false;
		goto TPSMain_end;
	}
	// 理论上说，要传入矩阵指针mtx_l, mtx_v ,传入L,U指针用于接收分解结果
	//if (LU_Solve_Ou(mtx_l, L, U) == -1)
	if (LU_Solve(mtx_l, L, U) == -1)
	{
		bRet = false;
		goto TPSMain_end;
	}

	// step3:求计算TPS变形系数矩阵，存放在mtx_v中
	if (get_X(mtx_l, L, U, mtx_v, iDimen) == -1)
	{
		bRet = false;
		goto TPSMain_end;
	}

	// step4: calculate transformation's smoothness: IBN
	m_dIbn = calcu_ibn(mtx_l, mtx_v, iDimen-3);

TPSMain_end:
	// 释放内存
	free_Array(mtx_l, iDimen);
	free_Array(U, iDimen);
	free_Array(L, iDimen);
	// 计算结果
	m_ParaMatrix = mtx_v;
	m_iParaDimen = iDimen;
	if (!bRet)
		Destroy();

	return bRet;
}

/********************************************************************
    函数：morph  -  根据已计算得到的变换矩阵，计算(x,y)变换后的坐标(dx,dy)
    参数：Coorder* Pts：传入(x,y)，输出(dx,dy)
*******************************************************************/
bool CTPS_2d_Morpher::morph(Coorder *Pts)
{
	// no TPS parameters
	if (m_ParaMatrix == nullptr)
		return false;
	double **mtx_v = m_ParaMatrix;

	int ItemNum = m_iParaDimen - 3;
	double x = Pts->x;
	double y = Pts->y;

	// 仿射变换部分
	double dx = mtx_v[ItemNum][0] + mtx_v[ItemNum + 1][0] * x + mtx_v[ItemNum + 2][0] * y;
	double dy = mtx_v[ItemNum][1] + mtx_v[ItemNum + 1][1] * x + mtx_v[ItemNum + 2][1] * y;
	// 扭曲部分
	auto diffite = m_pSamples->begin();
	for (int i = 0; i < ItemNum; i++)
	{
		double d = base_func(SQR(diffite->x - x) + SQR(diffite->y - y));
		dx += mtx_v[i][0] * d;
		dy += mtx_v[i][1] * d;
		diffite++;
	}
	Pts->dx = dx;
	Pts->dy = dy;

	return true;
}

/**< transform the special image depend on TPS parameters*/
bool CTPS_2d_Morpher::TransImg(ST_RGB* pSrc, int iW, int iH, ST_RGB* pDes, int oW, int oH)
{
	// no TPS parameters
	if (m_ParaMatrix == nullptr)
		return false;
	if (pSrc == nullptr || pDes == nullptr)
	{
		wxASSERT_MSG(false, _T("CTPS_2d_Morpher::TrasImg parameter is nullptr."));
		return false;
	}

	// if the input is not the output, use output, else, new
	ST_RGB* pTemp = nullptr;
	if (pDes != pSrc)
		pTemp = pDes;
	else
		pTemp = new ST_RGB[oW*oH];
	memset(pTemp, 0, oW*oH*sizeof(ST_RGB));

	Coorder difforder;
	ST_RGB* pImgB = pTemp;
	for (int y = 0; y < oH; ++y)
	{
		for (int x = 0; x < oW; ++x)
		{
			difforder.x = x;
			difforder.y = y;
			difforder.dx = 0.0;
			difforder.dy = 0.0;
			// 进行变换B->A
			morph(&difforder);
			int dy = floor(difforder.dy + 0.5);
			int dx = floor(difforder.dx + 0.5);
			// remain in the image
			if (0 <= dy && dy < iH && 0 <= dx && dx < iW)
			{
				ST_RGB* pImgA = pSrc + iW*dy + dx;
				*pImgB = *pImgA;
			}
			++pImgB;
		}
	}

	if (pDes == pSrc)
	{
		memcpy(pDes, pTemp, oW*oH*sizeof(ST_RGB));
		delete [] pTemp;
	}

	return true;
}

/**< destroy last result */
bool CTPS_2d_Morpher::Destroy()
{
	if (m_ParaMatrix != nullptr)
	{
		// release the control points pair
		delete m_pSamples;
		m_pSamples = nullptr;
		// lambda
		m_regularization = 0.0;
		// parameters' matrix
		free_Array(m_ParaMatrix, m_iParaDimen);
		m_ParaMatrix = nullptr;
		// parameters' dimention
		m_iParaDimen = 0;
		// transform's integral bending norm
		m_dIbn = -1.0;
	}
	return true;
}

/********************************************************************
    函数：morph  -  根据已计算得到的变换矩阵，计算(x,y)变换后的坐标(dx,dy)
    参数：Coorder* Pts：传入(x,y)，输出(dx,dy)
*******************************************************************/
int CTPS_2d_Morpher::CreateEquation(double **mtx_l, double **mtx_v)
{
	// A=(float**)new float[ItemNum+3];
	int ItemNum = m_iParaDimen - 3;
	double **mtx_orig_k;
	mtx_orig_k = create_Array(ItemNum, ItemNum); //源点径向基函数系数矩阵
	if (mtx_orig_k == nullptr)
		return -1;

	for (int i = 0; i < ItemNum; i++)
	{
		Coorder& pt_i = m_pSamples->at(i); //第i个点坐标
		for (int j = 0; j < ItemNum; j++)
		{
			Coorder& pt_j = m_pSamples->at(j); //第j个点坐标
			double r2 = SQR(pt_i.x - pt_j.x) + SQR(pt_i.y - pt_j.y); //i，j距离平方
			mtx_l[i][j] = mtx_orig_k[i][j] = base_func(r2);        //径向基函数系数
		}
	}

	double a = 0.5;
	for (int i = 0; i < ItemNum; ++i)
	{
		Coorder& pt_i = m_pSamples->at(i); //第i个点坐标

		// P (p x 3, upper right)
		mtx_l[i][ItemNum] = 1.0;
		mtx_l[i][ItemNum + 1] = pt_i.x;
		mtx_l[i][ItemNum + 2] = pt_i.y;

		if (i < ItemNum)
		{
			// diagonal: reqularization parameters (lambda * a^2)
			//  对角线上的0替换成正则化系数
			mtx_l[i][i] = mtx_orig_k[i][i] = m_regularization * (a * a);

			// P transposed (3 x p, bottom left)
			mtx_l[ItemNum][i] = 1.0;
			mtx_l[ItemNum + 1][i] = pt_i.x;
			mtx_l[ItemNum + 2][i] = pt_i.y;
		}
	}

	// 矩阵右下角3 x 3置零O (3 x 3, lower right)
	for ( int i = ItemNum; i < ItemNum + 3; ++i )
		for ( int j = ItemNum; j < ItemNum + 3; ++j )
			mtx_l[i][j] = 0.0;

	// 方程右边矩阵（目标点坐标）Fill the right hand matrix V
	for ( int i = 0; i < ItemNum; ++i )
	{
		Coorder& pt_i = m_pSamples->at(i); //第i个点坐标
		mtx_v[i][0] = pt_i.dx;
		mtx_v[i][1] = pt_i.dy;
	}
	// 右矩阵最后三行置零
	mtx_v[ItemNum][0] = mtx_v[ItemNum + 1][0] = mtx_v[ItemNum + 2][0] = 0.0;
	mtx_v[ItemNum][1] = mtx_v[ItemNum + 1][1] = mtx_v[ItemNum + 2][1] = 0.0;

	return 1;
}

/********************************************************************
    函数：LU_Solve  -  //计算线性方程的解,LU矩阵
    参数：double** a：   由控制点对坐标构成的线性方程组左矩阵
          double** L：  L矩阵
          double** U：  U矩阵

*******************************************************************/
int CTPS_2d_Morpher::LU_Solve(double **a, double **L, double **U)
{
	int n;
	int i, j, k;
	double sum = 0;

	n = m_iParaDimen;

	//初始化L矩阵
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < n; j++)
		{
			if (i == j)
				L[i][j] = 1;
			else
				L[i][j] = 0;
		}
	}

	// 初始化U矩阵
	for (i = 0; i < n; i++) // 第一行通过计算得到
	{
		U[0][i] = (double)(a[0][i] / L[0][0]);
	}

	for (i = 1; i < n; i++) // 其它行置零
		for (j = 0; j < n; j++)
			U[i][j] = 0;

	// 计算出L和U矩阵
	for (i = 0; i < n - 1; i++)
	{
		for (j = i + 1; j < n; j++)
		{
			for (k = 0, sum = 0; k < n; k++)
			{
				if (k != i)
					sum += L[j][k] * U[k][i];
			}
			if (U[i][i] == 0)
				L[j][i] = (double)((a[j][i] - sum) / (U[i][i] + 0.05)); //会崩溃
			else
				L[j][i] = (double)((a[j][i] - sum) / U[i][i]);
		}

		for (j = i + 1; j < n; j++)
		{
			for (k = 0, sum = 0; k < n; k++)
			{
				if (k != i + 1)
					sum += L[i + 1][k] * U[k][j];
			}
			U[i + 1][j] = (double)((a[i + 1][j] - sum));
		}
	}
	return 1;
}

/********************************************************************
    函数：LU_Solve  -  //计算线性方程的解,LU矩阵
    参数：double** a：   由控制点对坐标构成的线性方程组左矩阵
          double** L：  L矩阵
          double** U：  U矩阵

*******************************************************************/
int CTPS_2d_Morpher::LU_Solve_Ou(double **matrix, double **L_matrix, double **U_matrix)
{
	int i, j, k, r;
	double sum = 0;
	//初始化L矩阵（L为单位下三角阵）
	int num = m_iParaDimen;

	for (i = 0; i < num; i++)
		for (j = 0; j < num; j++)
		{
			if (i == j)
				L_matrix[j][i] = 1;
			//对角线元素置1

			else
			{
				L_matrix[j][i] = 0;
			}
		}
	//初始化U矩阵（所有元素置0）

	for (i = 0; i < num; i++)
		for (j = 0; j < num; j++)
			U_matrix[j][i] = 0;
	//求U矩阵的第一行

	for (i = 0; i < num; i++)
		U_matrix[0][i] = matrix[0][i];
	//求L矩阵的第一列
	for (i = 0; i < num; i++)
		L_matrix[i][0] = matrix[i][0] / U_matrix[0][0];
	for (r = 1; r < num; r++)
	{
		//求U的第r行的元素

		for (i = r; i < num; i++)
		{
			sum = 0.0;
			for (k = 0; k < r; k++)
			{
				sum += L_matrix[r][k] * U_matrix[k][i];
			}
			U_matrix[r][i] = matrix[r][i] - sum;
		}
		//求L的第r列的元素

		for (i = r; i < num; i++)
		{
			sum = 0.0;
			for (k = 0; k < r; k++)
			{
				sum += L_matrix[i][k] * U_matrix[k][r];
			}
			L_matrix[i][r] = (matrix[i][r] - sum) / U_matrix[r][r];
		}
	}

	return 1;
}
/********************************************************************
    函数：get_X  -  LU分解法回代过程，并求出方程组的解
    参数：double** a：  线性方程组系数矩阵
          double** L：  L矩阵
          double** U：  U矩阵
          double** b： 右矩阵
          int n：      行数
*******************************************************************/
int CTPS_2d_Morpher::get_X(double **a, double **L, double **U, double **b, int n)
{
	int i, j;

	double *x = (double *)malloc(sizeof(double) * n); //临时存放x方向的变形系数向量
	double *y = (double *)malloc(sizeof(double) * n); //临时存放y方向的变形系数向量

	double *b1 = (double *)malloc(sizeof(double) * n); //临时存放n个dx
	double *b2 = (double *)malloc(sizeof(double) * n); //临时存放n个dy
	if (x == nullptr || y == nullptr || b1 == nullptr || b2 == nullptr)
	{
		return -1;
	}

	for (int i = 0; i < n; i++)
	{
		b1[i] = b[i][0];
		b2[i] = b[i][1];
	}
	//--------------------------计算x方向变形系数向量---------------------//
	//计算Ly=b中的y  //求 y[i]
	for (i = 0; i < n; i++)
	{
		y[i] = b1[i];
		for (j = 0; j <= i - 1; j++)
			y[i] -= (L[i][j] * y[j]);
		y[i] /= L[i][i];
	}

	//计算Ux=y中的x
	for (i = n - 1; i >= 0; i--)
	{
		x[i] = y[i];
		for (j = i + 1; j < n; j++)
			x[i] -= (U[i][j] * x[j]);

		if (U[i][i] == 0)
			x[i] /= (U[i][i] + 0.05); //会崩溃
		else
			x[i] /= U[i][i];  //会崩溃
	}

	//输出x方向变形系数到b的第一列
	for (i = 0; i < n; i++)
	{
		b[i][0] = x[i];
	}

	//--------------------------计算y方向变形系数向量---------------------//
	//计算Ly=b中的y
	for (i = 0; i < n; i++)
	{
		y[i] = b2[i];
		for (j = 0; j <= i - 1; j++)
			y[i] -= (L[i][j] * y[j]);
		y[i] /= L[i][i];
	}

	//计算Ux=y中的x
	for (i = n - 1; i >= 0; i--)
	{
		x[i] = y[i];
		for (j = i + 1; j < n; j++)
			x[i] -= (U[i][j] * x[j]);
		if (U[i][i] == 0)
			x[i] /= (U[i][i] + 0.05); //会崩溃
		else
			x[i] /= U[i][i];  //会崩溃
	}

	//输出y方向变形系数到b的第二列
	for (i = 0; i < n; i++)
		b[i][1] = x[i];

	free(y);
	free(x);
	free(b1);
	free(b2);
	return 0;
}

// calculate the smoothness of th tansformation
double CTPS_2d_Morpher::calcu_ibn(double** mtxK, double** mtxW, int iDimen)
{
	double* pmtxTmp = new double[iDimen];
	// X维度的
	double dIbnX = 0.0;
	for (int i = 0; i < iDimen; ++i)
	{
		double dTmp = 0.0;
		for (int j = 0; j < iDimen; ++j)
			dTmp += mtxW[j][0]*mtxK[j][i];
		pmtxTmp[i] = dTmp;
	}
	for (int i = 0; i < iDimen; ++i)
		dIbnX += pmtxTmp[i]*mtxW[i][0];
	// Y维度的
	double dIbnY = 0.0;
	for (int i = 0; i < iDimen; ++i)
	{
		double dTmp = 0.0;
		for (int j = 0; j < iDimen; ++j)
			dTmp += mtxW[j][1]*mtxK[j][i];
		pmtxTmp[i] = dTmp;
	}
	for (int i = 0; i < iDimen; ++i)
		dIbnY += pmtxTmp[i]*mtxW[i][1];

	delete [] pmtxTmp;

	return dIbnX+dIbnY;
}

/********************************************************************
    函数：create_Array  -  创建动态二维数组
    参数：int n：行数
          int m：列数
*******************************************************************/
double **CTPS_2d_Morpher::create_Array(int n, int m)
{
	double **a = (double **)malloc(sizeof(double*)*n);
	if (a == nullptr)
		return a;
	for (int k = 0; k < n; ++k)
	{
		a[k] = (double *)malloc(sizeof(double)* m);
		if (a[k] == nullptr)
			return nullptr;
	}
	return a;
}
/********************************************************************
    函数：free_Array  -  释放动态二维数组的内存空间
    参数：double **a：动态二维数组指针
          int n：数组行数
*******************************************************************/
void CTPS_2d_Morpher::free_Array(double **a, int n)
{
	if (a != nullptr)
	{
		for (int m = 0; m < n; m++)
		{
			free(a[m]);
			a[m] = nullptr;
		}
		free(a);
		a = nullptr;
	}
}
