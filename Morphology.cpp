#include "Morphology.h"
#include "Graying.h"

// opening operate
bool Morphology::Opening(EffectPar& parEft, int iType)
{
	// the input image
	unsigned char* pSrc = parEft.Input();
	bool bModify = parEft.Modify();
	if (pSrc == nullptr)
		return false;

	// first erode
	// then dilate
	if (bModify)
	{
		Erode(parEft, iType);
		Dilate(parEft, iType);
		parEft.Output(pSrc);
	}
	else
	{
		parEft.Modify(false);
		Erode(parEft, iType);
		unsigned char* pDes = parEft.Output();
		parEft.Input(pDes, true);
		Dilate(parEft, iType);
		parEft.Input(pSrc, bModify);	// restore input
		parEft.Output(pDes);
	}
	return true;
}

// closeing operate
bool Morphology::Closeing(EffectPar& parEft, int iType)
{
	// the input image
	unsigned char* pSrc = parEft.Input();
	bool bModify = parEft.Modify();
	if (pSrc == nullptr)
		return false;

	// first dilate
	// then erode
	if (bModify)
	{
		Dilate(parEft, iType);
		Erode(parEft, iType);
		parEft.Output(pSrc);
	}
	else
	{
		parEft.Modify(false);
		Dilate(parEft, iType);
		unsigned char* pDes = parEft.Output();
		parEft.Input(pDes, true);
		Erode(parEft, iType);
		parEft.Input(pSrc, bModify);	// restore input
		parEft.Output(pDes);
	}
	return true;
}

// dilate - erode, form a morphological grade iamge
bool Morphology::Grading(EffectPar& parEft, int iType)
{
	bool bRet = true;
	unsigned char* pSrc = parEft.Input();
	bool bModify = parEft.Modify();
	if (pSrc == nullptr)
		return false;

	// prepare two image for dilate and erode
	unsigned char* pImg1 = nullptr;
	unsigned char* pImg2 = nullptr;
	unsigned char* pDes = nullptr;

	parEft.Modify(false);
	// dilate
	bRet = Dilate(parEft, iType);
	if (!bRet)
		return false;
	pImg1 = parEft.Output();
	// erode
	parEft.Modify(bModify);
	bRet = Erode(parEft, iType);
	if (!bRet)
		return false;
	pImg2 = parEft.Output();
	// subtract
	parEft.Input(pImg1, false);
	parEft.Input2(pImg2, true);
	bRet = Graying::Subtract(parEft);
	if (!bRet)
	{
		parEft.Recycle(pImg1);
		if (!bModify)
			parEft.Recycle(pImg2);
		return false;
	}

	// the result
	pDes = parEft.Output();
	parEft.Input(pSrc, bModify);		// restore data
	parEft.Output(pDes);
	parEft.Recycle(pImg1);

	return true;
}

/**< dilate operation, invoke _Dilate to perform */
bool Morphology::Dilate(EffectPar& parEft, int iType)
{
	bool bRet = true;

	// the input image
	unsigned char* pSrc = parEft.Input();
	bool bModify = parEft.Modify();
	if (pSrc == nullptr)
		return false;

	// temporary image for chaild process input
	unsigned char* pIn = parEft.GetCache();
	memcpy(pIn, pSrc, (size_t)parEft.PixNum()*3);
	parEft.Input(pIn, true);
	// go
	if (iType > 9)
	{
		// decade
		int iT = iType/10;
		for (int i =1; i <= iT; ++i)
		{
			if (!_Dilate(parEft, 1))	// 1col | 9point
			{
				bRet = false;
				break;
			}
			if (!_Dilate(parEft, 2))	// top-left to bottom-right \ 7point
			{
				bRet = false;
				break;
			}
			if (!_Dilate(parEft, 3))	// 1row - 9point
			{
				bRet = false;
				break;
			}
			if (!_Dilate(parEft, 4))	// top-right to bottom-left / 7point
			{
				bRet = false;
				break;
			}
		}
		// the int single
		iT = iType%10;
		for(int i = 1; i <= iT; ++i)
		{
			if (!_Dilate(parEft, 5))	// - 3point
			{
				bRet = false;
				break;
			}
			if (!_Dilate(parEft, 6))	// | 3point
			{
				bRet = false;
				break;
			}
		}
	}
	else
	{
		if (iType == 1 || iType == 2)
		{
			for (int i = 1; i <= iType; ++i)
			{
				if (!_Dilate(parEft, 9))	// + 5point
				{
					bRet = false;
					break;
				}
			}
		}
		else
		{
			for (int i = 1; i <= iType; ++i)
			{
				if ((i&1) == 1)		// odd
				{
					if (!_Dilate(parEft, 9))	// + 5point
					{
						bRet = false;
						break;
					}
				}
				else	// even
				{
					if (!_Dilate(parEft, 5))	// - 3point
					{
						bRet = false;
						break;
					}
					if (!_Dilate(parEft, 6))	// | 3point
					{
						bRet = false;
						break;
					}
				}
			}
		}
	}

	// the result
	parEft.Input(pSrc, bModify);	// restore the process data
	if (bRet == false)
		parEft.Recycle(pIn);
	else	if (bModify)
	{
		memcpy(pSrc, pIn, (size_t)parEft.PixNum()*3);
		parEft.Recycle(pIn);
		parEft.Output(pSrc);
	}
	else
		parEft.Output(pIn);
	return bRet;
}

