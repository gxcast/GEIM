
#include "Graying.h"

/////////////////////////////////////////////////////////
// Abandon image's color
bool Graying::Do(wxImage& img)
{
	// image valide
	if (!img.IsOk())
		return false;
	// image date
	int iN = img.GetHeight()*img.GetWidth();
	unsigned char*  pD = img.GetData();
	for (int n = 0; n < iN; ++n)
	{
		double dG = pD[0]*0.297 + pD[1]*0.589 + pD[2]*0.114;
		if (dG < 0.0)
			dG = 0.0;
		else if (dG > 255.0)
			dG = 255.0;
		pD[0] = (unsigned char)dG;
		pD[1] = (unsigned char)dG;
		pD[2] = (unsigned char)dG;
		pD += 3;
	}

	return true;
}