/**< erode operation, invoke _Erode to perform */
bool Morphology::Erode(EffectPar& parEft, int iType)
{
	bool bRet = true;

	// the input image
	unsigned char* pSrc = parEft.Input();
	bool bModify = parEft.Modify();
	if (pSrc == nullptr)
		return false;

	// temporary image for chaild process input
	unsigned char* pIn = parEft.GetCache();
	memcpy(pIn, pSrc, (size_t)parEft.PixNum()*3);
	parEft.Input(pIn, true);
	// go
	if (iType > 9)
	{
		// decade
		int iT = iType/10;
		for (int i =1; i <= iT; ++i)
		{
			if (!_Erode(parEft, 1))	// 1col | 9point
			{
				bRet = false;
				break;
			}
			if (!_Erode(parEft, 2))	// top-left to bottom-right \ 7point
			{
				bRet = false;
				break;
			}
			if (!_Erode(parEft, 3))	// 1row - 9point
			{
				bRet = false;
				break;
			}
			if (!_Erode(parEft, 4))	// top-right to bottom-left / 7point
			{
				bRet = false;
				break;
			}
		}
		// the int single
		iT = iType%10;
		for(int i = 1; i <= iT; ++i)
		{
			if (!_Erode(parEft, 5))	// - 3point
			{
				bRet = false;
				break;
			}
			if (!_Erode(parEft, 6))	// | 3point
			{
				bRet = false;
				break;
			}
		}
	}
	else
	{
		if (iType == 1 || iType == 2)
		{
			for (int i = 1; i <= iType; ++i)
			{
				if (!_Erode(parEft, 9))	// + 5point
				{
					bRet = false;
					break;
				}
			}
		}
		else
		{
			for (int i = 1; i <= iType; ++i)
			{
				if ((i&1) == 1)		// odd
				{
					if (!_Erode(parEft, 9))	// + 5point
					{
						bRet = false;
						break;
					}
				}
				else	// even
				{
					if (!_Erode(parEft, 5))	// - 3point
					{
						bRet = false;
						break;
					}
					if (!_Erode(parEft, 6))	// | 3point
					{
						bRet = false;
						break;
					}
				}
			}
		}
	}

	// the result
	parEft.Input(pSrc, bModify);	// restore the process data
	if (bRet == false)
		parEft.Recycle(pIn);
	else	if (bModify)
	{
		memcpy(pSrc, pIn, (size_t)parEft.PixNum()*3);
		parEft.Recycle(pIn);
		parEft.Output(pSrc);
	}
	else
		parEft.Output(pIn);
	return bRet;
}

/**< dilate, but the type only be 1-9 */
bool Morphology::_Dilate(EffectPar& parEft, int iType)
{
	bool bRet = true;
	// the input image
	unsigned char* pSrc = parEft.Input();
	bool bModify = parEft.Modify();
	if (pSrc == nullptr)
		return false;
	// cache
	unsigned char* pDes = parEft.GetCache();
	memcpy(pDes, pSrc, (size_t)parEft.PixNum()*3);
	// iamge info
	int iW = parEft.Width();
	int iH = parEft.Height();
	unsigned char pixM = 0xff;
	// go
	switch(iType)
	{
	case 1:		// | 9point
	{
		unsigned char* pLine = pDes;
		for(int y = 0; y < iH; ++y)
		{
			unsigned char* pPix = pLine;
			for(int x = 0; x < iW; ++x)
			{
				pixM = pPix[0];
				// scan |
				unsigned char* pTmp = pPix - 4*iW*3;
				for(int i = -4; i <= 4; ++i)
				{
					if(i != 0 && y+i >= 0 && y+i < iH)
					{
						if (pixM < pTmp[0])
							pixM = pTmp[0];
					}
					pTmp += iW*3;
				}
				pPix[0] = pixM;
				pPix[1] = pixM;
				pPix[2] = pixM;
				// next pixel
				pPix += 3;
			}
			// next line
			pLine += iW*3;
		}
	}
	break;
	case 2:		// \ 7point
	{
		unsigned char* pLine = pDes;
		for(int y = 0; y < iH; ++y)
		{
			unsigned char* pPix = pLine;
			for(int x = 0; x < iW; ++x)
			{
				pixM = pPix[0];
				// scan "\"
				unsigned char* pTmp = pPix - (3*iW+3)*3;
				for(int i = -3; i <= 3; ++i)
				{
					if(i != 0 && y+i >= 0 && y+i < iH && x+i >= 0 && x+i <iW)
					{
						if (pixM < pTmp[0])
							pixM = pTmp[0];
					}
					pTmp += iW*3 + 3;
				}
				pPix[0] = pixM;
				pPix[1] = pixM;
				pPix[2] = pixM;
				// next pixel
				pPix += 3;
			}
			// next line
			pLine += iW*3;
		}
	}
	break;
	case 3:		// - 9point
	{
		unsigned char* pLine = pDes;
		for(int y = 0; y < iH; ++y)
		{
			unsigned char* pPix = pLine;
			for(int x = 0; x < iW; ++x)
			{
				pixM = pPix[0];
				// scan -
				unsigned char* pTmp = pPix - 4*3;
				for(int i = -4; i <= 4; ++i)
				{
					if(i != 0 && x+i >= 0 && x+i < iW)
					{
						if (pixM < pTmp[0])
							pixM = pTmp[0];
					}
					pTmp += 3;
				}
				pPix[0] = pixM;
				pPix[1] = pixM;
				pPix[2] = pixM;
				// next pixel
				pPix += 3;
			}
			// next line
			pLine += iW*3;
		}
	}
	break;
	case 4:		// "/" 7point
	{
		unsigned char* pLine = pDes;
		for(int y = 0; y < iH; ++y)
		{
			unsigned char* pPix = pLine;
			for(int x = 0; x < iW; ++x)
			{
				pixM = pPix[0];
				// scan "/"
				unsigned char* pTmp = pPix - (3*iW - 3)*3;
				for(int i = -3; i <= 3; ++i)
				{
					if(i != 0 && y+i >= 0 && y+i < iH && x-i >= 0 && x-i < iW)
					{
						if (pixM < pTmp[0])
							pixM = pTmp[0];
					}
					pTmp += 3;
				}
				pPix[0] = pixM;
				pPix[1] = pixM;
				pPix[2] = pixM;
				// next pixel
				pPix += 3;
			}
			// next line
			pLine += iW*3;
		}
	}
	break;
	case 5:		// - 3point
	{
		unsigned char* pLine = pDes;
		for(int y = 0; y < iH; ++y)
		{
			unsigned char* pPix = pLine;
			for(int x = 0; x < iW; ++x)
			{
				pixM = pPix[0];
				// scan -
				if  (x-1  >= 0 && x-1 < iW)
				{
					unsigned char* pTmp = pPix - 3;
					if (pixM < pTmp[0])
						pixM = pTmp[0];
				}
				if (x+1 >= 0 && x+1 < iW)
				{
					unsigned char* pTmp = pPix + 3;
					if (pixM < pTmp[0])
						pixM = pTmp[0];
				}
				pPix[0] = pixM;
				pPix[1] = pixM;
				pPix[2] = pixM;
				// next pixel
				pPix += 3;
			}
			// next line
			pLine += iW*3;
		}
	}
	break;
	case 6:		// | 3point
	{
		unsigned char* pLine = pDes;
		for(int y = 0; y < iH; ++y)
		{
			unsigned char* pPix = pLine;
			for(int x = 0; x < iW; ++x)
			{
				pixM = pPix[0];
				// scan -
				if  (y-1  >= 0 && y-1 < iH)
				{
					unsigned char* pTmp = pPix - iW*3;
					if (pixM < pTmp[0])
						pixM = pTmp[0];
				}
				if (y+1 >= 0 && y+1 < iH)
				{
					unsigned char* pTmp = pPix + iW*3;
					if (pixM < pTmp[0])
						pixM = pTmp[0];
				}
				pPix[0] = pixM;
				pPix[1] = pixM;
				pPix[2] = pixM;
				// next pixel
				pPix += 3;
			}
			// next line
			pLine += iW*3;
		}
	}
	break;
	case 7:		// "/" 3point
	{
		unsigned char* pLine = pDes;
		for(int y = 0; y < iH; ++y)
		{
			unsigned char* pPix = pLine;
			for(int x = 0; x < iW; ++x)
			{
				pixM = pPix[0];
				// scan "/"
				if  (y-1  >= 0 && y-1 < iH && x+1 >= 0 && x+1 < iW )
				{
					unsigned char* pTmp = pPix - (iW-1)*3;
					if (pixM < pTmp[0])
						pixM = pTmp[0];
				}
				if (y+1 >= 0 && y+1 < iH && x-1 >= 0 && x-1 < iW)
				{
					unsigned char* pTmp = pPix + (iW-1)*3;
					if (pixM < pTmp[0])
						pixM = pTmp[0];
				}
				pPix[0] = pixM;
				pPix[1] = pixM;
				pPix[2] = pixM;
				// next pixel
				pPix += 3;
			}
			// next line
			pLine += iW*3;
		}
	}
	break;
	case 8:		// "\" 3point
	{
		unsigned char* pLine = pDes;
		for(int y = 0; y < iH; ++y)
		{
			unsigned char* pPix = pLine;
			for(int x = 0; x < iW; ++x)
			{
				pixM = pPix[0];
				// scan "\"
				if  (y-1  >= 0 && y-1 < iH && x-1 >= 0 && x-1 < iW )
				{
					unsigned char* pTmp = pPix - (iW+1)*3;
					if (pixM < pTmp[0])
						pixM = pTmp[0];
				}
				if (y+1 >= 0 && y+1 < iH && x+1 >= 0 && x+1 < iW)
				{
					unsigned char* pTmp = pPix + (iW+1)*3;
					if (pixM < pTmp[0])
						pixM = pTmp[0];
				}
				pPix[0] = pixM;
				pPix[1] = pixM;
				pPix[2] = pixM;
				// next pixel
				pPix += 3;
			}
			// next line
			pLine += iW*3;
		}
	}
	break;
	case 9:		// "+" 5point
	{
		unsigned char* pLine = pDes;
		for(int y = 0; y < iH; ++y)
		{
			unsigned char* pPix = pLine;
			for(int x = 0; x < iW; ++x)
			{
				pixM = pPix[0];
				// scan "+"
				if  (x-1 >= 0 && x-1 < iW )		// left
				{
					unsigned char* pTmp = pPix - 3;
					if (pixM < pTmp[0])
						pixM = pTmp[0];
				}
				if  (y-1  >= 0 && y-1 < iH)		// top
				{
					unsigned char* pTmp = pPix - iW*3;
					if (pixM < pTmp[0])
						pixM = pTmp[0];
				}
				if (x+1 >= 0 && x+1 < iW)		// right
				{
					unsigned char* pTmp = pPix + 3;
					if (pixM < pTmp[0])
						pixM = pTmp[0];
				}
				if (y+1 >= 0 && y+1 < iH)		// bottom
				{
					unsigned char* pTmp = pPix + iW*3;
					if (pixM < pTmp[0])
						pixM = pTmp[0];
				}
				pPix[0] = pixM;
				pPix[1] = pixM;
				pPix[2] = pixM;
				// next pixel
				pPix += 3;
			}
			// next line
			pLine += iW*3;
		}
	}
	break;
	default:
		bRet = false;
		wxASSERT_MSG(false, _T("Invalide Dilate Template type."));
	}

	// the result
	if (!bRet)
		parEft.Recycle(pDes);
	else if (bModify)
	{
		memcpy(pSrc, pDes, (size_t)parEft.PixNum()*3);
		parEft.Output(pSrc);
		parEft.Recycle(pDes);
	}
	else
		parEft.Output(pDes);
	return bRet;
}

/**< erode, but the type only be 1-9 */
bool Morphology::_Erode(EffectPar& parEft, int iType)
{
	bool bRet = true;
	// the input image
	unsigned char* pSrc = parEft.Input();
	bool bModify = parEft.Modify();
	if (pSrc == nullptr)
		return false;
	// cache
	unsigned char* pDes = parEft.GetCache();
	memcpy(pDes, pSrc, (size_t)parEft.PixNum()*3);
	// iamge info
	int iW = parEft.Width();
	int iH = parEft.Height();
	unsigned char pixM = 0xff;
	// go
	switch(iType)
	{
	case 1:		// | 9point
	{
		unsigned char* pLine = pDes;
		for(int y = 0; y < iH; ++y)
		{
			unsigned char* pPix = pLine;
			for(int x = 0; x < iW; ++x)
			{
				pixM = pPix[0];
				// scan |
				unsigned char* pTmp = pPix - 4*iW*3;
				for(int i = -4; i <= 4; ++i)
				{
					if(i != 0 && y+i >= 0 && y+i < iH)
					{
						if (pixM > pTmp[0])
							pixM = pTmp[0];
					}
					pTmp += iW*3;
				}
				pPix[0] = pixM;
				pPix[1] = pixM;
				pPix[2] = pixM;
				// next pixel
				pPix += 3;
			}
			// next line
			pLine += iW*3;
		}
	}
	break;
	case 2:		// \ 7point
	{
		unsigned char* pLine = pDes;
		for(int y = 0; y < iH; ++y)
		{
			unsigned char* pPix = pLine;
			for(int x = 0; x < iW; ++x)
			{
				pixM = pPix[0];
				// scan "\"
				unsigned char* pTmp = pPix - (3*iW+3)*3;
				for(int i = -3; i <= 3; ++i)
				{
					if(i != 0 && y+i >= 0 && y+i < iH && x+i >= 0 && x+i <iW)
					{
						if (pixM > pTmp[0])
							pixM = pTmp[0];
					}
					pTmp += iW*3 + 3;
				}
				pPix[0] = pixM;
				pPix[1] = pixM;
				pPix[2] = pixM;
				// next pixel
				pPix += 3;
			}
			// next line
			pLine += iW*3;
		}
	}
	break;
	case 3:		// - 9point
	{
		unsigned char* pLine = pDes;
		for(int y = 0; y < iH; ++y)
		{
			unsigned char* pPix = pLine;
			for(int x = 0; x < iW; ++x)
			{
				pixM = pPix[0];
				// scan -
				unsigned char* pTmp = pPix - 4*3;
				for(int i = -4; i <= 4; ++i)
				{
					if(i != 0 && x+i >= 0 && x+i < iW)
					{
						if (pixM > pTmp[0])
							pixM = pTmp[0];
					}
					pTmp += 3;
				}
				pPix[0] = pixM;
				pPix[1] = pixM;
				pPix[2] = pixM;
				// next pixel
				pPix += 3;
			}
			// next line
			pLine += iW*3;
		}
	}
	break;
	case 4:		// "/" 7point
	{
		unsigned char* pLine = pDes;
		for(int y = 0; y < iH; ++y)
		{
			unsigned char* pPix = pLine;
			for(int x = 0; x < iW; ++x)
			{
				pixM = pPix[0];
				// scan "/"
				unsigned char* pTmp = pPix - (3*iW - 3)*3;
				for(int i = -3; i <= 3; ++i)
				{
					if(i != 0 && y+i >= 0 && y+i < iH && x-i >= 0 && x-i < iW)
					{
						if (pixM > pTmp[0])
							pixM = pTmp[0];
					}
					pTmp += 3;
				}
				pPix[0] = pixM;
				pPix[1] = pixM;
				pPix[2] = pixM;
				// next pixel
				pPix += 3;
			}
			// next line
			pLine += iW*3;
		}
	}
	break;
	case 5:		// - 3point
	{
		unsigned char* pLine = pDes;
		for(int y = 0; y < iH; ++y)
		{
			unsigned char* pPix = pLine;
			for(int x = 0; x < iW; ++x)
			{
				pixM = pPix[0];
				// scan -
				if  (x-1  >= 0 && x-1 < iW)
				{
					unsigned char* pTmp = pPix - 3;
					if (pixM > pTmp[0])
						pixM = pTmp[0];
				}
				if (x+1 >= 0 && x+1 < iW)
				{
					unsigned char* pTmp = pPix + 3;
					if (pixM > pTmp[0])
						pixM = pTmp[0];
				}
				pPix[0] = pixM;
				pPix[1] = pixM;
				pPix[2] = pixM;
				// next pixel
				pPix += 3;
			}
			// next line
			pLine += iW*3;
		}
	}
	break;
	case 6:		// | 3point
	{
		unsigned char* pLine = pDes;
		for(int y = 0; y < iH; ++y)
		{
			unsigned char* pPix = pLine;
			for(int x = 0; x < iW; ++x)
			{
				pixM = pPix[0];
				// scan -
				if  (y-1  >= 0 && y-1 < iH)
				{
					unsigned char* pTmp = pPix - iW*3;
					if (pixM > pTmp[0])
						pixM = pTmp[0];
				}
				if (y+1 >= 0 && y+1 < iH)
				{
					unsigned char* pTmp = pPix + iW*3;
					if (pixM > pTmp[0])
						pixM = pTmp[0];
				}
				pPix[0] = pixM;
				pPix[1] = pixM;
				pPix[2] = pixM;
				// next pixel
				pPix += 3;
			}
			// next line
			pLine += iW*3;
		}
	}
	break;
	case 7:		// "/" 3point
	{
		unsigned char* pLine = pDes;
		for(int y = 0; y < iH; ++y)
		{
			unsigned char* pPix = pLine;
			for(int x = 0; x < iW; ++x)
			{
				pixM = pPix[0];
				// scan "/"
				if  (y-1  >= 0 && y-1 < iH && x+1 >= 0 && x+1 < iW )
				{
					unsigned char* pTmp = pPix - (iW-1)*3;
					if (pixM > pTmp[0])
						pixM = pTmp[0];
				}
				if (y+1 >= 0 && y+1 < iH && x-1 >= 0 && x-1 < iW)
				{
					unsigned char* pTmp = pPix + (iW-1)*3;
					if (pixM > pTmp[0])
						pixM = pTmp[0];
				}
				pPix[0] = pixM;
				pPix[1] = pixM;
				pPix[2] = pixM;
				// next pixel
				pPix += 3;
			}
			// next line
			pLine += iW*3;
		}
	}
	break;
	case 8:		// "\" 3point
	{
		unsigned char* pLine = pDes;
		for(int y = 0; y < iH; ++y)
		{
			unsigned char* pPix = pLine;
			for(int x = 0; x < iW; ++x)
			{
				pixM = pPix[0];
				// scan "\"
				if  (y-1  >= 0 && y-1 < iH && x-1 >= 0 && x-1 < iW )
				{
					unsigned char* pTmp = pPix - (iW+1)*3;
					if (pixM > pTmp[0])
						pixM = pTmp[0];
				}
				if (y+1 >= 0 && y+1 < iH && x+1 >= 0 && x+1 < iW)
				{
					unsigned char* pTmp = pPix + (iW+1)*3;
					if (pixM > pTmp[0])
						pixM = pTmp[0];
				}
				pPix[0] = pixM;
				pPix[1] = pixM;
				pPix[2] = pixM;
				// next pixel
				pPix += 3;
			}
			// next line
			pLine += iW*3;
		}
	}
	break;
	case 9:		// "+" 5point
	{
		unsigned char* pLine = pDes;
		for(int y = 0; y < iH; ++y)
		{
			unsigned char* pPix = pLine;
			for(int x = 0; x < iW; ++x)
			{
				pixM = pPix[0];
				// scan "+"
				if  (x-1 >= 0 && x-1 < iW )		// left
				{
					unsigned char* pTmp = pPix - 3;
					if (pixM > pTmp[0])
						pixM = pTmp[0];
				}
				if  (y-1  >= 0 && y-1 < iH)		// top
				{
					unsigned char* pTmp = pPix - iW*3;
					if (pixM > pTmp[0])
						pixM = pTmp[0];
				}
				if (x+1 >= 0 && x+1 < iW)		// right
				{
					unsigned char* pTmp = pPix + 3;
					if (pixM > pTmp[0])
						pixM = pTmp[0];
				}
				if (y+1 >= 0 && y+1 < iH)		// bottom
				{
					unsigned char* pTmp = pPix + iW*3;
					if (pixM > pTmp[0])
						pixM = pTmp[0];
				}
				pPix[0] = pixM;
				pPix[1] = pixM;
				pPix[2] = pixM;
				// next pixel
				pPix += 3;
			}
			// next line
			pLine += iW*3;
		}
	}
	break;
	default:
		bRet = false;
		wxASSERT_MSG(false, _T("Invalide Erode Template type."));
	}

	// the result
	if (!bRet)
		parEft.Recycle(pDes);
	else if (bModify)
	{
		memcpy(pSrc, pDes, (size_t)parEft.PixNum()*3);
		parEft.Output(pSrc);
		parEft.Recycle(pDes);
	}
	else
		parEft.Output(pDes);
	return bRet;
}
